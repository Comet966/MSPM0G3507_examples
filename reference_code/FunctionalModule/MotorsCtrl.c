#include "ti_msp_dl_config.h"
#include "MotorsCtrl.h"

fp32 BrushMotor_PID[3] = {BrushMotor_PID_KP , BrushMotor_PID_KI	,	BrushMotor_PID_KD	};
fp32 SelfTurn_PID[3] 	 = {SelfTurn_PID_KP , SelfTurn_PID_KI	,	SelfTurn_PID_KD	};

 void BrushMotorInit (BrushMotor_t* BrushMotor , int ERF , int EL , float TR)
{
    BrushMotor->state = Disable; 

    /*---------- ¿ØÖÆ²ÎÊý ----------*/
		BrushMotor->ERF= ERF;
		BrushMotor->EL=  EL;
		BrushMotor->TR=  TR;
		BrushMotor->ExpectOutput = 0;
		BrushMotor->Output = 0;
}

 void MotorInit (Motors_t** Motors)
{
	static Motors_t motors;
	*Motors = &motors ;                                                                                                                                                                                                                                                                                                                                                                      
	PWMStart(PWM_Motors_INST);
	PWM_Output(0,0,0,0);
	
	static BrushMotor_t motorLeftFront;
	static BrushMotor_t motorLeftRear;
	static BrushMotor_t motorRightFront;
	static BrushMotor_t motorRightRear;
	
	static Pid_t pidselfturn;
	static Pid_t pidLeftFront;
	static Pid_t pidLeftRear;
	static Pid_t pidRightFront;
	static Pid_t pidRightRear;
	
	motors.MotorLeftFront 	= &motorLeftFront;
	motors.MotorLeftRear 		= &motorLeftRear;
	motors.MotorRightFront	= &motorRightFront;
	motors.MotorRightRear 	= &motorRightRear;
	
	motors.PidSelfTurn 			= &pidselfturn;
	motors.PidLeftFront 		= &pidLeftFront;
	motors.PidLeftRear 		  = &pidLeftRear;
	motors.PidRightFront	  = &pidRightFront;
	motors.PidRightRear 	  = &pidRightRear;
	
	BrushMotorInit(motors.MotorLeftFront 	, EncoderReadingFrequency , EncoderLines , TireRadius);
	BrushMotorInit(motors.MotorLeftRear 	, EncoderReadingFrequency , EncoderLines , TireRadius);
	BrushMotorInit(motors.MotorRightFront , EncoderReadingFrequency , EncoderLines , TireRadius);
	BrushMotorInit(motors.MotorRightRear 	, EncoderReadingFrequency , EncoderLines , TireRadius);
	
	PID_init(motors.PidSelfTurn 	, SelfTurn_PID_mode , 	SelfTurn_PID , SelfTurn_PID_Maxout , SelfTurn_PID_MaxIout);
	PID_init(motors.PidLeftFront 	, BrushMotor_PID_mode , BrushMotor_PID , BrushMotor_PID_Maxout , BrushMotor_PID_MaxIout);
	PID_init(motors.PidLeftRear		, BrushMotor_PID_mode , BrushMotor_PID , BrushMotor_PID_Maxout , BrushMotor_PID_MaxIout);
	PID_init(motors.PidRightFront	, BrushMotor_PID_mode , BrushMotor_PID , BrushMotor_PID_Maxout , BrushMotor_PID_MaxIout);
	PID_init(motors.PidRightRear	, BrushMotor_PID_mode , BrushMotor_PID , BrushMotor_PID_Maxout , BrushMotor_PID_MaxIout);
}


 
 void MotorsPWMSet(Motors_t* Motors)
{
	int pwm[4] = {0};
	pwm[0] = ABS(Motors->MotorLeftFront->Output);
	pwm[1] = ABS(Motors->MotorLeftRear->Output);
	pwm[2] = ABS(Motors->MotorRightFront->Output);
	pwm[3] = ABS(Motors->MotorRightRear->Output);
	
	PWM_Output(pwm[0] , pwm[2] , pwm[3]  , pwm[1]);
}

 void MotorStop(Motors_t* Motors)
{
	Motors->MotorLeftFront->Output  = 0;
	Motors->MotorLeftRear->Output   = 0;
	Motors->MotorRightFront->Output = 0;
	Motors->MotorRightRear->Output  = 0;
	
	MotorDataUpdate(Motors);
}
 void MotorDirectionSet(Motors_t* Motors)
{
	(Motors->MotorLeftFront->Output>0) 	? DL_GPIO_clearPins(MotorDirection_PORT , MotorDirection_LeftFront_PIN)	  : DL_GPIO_setPins(MotorDirection_PORT , MotorDirection_LeftFront_PIN);
	(Motors->MotorLeftRear->Output>0) 	? DL_GPIO_setPins(MotorDirection_PORT , MotorDirection_LeftRear_PIN) 		: DL_GPIO_clearPins(MotorDirection_PORT , MotorDirection_LeftRear_PIN);
	(Motors->MotorRightFront->Output>0) ? DL_GPIO_clearPins(MotorDirection_PORT , MotorDirection_RightFront_PIN)  : DL_GPIO_setPins(MotorDirection_PORT , MotorDirection_RightFront_PIN);
	(Motors->MotorRightRear->Output>0) 	? DL_GPIO_setPins(MotorDirection_PORT , MotorDirection_RightRear_PIN) 	: DL_GPIO_clearPins(MotorDirection_PORT , MotorDirection_RightRear_PIN);
}

 void MotorPidCtrl (Motors_t* Motors,fp32 TurnAngleSet,fp32 AverageSpeedSet)
{
	float ExpectDifferentia 				= PID_Calc(Motors->PidSelfTurn   , TurnAngleSet								  , 0																			);
//	OLED_ShowFloat(90,40,TurnAngleSet,1,SIZE12,SpanSingle);
	Motors->MotorLeftFront->ExpectOutput  = AverageSpeedSet - ExpectDifferentia;
	Motors->MotorLeftRear->ExpectOutput	= AverageSpeedSet - ExpectDifferentia;
	Motors->MotorRightFront->ExpectOutput = AverageSpeedSet + ExpectDifferentia;
	Motors->MotorRightRear->ExpectOutput = AverageSpeedSet + ExpectDifferentia;
	
	Motors->MotorLeftFront->Output  = PID_Calc(Motors->PidLeftFront  , Motors->EncoderLeftFront->V   , Motors->MotorLeftFront->ExpectOutput);
	Motors->MotorLeftRear->Output   = PID_Calc(Motors->PidLeftRear	 , Motors->EncoderLeftFront->V 	 , Motors->MotorLeftRear->ExpectOutput);
	Motors->MotorRightFront->Output = PID_Calc(Motors->PidRightFront , Motors->EncoderRightFront->V  , Motors->MotorRightFront->ExpectOutput);
	Motors->MotorRightRear->Output  = PID_Calc(Motors->PidRightRear	 , Motors->EncoderRightFront->V	 , Motors->MotorRightRear->ExpectOutput);
}

 void MotorDataUpdate (Motors_t* Motors)
{
	MotorDirectionSet(Motors);
	MotorsPWMSet(Motors);
}