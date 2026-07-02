#include "ti_msp_dl_config.h"
#include "timer.h"
#include "pwm.h"

void PWM_Output(uint16_t width1,uint16_t width2,uint16_t width3,uint16_t width4)
{
	uint16_t pwm[4]={0};
	pwm[0]=1000-width1;
	pwm[1]=1000-width2;
	pwm[2]=1000-width3;
	pwm[3]=1000-width4;
	
	DL_TimerG_setCaptureCompareValue(PWM_Motors_INST,pwm[0],GPIO_PWM_Motors_C0_IDX);
	DL_TimerG_setCaptureCompareValue(PWM_Motors_INST,pwm[1],GPIO_PWM_Motors_C1_IDX);
	DL_TimerG_setCaptureCompareValue(PWM_Motors_INST,pwm[2],GPIO_PWM_Motors_C2_IDX);
	DL_TimerG_setCaptureCompareValue(PWM_Motors_INST,pwm[3],GPIO_PWM_Motors_C3_IDX);
}      

void PWMStart(GPTIMER_Regs *gptimer)
{
	DL_TimerA_startCounter(gptimer);
}      

void PWMStop(GPTIMER_Regs *gptimer)
{
	DL_TimerA_stopCounter(gptimer);
}      
