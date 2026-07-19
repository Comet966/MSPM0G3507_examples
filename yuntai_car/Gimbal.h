#ifndef __GIMBAL_H
#define __GIMBAL_H

#include <stdbool.h>
#include "datatype.h"

/*
 * Gimbal angle layer — converts degrees <-> stepper steps on top of Stepper.c.
 *
 * "Angle" is measured from the origin captured at boot (or via Gimbal_SetOrigin);
 * it is derived from the stepper step accumulator, so it stays truthful across
 * moves. steps = deg / STEP_ANGLE_DEG * gear (see headfile.h tunables).
 *
 * All moves are non-blocking (they hand off to Stepper_Move). Poll Gimbal_Busy
 * to know when the axes have settled.
 */

void  Gimbal_Init(Gimbal_t **g);   /* allocate + Stepper_Init + enable both axes */

void  Gimbal_MoveToPan(float deg);   /* absolute target angle */
void  Gimbal_MoveToTilt(float deg);
void  Gimbal_MoveByPan(float deg);   /* relative */
void  Gimbal_MoveByTilt(float deg);

float Gimbal_GetPan(void);           /* current angle from step count (deg) */
float Gimbal_GetTilt(void);

bool  Gimbal_Busy(void);             /* true if either axis is moving */
void  Gimbal_Home(void);             /* move both axes back to 0 deg */
void  Gimbal_SetOrigin(void);        /* define current pose as 0/0 */
void  Gimbal_Enable(bool on);        /* enable/disable both drivers */

/* --- Reserved aiming interfaces (next milestone; parse path is already live) ---
 * These are intentionally stubbed this phase so the vision closed loop and the
 * geometric open-loop fallback can drop in without touching the angle layer. */
void  Gimbal_AimVision(Vision_t *v);        /* K230 pixel-error closed loop */
void  Gimbal_AimGeometry(float x, float y); /* geometry open-loop fallback */

#endif /* __GIMBAL_H */
