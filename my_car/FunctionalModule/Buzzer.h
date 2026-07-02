#ifndef __BUZZER_H
#define __BUZZER_H

#include "datatype.h"

/* Passive buzzer on a PWM tone output (PB26 / TIMG6). Beep pattern advanced at 10 Hz.
 * BuzzerBee(n) queues n beeps; each beep is on for a few 10 Hz ticks, then off. */

void BuzzerInit(Buzzer_t** Buzzer);
void BuzzerDataUpdate(Buzzer_t* Buzzer);         /* call at 10 Hz */
void BuzzerBee(Buzzer_t* Buzzer, uint8_t times); /* queue `times` beeps */

#endif /* __BUZZER_H */
