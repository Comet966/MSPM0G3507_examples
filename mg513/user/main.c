#include "headfile.h"

/*
 * mg513 ENCODER DIAGNOSTIC mode: motors are held STOPPED; you turn each wheel
 * BY HAND and watch the encoder counts move on the OLED. This isolates the
 * encoder signal path completely — no motor PWM / motor-supply variable involved.
 *
 * OLED:
 *   line 0 : title
 *   line 1 : "turn wheels by hand"
 *   line 3 : CL / CR  = free-running total pulse count per wheel (never cleared).
 *            Turn a wheel forward -> its count should climb; reverse -> fall.
 *   line 4 : AL / AR  = live raw level of each wheel's A-phase pin (0/1).
 *            Turn the wheel slowly: this should toggle 0<->1. If it never
 *            changes, that A-phase has no signal (wiring / encoder power).
 */

Car_t Car;

static Task_t s_task;

int main(void)
{
    SYSCFG_DL_init();

    SystemInit_Timebase();

    Car.Tasks = &s_task;
    MotorInit(&Car.Motors);   /* allocates motors+encoders+PIDs, enables encoder IRQs, PWMInit */

    PWMStop();                /* HOLD MOTORS OFF: STBY low, both wheels coast/Hi-Z */

    TimerInit();              /* 200 Hz tick still runs EncoderDataUpdate for V/X */

    /* OLED after motors/tick so a non-ACKing display can't block bring-up. */
    OLED_init();
    OLED_clear();
    OLED_setCursor(0, 0);
    OLED_writeString("mg513 ENC diag");
    OLED_setCursor(0, 1);
    OLED_writeString("turn wheels byhand");
    OLED_display();

    while (1) {
        /* Free-running total pulse count (never cleared) — the health readout.
         * EncoderCount is zeroed every 5 ms by EncoderDataUpdate, so do NOT show it. */
        OLED_setCursor(0, 3);
        OLED_printf("CL=%-6ld CR=%-6ld",
                    (long)Car.Motors->EncoderLeft->TotalCount,
                    (long)Car.Motors->EncoderRight->TotalCount);

        /* Live A-phase pin levels: toggling by hand proves the signal reaches the pin. */
        uint32_t aL = DL_GPIO_readPins(GPIO_Encoder_EncoderL_A_PORT,
                                       GPIO_Encoder_EncoderL_A_PIN) ? 1U : 0U;
        uint32_t aR = DL_GPIO_readPins(GPIO_Encoder_EncoderR_A_PORT,
                                       GPIO_Encoder_EncoderR_A_PIN) ? 1U : 0U;
        OLED_setCursor(0, 4);
        OLED_printf("AL=%-3lu AR=%-3lu", (unsigned long)aL, (unsigned long)aR);

        OLED_display();

        delay_ms(50);
    }
}

/* 1 kHz SysTick tick — unused here. */
void duty_1000hz(void) {}

/* Master 200 Hz control tick (from TIM_CTRL_INST_IRQHandler in Timer.c). */
void duty_200hz(void)
{
    /* Snapshot encoder counts -> filtered velocity/distance. */
    EncoderDataUpdate(Car.Motors);

#ifdef DEMO_CLOSED_LOOP
    /* Closed-loop: hold both wheels at a fixed forward speed (no steering).
     * (Open-loop mode drives the motors from the main loop instead.) */
    MotorPidCtrl(Car.Motors, 0.0f, DEMO_TARGET_SPEED);
    MotorDataUpdate(Car.Motors);
#endif
}

/* Divided down from 200 Hz in Timer.c — unused here. */
void duty_100hz(void) {}
void duty_10hz(void) {}
