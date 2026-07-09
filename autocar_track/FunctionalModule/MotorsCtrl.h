#ifndef __MotorsCtrl_H
#define __MotorsCtrl_H

#include "headfile.h"

void MotorInit (Motors_t** Motors);
void MotorPidCtrl (Motors_t* Motors,fp32 TurnAngleSet,fp32 AverageSpeedSet);
void MotorDataUpdate (Motors_t* Motors);
void MotorStop(Motors_t* Motors);

#endif
