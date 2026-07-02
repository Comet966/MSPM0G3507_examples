#ifndef __GraySensor_H
#define __GraySensor_H

#include "headfile.h"

void GraySensorInit(GraySensor_t** GsSensor);
void GraySensorDataUpdate (GraySensor_t* GraySensor);
float GraySensorToTurnAngle(GraySensor_t* GraySensor);
#endif
