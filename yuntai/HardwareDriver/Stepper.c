#include "ti_msp_dl_config.h"
#include "Stepper.h"
#include "headfile.h"

/*
 * D36A dual stepper driver — see Stepper.h for the interface contract.
 *
 * STEP pulse generation: each axis is a TimerA PWM channel (50% duty). In
 * edge-aligned down-counting mode the output rests LOW and produces exactly one
 * rising edge per timer period, and the ZERO event fires once per completed
 * period. We count ZERO events to count steps; the ISR halts the timer on the
 * final step and forces the STEP pin low (CC=0) so it idles cleanly — matching
 * the SDK TIMER.md rule that a stopped timer + low pin is the reliable "off".
 *
 * Step rate is the timer period: load = timerClk / sps - 1, timerClk = 4 MHz.
 * We only change the load while the timer is stopped (before a move), so the
 * "don't touch LOAD while running" warning does not apply.
 */

typedef struct {
    GPTIMER_Regs    *timer;        /* PWM STEP timer instance */
    uint32_t         ccIndex;      /* capture/compare index for the STEP channel */
    uint32_t         timerClk;     /* timer clock (Hz) for load computation */
    GPIO_Regs       *dirPort;
    uint32_t         dirPin;
    GPIO_Regs       *enPort;
    uint32_t         enPin;
    int8_t           invert;       /* 1 = flip commanded direction sign */

    volatile int32_t remaining;    /* steps left in the current move */
    volatile int32_t position;     /* signed absolute step position */
    volatile int8_t  dir;          /* +1 / -1, applied to position per step */
    volatile bool    busy;
    bool             enabled;
    uint16_t         sps;          /* current step speed */
} StepperAxis;

static StepperAxis s_axis[STEPPER_COUNT];

/* Convert a step rate (steps/sec) to a timer LOAD value. */
static inline uint32_t sps_to_load(uint32_t timerClk, uint16_t sps)
{
    if (sps < STEP_SPS_MIN) sps = STEP_SPS_MIN;
    uint32_t load = timerClk / (uint32_t)sps;
    return (load > 0U) ? (load - 1U) : 0U;
}

void Stepper_Init(void)
{
    StepperAxis *pan = &s_axis[STEPPER_PAN];
    pan->timer    = PWM_PAN_INST;
    pan->ccIndex  = GPIO_PWM_PAN_C0_IDX;
    pan->timerClk = PWM_PAN_INST_CLK_FREQ;
    pan->dirPort  = GPIO_STEP_PORT;
    pan->dirPin   = GPIO_STEP_DIR_PAN_PIN;
    pan->enPort   = GPIO_STEP_PORT;
    pan->enPin    = GPIO_STEP_EN_PAN_PIN;
    pan->invert   = GIMBAL_PAN_INVERT;

    StepperAxis *tilt = &s_axis[STEPPER_TILT];
    tilt->timer    = PWM_TILT_INST;
    tilt->ccIndex  = GPIO_PWM_TILT_C0_IDX;
    tilt->timerClk = PWM_TILT_INST_CLK_FREQ;
    tilt->dirPort  = GPIO_STEP_PORT;
    tilt->dirPin   = GPIO_STEP_DIR_TILT_PIN;
    tilt->enPort   = GPIO_STEP_PORT;
    tilt->enPin    = GPIO_STEP_EN_TILT_PIN;
    tilt->invert   = GIMBAL_TILT_INVERT;

    for (int i = 0; i < STEPPER_COUNT; i++) {
        StepperAxis *a = &s_axis[i];
        a->remaining = 0;
        a->position  = 0;
        a->dir       = 1;
        a->busy      = false;
        a->enabled   = false;
        a->sps       = STEP_SPS_DEFAULT;

        /* Timer is configured but not started by SysConfig. Make sure it is
         * stopped and the STEP pin idles low (CC=0). */
        DL_TimerA_stopCounter(a->timer);
        DL_TimerA_setCaptureCompareValue(a->timer, 0, a->ccIndex);

        DL_GPIO_clearPins(a->enPort, a->enPin);    /* disabled (EN low) */
        DL_GPIO_clearPins(a->dirPort, a->dirPin);

        /* Step-complete interrupt (ZERO event) is enabled by SysConfig; arm NVIC. */
    }

    NVIC_ClearPendingIRQ(PWM_PAN_INST_INT_IRQN);
    NVIC_EnableIRQ(PWM_PAN_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(PWM_TILT_INST_INT_IRQN);
    NVIC_EnableIRQ(PWM_TILT_INST_INT_IRQN);
}

void Stepper_Enable(StepperId id, bool on)
{
    if (id >= STEPPER_COUNT) return;
    StepperAxis *a = &s_axis[id];
    a->enabled = on;
    if (on) {
        DL_GPIO_setPins(a->enPort, a->enPin);      /* D36A EN active HIGH */
    } else {
        Stepper_Stop(id);
        DL_GPIO_clearPins(a->enPort, a->enPin);
    }
}

bool Stepper_IsEnabled(StepperId id)
{
    if (id >= STEPPER_COUNT) return false;
    return s_axis[id].enabled;
}

void Stepper_SetSpeed(StepperId id, uint16_t sps)
{
    if (id >= STEPPER_COUNT) return;
    if (sps < STEP_SPS_MIN) sps = STEP_SPS_MIN;
    if (sps > STEP_SPS_MAX) sps = STEP_SPS_MAX;
    s_axis[id].sps = sps;
}

uint16_t Stepper_GetSpeed(StepperId id)
{
    if (id >= STEPPER_COUNT) return 0;
    return s_axis[id].sps;
}

void Stepper_Move(StepperId id, int32_t steps)
{
    if (id >= STEPPER_COUNT) return;
    StepperAxis *a = &s_axis[id];

    if (steps == 0) return;

    /* Re-target cleanly: stop any in-flight pulse train first. */
    DL_TimerA_stopCounter(a->timer);

    /* position tracks the LOGICAL command (positive steps raise it), so a read-back
     * always matches the commanded angle. invert only flips the physical DIR pin. */
    int8_t logical = (steps > 0) ? 1 : -1;
    int8_t physical = a->invert ? -logical : logical;

    if (physical > 0) {
        DL_GPIO_setPins(a->dirPort, a->dirPin);
    } else {
        DL_GPIO_clearPins(a->dirPort, a->dirPin);
    }

    uint32_t load = sps_to_load(a->timerClk, a->sps);
    DL_TimerA_setLoadValue(a->timer, load);
    DL_TimerA_setCaptureCompareValue(a->timer, load / 2U, a->ccIndex);  /* 50% duty */

    a->dir       = logical;
    a->remaining = (steps > 0) ? steps : -steps;
    a->busy      = true;

    if (!a->enabled) {                              /* auto-enable on a move */
        a->enabled = true;
        DL_GPIO_setPins(a->enPort, a->enPin);
    }

    DL_TimerA_clearInterruptStatus(a->timer, DL_TIMER_INTERRUPT_ZERO_EVENT);
    DL_TimerA_startCounter(a->timer);
}

bool Stepper_Busy(StepperId id)
{
    if (id >= STEPPER_COUNT) return false;
    return s_axis[id].busy;
}

int32_t Stepper_Position(StepperId id)
{
    if (id >= STEPPER_COUNT) return 0;
    return s_axis[id].position;
}

void Stepper_ZeroPosition(StepperId id)
{
    if (id >= STEPPER_COUNT) return;
    s_axis[id].position = 0;
}

void Stepper_Stop(StepperId id)
{
    if (id >= STEPPER_COUNT) return;
    StepperAxis *a = &s_axis[id];
    DL_TimerA_stopCounter(a->timer);
    DL_TimerA_setCaptureCompareValue(a->timer, 0, a->ccIndex);  /* STEP idle low */
    a->remaining = 0;
    a->busy      = false;
}

/* --- Step-counting ISRs (one per axis, ZERO event = one completed pulse) ---
 * Keep minimal: at 8 kHz the two ISRs together are the tightest real-time path. */
static inline void stepper_isr(StepperAxis *a)
{
    if (DL_TimerA_getPendingInterrupt(a->timer) == DL_TIMER_IIDX_ZERO) {
        if (a->remaining > 0) {
            a->position += a->dir;
            if (--a->remaining == 0) {
                DL_TimerA_stopCounter(a->timer);
                DL_TimerA_setCaptureCompareValue(a->timer, 0, a->ccIndex);
                a->busy = false;
            }
        }
    }
}

void PWM_PAN_INST_IRQHandler(void)
{
    stepper_isr(&s_axis[STEPPER_PAN]);
}

void PWM_TILT_INST_IRQHandler(void)
{
    stepper_isr(&s_axis[STEPPER_TILT]);
}
