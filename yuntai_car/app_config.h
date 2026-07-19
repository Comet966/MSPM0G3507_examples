#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/*
 * 整机（巡迹作业一体平台）集中可调参数。
 * 纯 #define 头文件，不 include 任何工程头，可被任意模块安全包含。
 * 现场标定时主要改这里。
 */

/*==================== 几何瞄准（独立于视觉，赛题硬要求） ====================*/
/* 靶心相对云台的水平面坐标默认值 (mm)。定点瞄准/联动到靶位时使用。
 * 赛题：靶在 AB 外侧 50cm，靶面与 AB 平行，靶高 ≤ 50cm。
 * 现场用卷尺量云台俯仰轴到靶心的实际几何后填入。 */
#define AIM_TARGET_X_MM     0        /* 靶心横向偏移，+右 */
#define AIM_TARGET_Y_MM     500      /* 云台到靶面水平距离(AB外侧50cm) */
#define AIM_TARGET_H_MM     250      /* 靶心离地高度(靶心在A4靶面中央) */
#define AIM_PIVOT_H_MM      120      /* 云台俯仰轴离地高度(装车后实测) */

/*==================== 循迹 ====================*/
/* 循迹 PD 参数在 huidu.c 内（LINE_BASE/KP/KD）。此处放整机级节奏参数。 */
#define TRACK_STOP_SETTLE_MS     400U   /* 到点停车后稳定时间 */

/* 运动模式(与 motor.c g_motion_mode 对应) */
#define MOTION_STOP        0
#define MOTION_TRACK       1
#define MOTION_STRAIGHT    2

/* 穿越十字路口: 直行速度 + 持续时间(盲走, 走过黑线区回到单线) */
#define CROSS_STRAIGHT_SPS   250      /* 穿越时目标轮速 mm/s, 略低于巡航 */
#define CROSS_PASS_MS        400U     /* 直行穿越持续时间, 按线宽/车速标定 */
#define KEYPOINT_MIN_BLACK   4        /* 判定到达关键点的最少黑线路数(4或5) */

/*==================== 声光提示（蜂鸣器 PB21 + LED） ====================*/
#define BEEP_SHORT_MS       120U     /* 关键点短鸣 */
#define BEEP_LONG_MS        600U     /* 终点/完成长鸣 */

/*==================== 定点瞄准 ====================*/
#define AIM_SETTLE_MS       800U     /* 云台到位后激光指向保持时间 */
#define AIM_VISION_ASSIST   1        /* 1=几何到位后用K230做微调, 0=纯几何 */
#define AIM_VISION_MS       1500U    /* 视觉微调窗口时长 */

/*==================== 联动路线 ====================*/
/* A→B(停+瞄)→C→D→A，每关键点声光。关键点靠灰度"全黑"计数识别。
 * KEYPOINTS_PER_LAP = 一圈经过的关键点数(含回到A)。 */
#define KEYPOINTS_PER_LAP   4        /* B,C,D,A */
#define AIM_AT_KEYPOINT_B   1        /* 仅在B点做对靶作业(基本要求3) */

#endif /* __APP_CONFIG_H */
