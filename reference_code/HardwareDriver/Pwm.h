#ifndef __NPWM_H
#define __NPWM_H

#include "ti_msp_dl_config.h"

void PWM_Output(uint16_t width1,uint16_t width2,uint16_t width3,uint16_t width4);
void PWMStart(GPTIMER_Regs *gptimer);
void PWMStop(GPTIMER_Regs *gptimer);
#endif




