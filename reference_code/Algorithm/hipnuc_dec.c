/*
 * Copyright (c) 2006-2024, HiPNUC
 * SPDX-License-Identifier: Apache-2.0
 */

#include "hipnuc_dec.h"

/* HiPNUC协议相关常量定义 */
#define CHSYNC1                 (0x5A)              /* 协议同步头第一个字节 */
#define CHSYNC2                 (0xA5)              /* 协议同步头第二个字节 */
#define CH_HDR_SIZE             (0x06)              /* 协议头部长度（包含同步字） */

/* 旧版传感器数据类型标识（兼容HI226/HI229等设备） */
#define HIPNUC_ID_USRID         (0x90)              /* 用户自定义ID */
#define HIPNUC_ID_ACC_RAW       (0xA0)              /* 原始加速度数据 */
#define HIPNUC_ID_ACC_CAL       (0xA1)              /* 校准后加速度数据 */
#define HIPNUC_ID_GYR_RAW       (0xB0)              /* 原始陀螺仪数据 */
#define HIPNUC_ID_GYR_CAL       (0xB1)              /* 校准后陀螺仪数据 */
#define HIPNUC_ID_MAG_RAW       (0xC0)              /* 原始磁力计数据 */
#define HIPNUC_ID_EUL           (0xD0)              /* 欧拉角数据 */
#define HIPNUC_ID_QUAT          (0xD1)              /* 四元数数据 */
#define HIPNUC_ID_PRS           (0xF0)              /* 气压数据 */

/* 新版HiPNUC标准协议数据类型标识 */
#define HIPNUC_ID_HI91        (0x91)              /* HI91类型数据包（IMU数据） */
#define HIPNUC_ID_HI92        (0x92)              /* HI92类型数据包（扩展IMU数据） */
#define HIPNUC_ID_HI81        (0x81)              /* HI81类型数据包（组合导航数据） */

/* 单位转换常量 */
#ifndef D2R
#define D2R (0.0174532925199433F)                 /* 度转弧度系数 */
#endif

#ifndef R2D
#define R2D (57.2957795130823F)                   /* 弧度转度系数 */
#endif

#ifndef GRAVITY
#define GRAVITY (9.8F)                            /* 重力加速度(m/s²) */
#endif

/* 局部函数声明 */
static void hipnuc_crc16(uint16_t *inital, const uint8_t *buf, uint32_t len);

/* 类型转换辅助宏 */
#define I2(p) (*((int16_t *)(p)))                 /* 从缓冲区读取有符号16位整数 */
static uint16_t U2(uint8_t *p) {                  /* 从缓冲区读取无符号16位整数 */
    uint16_t u;
    memcpy(&u, p, 2);
    return u;
}

static float R4(uint8_t *p) {                     /* 从缓冲区读取32位浮点数 */
    float r;
    memcpy(&r, p, 4);
    return r;
}

/**
 * @brief 解析数据包有效载荷到对应数据结构
 * @param raw 包含原始数据和解析结果的结构体指针
 * @return 成功返回1，失败返回0
 */
static int parse_data(hipnuc_raw_t *raw) {
    int ofs = 0;                                 /* 数据包内偏移量 */
    uint8_t *p = &raw->buf[CH_HDR_SIZE];         /* 指向有效载荷起始位置 */
    
    /* 重置数据结构标志位 */
    raw->hi91.tag = 0;
    raw->hi81.tag = 0;
    raw->hi92.tag = 0;

    /* 遍历有效载荷，根据数据类型标识解析 */
    while (ofs < raw->len) {
        switch (p[ofs]) {
        case HIPNUC_ID_USRID:                    /* 用户自定义ID，无数据内容 */
            ofs += 2;
            break;
        case HIPNUC_ID_ACC_RAW:                  /* 原始/校准加速度处理 */
        case HIPNUC_ID_ACC_CAL:
            raw->hi91.tag = HIPNUC_ID_HI91;
            /* 加速度数据转换：原始值/1000 -> m/s² */
            raw->hi91.acc[0] = (float)I2(p + ofs + 1) / 1000;
            raw->hi91.acc[1] = (float)I2(p + ofs + 3) / 1000;
            raw->hi91.acc[2] = (float)I2(p + ofs + 5) / 1000;
            ofs += 7;  // 1字节ID + 3个int16(6字节)
            break;
        case HIPNUC_ID_GYR_RAW:                  /* 原始/校准陀螺仪处理 */
        case HIPNUC_ID_GYR_CAL:
            raw->hi91.tag = HIPNUC_ID_HI91;
            /* 陀螺仪数据转换：原始值/10 -> deg/s */
            raw->hi91.gyr[0] = (float)I2(p + ofs + 1) / 10;
            raw->hi91.gyr[1] = (float)I2(p + ofs + 3) / 10;
            raw->hi91.gyr[2] = (float)I2(p + ofs + 5) / 10;
            ofs += 7;
            break;
        case HIPNUC_ID_MAG_RAW:                  /* 原始磁力计数据处理 */
            raw->hi91.tag = HIPNUC_ID_HI91;
            /* 磁力计数据转换：原始值/10 -> μT */
            raw->hi91.mag[0] = (float)I2(p + ofs + 1) / 10;
            raw->hi91.mag[1] = (float)I2(p + ofs + 3) / 10;
            raw->hi91.mag[2] = (float)I2(p + ofs + 5) / 10;
            ofs += 7;
            break;
        case HIPNUC_ID_EUL:                      /* 欧拉角数据处理 */
            raw->hi91.tag = HIPNUC_ID_HI91;
            /* 角度转换：原始值/100或/10 -> 度 */
            raw->hi91.pitch = (float)I2(p + ofs + 1) / 100;
            raw->hi91.roll  = (float)I2(p + ofs + 3) / 100;
            raw->hi91.yaw   = (float)I2(p + ofs + 5) / 10;
            ofs += 7;
            break;
        case HIPNUC_ID_QUAT:                     /* 四元数数据处理 */
            raw->hi91.tag = HIPNUC_ID_HI91;
            /* 直接读取4个float（小端序） */
            raw->hi91.quat[0] = R4(p + ofs + 1);
            raw->hi91.quat[1] = R4(p + ofs + 5);
            raw->hi91.quat[2] = R4(p + ofs + 9);
            raw->hi91.quat[3] = R4(p + ofs + 13);
            ofs += 17;  // 1字节ID + 4个float(16字节)
            break;
        case HIPNUC_ID_PRS:                      /* 气压数据处理 */
            raw->hi91.tag = HIPNUC_ID_HI91;
            raw->hi91.air_pressure = R4(p + ofs + 1);
            ofs += 5;   // 1字节ID + 1个float(4字节)
            break;
        case HIPNUC_ID_HI91:                     /* HI91标准数据包 */
            memcpy(&raw->hi91, p + ofs, sizeof(hi91_t));
            ofs += sizeof(hi91_t);
            break;
        case HIPNUC_ID_HI81:                     /* HI81组合导航数据包 */
            memcpy(&raw->hi81, p + ofs, sizeof(hi81_t));
            ofs += sizeof(hi81_t);
            break;
        case HIPNUC_ID_HI92:                     /* HI92扩展IMU数据包 */
            memcpy(&raw->hi92, p + ofs, sizeof(hi92_t));
            ofs += sizeof(hi92_t);
            break;
        default:                                /* 未知数据类型，跳过 */
            ofs++;
            break;
        }
    }
    return 1;
}

/**
 * @brief 解码完整数据帧
 * @param raw 包含原始数据和解析结果的结构体指针
 * @return 成功返回1，校验失败返回-1
 */
static int decode_hipnuc(hipnuc_raw_t *raw) {
    uint16_t crc = 0;

    /* CRC校验计算：头部（除最后2字节CRC） + 有效载荷 */
    hipnuc_crc16(&crc, raw->buf, (CH_HDR_SIZE-2));
    hipnuc_crc16(&crc, raw->buf + CH_HDR_SIZE, raw->len);
    
    /* 校验失败处理 */
    if (crc != U2(raw->buf + (CH_HDR_SIZE-2))) {
        return -1;
    }

    /* 校验通过，解析数据 */
    return parse_data(raw);
}

/**
 * @brief 同步头检测
 * @param buf 包含两个字节的缓冲区
 * @param data 新接收的字节
 * @return 同步成功返回1，否则返回0
 */
static int sync_hipnuc(uint8_t *buf, uint8_t data) {
    /* 滑动窗口检测同步头0x5A 0xA5 */
    buf[0] = buf[1];
    buf[1] = data;
    return (buf[0] == CHSYNC1) && (buf[1] == CHSYNC2);
}

/**
 * @brief 解码器输入处理（字节级）
 * @param raw 解码器状态结构体
 * @param data 输入的单个字节
 * @return >0: 成功解码一帧，0: 解码中，-1: 错误
 */
int hipnuc_input(hipnuc_raw_t *raw, uint8_t data) {
    /* 同步阶段处理 */
    if (raw->nbyte == 0) {
        if (!sync_hipnuc(raw->buf, data))
            return 0;
        raw->nbyte = 2;  // 同步头已接收
        return 0;
    }

    /* 存储当前字节 */
    raw->buf[raw->nbyte++] = data;

    /* 处理协议头部 */
    if (raw->nbyte == CH_HDR_SIZE) {
        /* 从头部获取有效载荷长度 */
        raw->len = U2(raw->buf + 2);
        /* 长度有效性检查 */
        if (raw->len > (HIPNUC_MAX_RAW_SIZE - CH_HDR_SIZE)) {
            raw->nbyte = 0;  // 重置状态
            return -1;
        }
    }

    /* 检查数据完整性 */
    if (raw->nbyte < CH_HDR_SIZE || raw->nbyte < (raw->len + CH_HDR_SIZE)) {
        return 0;  // 数据未接收完整
    }

    /* 完整帧接收完成，重置状态并解码 */
    raw->nbyte = 0;
    return decode_hipnuc(raw);
}

/**
 * @brief 将解码后的数据包转换为JSON格式字符串
 * @param raw 解码后的数据结构指针
 * @param buf 输出缓冲区指针（建议至少256字节）
 * @param buf_size 缓冲区总大小
 * @return 成功写入的字符数（包含结尾空字符）
 * 
 * 功能说明：
 * 1. 根据数据包类型（HI91/HI92/HI81）生成对应的JSON字符串
 * 2. 自动处理原始数据的单位转换：
 *    - 加速度：原始值转m/s²
 *    - 陀螺仪：原始值转deg/s
 *    - 磁力计：原始值转μT
 *    - 角度：原始值转deg
 *    - 气压：原始值转Pa
 *    - GPS坐标：整数转浮点度数
 * 3. 格式化为易读的JSON结构，包含嵌套数据结构
 * 4. 注意缓冲区溢出防护（使用snprintf）
 */
int hipnuc_dump_packet(hipnuc_raw_t *raw, char *buf, size_t buf_size)
{
    int written = 0;  // 已写入字符计数器
    int ret;          // 单次snprintf返回值

    /* HI91类型数据包处理（标准IMU数据） */
    if(raw->hi91.tag == HIPNUC_ID_HI91)
    {
        /* JSON字段说明：
         * system_time : 设备内部时间戳（毫秒）
         * acc : [X,Y,Z] 加速度（m/s²），原始值除以1000后乘以重力常数
         * gyr : [X,Y,Z] 陀螺仪（deg/s），原始值直接除以10
         * mag : [X,Y,Z] 磁力计（μT），原始值除以10
         * euler角 : 姿态角（度）
         * quat : 四元数[W,X,Y,Z]，直接读取浮点数
         * air_pressure : 气压（Pa）
         */
        ret = snprintf(buf + written, buf_size - written,
            "{\n"
            "  \"type\": \"HI91\",\n"
            "  \"system_time\": %d,\n"
            "  \"acc\": [%.3f, %.3f, %.3f],\n"    // acc = raw/1000 * 9.8
            "  \"gyr\": [%.3f, %.3f, %.3f],\n"    // gyr = raw/10 
            "  \"mag\": [%.3f, %.3f, %.3f],\n"    // mag = raw/10
            "  \"pitch\": %.2f,\n"                // 俯仰角（-90~90度）
            "  \"roll\": %.2f,\n"                 // 横滚角（-180~180）
            "  \"yaw\": %.2f,\n"                  // 航向角（0~360）
            "  \"quat\": [%.3f, %.3f, %.3f, %.3f],\n" // 四元数
            "  \"air_pressure\": %.1f\n"          // 气压值
            "}\n",
            raw->hi91.system_time,
            raw->hi91.acc[0]*GRAVITY, raw->hi91.acc[1]*GRAVITY, raw->hi91.acc[2]*GRAVITY,
            raw->hi91.gyr[0], raw->hi91.gyr[1], raw->hi91.gyr[2],
            raw->hi91.mag[0], raw->hi91.mag[1], raw->hi91.mag[2],
            raw->hi91.pitch, raw->hi91.roll, raw->hi91.yaw,
            raw->hi91.quat[0], raw->hi91.quat[1], raw->hi91.quat[2], raw->hi91.quat[3],
            raw->hi91.air_pressure);
    }
    
    /* HI92类型数据包处理（扩展IMU数据） */
    else if(raw->hi92.tag == HIPNUC_ID_HI92)
    {
        /* 特别说明：
         * 加速度转换系数0.0048828 = (9.8 m/s²)/(2000 LSB/g)
         * 陀螺仪系数0.001*57.2958 = 0.001rad/s -> 0.0573deg/s
         * 磁力计系数0.030517 ≈ (4912uT)/(160000 LSB)
         * 注意：yaw字段后缺少逗号会导致JSON语法错误！
         */
        ret = snprintf(buf + written, buf_size - written,
            "{\n"
            "  \"type\": \"HI92\",\n"
            "  \"status\": %d,\n"                // 设备状态字
            "  \"temperature\": %d,\n"           // 芯片温度（℃）
            "  \"acc\": [%.3f, %.3f, %.3f],\n"   // 加速度 m/s²
            "  \"gyr\": [%.3f, %.3f, %.3f],\n"  // 陀螺仪 deg/s
            "  \"mag\": [%.3f, %.3f, %.3f],\n"  // 磁力计 μT
            "  \"pitch\": %.2f,\n"              // 注意：此处缺少逗号！
            "  \"roll\": %.2f,\n"
            "  \"yaw\": %.2f\n"                 // 导致JSON语法错误
            "  \"quat\": [%.3f, %.3f, %.3f, %.3f],\n"
            "}\n",
            raw->hi92.status,
            raw->hi92.temperature,
            raw->hi92.acc_b[0]*0.0048828, raw->hi92.acc_b[1]*0.0048828, raw->hi92.acc_b[2]*0.0048828,
            raw->hi92.gyr_b[0]*(0.001*R2D), raw->hi92.gyr_b[1]*(0.001*R2D), raw->hi92.gyr_b[2]*(0.001*R2D),
            raw->hi92.mag_b[0]*0.030517, raw->hi92.mag_b[1]*0.030517, raw->hi92.mag_b[2]*0.030517,
            raw->hi92.pitch*0.001, raw->hi92.roll*0.001, raw->hi92.yaw*0.001,
            raw->hi91.quat[0], raw->hi91.quat[1], raw->hi91.quat[2], raw->hi91.quat[3]);
    }

    /* HI81类型数据包处理（组合导航数据） */
    else if(raw->hi81.tag == HIPNUC_ID_HI81)
    {
        /* 关键转换说明：
         * UTC时间：原始数据为压缩BCD码，需展开为20XX-MM-DD格式
         * 经纬度：整数除以1e7转为浮点度数
         * 高程：原始单位为毫米转米
         * 速度：cm/s转m/s
         * DOP值：放大10倍存储，需还原
         */
        ret = snprintf(buf + written, buf_size - written,
            "{\n"
            "  \"type\": \"HI81\",\n"
            "  \"status\": %d,\n"                // 设备状态位图
            "  \"ins_status\": %d,\n"            // 组合导航状态
            "  \"gpst_wn\": %d,\n"               // GPS周数
            "  \"gpst_tow\": %d,\n"              // GPS周内秒
            "  \"gyr\": [%.3f, %.3f, %.3f],\n"  // 陀螺仪 deg/s
            "  \"acc\": [%.3f, %.3f, %.3f],\n"  // 加速度 m/s²
            "  \"mag\": [%.3f, %.3f, %.3f],\n"  // 磁力计 μT
            "  \"air_pressure\": %.1f,\n"       // 气压 (hPa)
            "  \"temperature\": %d,\n"          // 温度 (℃)
            "  \"utc\": \"20%02d-%02d-%02d %02d:%02d:%02d.%03d\",\n" // UTC时间
            "  \"pitch\": %.2f,\n"              // 俯仰角
            "  \"roll\": %.2f,\n"               // 横滚角
            "  \"yaw\": %.2f,\n"                // 航向角
            "  \"quat\": [%.3f, %.3f, %.3f, %.3f],\n" // 四元数
            "  \"ins_lat\": %.7f,\n"            // 纬度（度）
            "  \"ins_lon\": %.7f,\n"            // 经度（度）
            "  \"ins_msl\": %.2f,\n"            // 海拔高度（米）
            "  \"pdop\": %.1f,\n"               // 位置精度因子
            "  \"hdop\": %.1f,\n"               // 水平精度因子
            "  \"solq_pos\": %d,\n"             // 定位质量标志
            "  \"nv_pos\": %d,\n"               // 定位卫星数
            "  \"solq_heading\": %d,\n"        // 定向解状态
            "  \"nv_heading\": %d,\n"           // 定向卫星数
            "  \"diff_age\": %d,\n"             // 差分数据龄期
            "  \"undulation\": %.2f,\n"         // 大地水准面起伏
            "  \"vel_enu\": [%.2f, %.2f, %.2f],\n" // ENU速度 m/s
            "  \"acc_enu\": [%.2f, %.2f, %.2f],\n" // ENU加速度
            "}\n",
            // 原始数据字段 >>>
            raw->hi81.status,
            raw->hi81.ins_status,
            raw->hi81.gpst_wn,
            raw->hi81.gpst_tow,
            // 单位转换处理 >>>
            raw->hi81.gyr_b[0]*(0.001*R2D),      // 0.001rad/s -> deg/s
            raw->hi81.gyr_b[1]*(0.001*R2D),
            raw->hi81.gyr_b[2]*(0.001*R2D),
            raw->hi81.acc_b[0]*0.0048828,        // LSB转m/s²
            raw->hi81.acc_b[1]*0.0048828,
            raw->hi81.acc_b[2]*0.0048828,
            raw->hi81.mag_b[0]*0.030517,         // LSB转μT
            raw->hi81.mag_b[1]*0.030517,
            raw->hi81.mag_b[2]*0.030517,
            (float)raw->hi81.air_pressure,       // 直接转浮点
            raw->hi81.temperature,               // 温度原始值
            // UTC时间分解（假设原始为BCD编码） >>>
            raw->hi81.utc_year,                  // BCD码年（20XX）
            raw->hi81.utc_month,                 // BCD码月
            raw->hi81.utc_day,                   // BCD码日
            raw->hi81.utc_hour,                  // BCD码时
            raw->hi81.utc_min,                   // BCD码分
            raw->hi81.utc_msec/1000,             // 毫秒转秒
            raw->hi81.utc_msec%1000,             // 毫秒部分
            // 姿态角处理（单位：0.01度） >>>
            raw->hi81.pitch*0.01,                // 俯仰角
            raw->hi81.roll*0.01,                 // 横滚角
            raw->hi81.yaw*0.01,                  // 航向角
            // 四元数处理（Q30格式转浮点） >>>
            raw->hi81.quat[0]*0.0001,            // Q30 -> float
            raw->hi81.quat[1]*0.0001,
            raw->hi81.quat[2]*0.0001,
            raw->hi81.quat[3]*0.0001,
            // 定位数据转换 >>>
            raw->hi81.ins_lat*1e-7,              // 纬度（度）
            raw->hi81.ins_lon*1e-7,              // 经度（度）
            raw->hi81.ins_msl*1e-3,              // 高程（毫米转米）
            raw->hi81.pdop*0.1,                  // PDOP放大10倍存储
            raw->hi81.hdop*0.1,                  // HDOP同理
            // 定位质量信息 >>>
            raw->hi81.solq_pos,
            raw->hi81.nv_pos,
            raw->hi81.solq_heading,
            raw->hi81.nv_heading,
            raw->hi81.diff_age,
            // 地理信息 >>>
            raw->hi81.undulation*0.01,           // 单位：厘米转米
            // 速度/加速度转换 >>>
            raw->hi81.vel_enu[0]*0.01,           // cm/s -> m/s
            raw->hi81.vel_enu[1]*0.01,
            raw->hi81.vel_enu[2]*0.01,
            raw->hi81.acc_enu[0]*0.0048828,      // 同加速度转换
            raw->hi81.acc_enu[1]*0.0048828,
            raw->hi81.acc_enu[2]*0.0048828);
    }

    /* 处理snprintf结果 */
    if (ret > 0) 
        written += ret;  // 累加成功写入量
    else if (ret < 0) 
        return -1;       // 编码错误
    
    return written;      // 返回总写入量
}

/**
 * @brief HiPNUC协议CRC16校验计算
 * @param initial 初始值（需传入0xFFFF）
 * @param buf 待校验数据指针
 * @param len 数据长度
 * 
 * 算法特性：
 * - 多项式：0x1021（CCITT标准）
 * - 初始值：0xFFFF
 * - 输入反转：无
 * - 输出反转：无
 * - 结果异或：0x0000
 * 典型调用方式：
 * uint16_t crc = 0xFFFF;
 * hipnuc_crc16(&crc, data, len);
 * crc ^= 0xFFFF; // 根据协议决定是否需要取反
 */
static void hipnuc_crc16(uint16_t *initial, const uint8_t *buf, uint32_t len)
{
    uint32_t crc = *initial;  // 加载初始值
    for (uint32_t j = 0; j < len; ++j)
    {
        crc ^= (uint16_t)buf[j] << 8;  // 字节移至高8位异或
        /* 逐位处理 */
        for (uint32_t i = 0; i < 8; ++i)
        {
            uint32_t temp = crc << 1;  // 左移一位
            if (crc & 0x8000)          // 检测最高位
                temp ^= 0x1021;        // 异或多项式
            crc = temp;                // 更新CRC
        }
    } 
    *initial = (uint16_t)crc;  // 回写结果
}
