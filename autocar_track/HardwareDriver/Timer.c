#include "ti_msp_dl_config.h"
#include "Timer.h"
#include "main.h"

void TimerInit(void)
{
    NVIC_ClearPendingIRQ(TIM_CTRL_INST_INT_IRQN);
    NVIC_EnableIRQ(TIM_CTRL_INST_INT_IRQN);
    DL_TimerG_startCounter(TIM_CTRL_INST);
}

void TIM_CTRL_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TIM_CTRL_INST)) {
        case DL_TIMERG_IIDX_ZERO: {
            static uint32_t div = 0;
            div++;
            duty_200hz();
            if ((div % 2U)  == 0U) duty_100hz();
            if ((div % 20U) == 0U) duty_10hz();
            break;
        }
        default:
            break;
    }
}
