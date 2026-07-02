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
	EncodersInit (Car.Motors);
	BuzzerInit(&Car.Buzzer);
  OLED_Init();
	
	TimerInit();
//	usart0_send_string("OK");
	
    while(1){
		OledDataUpdate();
		} ;
}

void SysTick_Handler(void)
{

}
void duty_1000hz(void)
{
    
}

void duty_200hz(void)
{
	KeyDataUpdate(&Car);
	GraySensorDataUpdate(Car.GraySensor);
	EncoderDataUpdate(Car.Motors);
	if(Car.Tasks->CarStartFlag){
		Task(Car);
		//uart_printf(10*EncoderTotalLengthGet(Car.Motors));
		//Car.Tasks->Deltayaw = GraySensorToTurnAngle(Car.GraySensor);
		//uart_printf(Car.Tasks->Deltayaw);
		//uart_printf(Car.Tasks->AverageSpeed);
		MotorPidCtrl(Car.Motors ,Car.Tasks->Deltayaw ,Car.Tasks->AverageSpeed);
		MotorDataUpdate (Car.Motors);
	}
}

void duty_100hz(void)
{
//	GraySensorDataUpdate(Car.GraySensor);
	if(!Car.Tasks->CarStartFlag)
		MotorStop(Car.Motors);
}
void duty_10hz(void)
{
//	DL_GPIO_setPins(GraySensor_PORT,GraySensor_PIN_0_PIN);
		BuzzerDataUpdate(Car.Buzzer);
}
