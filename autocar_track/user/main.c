#include "ti_msp_dl_config.h"
#include "headfile.h"

Car_t Car;

int main(void)
{
    SYSCFG_DL_init();

    UsartInit();
    TaskInit(&Car.Tasks);
    MotorInit(&Car.Motors);
    GraySensorInit(&Car.GraySensor);
    EncodersInit(Car.Motors);
    BuzzerInit(&Car.Buzzer);
    OLED_init();

    TimerInit();

    while (1) {
        OLED_display();
    }
}

void SysTick_Handler(void)
{
}

void duty_200hz(void)
{
    KeyDataUpdate(&Car);
    GraySensorDataUpdate(Car.GraySensor);
    EncoderDataUpdate(Car.Motors);
    if (Car.Tasks->CarStartFlag) {
        Task(Car);
        MotorPidCtrl(Car.Motors, Car.Tasks->Deltayaw, Car.Tasks->AverageSpeed);
        MotorDataUpdate(Car.Motors);
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
