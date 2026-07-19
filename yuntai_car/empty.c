/*
 * 巡迹作业一体平台 —— 整合主程序 (yuntai 云台 + PID 循迹小车)
 *
 * 按键: KEY9=切换模式(待机时)   KEY10=启动当前模式 / 运行中急停
 * 模式: 0 待机  1 循迹一圈  2 定点瞄准  3 联动作业(A→B瞄→C→D→A)  4 动态跟踪
 * 声光: 蜂鸣器 PB21 + 状态灯 PB27, 每到关键点提示。
 * 循迹/瞄准均不依赖视觉(赛题硬要求); 视觉仅在动态/微调时增强。
 */
#include "headfile.h"     /* 云台聚合头: System/Uart/Stepper/Gimbal/Laser/Vision + 调参 */
#include "oled.h"
#include "motor.h"
#include "huidu.h"
#include "key.h"
#include "app_config.h"
#include <stdlib.h>

/* ---- 来自各驱动模块的全局 ---- */
extern volatile uint8_t g_motion_mode;    /* motor.c: 0停/1循迹/2直行 */
extern volatile float   g_straight_sps;   /* motor.c: 直行模式目标轮速 */
extern volatile uint8_t open_loop_test;   /* motor.c: 保持0=闭环 */
extern float target_speed_1, target_speed_2;
extern float speed_1, speed_2;
extern uint8_t huidu_value[];
extern volatile uint8_t key_mode_req;     /* key.c: KEY9 请求 */
extern volatile uint8_t key_action_req;   /* key.c: KEY10 请求 */

/* ---- 运行模式 ---- */
typedef enum { MODE_IDLE=0, MODE_TRACK, MODE_AIM, MODE_LINKAGE, MODE_DYNAMIC, MODE_COUNT } AppMode;
static const char *MODE_NAME[MODE_COUNT] = { "IDLE ", "TRACK", "AIM  ", "LINK ", "DYNMC" };

static Vision_t *g_vision;
static Gimbal_t *g_gimbal;

/*==================== 声光辅助 ====================*/
static void led_set(bool on)
{
    if (on) DL_GPIO_setPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);
    else    DL_GPIO_clearPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);
}
static void buzzer_set(bool on)
{
    if (on) DL_GPIO_setPins(GPIO_BUZZER_PORT, GPIO_BUZZER_BEEP_PIN);
    else    DL_GPIO_clearPins(GPIO_BUZZER_PORT, GPIO_BUZZER_BEEP_PIN);
}
/* 阻塞式声光提示(仅在停车/待机时调用) */
static void sig_beep(uint32_t ms)
{
    buzzer_set(true);  led_set(true);
    delay_ms(ms);
    buzzer_set(false); led_set(false);
}

/*==================== 关键点(全黑十字线)检测 ====================*/
/* 边沿触发: 黑线路数 >= KEYPOINT_MIN_BLACK 立即算到点, 离开(<2路)后才重新武装。
 * 车高速通过路口只有 ~几十 ms, 不能用"持续时长"判定, 否则永远数不到。
 * 返回 true 表示"新到达一个关键点", 每个物理路口只报一次。 */
static bool keypoint_hit(void)
{
    static bool armed = true;          /* 离开关键点后重新武装 */
    uint8_t n = huidu_black_count();

    if (armed && n >= KEYPOINT_MIN_BLACK) {
        armed = false;                 /* 本次已报, 等离开再武装 */
        return true;
    }
    if (n <= 1) armed = true;          /* 回到单线/丢线, 重新允许触发 */
    return false;
}

/*==================== 定点瞄准(几何, 可选视觉微调) ====================*/
/* 停车状态下把云台指向靶心并开激光, 保持 AIM_SETTLE_MS。返回后激光留开由调用方决定。*/
static void do_aim(float x_mm, float y_mm)
{
    Laser_Set(true);
    Gimbal_AimGeometry(x_mm, y_mm);          /* 几何开环指向 */
    while (Gimbal_Busy()) { /* 等云台到位 */ }
    delay_ms(AIM_SETTLE_MS);

#if AIM_VISION_ASSIST
    /* 几何到位后, 若 K230 有新鲜跟踪帧则做像素级闭环微调 */
    uint32_t t0 = millis();
    uint32_t last_frame = g_vision->FrameCount;
    while (millis() - t0 < AIM_VISION_MS) {
        if (g_vision->FrameCount != last_frame) {
            last_frame = g_vision->FrameCount;
            if (Vision_IsFresh() && g_vision->Flag == VISION_FLAG_TRACK)
                Gimbal_AimVision(g_vision);
        }
    }
#endif
}

/* KEY10 在运行中按下 = 急停请求 */
static bool estop_pressed(void)
{
    if (key_action_req) { key_action_req = 0; return true; }
    return false;
}

/*==================== 模式1: 循迹一圈 ====================*/
/* 沿黑线行驶, 数够 KEYPOINTS_PER_LAP 个关键点(回到A)后停车; 每点声光提示。*/
static void run_track(void)
{
    uint8_t kp = 0;
    g_track_run = 1;                 /* ISR 开始跑 adjust_motor */
    while (1) {
        if (estop_pressed()) break;
        if (keypoint_hit()) {
            kp++;
            sig_beep(BEEP_SHORT_MS); /* 注意: 短鸣时不停车, 车会滑一点, 可接受 */
            if (kp >= KEYPOINTS_PER_LAP) break;
        }
    }
    g_track_run = 0;                 /* 锁 0 速停车 */
    delay_ms(TRACK_STOP_SETTLE_MS);
    sig_beep(BEEP_LONG_MS);          /* 完成长鸣 */
}

/*==================== 模式2: 定点瞄准 ====================*/
static void run_aim(void)
{
    g_track_run = 0;                 /* 确保停车 */
    do_aim((float)AIM_TARGET_X_MM, (float)AIM_TARGET_Y_MM);
    sig_beep(BEEP_SHORT_MS);
    /* 保持指向直到急停 */
    while (!estop_pressed()) { /* 激光常亮指靶 */ }
    Laser_Set(false);
}

/*==================== 模式3: 联动作业 A→B(瞄)→C→D→A ====================*/
/* 循迹前进, 到每个关键点停车声光; 在 B 点(第1个关键点)做定点对靶。*/
static void run_linkage(void)
{
    uint8_t kp = 0;
    while (kp < KEYPOINTS_PER_LAP) {
        g_track_run = 1;
        bool arrived = false;
        while (!arrived) {
            if (estop_pressed()) { g_track_run = 0; Laser_Set(false); return; }
            if (keypoint_hit()) arrived = true;
        }
        g_track_run = 0;                       /* 到点停车 */
        delay_ms(TRACK_STOP_SETTLE_MS);
        kp++;
        sig_beep(BEEP_SHORT_MS);
#if AIM_AT_KEYPOINT_B
        if (kp == 1) {                         /* B 点对靶作业 */
            do_aim((float)AIM_TARGET_X_MM, (float)AIM_TARGET_Y_MM);
            Laser_Set(false);
        }
#endif
    }
    sig_beep(BEEP_LONG_MS);                     /* 回到 A, 完成 */
}

/*==================== 模式4: 动态跟踪(边循迹边视觉跟踪) ====================*/
static void run_dynamic(void)
{
    uint32_t last_frame = g_vision->FrameCount;
    Laser_Set(true);
    g_track_run = 1;                            /* 底盘循迹 */
    while (!estop_pressed()) {
        if (g_vision->FrameCount != last_frame) {   /* 云台每帧闭环跟踪 */
            last_frame = g_vision->FrameCount;
            if (Vision_IsFresh() && g_vision->Flag == VISION_FLAG_TRACK)
                Gimbal_AimVision(g_vision);
        }
    }
    g_track_run = 0;
    Laser_Set(false);
}

/*==================== 待机菜单 OLED ====================*/
static void oled_idle(AppMode m)
{
    OLED_clear();
    OLED_setCursor(0, 0); OLED_writeString("== ROBOT  READY ==");
    OLED_setCursor(0, 2); OLED_printf("Mode> %s", MODE_NAME[m]);
    OLED_setCursor(0, 4); OLED_writeString("K9 : switch mode");
    OLED_setCursor(0, 5); OLED_writeString("K10: START");
    OLED_setCursor(0, 7); OLED_printf("Vis:%s", Vision_IsFresh() ? "OK " : "-- ");
    OLED_display();
}
static void oled_running(AppMode m)
{
    OLED_clear();
    OLED_setCursor(0, 0); OLED_printf("RUN: %s", MODE_NAME[m]);
    OLED_setCursor(0, 2); OLED_printf("T1:%-4d T2:%-4d", (int)target_speed_1, (int)target_speed_2);
    OLED_setCursor(0, 3); OLED_printf("S1:%-4d S2:%-4d", (int)speed_1, (int)speed_2);
    OLED_setCursor(0, 5); OLED_printf("Pan:%+06ld", (long)Stepper_Position(STEPPER_PAN));
    OLED_setCursor(0, 6); OLED_printf("Tlt:%+06ld", (long)Stepper_Position(STEPPER_TILT));
    OLED_setCursor(0, 7); OLED_writeString("K10 = STOP");
    OLED_display();
}

int main(void)
{
    SYSCFG_DL_init();
    SystemInit_Timebase();
    Uart_Init();                 /* 使能两个 UART RX 中断 */
    OLED_init();

    Laser_Init();
    Vision_Init(&g_vision);
    Gimbal_Init(&g_gimbal);      /* Stepper_Init + 使能两轴 */

    /* 手动使能 GPIO 组中断: 按键(PB) + 编码器 AA(PA)/BA(PB) */
    NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);
    NVIC_EnableIRQ(DC_MOTOR_GPIOA_INT_IRQN);

    /* 直流电机闭环: 初始化后锁 0 速待命 */
    open_loop_test = 0;
    g_track_run    = 0;
    motor_init(1);
    motor_init(2);
    target_speed_1 = 0;
    target_speed_2 = 0;

    AppMode mode = MODE_IDLE;
    uint32_t t_oled = 0;
    sig_beep(BEEP_SHORT_MS);     /* 上电自检提示 */

    while (1) {
        /* KEY9: 待机时循环切换模式(跳过 IDLE 本身) */
        if (key_mode_req) {
            key_mode_req = 0;
            mode = (AppMode)(mode + 1);
            if (mode >= MODE_COUNT) mode = MODE_TRACK;
        }
        /* KEY10: 启动选中的模式 */
        if (key_action_req) {
            key_action_req = 0;
            oled_running(mode);
            switch (mode) {
                case MODE_TRACK:   run_track();   break;
                case MODE_AIM:     run_aim();     break;
                case MODE_LINKAGE: run_linkage(); break;
                case MODE_DYNAMIC: run_dynamic(); break;
                default: break;
            }
            mode = MODE_IDLE;    /* 作业完成回到待机 */
        }
        /* 待机菜单 5 Hz 刷新 */
        uint32_t now = millis();
        if (now - t_oled >= 200) { t_oled = now; oled_idle(mode); }
    }
}
