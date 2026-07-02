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
/* #include "EncoderExti.h" (Phase 4) */
/* #include "Stepper.h"  (Phase 9) */
/* #include "IIC.h"      (Phase 7) */

/* ---- Algorithm layer ---- */
/* #include "pid.h"      (Phase 4) */

/* ---- FunctionalModule layer ---- */
#include "MotorsCtrl.h"  /* Phase 3 */
/* #include "Encoder.h"     (Phase 4) */
/* #include "GraySensor.h"  (Phase 5) */
#include "Buzzer.h"      /* Phase 2 */
#include "Key.h"         /* Phase 2 */
/* #include "Oled.h"        (Phase 7) */
/* #include "Imu.h"         (Phase 8) */
/* #include "Gimbal.h"      (Phase 9) */
/* #include "Vision.h"      (Phase 10) */

/* ---- ApplicationLayer ---- */
/* #include "Task.h"     (Phase 6) */

/*========================= Tunable parameters =========================*/

/* Wheel-speed PID (per wheel). Retune for MG513 + this chassis. */
#define BrushMotor_PID_mode     PID_POSITION
#define BrushMotor_PID_KP       10.0f
#define BrushMotor_PID_KI       0.8f
#define BrushMotor_PID_KD       5.0f
#define BrushMotor_PID_Maxout   1000.0f
#define BrushMotor_PID_MaxIout  1000.0f

/* Self-turn (yaw / differential) PID. */
#define SelfTurn_PID_mode       PID_POSITION
#define SelfTurn_PID_KP         0.8f
#define SelfTurn_PID_KI         0.0f
#define SelfTurn_PID_KD         0.15f
#define SelfTurn_PID_Maxout     200.0f
#define SelfTurn_PID_MaxIout    10.0f

/* Mechanical / odometry constants. */
#define ControlFrequency        200.0f   /* Hz, master control-loop rate */
#define EncoderLines            260.0f   /* encoder edges per wheel revolution (measure!) */
#define TireRadius              3.0f      /* wheel radius, cm (measure MG513 wheel!) */

/* Track geometry (北邮 2026 track: 2 straights + 2 semicircle arcs). Calibrate on-site.
 * AB straight = 100 cm (from PDF Figure 1). Arc radii to be measured (~40/60 cm labels). */
#define AB_STRAIGHT_LENGTH      100.0f   /* cm */

#endif /* __HEADFILE_H */
