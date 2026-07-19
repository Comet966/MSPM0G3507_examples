#ifndef __TIMER_H
#define __TIMER_H

/* Master control-loop timer: one hardware timer at 200 Hz, software-divided
 * to 100 Hz and 10 Hz. Invokes the duty_*hz() callbacks defined in main.c.
 * (duty_1000hz is driven separately by SysTick — see System.c.) */

void TimerInit(void);

#endif /* __TIMER_H */
