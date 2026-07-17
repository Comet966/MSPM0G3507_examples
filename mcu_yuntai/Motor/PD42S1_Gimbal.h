#ifndef __PD42S1_GIMBAL_H
#define __PD42S1_GIMBAL_H

/*
 * PD42S1_Gimbal.h — 双轴(Pan/Tilt) PD42S1 云台角度层.
 *
 * 两台 PD42S1 通过不同从机地址挂在同一条 RS485 总线上, 共用一个 pd42_bus_t.
 * 本层把"角度(度)"换算成 51200/圈 的脉冲数并调用绝对/相对位置模式.
 *   counts = angle_deg / 360 * PD42_COUNTS_PER_REV * gear
 * 其中 gear = 云台轴一圈 / 电机轴一圈 (直驱=1.0). sign 用于翻转正方向.
 *
 * 角度以开机点(或 gimbal_set_origin)为 0. 绝对角度记录在软件里(target_deg),
 * 也可用 gimbal_read_angle 从驱动器实时位置回读真实角度.
 */

#include "PD42S1.h"

typedef struct {
    pd42_t   drv;         /* 该轴驱动器句柄 */
    float    gear;        /* 减速比(云台:电机), 直驱=1.0 */
    int8_t   sign;        /* +1 或 -1: 正角度对应的电机方向 */
    uint16_t rpm;         /* 位置模式最大速度(RPM) */
    uint8_t  acc;         /* 加减速 0~200, 0=直接启动 */
    float    origin_deg;  /* 原点偏移(度), 由 set_origin 记录 */
    float    target_deg;  /* 最近一次目标角度(软件记账) */
} pd42_axis_t;

typedef struct {
    pd42_axis_t pan;
    pd42_axis_t tilt;
} pd42_gimbal_t;

/* 单轴参数配置 (在 gimbal_init 前逐轴填好, 或用默认后覆盖) */
void pd42_axis_config(pd42_axis_t *ax, const pd42_bus_t *bus, uint8_t addr,
                      float gear, int8_t sign, uint16_t rpm, uint8_t acc);

/*
 * 初始化云台: 对两轴设为通信位置模式、设细分、使能.
 * microstep 传 0 表示不改动驱动器当前细分设置.
 * 注意: counts/圈=51200 与细分无关(协议固定), 细分只影响运行平滑度.
 */
void pd42_gimbal_init(pd42_gimbal_t *g, uint16_t microstep);

/* 绝对角度移动(度), 非阻塞: 下发一条位置指令后立即返回 */
void pd42_gimbal_move_pan(pd42_gimbal_t *g, float deg);
void pd42_gimbal_move_tilt(pd42_gimbal_t *g, float deg);
/* 相对角度移动(度) */
void pd42_gimbal_move_by_pan(pd42_gimbal_t *g, float deg);
void pd42_gimbal_move_by_tilt(pd42_gimbal_t *g, float deg);

/* 软件记账的当前目标角度 */
float pd42_gimbal_get_pan(const pd42_gimbal_t *g);
float pd42_gimbal_get_tilt(const pd42_gimbal_t *g);

/* 从驱动器回读实时角度(度). 需 bus->read 有效, 成功返回 true */
bool  pd42_gimbal_read_pan(pd42_gimbal_t *g, float *deg, uint32_t timeout_ms);
bool  pd42_gimbal_read_tilt(pd42_gimbal_t *g, float *deg, uint32_t timeout_ms);

/* 两轴是否都到位(需 read). 任一轴查询失败返回 false */
bool  pd42_gimbal_arrived(pd42_gimbal_t *g, uint32_t timeout_ms);

void  pd42_gimbal_home(pd42_gimbal_t *g);        /* 回到 0/0 */
void  pd42_gimbal_set_origin(pd42_gimbal_t *g);  /* 把当前位置定义为 0/0(驱动器清零) */
void  pd42_gimbal_enable(pd42_gimbal_t *g, bool on);
void  pd42_gimbal_stop(pd42_gimbal_t *g);        /* 两轴立即刹车 */

#endif /* __PD42S1_GIMBAL_H */
