#ifndef __GRAYSENSOR_H
#define __GRAYSENSOR_H

#include "datatype.h"

void  GraySensorInit(GraySensor_t** GraySensor);
void  GraySensorDataUpdate(GraySensor_t* GraySensor);
float GraySensorToTurnAngle(GraySensor_t* GraySensor);

#endif /* __GRAYSENSOR_H */
