#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stdint.h>

/* Timebase + blocking delays, built on the Cortex-M0+ SysTick (1 ms tick).
 * SystemInit_Timebase() must be called once at startup (before delays are used). */

void SystemInit_Timebase(void);

uint32_t millis(void);   /* ms since boot */
uint32_t micros(void);   /* us since boot (wraps ~71 min) */

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

/* Aliases kept for source-compatibility with reused reference modules. */
void Delay_Ms(uint32_t ms);
void Delay_Us(uint32_t us);

#endif /* __SYSTEM_H */
