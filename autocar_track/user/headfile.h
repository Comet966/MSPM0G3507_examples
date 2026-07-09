#ifndef __HEADFILE_H
#define __HEADFILE_H

#include "ti_msp_dl_config.h"
#include "datatype.h"
#include "main.h"

/* HardwareDriver */
#include "System.h"
#include "Timer.h"
#include "Uart.h"
#include "Pwm.h"
#include "EncoderExti.h"

/* Algorithm */
#include "pid.h"

/* FunctionalModule */
#include "MotorsCtrl.h"
#include "Encoder.h"
#include "GraySensor.h"
#include "Buzzer.h"
#include "Key.h"
#include "Oled.h"

/* ApplicationLayer */
#include "Task.h"

/*========================= Tunable parameters =========================*/

#define BrushMotor_PID_mode     PID_POSITION
#define BrushMotor_PID_KP       3.0f
#define BrushMotor_PID_KI       0.5f
#define BrushMotor_PID_KD       0.0f
#define BrushMotor_PID_Maxout   1000.0f
#define BrushMotor_PID_MaxIout  700.0f

#define SelfTurn_PID_mode       PID_POSITION
#define SelfTurn_PID_KP         0.3f
#define SelfTurn_PID_KI         0.0f
#define SelfTurn_PID_KD         0.15f
#define SelfTurn_PID_Maxout     200.0f
#define SelfTurn_PID_MaxIout    10.0f

#define ControlFrequency        200.0f
#define EncoderLines            260.0f
#define TireRadius              3.0f

/* Race strategy (北邮 2026 racetrack: 2 straights + 2 semicircle arcs, no corners).
 * Lap completion is judged purely by accumulated centerline odometry.
 * Constant cruise speed the whole way; calibrate LAP_LENGTH on-site. */
#define CAR_CRUISE_SPEED        30.0f    /* constant speed command (per-wheel PID target) */
#define LAP_LENGTH              300.0f   /* cm, one full lap centerline length — MEASURE on-site */
#define LAP_COUNT               3        /* laps to run before auto-stop */

#endif /* __HEADFILE_H */
