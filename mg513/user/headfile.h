#ifndef __HEADFILE_H
#define __HEADFILE_H

/*
 * Central aggregate include + tunable parameters (mg513 motor/encoder/OLED demo).
 *
 * This is the pruned migration of my_car's headfile.h: only the motor-drive,
 * quadrature-encoder, and OLED subsystems were carried over, so the sensor /
 * gimbal / vision / UART / key / buzzer includes are intentionally dropped.
 * Include this (and nothing lower-level) from module .c files.
 */

#include "ti_msp_dl_config.h"
#include "datatype.h"
#include "main.h"

/* ---- HardwareDriver layer ---- */
#include "System.h"       /* SysTick timebase + delays */
#include "Timer.h"        /* 200 Hz control tick (TIM_CTRL / TIMG7) */
#include "Pwm.h"          /* TB6612 PWM over TIMG0 */
#include "EncoderExti.h"  /* quadrature edge ISR (GPIOA + GPIOB) */

/* ---- Algorithm layer ---- */
#include "pid.h"

/* ---- FunctionalModule layer ---- */
#include "MotorsCtrl.h"   /* differential drivetrain + PID */
#include "Encoder.h"      /* wheel odometry */
#include "oled.h"         /* SSD1306 over I2C1 */

/*========================= Tunable parameters =========================*/

/* Wheel-speed PID (per wheel). See my_car headfile.h for the tuning rationale;
 * feedback is coarsely quantized so KP is kept low and KI supplies steady-state. */
#define BrushMotor_PID_mode     PID_POSITION
#define BrushMotor_PID_KP       3.0f
#define BrushMotor_PID_KI       0.5f
#define BrushMotor_PID_KD       0.0f
#define BrushMotor_PID_Maxout   1000.0f
#define BrushMotor_PID_MaxIout  700.0f

/* Self-turn (yaw / differential) PID — used by the closed-loop MotorPidCtrl path. */
#define SelfTurn_PID_mode       PID_POSITION
#define SelfTurn_PID_KP         0.3f
#define SelfTurn_PID_KI         0.0f
#define SelfTurn_PID_KD         0.15f
#define SelfTurn_PID_Maxout     200.0f
#define SelfTurn_PID_MaxIout    10.0f

/* Motor polarity / swap fixups (validate on hardware).
 *   MOTOR_SWAP_LR  = 1 only if the physical L/R wheels are swapped.
 *   MOTOR_INVERT_DIR = 1 flips forward/reverse for both wheels (steering preserved). */
#define MOTOR_SWAP_LR           0
#define MOTOR_INVERT_DIR        0

/* Mechanical / odometry constants (my_car values — re-measure for this chassis).
 * cm/s readouts are only accurate once EncoderLines / TireRadius are calibrated;
 * pulse counting and direction verification do not depend on them. */
#define ControlFrequency        200.0f   /* Hz, master control-loop rate (TIM_CTRL) */
#define EncoderLines            260.0f   /* encoder edges per wheel revolution (measure!) */
#define TireRadius              3.0f      /* wheel radius, cm (measure!) */

#endif /* __HEADFILE_H */
