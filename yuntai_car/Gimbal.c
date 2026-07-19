#include "Gimbal.h"
#include "Stepper.h"
#include "headfile.h"
#include "app_config.h"
#include <stdlib.h>
#include <math.h>

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
    if (!v || v->Flag != VISION_FLAG_TRACK) {
        Stepper_Stop(STEPPER_PAN);
        Stepper_Stop(STEPPER_TILT);
        return;
    }

    bool coarse = (abs((int)v->DxCenter) > VIS_COARSE_THRESH ||
                   abs((int)v->DyCenter) > VIS_COARSE_THRESH);

    float    kp  = coarse ? VIS_KP_COARSE   : VIS_KP_FINE;
    int16_t  ex  = coarse ? v->DxCenter     : v->DxBase;
    int16_t  ey  = coarse ? v->DyCenter     : v->DyBase;
    uint16_t spd = coarse ? STEP_SPS_COARSE : STEP_SPS_FINE;

    /* Negative sign: positive pixel error means target is ahead of laser,
     * so we move in the positive direction to close the gap. Whether that
     * maps to +steps or -steps depends on physical mounting; flip
     * GIMBAL_PAN_INVERT / GIMBAL_TILT_INVERT in headfile.h if reversed. */
    int32_t dp = -(int32_t)lroundf(kp * (float)ex);
    int32_t dt = -(int32_t)lroundf(kp * (float)ey);

    Stepper_SetSpeed(STEPPER_PAN,  spd);
    Stepper_SetSpeed(STEPPER_TILT, spd);

    /* Only issue a new Move when the axis is idle — avoids repeated stopCounter
     * calls at 100 Hz which cause the motor to restart before completing a step
     * and produce jitter. The next frame will re-evaluate and re-issue if needed. */
    if (abs((int)dp) >= VIS_DEAD_ZONE && !Stepper_Busy(STEPPER_PAN))
        Stepper_Move(STEPPER_PAN,  dp);
    if (abs((int)dt) >= VIS_DEAD_ZONE && !Stepper_Busy(STEPPER_TILT))
        Stepper_Move(STEPPER_TILT, dt);
}

/*
 * 几何开环瞄准（独立于视觉，满足赛题"作业独立于视觉"硬要求）。
 * 入参 (x, y) = 靶心相对云台在水平面的坐标 (mm)：
 *   x = 横向偏移 (+ 表示靶心在云台正前方的右侧)
 *   y = 前向距离 (云台到靶面的水平距离)
 * pan  = atan2(x, y)                         —— 偏航对准靶心水平方向
 * tilt = atan2(靶心高度 - 云台俯仰轴高度, d) —— 俯仰对准靶心竖直方向
 * 其中 d = sqrt(x^2 + y^2) 为水平斜距。角度单位 deg，正负与云台原点约定一致，
 * 装反可用 headfile.h 的 GIMBAL_PAN_INVERT / GIMBAL_TILT_INVERT 翻转。
 */
void Gimbal_AimGeometry(float x, float y)
{
    float d = sqrtf(x * x + y * y);
    if (d < 1.0f) d = 1.0f;   /* 防止贴脸时 atan2 病态 */

    float pan_deg  = atan2f(x, y) * 180.0f / PI;
    float dh       = (float)(AIM_TARGET_H_MM) - (float)(AIM_PIVOT_H_MM);
    float tilt_deg = atan2f(dh, d) * 180.0f / PI;

    Gimbal_MoveToPan(pan_deg);
    Gimbal_MoveToTilt(tilt_deg);
}
