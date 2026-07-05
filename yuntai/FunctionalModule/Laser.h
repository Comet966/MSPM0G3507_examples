#ifndef __LASER_H
#define __LASER_H

#include <stdbool.h>

/* Laser pointer on/off (active-HIGH GPIO, LASER pin = PB12). */

void Laser_Init(void);   /* drive off */
void Laser_On(void);
void Laser_Off(void);
void Laser_Set(bool on);
bool Laser_IsOn(void);

#endif /* __LASER_H */
