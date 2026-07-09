#ifndef __HEADFILE_H
#define __HEADFILE_H

/*
 * Central aggregate include + tunable parameters for the whole firmware.
 * Include this (and nothing lower-level) from module .c files.
 *
 * Layer includes are added phase-by-phase as each module lands, so the project
 * always builds. See my_car/CLAUDE.md for the phase plan.
 */

#include "ti_msp_dl_config.h"
#include "datatype.h"
#include "main.h"

/* ---- HardwareDriver layer ---- */
#include "System.h"   /* Phase 1 */
#include "Timer.h"    /* Phase 1 */
#include "Uart.h"     /* Phase 1 */
#include "Pwm.h"      /* Phase 3 */
#include "EncoderExti.h" /* Phase 4 */
/* #include "EncoderExti.h" (Phase 4) */
/* #include "Stepper.h"  (Phase 9) */
/* #include "IIC.h"      (Phase 7) */

/* ---- Algorithm layer ---- */
#include "pid.h"      /* Phase 4 */

/* ---- FunctionalModule layer ---- */
#include "MotorsCtrl.h"  /* Phase 3 */
#include "Encoder.h"     /* Phase 4 */
#include "GraySensor.h"  /* Phase 5 */
#include "Buzzer.h"      /* Phase 2 */
#include "Key.h"         /* Phase 2 */
#include "oled.h"        /* Phase 7 (pulled in early as a debug display) */
/* #include "Imu.h"         (Phase 8) */
/* #include "Gimbal.h"      (Phase 9) */
/* #include "Vision.h"      (Phase 10) */

/* ---- ApplicationLayer ---- */
/* #include "Task.h"     (Phase 6) */

/*========================= Tunable parameters =========================*/

/* Wheel-speed PID (per wheel). MG513 + this chassis, EncoderLines still uncal.
 * Feedback is coarsely quantized: 1 count/5ms window ~= 14.5 cm/s, so a big KP
 * turns ±1-count noise into ±145 output swing -> the violent jitter seen when
 * both wheels run straight (turnAngle=0). Fix: low KP (small proportional kick),
 * higher KI so the integrator supplies the steady-state duty smoothly, KD=0.
 * Iout is capped well below Maxout so a stalled wheel can't wind up. */
#define BrushMotor_PID_mode     PID_POSITION
#define BrushMotor_PID_KP       3.0f
#define BrushMotor_PID_KI       0.5f
#define BrushMotor_PID_KD       0.0f
#define BrushMotor_PID_Maxout   1000.0f
#define BrushMotor_PID_MaxIout  700.0f

/* Self-turn (yaw / differential) PID. Used by the closed-loop MotorPidCtrl path
 * (currently NOT the line-follow path — see line-follow block below). */
#define SelfTurn_PID_mode       PID_POSITION
#define SelfTurn_PID_KP         0.3f
#define SelfTurn_PID_KI         0.0f
#define SelfTurn_PID_KD         0.15f
#define SelfTurn_PID_Maxout     200.0f
#define SelfTurn_PID_MaxIout    10.0f

/* Line-following: open-loop base throttle + proportional steer (MotorLineFollow).
 * Deliberately bypasses the wheel-speed PID because EncoderLines is still
 * uncalibrated (Task #14) — velocity feedback is quantized ~14.5 cm/s/count, too
 * coarse to hold a low speed, which is why the closed-loop path jittered instead of
 * driving forward. A fixed base duty guarantees forward motion; steer splits it into
 * a differential.
 *
 * turnAngle (GraySensorToTurnAngle) is ~[-30,+30], POSITIVE = line to the LEFT.
 * Correct-sign mapping (steer the car toward the line):
 *   left  = base - Kp*turnAngle   (line left → left wheel slower → curve left)
 *   right = base + Kp*turnAngle
 * PWM_SetDuty clamps to +-1000, so a large steer that drives one wheel negative just
 * pivots that wheel (sharp turn) — safe. */
#define LineFollow_BaseDuty     80      /* forward duty 0..1000 (~8%, crawl). Likely below the MG513 stall threshold —
                                         * if a wheel buzzes without turning or the two wheels start unevenly, raise it. */
#define LineFollow_SteerKp      2.5f    /* duty per unit turnAngle; scaled down with BaseDuty so slow speed doesn't
                                         * over-steer (a low base makes the same steer a bigger fraction). raise=sharper */
#define MOTOR_SWAP_LR           0       /* set to 1 ONLY if on-board test shows the L/R wheels are physically swapped */
#define MOTOR_INVERT_DIR        1       /* 1 = flip forward/reverse polarity. On-board test showed +duty spun the
                                         * wheels backward (motor leads rewired since Phase 3). Negating both wheels
                                         * equally also keeps steering correct — it only reverses travel direction.
                                         * Set back to 0 if the motor leads are ever restored to Phase-3 wiring. */

/* Mechanical / odometry constants. */
#define ControlFrequency        200.0f   /* Hz, master control-loop rate */
#define EncoderLines            260.0f   /* encoder edges per wheel revolution (measure!) */
#define TireRadius              3.0f      /* wheel radius, cm (measure MG513 wheel!) */

/* Track geometry (北邮 2026 track: 2 straights + 2 semicircle arcs). Calibrate on-site.
 * AB straight = 100 cm (from PDF Figure 1). Arc radii to be measured (~40/60 cm labels). */
#define AB_STRAIGHT_LENGTH      100.0f   /* cm */

#endif /* __HEADFILE_H */
