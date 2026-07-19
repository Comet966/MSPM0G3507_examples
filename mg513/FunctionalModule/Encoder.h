#ifndef __ENCODER_H
#define __ENCODER_H

#include "datatype.h"

/*
 * Wheel odometry (FunctionalModule layer). Consumes the ISR-accumulated
 * EncoderCount (EncoderExti.c) and produces filtered wheel speed + distance
 * at the 200 Hz control rate.
 *
 *   sample  = counts captured this 200 Hz window (then EncoderCount cleared)
 *   vel     = angular velocity (rad/s), low-pass filtered
 *   V       = linear velocity (cm/s) = vel * TireRadius
 *   X       = accumulated distance (cm)
 */

void EncodersInit(Motors_t* Motors);              /* alloc Encoder_t + enable EXTI */
void EncoderDataUpdate(Motors_t* Motors);         /* call at 200 Hz */
float EncoderTotalLengthGet(Motors_t* Motors);    /* mean forward distance (cm) */
float EncoderDeltaLengthGet(Motors_t* Motors);    /* left-right distance diff (cm) */
void EncoderLengthClear(Motors_t* Motors);

#endif /* __ENCODER_H */
