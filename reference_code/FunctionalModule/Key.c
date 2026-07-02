#include "ti_msp_dl_config.h"
#include "Key.h"


#define read_key   ((Key_PORT->DIN31_0 & Key_KeyPutIN_PIN ) ? 0x01 : 0x00)

// 短按处理函数
void KeyShortPressProcess(Car_t* Car)
{
	BuzzerBee(Car->Buzzer,1);
	Car->Tasks->CircleCount++;
	Car->Tasks->KeyPressCount = 0;
}

// 长按处理函数
void KeyLongPressProcess(Car_t* Car)
{
	//BuzzerBee(Car->Buzzer);
	EncoderLengthClear(Car->Motors);
	Car->Tasks->CarBeginLengthCountFlag = 1;
	Car->Tasks->KeyPressCount = 0;
	CarStop(*Car);
	Delay_Ms(1000);
	BuzzerBee(Car->Buzzer,2);
	CarStart(*Car);
}

void KeyDataUpdate(Car_t* Car)
{
	Car->Tasks->KeyPressFlag = read_key;
	if(Car->Tasks->KeyPressFlag)
	{
		Car->Tasks->KeyPressCount++;
	}
	if(!Car->Tasks->KeyPressFlag && Car->Tasks->LastKeyPressFlag)
	{
		if(Car->Tasks->KeyPressCount<50)
		{
			KeyShortPressProcess(Car);  // 调用短按处理函数
		}
		
		else if(Car->Tasks->KeyPressCount>=50)
		{
			KeyLongPressProcess(Car);   // 调用长按处理函数
		}
	}
	Car->Tasks->LastKeyPressFlag = Car->Tasks->KeyPressFlag;
}