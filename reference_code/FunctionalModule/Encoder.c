#include "ti_msp_dl_config.h"
#include "Encoder.h"

// extern Car_t Car ;
 
 
  void EncoderInit(Encoder_t* Encoder)
{
			/*---------- 原始整型数据 ----------*/
	Encoder->v_int = 0; 
	Encoder->t_int = 0;  
	Encoder->EncoderCount=0;

	/*---------- 转换后浮点数据 ----------*/
	Encoder->pos=0;
	Encoder->vel=0;
	Encoder->V=0;
	Encoder->X=0;
}
 void EncodersInit(Motors_t* Motors)
{
	static Encoder_t encoderLeftFront;
	static Encoder_t encoderLeftRear;
	static Encoder_t encoderRightFront;
	static Encoder_t encoderRightRear;
	
	Motors->EncoderLeftFront 	= &encoderLeftFront;
	Motors->EncoderLeftRear 	= &encoderLeftRear;
	Motors->EncoderRightFront	= &encoderRightFront;
	Motors->EncoderRightRear 	= &encoderRightRear;
	
	NVIC_EnableIRQ(Encoder1_INT_IRQN);//开启按键引脚的GPIOA端口中断
	NVIC_EnableIRQ(Encoder2_INT_IRQN);//开启按键引脚的GPIOA端口中断
	
	EncoderInit(Motors->EncoderLeftFront);
	EncoderInit(Motors->EncoderLeftRear);
	EncoderInit(Motors->EncoderRightFront);
	EncoderInit(Motors->EncoderRightRear);
}

 void EncoderDataUpdate(Motors_t* Motors)
{
	Motors->EncoderLeftFront->v_int  = Motors->EncoderLeftFront->EncoderCount;
	Motors->EncoderLeftRear->v_int   = Motors->EncoderLeftRear->EncoderCount;
	Motors->EncoderRightFront->v_int = Motors->EncoderRightFront->EncoderCount;
	Motors->EncoderRightRear->v_int  = Motors->EncoderRightRear->EncoderCount;
	
	Motors->EncoderLeftFront->EncoderCount =  0;
	Motors->EncoderLeftRear->EncoderCount =   0;
	Motors->EncoderRightFront->EncoderCount = 0;
	Motors->EncoderRightRear->EncoderCount =  0;
	
	float temp = (EncoderReadingFrequency / EncoderLines) * 2 * PI;
	Motors->EncoderLeftFront->vel  = (Motors->EncoderLeftFront->v_int  * temp)*0.2 + Motors->EncoderLeftFront->vel*0.8;
	Motors->EncoderLeftRear->vel   = (Motors->EncoderLeftRear->v_int   * temp)*0.2 + Motors->EncoderLeftRear->vel*0.8;
	Motors->EncoderRightFront->vel = (Motors->EncoderRightFront->v_int * temp)*0.2 + Motors->EncoderRightFront->vel*0.8;
	Motors->EncoderRightRear->vel  = (Motors->EncoderRightRear->v_int  * temp)*0.2 + Motors->EncoderRightRear->vel*0.8;

	Motors->EncoderLeftFront->V    = Motors->EncoderLeftFront->vel * TireRadius;
	Motors->EncoderLeftRear->V     = Motors->EncoderLeftRear->vel * TireRadius;
	Motors->EncoderRightFront->V   = Motors->EncoderRightFront->vel * TireRadius;
	Motors->EncoderRightRear->V    = Motors->EncoderRightRear->vel * TireRadius;
	
	Motors->EncoderLeftFront->X   += Motors->EncoderLeftFront->V/EncoderReadingFrequency;
	Motors->EncoderLeftRear->X    += Motors->EncoderLeftRear->V/EncoderReadingFrequency;
	Motors->EncoderRightFront->X  += Motors->EncoderRightFront->V/EncoderReadingFrequency;
	Motors->EncoderRightRear->X   += Motors->EncoderRightRear->V/EncoderReadingFrequency;
}

 float EncoderTotalLengthGet(Motors_t* Motors)
{
	return (Motors->EncoderLeftFront->X + Motors->EncoderRightRear->X)/2;
}

 float EncoderDeltaLengthGet(Motors_t* Motors)
{
	return (Motors->EncoderLeftFront->X - Motors->EncoderRightRear->X)/2;
}


 void EncoderLengthClear(Motors_t* Motors)
{
	Motors->EncoderLeftFront->X   = 0;
	Motors->EncoderLeftRear->X    = 0;
	Motors->EncoderRightFront->X  = 0;
	Motors->EncoderRightRear->X   = 0;
}
