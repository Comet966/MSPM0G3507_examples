#ifndef __NPWM_H
#define __NPWM_H

#include "ti_msp_dl_config.h"

void PWM_Output(uint16_t leftWidth, uint16_t rightWidth);
void PWMStart(GPTIMER_Regs *gptimer);
void PWMStop(GPTIMER_Regs *gptimer);
#endif




