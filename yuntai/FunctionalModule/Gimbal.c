#include "Gimbal.h"
#include "Stepper.h"
#include "headfile.h"

/*
 * Angle <-> step conversion for the 2-axis gimbal.
 *
 *   steps_per_deg = STEPS_PER_REV * gear / 360
 *
 * Absolute moves compute the delta from the stepper's current step position to the
 * requested target and issue a relative Stepper_Move; the step accumulator remains
 * the single source of truth for the current angle.
 */

static Gimbal_t s_gimbal;

static inline float pan_steps_per_deg(void)
{
    return (float)STEPS_PER_REV * GIMBAL_PAN_GEAR / 360.0f;
}
static inline float tilt_steps_per_deg(void)
{
    return (float)STEPS_PER_REV * GIMBAL_TILT_GEAR / 360.0f;
}

void Gimbal_Init(Gimbal_t **g)
{
    s_gimbal.PanAngle  = 0.0f;
    s_gimbal.TiltAngle = 0.0f;
    s_gimbal.AimMode   = 0;

    Stepper_Init();
    Stepper_SetSpeed(STEPPER_PAN,  STEP_SPS_DEFAULT);
    Stepper_SetSpeed(STEPPER_TILT, STEP_SPS_DEFAULT);
    Stepper_ZeroPosition(STEPPER_PAN);
    Stepper_ZeroPosition(STEPPER_TILT);
    Stepper_Enable(STEPPER_PAN,  true);
    Stepper_Enable(STEPPER_TILT, true);

    if (g) *g = &s_gimbal;
}

void Gimbal_MoveToPan(float deg)
{
    int32_t target = (int32_t)lroundf(deg * pan_steps_per_deg());
    int32_t delta  = target - Stepper_Position(STEPPER_PAN);
    s_gimbal.PanAngle = deg;
    if (delta != 0) Stepper_Move(STEPPER_PAN, delta);
}

void Gimbal_MoveToTilt(float deg)
{
    int32_t target = (int32_t)lroundf(deg * tilt_steps_per_deg());
    int32_t delta  = target - Stepper_Position(STEPPER_TILT);
    s_gimbal.TiltAngle = deg;
    if (delta != 0) Stepper_Move(STEPPER_TILT, delta);
}

void Gimbal_MoveByPan(float deg)
{
    Gimbal_MoveToPan(Gimbal_GetPan() + deg);
}

void Gimbal_MoveByTilt(float deg)
{
    Gimbal_MoveToTilt(Gimbal_GetTilt() + deg);
}

float Gimbal_GetPan(void)
{
    return (float)Stepper_Position(STEPPER_PAN) / pan_steps_per_deg();
}

float Gimbal_GetTilt(void)
{
    return (float)Stepper_Position(STEPPER_TILT) / tilt_steps_per_deg();
}

bool Gimbal_Busy(void)
{
    return Stepper_Busy(STEPPER_PAN) || Stepper_Busy(STEPPER_TILT);
}

void Gimbal_Home(void)
{
    Gimbal_MoveToPan(0.0f);
    Gimbal_MoveToTilt(0.0f);
}

void Gimbal_SetOrigin(void)
{
    Stepper_Stop(STEPPER_PAN);
    Stepper_Stop(STEPPER_TILT);
    Stepper_ZeroPosition(STEPPER_PAN);
    Stepper_ZeroPosition(STEPPER_TILT);
    s_gimbal.PanAngle  = 0.0f;
    s_gimbal.TiltAngle = 0.0f;
}

void Gimbal_Enable(bool on)
{
    Stepper_Enable(STEPPER_PAN,  on);
    Stepper_Enable(STEPPER_TILT, on);
}

/*=========================== Reserved aiming (stubs) ===========================*/
/* Next milestone. The Vision parser (Vision.c) already fills Vision_t from the
 * K230 link; these turn that error signal into gimbal motion. */

void Gimbal_AimVision(Vision_t *v)
{
    (void)v;
    /* TODO(vision closed loop):
     *   if (v->Flag == 0xBB) {
     *       // Use DxBase/DyBase for fine aim at the active reference point.
     *       // Map pixel error -> small MoveBy (or a proper PID on the error),
     *       // e.g. Gimbal_MoveByPan(-Kp_x * v->DxBase);
     *       //      Gimbal_MoveByTilt(-Kp_y * v->DyBase);
     *   } else {                       // 0xCC: target lost
     *       Stepper_Stop(...);         // hold position, wait for re-acquire
     *   }
     */
}

void Gimbal_AimGeometry(float x, float y)
{
    (void)x; (void)y;
    /* TODO(geometry open-loop fallback):
     *   Given the target's physical (x,y) relative to the gimbal and the known
     *   mounting geometry, compute pan/tilt angles by atan2 and call
     *   Gimbal_MoveToPan/Tilt. Used when vision is unavailable. */
}
