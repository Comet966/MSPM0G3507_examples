#include "ti_msp_dl_config.h"
#include "Pwm.h"

/* Period = 1000 counts (SysConfig). CCR = period - width gives active-high duty. */
void PWM_Output(uint16_t leftWidth, uint16_t rightWidth)
{
    DL_TimerA_setCaptureCompareValue(PWM_Motors_INST, 1000U - leftWidth,  GPIO_PWM_Motors_C0_IDX);
    DL_TimerA_setCaptureCompareValue(PWM_Motors_INST, 1000U - rightWidth, GPIO_PWM_Motors_C1_IDX);
}

void PWMStart(GPTIMER_Regs* gptimer)
{
    DL_TimerA_startCounter(gptimer);
}

void PWMStop(GPTIMER_Regs* gptimer)
{
    DL_TimerA_stopCounter(gptimer);
}
