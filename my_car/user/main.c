#include "headfile.h"

Car_t Car;

static Task_t s_task;

/* Phase 5: line-sensor + closed-loop line-following test.
 * Stopped: OLED shows live 8-bit sensor reading + turn angle (hand the car over
 *          the line to confirm correct polarity before pressing S2).
 * Running (press S2): GraySensorDataUpdate -> turnAngle -> MotorPidCtrl (closed loop).
 * Press S2 again to stop. */
#define TARGET_SPEED  25.0f   /* cm/s forward setpoint for line-follow test */

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
    EncoderDataUpdate(Car.Motors);

    /* GraySensorDataUpdate runs in while(1) — reads cached Deltayaw here. */
    if (Car.Tasks->CarStartFlag) {
        DL_GPIO_setPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);

        if (!Car.GraySensor->GraySensorNoData) {
            MotorPidCtrl(Car.Motors, Car.Tasks->Deltayaw, TARGET_SPEED);
            MotorDataUpdate(Car.Motors);
        }
    } else {
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
