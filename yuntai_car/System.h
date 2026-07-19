#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stdint.h>

/* Timebase + blocking delays, built on the Cortex-M0+ SysTick (1 ms tick).
 * SystemInit_Timebase() must be called once at startup (before delays/millis are used).
 *
 * SysTick is a core timer, so all TIMA/TIMG instances stay free for the STEP
 * pulse generators (PWM_PAN=TIMA1, PWM_TILT=TIMA0). */

void SystemInit_Timebase(void);

uint32_t millis(void);   /* ms since boot */
uint32_t micros(void);   /* us since boot (wraps ~71 min) */

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif /* __SYSTEM_H */
