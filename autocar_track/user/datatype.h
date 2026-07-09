#ifndef __DATATYPE_H
#define __DATATYPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

typedef   signed char       int8;
typedef unsigned char       u8;
typedef unsigned char       uint8;
typedef unsigned char       byte;
typedef   signed short int  int16;
typedef unsigned short int  uint16;
typedef unsigned short int  u16;
typedef unsigned long int   u32;
typedef float               fp32;
typedef double              fp64;

#define ABS(X)   (((X)>0)?(X):-(X))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)>(b)?(b):(a))
#ifndef PI
#define PI 3.14159265358979f
#endif

/* Gray sensor */
typedef struct {
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
    uint8_t BinaryData;
    uint8_t GraySensorNoData;
} GraySensor_t;

/* Buzzer */
typedef struct {
    int8_t  BuzzerBeeCount;
    int8_t  BuzzerFlag;
    int16_t BuzzerCount;
} Buzzer_t;

/* Encoder */
typedef struct {
    int   EncoderCount;
    int   sample;
    float vel;
    float V;
    float X;
} Encoder_t;

/* PID */
typedef struct {
    uint8_t mode;
    fp32 Kp, Ki, Kd;
    fp32 max_out, max_iout;
    fp32 set, fdb;
    fp32 out, Pout, Iout, Dout;
    fp32 Dbuf[3];
    fp32 error[3];
} Pid_t;

/* Motor state */
typedef enum { Enable = 1, Disable = 0 } Motor_State;

/* Brush motor */
typedef struct {
    Motor_State state;
    int   ExpectOutput;
    int   Output;
} BrushMotor_t;

/* 2-wheel Motors aggregate */
typedef struct {
    Pid_t*        PidSelfTurn;
    BrushMotor_t* MotorLeft;
    Encoder_t*    EncoderLeft;
    Pid_t*        PidLeft;
    BrushMotor_t* MotorRight;
    Encoder_t*    EncoderRight;
    Pid_t*        PidRight;
} Motors_t;

/* Task state */
typedef struct {
    int8_t   CarStartFlag;
    float    AverageSpeed;
    int8_t   KeyPressFlag;
    int8_t   LastKeyPressFlag;
    int32_t  KeyPressCount;
    int8_t   CircleCount;
    int8_t   RightAngleCount;
    int8_t   CarBeginLengthCountFlag;
    float    CarBeginLengthCount;
    float    Deltayaw;
} Task_t;

/* Top-level car */
typedef struct {
    GraySensor_t* GraySensor;
    Buzzer_t*     Buzzer;
    Motors_t*     Motors;
    Task_t*       Tasks;
} Car_t;

#endif /* __DATATYPE_H */
