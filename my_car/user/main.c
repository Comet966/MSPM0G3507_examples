#include "headfile.h"

/* Global aggregator: holds pointers to every subsystem (populated by *Init() calls). */
Car_t Car;

/* Phase 2: minimal Task allocation so Key can toggle CarStartFlag.
 * Full TaskInit (state machine) lands in Phase 6. */
static Task_t s_task;

/* ---- Phase 3 open-loop motor self-test ---------------------------------
 * When running (press S2), step through a fixed pattern so each wheel's
 * direction/PWM can be verified with wheels off the ground. One step per
 * ~1.5 s (advanced from duty_10hz). Stop (S2 again) -> wheels off. */
#define TEST_DUTY        300   /* 30% of 1000 — gentle, audible spin */
#define STEP_TICKS_10HZ  15    /* 15 * 100 ms = 1.5 s per step */

static const char* const s_stepName[6] = {
    "FWD  (both +)", "STOP", "REV  (both -)", "STOP",
    "LEFT fwd only", "RIGHT fwd only",
};

static void motor_test_step(uint8_t step)
{
    switch (step) {
        case 0: MotorSetOpenLoop(Car.Motors,  TEST_DUTY,  TEST_DUTY); break; /* forward */
        case 1: MotorSetOpenLoop(Car.Motors,  0,          0);         break;
        case 2: MotorSetOpenLoop(Car.Motors, -TEST_DUTY, -TEST_DUTY); break; /* reverse */
        case 3: MotorSetOpenLoop(Car.Motors,  0,          0);         break;
        case 4: MotorSetOpenLoop(Car.Motors,  TEST_DUTY,  0);         break; /* left only */
        case 5: MotorSetOpenLoop(Car.Motors,  0,          TEST_DUTY); break; /* right only */
        default: break;
    }
}

int main(void)
{
    SYSCFG_DL_init();

    SystemInit_Timebase();   /* SysTick 1 ms timebase + duty_1000hz */
    UsartInit();             /* UART0 debug @115200 */

    Car.Tasks = &s_task;     /* zero-inited (static): CarStartFlag=0 */
    BuzzerInit(&Car.Buzzer);
    MotorInit(&Car.Motors);  /* TB6612: STBY on, TIMA0 running, wheels stopped */

    TimerInit();             /* 200 Hz control tick -> duty_200hz/100hz/10hz */

    uart0_send_string("\r\nmy_car Phase3: TB6612 open-loop test. Press S2 (PB21) to run pattern.\r\n");

    int8_t lastReport = -1;
    while (1) {
        if (Car.Tasks->CarStartFlag != lastReport) {
            lastReport = Car.Tasks->CarStartFlag;
            uart0_send_string(lastReport ? "STATE: RUN (motor test)\r\n" : "STATE: STOP\r\n");
        }
    }
}

/* SysTick-driven 1 kHz duty. */
void duty_1000hz(void) {}

/* 200 Hz master control tick. */
void duty_200hz(void)
{
    KeyDataUpdate(&Car);

    /* Status LED: solid ON when running, OFF when stopped (visual run indicator). */
    if (Car.Tasks->CarStartFlag) {
        DL_GPIO_setPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);
    } else {
        DL_GPIO_clearPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);
    }
}

/* 100 Hz (software-divided from 200 Hz). */
void duty_100hz(void) {}

/* 10 Hz: buzzer pattern + open-loop motor test stepper. */
void duty_10hz(void)
{
    BuzzerDataUpdate(Car.Buzzer);

    /* Motor self-test: advance one step every STEP_TICKS_10HZ while running;
     * hold wheels stopped when not running. */
    static uint8_t step = 0;
    static uint8_t divCount = 0;
    static uint8_t prevRun = 0;

    uint8_t running = Car.Tasks->CarStartFlag ? 1 : 0;

    if (running) {
        if (!prevRun) {                 /* just started: begin at step 0 immediately */
            step = 0; divCount = 0;
            motor_test_step(step);
            uart0_send_string("  step: ");
            uart0_send_string(s_stepName[step]);
            uart0_send_string("\r\n");
        } else if (++divCount >= STEP_TICKS_10HZ) {
            divCount = 0;
            step = (uint8_t)((step + 1) % 6);
            motor_test_step(step);
            uart0_send_string("  step: ");
            uart0_send_string(s_stepName[step]);
            uart0_send_string("\r\n");
        }
    } else if (prevRun) {               /* just stopped: wheels off */
        MotorStop(Car.Motors);
    }

    prevRun = running;
}
