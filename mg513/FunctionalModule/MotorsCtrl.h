#ifndef __MOTORSCTRL_H
#define __MOTORSCTRL_H

#include "datatype.h"

/*
 * Two-wheel differential drivetrain (FunctionalModule layer).
 *
 * Phase 3 (open loop): MotorSetOpenLoop drives fixed signed duty to the TB6612.
 * Phase 4 (closed loop): MotorPidCtrl runs a self-turn PID (turnAngle -> wheel
 * differential) on top of per-wheel speed PIDs (target cm/s -> PWM duty), using
 * encoder feedback (Motors->EncoderLeft/Right->V).
 *
 * Sign convention: +duty / +speed = car forward. Left = TB6612 A-channel
 * (PWMA/AIN), right = B-channel (PWMB/BIN).
 */

void MotorInit(Motors_t** Motors);            /* allocate motors + encoders + PIDs, PWMInit */
void MotorStop(Motors_t* Motors);             /* zero both wheels (keeps driver enabled) */

/* Phase 3 open-loop helper: set signed duty per wheel (-1000..1000). */
void MotorSetOpenLoop(Motors_t* Motors, int16_t left, int16_t right);

/* Phase 4 closed-loop: turnAngle (steering, +=turn one way) + avgSpeed (cm/s).
 * Runs self-turn PID then per-wheel speed PID; writes each motor's Output.
 * NOTE: needs a calibrated EncoderLines to hold speed — see MotorLineFollow for the
 * open-loop path currently used by Phase 5. */
void MotorPidCtrl(Motors_t* Motors, fp32 turnAngle, fp32 avgSpeed);

/* Push each motor's Output field to hardware (direction + PWM). */
void MotorDataUpdate(Motors_t* Motors);

#endif /* __MOTORSCTRL_H */
