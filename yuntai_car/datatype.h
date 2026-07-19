#ifndef __DATATYPE_H
#define __DATATYPE_H

/* Standard headers */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

/*------------------------- Math macros -------------------------*/
#ifndef ABS
#define ABS(X)    (((X) > 0) ? (X) : -(X))
#endif
#ifndef PI
#define PI 3.14159265358979323846f
#endif

/*========================= Subsystem structs =========================*/

/* 2-axis gimbal: pan (yaw) + tilt (pitch), each a MS42C stepper via D36A in STEP/DIR.
 * Angles are the commanded target; the authoritative position is the stepper's
 * step accumulator (see Stepper_Position / Gimbal_GetPan/Tilt). */
typedef struct
{
    float   PanAngle;    /* last commanded pan angle (deg) */
    float   TiltAngle;   /* last commanded tilt angle (deg) */
    uint8_t AimMode;     /* 0=idle, 1=geometry open-loop, 2=vision closed-loop */
} Gimbal_t;

/* K230 vision link (UART2). Decoded from the 13-byte frame (see profiles/下位机通信协议).
 * dx/dy are already sign-restored: deviation = target - detected, so a positive value
 * means "move the gimbal in the positive direction to close the error". */
typedef struct
{
    uint8_t  Flag;        /* 0xBB = tracking, 0xCC = target lost */
    int16_t  DxCenter;    /* target-center X offset (px), +=target to the right */
    int16_t  DyCenter;    /* target-center Y offset (px), +=target below */
    uint8_t  BaseIndex;   /* current reference point index on the circle (0..15) */
    int16_t  DxBase;      /* reference-point X offset (px) */
    int16_t  DyBase;      /* reference-point Y offset (px) */
    uint32_t FrameCount;  /* total valid frames parsed (telemetry) */
    uint32_t LastRxMs;    /* millis() of last valid frame (staleness check) */
} Vision_t;

#endif /* __DATATYPE_H */
