/* 
 * Copyright (c) 2006-2024, HiPNUC
 * SPDX-License-Identifier: Apache-2.0
 *
 * HiPNUC Decoder Library
 * 
 * 功能概述：
 * 1. 提供HiPNUC协议解析的数据结构定义
 * 2. 定义IMU（0x91/0x92）和组合导航（0x81）数据包结构
 * 3. 声明协议解码接口函数
 * 
 * 兼容性说明：
 * - 支持C/C++混合编程环境
 * - 针对QT框架进行内存对齐优化
 */

#ifndef __HIPNUC_DEC_H__
#define __HIPNUC_DEC_H__

#ifdef __cplusplus
extern "C" {  // 确保C++编译器使用C语言链接规范
#endif

/* 依赖项 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "headfile.h"  // 项目自定义头文件

/* QT框架内存对齐设置（1字节对齐） */
#ifdef QT_CORE_LIB
#pragma pack(push)
#pragma pack(1)
#endif

/* 协议常量定义 */
#define HIPNUC_MAX_RAW_SIZE     (256)   /* 原始数据缓冲区最大容量（含协议头） */

/**
 * @struct hi91_t
 * @brief 0x91类型数据包（浮点型IMU数据）
 * 
 * 数据说明：
 * - 所有传感器数据已转换为浮点物理量
 * - 时间戳单位为毫秒
 * - 姿态角单位为度
 * - 四元数采用标准[w,x,y,z]顺序
 */
typedef struct __attribute__((__packed__)) {
    uint8_t  tag;            /* 包标识符，0x91表示有效数据 */
    uint16_t sttaus;         /* 保留字段（疑似status拼写错误） */
    int8_t   temp;           /* 温度值（℃） */
    float    air_pressure;   /* 气压值（Pa） */
    uint32_t system_time;    /* 系统时间戳（ms） */
    float    acc[3];         /* 三轴加速度（m/s2） */
    float    gyr[3];         /* 三轴陀螺仪（deg/s） */
    float    mag[3];         /* 三轴磁力计（μT） */
    float    roll;           /* 横滚角（-180~180度） */
    float    pitch;          /* 俯仰角（-90~90度） */
    float    yaw;            /* 航向角（0~360度） */
    float    quat[4];        /* 四元数（归一化） */
} hi91_t;

/**
 * @struct hi92_t
 * @brief 0x92类型数据包（整型IMU原始数据）
 * 
 * 数据说明：
 * - 原始数据需按系数转换：
 *   加速度：0.0048828 m/s2/LSB（对应±2000LSB/g）
 *   陀螺仪：0.001 rad/s/LSB → 0.0573 deg/s/LSB
 *   磁力计：0.030517 μT/LSB（量程±4912μT）
 */
typedef struct __attribute__((__packed__)) {
    uint8_t  tag;            /* 包标识符，0x92表示有效数据 */
    uint16_t status;         /* 设备状态位图（见协议文档） */
    int8_t   temperature;    /* 温度（℃） */
    uint16_t rev;            /* 保留字段 */
    int16_t  air_pressure;   /* 原始气压值（需转换） */
    int16_t  reserved;       /* 保留字段 */
    int16_t  gyr_b[3];       /* 原始陀螺仪X/Y/Z（LSB） */
    int16_t  acc_b[3];       /* 原始加速度X/Y/Z（LSB） */
    int16_t  mag_b[3];       /* 原始磁力计X/Y/Z（LSB） */
    int32_t  roll;           /* 原始横滚角（0.001度/LSB） */
    int32_t  pitch;          /* 原始俯仰角（0.001度/LSB） */
    int32_t  yaw;            /* 原始航向角（0.001度/LSB） */
    int16_t  quat[4];        /* 原始四元数（Q14格式，需/16384.0） */
} hi92_t;

/**
 * @struct hi81_t
 * @brief 0x81类型数据包（组合导航数据）
 * 
 * 数据说明：
 * - 定位数据采用整型存储，需转换：
 *   经度/纬度：1e-7度/LSB（例如12345678 → 1.2345678度）
 *   高程：毫米转米（1e-3）
 * - UTC时间字段为BCD编码（例如0x21 → 2021年）
 */
typedef struct __attribute__((__packed__)) {
    uint8_t  tag;            /* 包标识符，0x81表示有效数据 */
    uint16_t status;         /* 系统状态字（见协议文档） */
    uint8_t  ins_status;     /* INS状态（0:无效 1:有效） */
    uint16_t gpst_wn;        /* GPS周数（0-8191） */
    uint32_t gpst_tow;       /* GPS周内秒（0-604799） */
    uint16_t reserved;       /* 保留字段 */
    int16_t  gyr_b[3];       /* 原始陀螺仪X/Y/Z（同hi92_t） */
    int16_t  acc_b[3];       /* 原始加速度X/Y/Z（同hi92_t） */
    int16_t  mag_b[3];       /* 原始磁力计X/Y/Z（同hi92_t） */
    int16_t  air_pressure;   /* 原始气压值（同hi92_t） */
    int16_t  reserved1;      /* 保留字段 */
    int8_t   temperature;    /* 温度（℃） */
    uint8_t  utc_year;       /* UTC年（BCD码，例0x23表示2023） */
    uint8_t  utc_month;      /* UTC月（BCD码，1-12） */
    uint8_t  utc_day;        /* UTC日（BCD码，1-31） */
    uint8_t  utc_hour;       /* UTC时（BCD码，0-23） */
    uint8_t  utc_min;        /* UTC分（BCD码，0-59） */
    uint16_t utc_msec;       /* UTC毫秒（0-999） */
    int16_t  roll;           /* 横滚角（0.01度/LSB） */
    int16_t  pitch;          /* 俯仰角（0.01度/LSB） */
    uint16_t yaw;            /* 航向角（0.01度/LSB） */
    int16_t  quat[4];        /* 四元数（Q30格式，需/32768.0） */
    int32_t  ins_lon;        /* INS经度（1e-7度） */
    int32_t  ins_lat;        /* INS纬度（1e-7度） */
    int32_t  ins_msl;        /* INS海拔（毫米） */
    uint8_t  pdop;           /* 位置精度因子（0.1/LSB） */
    uint8_t  hdop;           /* 水平精度因子（0.1/LSB） */
    uint8_t  solq_pos;       /* 定位质量（0:无效 1:单点 5:固定解） */
    uint8_t  nv_pos;         /* 定位卫星数（0-99） */
    uint8_t  solq_heading;   /* 定向质量（0:无效 4:载波相位） */
    uint8_t  nv_heading;     /* 定向卫星数（0-99） */
    uint8_t  diff_age;       /* 差分龄期（秒） */
    int16_t  undulation;     /* 大地水准面起伏（厘米） */
    uint8_t  ant_status;     /* 天线状态（保留） */
    int16_t  vel_enu[3];     /* ENU速度（cm/s） */
    int16_t  acc_enu[3];     /* ENU加速度（同acc_b） */
    int32_t  gnss_lon;       /* GNSS经度（同ins_lon） */
    int32_t  gnss_lat;       /* GNSS纬度（同ins_lat） */
    int32_t  gnss_msl;       /* GNSS海拔（同ins_msl） */
    uint8_t  reserved2[2];   /* 保留字段 */
} hi81_t;

/**
 * @struct hipnuc_raw_t
 * @brief HiPNUC协议解码器状态结构体
 * 
 * 工作流程：
 * 1. 通过hipnuc_input()逐字节填充buf
 * 2. 完整帧接收后自动解析到hi91/hi92/hi81结构
 * 3. 使用hipnuc_dump_packet()输出可读字符串
 */
typedef struct {
    int nbyte;          /* 当前缓冲区已接收字节数 */
    int len;            /* 有效载荷长度（从协议头获取） */
    uint8_t buf[HIPNUC_MAX_RAW_SIZE]; /* 原始数据缓冲区 */
    hi91_t hi91;        /* 解析后的0x91数据包 */
    hi92_t hi92;        /* 解析后的0x92数据包 */
    hi81_t hi81;        /* 解析后的0x81数据包 */
} hipnuc_raw_t;

/* 恢复默认内存对齐 */
#ifdef QT_CORE_LIB
#pragma pack(pop)
#endif

/******************** 函数声明 ********************/

/**
 * @brief HiPNUC协议解码器输入处理
 * @param raw 解码器状态结构体指针
 * @param data 输入的单个字节
 * @return 解码状态：
 *        1 = 成功解析完整帧
 *        0 = 需要更多数据
 *       -1 = 校验错误或数据异常
 */
int hipnuc_input(hipnuc_raw_t *raw, uint8_t data);

/**
 * @brief 将解析后的数据转换为JSON格式字符串
 * @param raw 包含已解析数据的结构体指针
 * @param buf 输出缓冲区（建议≥512字节）
 * @param buf_size 缓冲区容量
 * @return 成功写入的字符数（不含终止符）
 * 
 * 注意：
 * - 根据tag字段自动选择数据包类型
 * - 生成标准JSON格式，含缩进和换行
 */
int hipnuc_dump_packet(hipnuc_raw_t *raw, char *buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif /* __HIPNUC_DEC_H__ */
