#ifndef __Buzzer_H
#define __Buzzer_H

#include "headfile.h"

void BuzzerInit(Buzzer_t** Buzzer);
void BuzzerDataUpdate(Buzzer_t* Buzzer);
void BuzzerBee(Buzzer_t* Buzzer,uint8_t BuzzerBeeTimes);
#endif
