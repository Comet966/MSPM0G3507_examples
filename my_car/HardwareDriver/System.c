#include "ti_msp_dl_config.h"
#include "System.h"
#include "main.h"

/*
 * Timebase on the Cortex-M0+ SysTick.
 *
 * SysTick is configured to fire every 1 ms. millis() returns the tick count;
 * micros() interpolates within the current tick using the down-counting VAL
 * register for ~us resolution. The 1 ms ISR also drives duty_1000hz().
 *
 * Using SysTick (a core timer) leaves all TIMA/TIMG instances free for PWM,
 * stepper pulses, the control-loop tick, and the buzzer.
 */

static volatile uint32_t s_uptime_ms = 0;
static uint32_t s_ticks_per_ms = 0;   /* SysTick reload+1 = CPU cycles per ms */

void SystemInit_Timebase(void)
{
    s_ticks_per_ms = CPUCLK_FREQ / 1000U;   /* 80 MHz -> 80000 */

    /* SysTick: reload for 1 ms, core clock source, enable IRQ + counter. */
    SysTick->CTRL = 0;
    SysTick->LOAD = s_ticks_per_ms - 1U;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk   /* processor clock */
                  | SysTick_CTRL_TICKINT_Msk     /* enable SysTick IRQ */
                  | SysTick_CTRL_ENABLE_Msk;      /* enable counter */
}

/* Core SysTick exception handler (weak symbol in startup; this overrides it). */
void SysTick_Handler(void)
{
    s_uptime_ms++;
    duty_1000hz();
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

void delay_us(uint32_t us)
{
    uint32_t start = micros();
    while ((micros() - start) < us) { /* busy-wait */ }
}

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while ((millis() - start) < ms) { /* busy-wait */ }
}

void Delay_Ms(uint32_t ms) { delay_ms(ms); }
void Delay_Us(uint32_t us) { delay_us(us); }
