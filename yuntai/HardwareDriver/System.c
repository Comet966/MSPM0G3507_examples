#include "ti_msp_dl_config.h"
#include "System.h"

/*
 * Timebase on the Cortex-M0+ SysTick.
 *
 * SysTick fires every 1 ms. millis() returns the tick count; micros() interpolates
 * within the current tick from the down-counting VAL register for ~us resolution.
 *
 * CPUCLK_FREQ is emitted by SysConfig (32 MHz here, default SYSOSC). Using SysTick
 * (a core timer) leaves every TIMA/TIMG instance free for the two STEP generators.
 */

static volatile uint32_t s_uptime_ms = 0;
static uint32_t s_ticks_per_ms = 0;   /* SysTick reload+1 = CPU cycles per ms */

void SystemInit_Timebase(void)
{
    s_ticks_per_ms = CPUCLK_FREQ / 1000U;   /* 32 MHz -> 32000 */

    SysTick->CTRL = 0;
    SysTick->LOAD = s_ticks_per_ms - 1U;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk   /* processor clock */
                  | SysTick_CTRL_TICKINT_Msk     /* enable SysTick IRQ */
                  | SysTick_CTRL_ENABLE_Msk;     /* enable counter */
}

/* Core SysTick exception handler (weak in startup; this overrides it). */
void SysTick_Handler(void)
{
    s_uptime_ms++;
}

uint32_t millis(void)
{
    return s_uptime_ms;
}

uint32_t micros(void)
{
    uint32_t ms, val;
    /* Re-read on wrap to avoid a torn ms/VAL pair when SysTick reloads mid-read. */
    do {
        ms  = s_uptime_ms;
        val = SysTick->VAL;
    } while (ms != s_uptime_ms);

    /* VAL counts down from (ticks_per_ms-1); elapsed within this ms = LOAD - VAL. */
    uint32_t elapsed_cycles = (s_ticks_per_ms - 1U) - val;
    return ms * 1000U + (elapsed_cycles / (s_ticks_per_ms / 1000U));
}

/*
 * ISR-safe busy-wait: counts SysTick->VAL hardware ticks directly, so it works even
 * when the SysTick IRQ is masked by a higher/equal-priority ISR. Handles the
 * down-counter rollover (reload on underflow).
 */
void delay_us(uint32_t us)
{
    uint32_t ticks = us * (CPUCLK_FREQ / 1000000U);   /* 32 cycles per us at 32 MHz */
    uint32_t told  = SysTick->VAL;
    uint32_t tcnt  = 0;
    while (tcnt < ticks) {
        uint32_t tnow = SysTick->VAL;
        if (tnow != told) {
            tcnt += (tnow < told) ? (told - tnow)
                                  : (SysTick->LOAD - tnow + told);
            told = tnow;
        }
    }
}

void delay_ms(uint32_t ms)
{
    while (ms--) {
        delay_us(1000U);
    }
}
