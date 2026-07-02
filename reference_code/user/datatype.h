#ifndef __DATATYPE_H
#define __DATATYPE_H

// 标准头文件包含
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/*------------------------- 基本数据类型定义 -------------------------*/
// 8位整型数据定义
typedef   signed char 			int8;       // 有符号8位整型
typedef unsigned char 			_u8;        // 无符号8位整型(旧定义，与u8相同)
typedef unsigned char 			u8;         // 无符号8位整型
typedef unsigned char 			uint8;      // 无符号8位整型
typedef unsigned char 			byte;       // 字节类型(8位)

// 16位整型数据定义
typedef   signed short int  int16;       // 有符号16位整型
typedef unsigned short int  uint16;      // 无符号16位整型
typedef unsigned short int  _u16;        // 无符号16位整型(旧定义，与u16相同)
typedef unsigned short int  u16;         // 无符号16位整型

// 32位整型数据定义
typedef unsigned long int 	_u32;        // 无符号32位整型
typedef unsigned long int 	u32;        // 无符号32位整型
typedef float 							fp32;       // 32位浮点数
typedef double 							fp64;       // 64位浮点数

/*------------------------- 基础数学宏定义 -------------------------*/
#define ABS(X)  (((X)>0)?(X):-(X))         // 绝对值计算
#define MAX(a,b)  ((a)>(b)?(a):(b))        // 最大值
#define MIN(a,b)  ((a)>(b)?(b):(a))        // 最小值

/*------------------------- 传感器数据结构体 -------------------------*/

// BMI088传感器数据结构
typedef struct
{
    /* 原始传感器数据 */
    float Accel[3];         // 加速度计原始值(单位：m/s2，顺序[X,Y,Z])
    float Gyro[3];          // 陀螺仪原始值(单位：deg/s，顺序[X,Y,Z])
    
    /* 温度相关 */
    float TempWhenCali;     // 校准时传感器温度(单位：℃)
    float Temperature;      // 当前传感器温度(单位：℃)
    
    /* 校准参数 */
    float AccelScale;       // 加速度计比例校准系数
    float GyroOffset[3];    // 陀螺仪零偏校准值(单位：deg/s)
    float AccelOffset[3];   // 加速度计零偏校准值(单位：m/s2)
    
    /* 物理常量 */
    float gNorm;            // 重力加速度标准值(单位：m/s2，默认9.80665)
} BMI088_Data_t;

// MPU6050传感器数据结构
typedef struct
{
    /* 原始传感器数据 */
    float Accel[3];         // 加速度计原始值(单位：m/s2，顺序[X,Y,Z])
    float Gyro[3];          // 陀螺仪原始值(单位：deg/s，顺序[X,Y,Z])
    
    /* 温度相关 */
    float TempWhenCali;     // 校准时传感器温度(单位：℃)
    float Temperature;      // 当前传感器温度(单位：℃)
    
    /* 校准参数 */
    float AccelScale;       // 加速度计比例校准系数
    float GyroOffset[3];    // 陀螺仪零偏校准值(单位：deg/s)
    float AccelOffset[3];   // 加速度计零偏校准值(单位：m/s2)
    
    /* 物理常量 */
    float gNorm;            // 重力加速度标准值(单位：m/s2，默认9.80665)
} MPU6050_Data_t;

// HiPnuc传感器数据结构（字节对齐）
typedef struct __attribute__((__packed__)) {
    uint8_t  tag;            // 数据包标识 (0x91=有效数据)
    uint16_t status;         // 系统状态 
    int8_t   temp;           // 温度原始值 (单位：℃，范围：-40~85℃)
    float    air_pressure;   // 气压值 (单位：Pa)
    uint32_t system_time;    // 系统时间戳 (单位：ms)
    float    acc[3];         // 加速度值 (单位：m/s2)
    float    gyr[3];         // 陀螺仪值 (单位：deg/s)
    float    mag[3];         // 磁力计值 (单位：μT)
    float    roll;           // 横滚角 (-180° ~ 180°)
    float    pitch;          // 俯仰角 (-90° ~ 90°)
    float    yaw;            // 偏航角 (0° ~ 360°)
    float    quat[4];        // 姿态四元数 [w, x, y, z]
} HiPnuc_t;

// 惯性导航系统(INS)数据结构
typedef struct
{
    float q[4];             // 四元数姿态值

    float Gyro[3];          // 陀螺仪数据
    float Accel[3];         // 加速度计数据
    float MotionAccel_b[3];  // 机体坐标系运动加速度
    float MotionAccel_n[3]; // 导航坐标系运动加速度

    float AccelLPF;          // 加速度低通滤波系数

    // 机体坐标系各轴在导航坐标系中的表示
    float xn[3];
    float yn[3];
    float zn[3];

    float atanxz;           // XZ平面反正切值
    float atanyz;           // YZ平面反正切值

    // 姿态角
    float Roll;             // 横滚角
    float Pitch;            // 俯仰角
    float Yaw;              // 偏航角
    float YawTotalAngle;    // 总偏航角
    float YawAngleLast;     // 上次偏航角
    float YawRoundCount;    // 偏航圈数计数
		
    float v_n;              // 导航坐标系水平速度
    float x_n;              // 导航坐标系水平位置
		
    uint8_t ins_flag;       // INS状态标志位
} INS_t;

// 灰度传感器数据结构
typedef struct
{
    // 传感器位状态
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
    uint8_t BinaryData;        // 二进制数据
    uint8_t GraySensorNoData;  // 传感器无数据标志
    
    // ADC原始值存储
    uint16_t adc0_raw;  // bit0对应的ADC原始值
    uint16_t adc1_raw;  // bit1对应的ADC原始值
    uint16_t adc2_raw;  // bit2对应的ADC原始值
    uint16_t adc3_raw;  // bit3对应的ADC原始值
    uint16_t adc4_raw;  // bit4对应的ADC原始值
    uint16_t adc5_raw;  // bit5对应的ADC原始值
    uint16_t adc6_raw;  // bit6对应的ADC原始值
    uint16_t adc7_raw;  // bit7对应的ADC原始值
    
    // 校准数据
    uint16_t calibrated_white[8];  // 白色校准值数组
    uint16_t calibrated_black[8];  // 黑色校准值数组
    
    // 归一化系数
    float normal_factor[8];        // 每个传感器的归一化系数
    uint16_t adc_bits;             // ADC位数对应的满量程值(如12位=4096)
} GraySensor_t;

// 蜂鸣器控制结构
typedef struct
{
    int8_t  BuzzerBeeCount;  // 蜂鸣次数计数
    int8_t  BuzzerFlag;      // 蜂鸣标志
    int16_t BuzzerCount;     // 蜂鸣计数器
} Buzzer_t;

// 编码器数据结构
typedef struct {
    /*---------- 原始数据 ----------*/
    int v_int;           // 速度原始值 (12位有符号整型，范围：-2048~2047)
    int t_int;           // 占空比原始值 (12位有符号整型，范围：-2048~2047)
    int EncoderCount;    // 编码器脉冲计数
    
    /*---------- 转换后的物理量 ----------*/
    float pos;           // 位置(单位：rad)
    float vel;           // 速度(单位：rad/s)
    float V;             // 电压值
    float X;             // 位置值
} Encoder_t;

// PID控制器结构
typedef struct
{
    uint8_t mode;        // PID模式
    // PID 参数
    fp32 Kp;             // 比例系数
    fp32 Ki;             // 积分系数
    fp32 Kd;             // 微分系数

    fp32 max_out;        // 输出限幅
    fp32 max_iout;       // 积分限幅

    fp32 set;            // 设定值
    fp32 fdb;            // 反馈值

    fp32 out;            // 总输出
    fp32 Pout;           // 比例输出
    fp32 Iout;           // 积分输出
    fp32 Dout;           // 微分输出
    fp32 Dbuf[3];        // 微分项缓存 [0当前 1上一次 2上上次]
    fp32 error[3];       // 误差项 [0当前 1上一次 2上上次]
} Pid_t;

// 电机状态枚举
typedef enum {
  Enable   = 1,  // 使能
  Disable  = 0,   // 禁用
} Motor_State;

// 直流电机结构
typedef struct
{
    /*---------- 电机信息 ----------*/
    Motor_State state;  // 电机状态
    
    /*---------- 电机参数 ----------*/
    int  ERF;           // 编码器读取频率
    int  EL;            // 编码器线数
    float TR;           // 轮胎半径
    
    int   ExpectOutput;  // 期望输出
    int   Output;        // 实际输出
} BrushMotor_t;

// 电机组结构
typedef struct
{
    Pid_t*          PidSelfTurn;      // 自转PID
    
    BrushMotor_t*   MotorLeftFront;   // 左前电机
    Encoder_t*      EncoderLeftFront; // 左前编码器
    Pid_t*          PidLeftFront;     // 左前PID
    
    BrushMotor_t*   MotorLeftRear;    // 左后电机
    Encoder_t*      EncoderLeftRear;  // 左后编码器
    Pid_t*          PidLeftRear;      // 左后PID
    
    BrushMotor_t*   MotorRightFront;  // 右前电机
    Encoder_t*      EncoderRightFront;// 右前编码器
    Pid_t*          PidRightFront;    // 右前PID
    
    BrushMotor_t*   MotorRightRear;   // 右后电机
    Encoder_t*      EncoderRightRear; // 右后编码器
    Pid_t*          PidRightRear;     // 右后PID
} Motors_t;

// 任务状态结构
typedef struct
{
    int8_t   CarStartFlag;      // 小车启动标志
    float    AverageSpeed;      // 平均速度
    int8_t   KeyPressFlag;      // 按键按下标志
    int8_t   LastKeyPressFlag;  // 上次按键状态
    int32_t  KeyPressCount;     // 按键计数
    int8_t   CircleCount;       // 圈数计数
    int8_t   RightAngleCount;   // 直角计数
	
		int8_t   CarBeginLengthCountFlag;
		float    CarBeginLengthCount;
    
    float    Deltayaw;          // 偏航角变化量
} Task_t;

// 整车数据结构
typedef struct {
    BMI088_Data_t*  BMI088;      // BMI088传感器数据
    MPU6050_Data_t* MPU6050;     // MPU6050传感器数据
    HiPnuc_t*       HiPnuc;      // HiPnuc传感器数据
    INS_t*          INS;         // 惯性导航系统数据
    GraySensor_t*   GraySensor;  // 灰度传感器数据
    Buzzer_t*       Buzzer;      // 蜂鸣器控制
    Motors_t*       Motors;      // 电机组
    Task_t*         Tasks;       // 任务状态
} Car_t;

#endif