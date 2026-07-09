#include "headfile.h"

Car_t Car;

static Task_t s_task;

/* Phase 5: line-sensor + open-loop line-following.
 * Stopped: OLED shows live 8-bit sensor reading + turn angle (hand the car over
 *          the line to confirm correct polarity before starting).
 * Running (press S2 or IR remote): GraySensorToTurnAngle -> MotorLineFollow
 *          (base duty + proportional steer, no velocity loop).
 * Press S2 / IR again to stop. Tunables: LineFollow_BaseDuty / _SteerKp in headfile.h. */

int main(void)
{
    SYSCFG_DL_init();

    SystemInit_Timebase();
    UsartInit();
    OLED_init();

    OLED_clear();
    OLED_setCursor(0, 0);
    OLED_writeString("my_car Phase5");
    OLED_setCursor(0, 1);
    OLED_writeString("LineFollow");
    OLED_display();

    Car.Tasks = &s_task;
    BuzzerInit(&Car.Buzzer);
    GraySensorInit(&Car.GraySensor);
    MotorInit(&Car.Motors);

    TimerInit();

    while (1) {
        /* GraySensorDataUpdate calls delay_ms(1) — must run here, not in ISR,
         * so the I2C IRQ can fire between sensor frames and OLED can flush. */
        GraySensorDataUpdate(Car.GraySensor);
        Car.Tasks->Deltayaw = GraySensorToTurnAngle(Car.GraySensor);

        uint8_t b = Car.GraySensor->BinaryData;

        OLED_setCursor(0, 0);
        OLED_writeString(Car.Tasks->CarStartFlag ? "RUN             " : "STOP            ");

        /* 8 chars: '#'=black line detected, '.'=white/no line. bit0=CH1=leftmost */
        OLED_setCursor(0, 1);
        char bits[9];
        for (int i = 0; i < 8; i++)
            bits[i] = (b & (1u << i)) ? '#' : '.';
        bits[8] = '\0';
        OLED_writeString(bits);

        OLED_setCursor(0, 2);
        OLED_printf("turn=%-6d", (int)(Car.Tasks->Deltayaw));

        OLED_setCursor(0, 3);
        OLED_printf("LV=%-4d RV=%-4d",
                    (int)Car.Motors->EncoderLeft->V,
                    (int)Car.Motors->EncoderRight->V);

        OLED_setCursor(0, 5);
        OLED_printf("Lo=%-5d Ro=%-5d",
                    (int)Car.Motors->MotorLeft->Output,
                    (int)Car.Motors->MotorRight->Output);

        OLED_display();
    }
}

void duty_1000hz(void) {}

void duty_200hz(void)
{
    KeyDataUpdate(&Car);
    IrDataUpdate(&Car);      /* IR remote = second start/stop, same toggle as S2 */
    EncoderDataUpdate(Car.Motors);

    /* GraySensorDataUpdate runs in while(1) — reads cached Deltayaw here.
     *
     * Line-lost grace: a thin line dropping into the GAP between two channels reads
     * as "no line" for a few frames — happens most on straights where the line sits
     * dead-centre between the middle sensors. Stopping instantly on that made the car
     * freeze mid-track. So on a brief dropout we KEEP driving with the last steering
     * command (Deltayaw is held, MotorLineFollow keeps the base duty), and only stop
     * once the line has been absent for LINE_LOST_STOP_SAMPLES consecutive ticks
     * (~0.4 s @200 Hz) — that long a gap means genuinely off-track, not a sensor gap. */
    #define LINE_LOST_STOP_SAMPLES  80   /* ~0.4 s @200 Hz before a real stop */
    static uint16_t lineLostCount = 0;

    if (Car.Tasks->CarStartFlag) {
        DL_GPIO_setPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);

        if (!Car.GraySensor->GraySensorNoData) {
            lineLostCount = 0;
            /* Open-loop base duty + proportional steer (see MotorLineFollow).
             * Bypasses the uncalibrated wheel-speed PID so the car actually drives
             * forward; steering sign is corrected there. */
            MotorLineFollow(Car.Motors, Car.Tasks->Deltayaw);
        } else if (lineLostCount < LINE_LOST_STOP_SAMPLES) {
            /* Brief dropout — line is almost certainly still under the array, just in
             * a sensor gap. Hold the last steering and keep driving through it. */
            lineLostCount++;
            MotorLineFollow(Car.Motors, Car.Tasks->Deltayaw);
        } else {
            /* Line gone long enough → truly off-track. Coast to a stop. */
            MotorStop(Car.Motors);
        }
    } else {
        lineLostCount = 0;
        DL_GPIO_clearPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);
    }
}

void duty_100hz(void)
{
    if (!Car.Tasks->CarStartFlag)
        MotorStop(Car.Motors);
}

void duty_10hz(void)
{
    BuzzerDataUpdate(Car.Buzzer);
}
