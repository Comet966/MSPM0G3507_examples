#ifndef __MOTORSCTRL_H
#define __MOTORSCTRL_H

#include "datatype.h"

/*
 * Two-wheel differential drivetrain (FunctionalModule layer).
 *
 * Phase 3 (open loop): allocate the Motors_t aggregate + left/right BrushMotor_t,
 * and drive fixed signed duty straight to the TB6612 via Pwm.c. Encoder feedback
 * and the wheel-speed / self-turn PID land in Phase 4 (MotorPidCtrl).
 *
 * Sign convention: +duty / +speed = car forward. Left uses TB6612 A-channel
 * (PWMA/AIN), right uses B-channel (PWMB/BIN).
 */

void MotorInit(Motors_t** Motors);            /* allocate + PWMInit (STBY on, duty 0) */
void MotorStop(Motors_t* Motors);             /* zero both wheels (keeps driver enabled) */

/* Phase 3 open-loop helper: set signed duty per wheel (-1000..1000). */
void MotorSetOpenLoop(Motors_t* Motors, int16_t left, int16_t right);

/* Push each motor's Output field to hardware (direction + PWM). */
void MotorDataUpdate(Motors_t* Motors);

#endif /* __MOTORSCTRL_H */
