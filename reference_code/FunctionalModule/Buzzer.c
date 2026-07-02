#include "ti_msp_dl_config.h"
#include "Buzzer.h"

void BuzzerInit(Buzzer_t** Buzzer)
{
	static Buzzer_t buzzer;
	*Buzzer = &buzzer;
	buzzer.BuzzerFlag = 1 ;
	buzzer.BuzzerCount = 2;
	buzzer.BuzzerBeeCount = 0 ;
}

void BuzzerDataUpdate(Buzzer_t* Buzzer)
{
	if(Buzzer->BuzzerBeeCount !=0)
	{
		if(Buzzer->BuzzerFlag){
			PWMStart(PWM_Buzzer_INST);
			Buzzer->BuzzerCount--;
			if(Buzzer->BuzzerCount == 0)
			{
				Buzzer->BuzzerFlag = 0;
				Buzzer->BuzzerCount = 2;
			}
		}
		
		if(!Buzzer->BuzzerFlag){
			PWMStop(PWM_Buzzer_INST);
			Buzzer->BuzzerCount--;
			if(Buzzer->BuzzerCount == 0)
			{
				Buzzer->BuzzerFlag = 1;
				Buzzer->BuzzerCount = 2;
				Buzzer->BuzzerBeeCount--;
			}
		}
	}
	else
		PWMStop(PWM_Buzzer_INST);
}

void BuzzerBee(Buzzer_t* Buzzer,uint8_t BuzzerBeeTimes)
{
	Buzzer->BuzzerBeeCount += BuzzerBeeTimes;

}