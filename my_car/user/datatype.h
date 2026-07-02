#ifndef __DATATYPE_H
#define __DATATYPE_H

/* Standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/*------------------------- Fixed-width type aliases -------------------------*/
/* Kept for source-compatibility with reused reference_code modules (Algorithm/, Task). */
typedef   signed char       int8;
typedef unsigned char       u8;
typedef unsigned char       uint8;
typedef unsigned char       byte;
typedef   signed short int  int16;
typedef unsigned short int  u16;
typedef unsigned short int  uint16;
typedef unsigned long  int  u32;
typedef float               fp32;
typedef double              fp64;

/*------------------------- Math macros -------------------------*/
#define ABS(X)    (((X) > 0) ? (X) : -(X))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

#ifndef PI
#define PI 3.14159265358979323846f
#endif

/*========================= Sensor / actuator structs =========================*/

/* JY901S (WitMotion) attitude — filled from UART 0x55 frames. */
typedef struct
{
    float Accel[3];   /* m/s^2  [X,Y,Z] */
    float Gyro[3];    /* deg/s  [X,Y,Z] */
    float Roll;       /* deg */
    float Pitch;      /* deg */
    float Yaw;        /* deg, -180..180 as reported by device */
    float YawTotal;   /* deg, unwrapped (accumulates across +-180 wrap) */
    float YawLast;    /* deg, previous raw yaw (for unwrap) */
    uint8_t Ready;    /* 1 once a valid frame has been parsed */
} Imu_t;

/* 感为 (Ganwei) 8-way DIGITAL line sensor.
 * Bits come straight from LineWalk_read(): bit0=CH1 (leftmost) .. bit7=CH8.
 * Convention in this project: 1 = line detected (black/dark under sensor), 0 = white.
 * (LineWalk_read returns 1=white; GraySensorDataUpdate inverts to this convention.) */
typedef struct
{
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
    uint8_t BinaryData;       /* raw 8-bit field, bit0=CH1 .. bit7=CH8 (line=1) */
    uint8_t GraySensorNoData; /* 1 = no channel sees the line (all lost) */
} GraySensor_t;

/* Buzzer beat-pattern state (updated at 10 Hz). */
typedef struct
{
    int8_t  BuzzerBeeCount; /* remaining beeps queued */
    int8_t  BuzzerFlag;     /* current on/off phase */
    int16_t BuzzerCount;    /* down-counter for current phase */
} Buzzer_t;

/* Incremental encoder (one per drive wheel). EncoderCount written by EXTI ISR. */
typedef struct
{
    volatile int32_t EncoderCount; /* signed edge count accumulated by ISR since last sample */
    int32_t          sample;       /* edges captured in the last 200 Hz window (count snapshot) */
    float            vel;          /* wheel angular velocity (rad/s), low-pass filtered */
    float            V;            /* wheel linear velocity (cm/s) = vel * TireRadius */
    float            X;            /* accumulated distance travelled (cm) */
} Encoder_t;

/* PID controller (Algorithm/pid.c). Layout must match reference pid.c. */
typedef struct
{
    uint8_t mode;
    fp32 Kp, Ki, Kd;
    fp32 max_out, max_iout;
    fp32 set, fdb;
    fp32 out, Pout, Iout, Dout;
    fp32 Dbuf[3];
    fp32 error[3];
} Pid_t;

typedef enum { Enable = 1, Disable = 0 } Motor_State;

/* One brushed DC gear-motor (MG513) driven through the TB6612. */
typedef struct
{
    Motor_State state;
    int   ExpectOutput; /* PID target (wheel velocity setpoint domain) */
    int   Output;       /* last applied signed PWM (-1000..1000) */
} BrushMotor_t;

/* Two-wheel differential drivetrain: left + right, each wheel = motor+encoder+speed PID,
 * plus one self-turn (yaw) PID that splits average speed into a differential. */
typedef struct
{
    Pid_t*        PidSelfTurn;   /* turn-angle -> differential */

    BrushMotor_t* MotorLeft;
    Encoder_t*    EncoderLeft;
    Pid_t*        PidLeft;       /* left wheel speed PID */

    BrushMotor_t* MotorRight;
    Encoder_t*    EncoderRight;
    Pid_t*        PidRight;      /* right wheel speed PID */
} Motors_t;

/* 2-axis gimbal: pan (yaw) + tilt (pitch), each a MS42C stepper via D36A in STEP/DIR. */
typedef struct
{
    float   PanAngle;    /* commanded pan angle (deg) */
    float   TiltAngle;   /* commanded tilt angle (deg) */
    uint8_t AimMode;     /* 0=idle, 1=geometry open-loop, 2=vision closed-loop, 3=draw */
} Gimbal_t;

/* K230 vision link (UART). Target-center pixel offset from image center. */
typedef struct
{
    uint8_t Found;   /* 1 = target currently detected */
    int16_t Dx;      /* target-center X offset from image center (px) */
    int16_t Dy;      /* target-center Y offset from image center (px) */
    int16_t Dist;    /* estimated distance (cm), optional/0 */
    uint32_t LastRxMs; /* millis() of last valid frame (staleness check) */
} Vision_t;

/* Top-level task / track state machine. */
typedef struct
{
    int8_t   CarStartFlag;   /* 1 = running */
    float    AverageSpeed;   /* commanded forward speed */
    int8_t   KeyPressFlag;
    int8_t   LastKeyPressFlag;
    int32_t  KeyPressCount;
    int8_t   CircleCount;    /* laps remaining / completed */
    int8_t   KeyPointCount;  /* A/B/C/D progress within a lap */
    float    Deltayaw;       /* steering command fed to MotorPidCtrl */
} Task_t;

/* Global aggregator: single Car_t Car in main.c holds pointers to every subsystem. */
typedef struct
{
    Imu_t*        Imu;        /* JY901S */
    GraySensor_t* GraySensor; /* 感为 8-way */
    Buzzer_t*     Buzzer;
    Motors_t*     Motors;     /* 2-wheel differential */
    Gimbal_t*     Gimbal;     /* 2-axis stepper gimbal */
    Vision_t*     Vision;     /* K230 */
    Task_t*       Tasks;
} Car_t;

#endif /* __DATATYPE_H */
