#ifndef __HEADFILE_H
#define __HEADFILE_H

/*
 * Single aggregate include + all firmware tunables for the yuntai (云台) board.
 * Every module includes this one header.
 */

#include "ti_msp_dl_config.h"

#include "datatype.h"

/* HardwareDriver */
#include "System.h"
#include "Uart.h"
#include "Stepper.h"

/* FunctionalModule */
#include "Gimbal.h"
#include "Laser.h"
#include "Vision.h"

/*========================= Tunables =========================*/

/* --- Microstepping / mechanics ---
 * MS42C: 1.8° full step => 200 full steps/rev. D36A DIP set to 1/16 microstep.
 * MUST match the hardware DIP switch, or every angle will be wrong by the ratio. */
#define MICROSTEP               16
#define FULL_STEPS_PER_REV      200
#define STEPS_PER_REV           (FULL_STEPS_PER_REV * MICROSTEP)   /* 3200 */
#define STEP_ANGLE_DEG          (360.0f / (float)STEPS_PER_REV)    /* 0.1125 deg/step */

/* Gear reduction from the stepper shaft to the gimbal axis (belt/pulley).
 * 1.0 = motor directly on the axis. Set >1.0 if a reduction is fitted. */
#define GIMBAL_PAN_GEAR         1.0f
#define GIMBAL_TILT_GEAR        1.0f

/* Direction polarity: flip if a positive angle command turns the axis the wrong way
 * (software alternative to swapping the motor A+/A- pair, see profiles/bujin.md). */
#define GIMBAL_PAN_INVERT       0
#define GIMBAL_TILT_INVERT      0

/* --- Step speed ---
 * STEP pulse timers run at 4 MHz (PWM_*_INST_CLK_FREQ). A step rate of f_step Hz
 * needs timer load = 4e6 / f_step - 1. sps = steps per second.
 * Defaults chosen conservative for open-loop MS42C (no missed steps at start). */
#define STEP_SPS_DEFAULT        1000U     /* default move speed */
#define STEP_SPS_MIN            100U
#define STEP_SPS_MAX            8000U

/* --- Vision staleness ---
 * K230 sends a frame every 10 ms; treat the link as stale after this many ms. */
#define VISION_STALE_MS         100U

/* --- Vision closed-loop tuning ---
 * Coarse phase: drive on DxCenter/DyCenter at higher speed until error < THRESH.
 * Fine phase:   drive on DxBase/DyBase at lower speed.
 * Kp: pixel error -> steps  (1 px * Kp = step count to move).
 * Calibrate Kp on-bench: move gimbal a known angle, count pixels that shift. */
#define VIS_COARSE_THRESH       20          /* px: |center err| to enter fine phase */
#define VIS_DEAD_ZONE           1           /* steps: suppress moves smaller than this */
#define VIS_KP_COARSE           0.08f       /* gain for coarse (center) error */
#define VIS_KP_FINE             0.05f       /* gain for fine (base-point) error */
#define STEP_SPS_COARSE         2000U       /* sps while acquiring (coarse) */
#define STEP_SPS_FINE           800U        /* sps while tracking (fine) */

#endif /* __HEADFILE_H */
