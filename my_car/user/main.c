#include "headfile.h"

/* Global aggregator: holds pointers to every subsystem (populated by *Init() calls). */
Car_t Car;

/* Phase 2: minimal Task allocation so Key can toggle CarStartFlag.
 * Full TaskInit (state machine) lands in Phase 6. */
static Task_t s_task;

/* ---- Phase 4 closed-loop speed test (OLED-instrumented) ----------------
 * Serial telemetry proved unreliable (log-replay artifact in the tooling), so
 * we display encoder + PID state on the OLED instead. Transparent to the logic.
 *
 * Stopped: hand-spin each wheel; watch L/R counts + velocity to check sign.
 * Running (press S2): drive straight at TARGET_SPEED via wheel-speed PID;
 * the display shows setpoint vs measured V so we can confirm the loop holds. */
#define TARGET_SPEED  30.0f   /* cm/s forward setpoint for the closed-loop test */

int main(void)
{
    SYSCFG_DL_init();

    SystemInit_Timebase();   /* SysTick 1 ms timebase + duty_1000hz */
    UsartInit();             /* UART0 debug @115200 (kept for optional use) */
    OLED_init();             /* SSD1306 over I2C1 (PB2/PB3) — debug display */

    OLED_clear();
    OLED_setCursor(0, 0);
    OLED_writeString("my_car Phase4");
    OLED_setCursor(0, 1);
    OLED_writeString("Encoder + PID");
    OLED_display();

    Car.Tasks = &s_task;     /* zero-inited (static): CarStartFlag=0 */
    BuzzerInit(&Car.Buzzer);
    MotorInit(&Car.Motors);  /* motors + encoders + PIDs; TB6612 live, wheels stopped */

    TimerInit();             /* 200 Hz control tick -> duty_200hz/100hz/10hz */

    while (1) {
        /* Live OLED dashboard (while(1) is for display per architecture). */
        OLED_setCursor(0, 0);
        OLED_writeString(Car.Tasks->CarStartFlag ? "RUN  set=30cm/s " : "STOP hand-spin  ");

        OLED_setCursor(0, 2);
        OLED_printf("L V=%-5d cnt=%-4d", (int)Car.Motors->EncoderLeft->V,
                                          (int)Car.Motors->EncoderLeft->sample);
        OLED_setCursor(0, 3);
        OLED_printf("R V=%-5d cnt=%-4d", (int)Car.Motors->EncoderRight->V,
                                          (int)Car.Motors->EncoderRight->sample);

        OLED_setCursor(0, 5);
        OLED_printf("Lout=%-5d", (int)Car.Motors->MotorLeft->Output);
        OLED_setCursor(0, 6);
        OLED_printf("Rout=%-5d", (int)Car.Motors->MotorRight->Output);

        OLED_display();
    }
}

/* SysTick-driven 1 kHz duty. */
void duty_1000hz(void) {}

/* 200 Hz master control tick: encoder update -> control. */
void duty_200hz(void)
{
    KeyDataUpdate(&Car);
    EncoderDataUpdate(Car.Motors);

    if (Car.Tasks->CarStartFlag) {
        DL_GPIO_setPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);
        MotorPidCtrl(Car.Motors, 0.0f, TARGET_SPEED);   /* compute Output (closed loop) */
        MotorDataUpdate(Car.Motors);                    /* push Output to TB6612 */
    } else {
        DL_GPIO_clearPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);
    }
}

/* 100 Hz: stop-protection when idle (wheels held off). */
void duty_100hz(void)
{
    if (!Car.Tasks->CarStartFlag) {
        MotorStop(Car.Motors);
    }
}

/* 10 Hz: buzzer pattern. */
void duty_10hz(void)
{
    BuzzerDataUpdate(Car.Buzzer);
}
