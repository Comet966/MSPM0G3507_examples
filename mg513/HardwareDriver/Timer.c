#include "ti_msp_dl_config.h"
#include "Timer.h"
#include "main.h"

/*
 * Single 200 Hz timer (SysConfig instance TIM_CTRL). The ISR calls duty_200hz()
 * every tick and software-divides that down:
 *   200 Hz / 2  = 100 Hz -> duty_100hz()
 *   200 Hz / 20 = 10  Hz -> duty_10hz()
 *
 * This frees the extra TIMG instances the reference firmware spent on separate
 * 100/10 Hz timers. duty_1000hz is driven by SysTick (System.c), not here.
 */

void TimerInit(void)
{
    NVIC_ClearPendingIRQ(TIM_CTRL_INST_INT_IRQN);
    NVIC_EnableIRQ(TIM_CTRL_INST_INT_IRQN);
    DL_TimerG_startCounter(TIM_CTRL_INST);   /* SysConfig does not emit startCounter */
}

void TIM_CTRL_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TIM_CTRL_INST)) {
        case DL_TIMERG_IIDX_ZERO: {
            static uint32_t div = 0;
            div++;

            duty_200hz();

            if ((div % 2U) == 0U) {
                duty_100hz();
            }
            if ((div % 20U) == 0U) {
                duty_10hz();
            }
            break;
        }
        default:
            break;
    }
}
