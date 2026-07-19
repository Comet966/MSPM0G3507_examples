#ifndef __PD42S1_H
#define __PD42S1_H

/*
 * PD42S1.h — 正点原子 PD42S1 闭环步进驱动器 串口/RS485 协议层 (可移植, 无 TI 依赖)
 *
 * 帧格式(下行): 0xC5 | addr | code | data... | checksum | 0x5C
 *   checksum = (帧头 + 地址 + 功能码 + 全部数据字节) & 0xFF
 * 帧格式(上行): 0xC5 | addr | code | err | data... | checksum | 0x5C
 *   err=0x01 应答成功; 0xE1~0xE6 为各类错误
 * 多字节整数一律大端(高位在前); float 亦按大端 4 字节发送.
 * 位置模式: 51200 counts = 电机轴一圈.
 *
 * 收发通过 pd42_bus_t 回调解耦: 你只需实现 write() (RS485 需在其中翻转 DE),
 * 以及可选的 read() (仅当调用读取类命令时需要). 这样本模块可移植到任意 MCU.
 */

#include <stdint.h>
#include <stdbool.h>

#define PD42_FRAME_HEAD      0xC5
#define PD42_FRAME_TAIL      0x5C
#define PD42_ADDR_BROADCAST  0x00           /* 广播/分组地址 -> 从机不应答 */
#define PD42_COUNTS_PER_REV  51200L         /* 位置模式: 51200 = 一圈 */
#define PD42_MAX_FRAME       64
#define PD42_ACK_OK          0x01           /* 应答成功错误码 */

/* 旋转方向字节 */
#define PD42_DIR_CW          0              /* 正转 */
#define PD42_DIR_CCW         1              /* 反转 */

/* 使能字节 (0xFA) */
#define PD42_EN_ON           0              /* 使能电机 */
#define PD42_EN_OFF          1              /* 失能电机 */

/* 工作模式 (0x62) */
typedef enum {
    PD42_MODE_POS_COMM   = 0x00,   /* 通信位置模式 */
    PD42_MODE_SPEED_COMM = 0x01,   /* 通信速度模式 */
    PD42_MODE_TORQUE     = 0x02,   /* 通信力矩模式 */
    PD42_MODE_PULSE      = 0x03,   /* 脉冲模式 */
    PD42_MODE_HOMING     = 0x07,   /* 回零模式 */
    PD42_MODE_OL_SPEED   = 0x08,   /* 开环速度 */
    PD42_MODE_OL_POS     = 0x09,   /* 开环位置 */
} pd42_mode_t;

/* 电机运行状态 (读 0x2C 返回 data[0]) */
typedef enum {
    PD42_RUN_STOP     = 0,   /* 停止 */
    PD42_RUN_DONE     = 1,   /* 任务完成 */
    PD42_RUN_MOVING   = 2,   /* 正在运行 */
    PD42_RUN_OVERLOAD = 3,   /* 过载 */
    PD42_RUN_STALL    = 4,   /* 堵转 */
    PD42_RUN_UNDERVOLT= 5,   /* 欠压 */
} pd42_run_state_t;

/*
 * 总线抽象: 收发字节的回调.
 *  write:  把 len 字节整帧一次性发出. RS485 时须在函数内 DE=发送 -> 发送 ->
 *          等发送完成 -> DE=接收. 返回实际发送字节数(或 <0 出错).
 *  read:   在 timeout_ms 内尽量读取字节到 buf(最多 max), 返回读到的字节数(可为 0).
 *          仅在使用读取类 API 时需要; 只发不收可传 NULL.
 *  ctx:    透传给回调的用户上下文(例如串口句柄/DE 引脚描述).
 */
typedef struct {
    int  (*write)(void *ctx, const uint8_t *buf, uint16_t len);
    int  (*read )(void *ctx, uint8_t *buf, uint16_t max, uint32_t timeout_ms);
    void  *ctx;
} pd42_bus_t;

/* 单个驱动器句柄 */
typedef struct {
    const pd42_bus_t *bus;
    uint8_t addr;        /* 从机地址 1~255 (0=广播,不应答) */
} pd42_t;

/* 解析后的上行帧 */
typedef struct {
    uint8_t addr;
    uint8_t code;        /* 功能码 */
    uint8_t err;         /* 错误码, PD42_ACK_OK=成功 */
    uint8_t data[PD42_MAX_FRAME];
    uint8_t data_len;    /* data 有效长度 */
} pd42_reply_t;

/* ---- 底层: 组帧 / 校验 / 解析 (无收发, 便于单测) ---- */
uint8_t  pd42_checksum(const uint8_t *data, uint8_t len);
/* 组一帧到 out(容量>=PD42_MAX_FRAME): head+addr+code+payload+checksum+tail. 返回帧长. */
uint16_t pd42_build(uint8_t *out, uint8_t addr, uint8_t code,
                    const uint8_t *payload, uint8_t payload_len);
/* 解析上行帧到 r. 校验帧头/帧尾/校验和. 成功且 err==OK 返回 true. */
bool     pd42_parse(const uint8_t *buf, uint16_t len, pd42_reply_t *r);

/* ---- 句柄初始化 ---- */
void pd42_init(pd42_t *m, const pd42_bus_t *bus, uint8_t addr);

/* ---- 运动控制 (只发, 不解析应答) ---- */
void pd42_enable(pd42_t *m, bool on);                 /* 0xFA 使能/失能 */
void pd42_stop(pd42_t *m);                            /* 0xFC 立即刹车 */
void pd42_zero_angle(pd42_t *m);                      /* 0xF8 当前位置清零 */
void pd42_clear_state(pd42_t *m);                     /* 0xFB 清除堵转/刹车/失能 */
void pd42_torque(pd42_t *m, uint8_t dir, uint16_t ma);/* 0xF0 力矩(mA) */
void pd42_speed(pd42_t *m, uint8_t dir, uint8_t acc, float rpm);          /* 0xF1 速度 */
void pd42_pos_abs(pd42_t *m, uint8_t dir, uint8_t acc, uint16_t rpm, uint32_t counts); /* 0xF2 */
void pd42_pos_rel(pd42_t *m, uint8_t dir, uint8_t acc, uint16_t rpm, uint32_t counts); /* 0xF3 */

/* ---- 配置 (须开机时一次性下发; 建议随后 pd42_param_save) ---- */
void pd42_set_mode(pd42_t *m, pd42_mode_t mode);      /* 0x62 */
void pd42_set_microstep(pd42_t *m, uint16_t step);    /* 0x65 细分 1~256 */
void pd42_set_pos_torque(pd42_t *m, int16_t ma);      /* 0x64 位置环最大力矩 */
void pd42_param_save(pd42_t *m);                      /* 0x04 保存到 flash */

/* ---- 读取类 (需 bus->read 有效; 阻塞至超时). 成功返回 true 并填 *out ---- */
bool pd42_read_pos(pd42_t *m, int32_t *counts, uint32_t timeout_ms);      /* 0x2A */
bool pd42_read_speed(pd42_t *m, int16_t *rpm, uint32_t timeout_ms);       /* 0x29 */
bool pd42_read_run_state(pd42_t *m, uint8_t *state, uint32_t timeout_ms); /* 0x2C */
bool pd42_read_arrived(pd42_t *m, bool *arrived, uint32_t timeout_ms);    /* 0x30 到位标志 */

#endif /* __PD42S1_H */
