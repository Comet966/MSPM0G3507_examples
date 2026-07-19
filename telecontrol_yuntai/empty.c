/*
 * telecontrol_yuntai — 红外遥控 PD42S1 双轴云台（速度模式）
 *
 * 硬件 (LP-MSPM0G3507):
 *   PA18               → 红外接收头 OUT (GPIO_IR / PIN_REC, RISE_FALL 中断)
 *   UART1_PAN  PA8/PA9  → 下方 Pan  PD42S1 (addr=0x01)  — X 轴控制
 *   UART0_TILT PA28/PA31→ 上方 Tilt PD42S1 (addr=0x02)  — Y 轴控制
 *   I2C_0  PB2/PB3     → OLED SSD1306
 *   TIMG_IR (TIMG0)    → 1 MHz 自由计数，用于 IR 脉宽测量
 */
#include "ti_msp_dl_config.h"
#include "Motor/PD42S1_Gimbal.h"
#include "Motor/rs485_bus.h"
#include "oled.h"
#include "ir_decode.h"
#include <stdio.h>

#define LOOP_MS          20u      /* 主循环周期 */
#define IR_TIMEOUT_TICK  25u      /* 25×20ms=500ms 无帧则停机 */

/* 两路 UART 相互独立，均可使用 PD42S1 出厂默认地址 0x01。 */
#define PAN_PD42_ADDR    0x01u    /* PA8/PA9，下方电机 */
#define TILT_PD42_ADDR   0x01u    /* PA28/PA31，上方电机；若已改址则在此修改 */

static pd42_gimbal_t g_gimbal;
static pd42_joy_speed_config_t g_speed_cfg;

static void delay_ms(uint32_t ms)
{
    while (ms--) DL_Common_delayCycles(32000u);
}

void GROUP1_IRQHandler(void)
{
    uint32_t st = DL_GPIO_getEnabledInterruptStatus(
        GPIO_IR_PORT, GPIO_IR_PIN_REC_PIN);
    if (st & GPIO_IR_PIN_REC_PIN) {
        DL_GPIO_clearInterruptStatus(GPIO_IR_PORT, GPIO_IR_PIN_REC_PIN);
        ir_decode_tick();
    }
}

int main(void)
{
    SYSCFG_DL_init();
    ir_decode_init();
    rs485_bus_init();
    OLED_init();
    OLED_clear();

    OLED_setCursor(0, 0); OLED_writeString("IR Gimbal");
    OLED_setCursor(0, 1); OLED_writeString("Init...");
    OLED_display();

    pd42_axis_config(&g_gimbal.pan,  rs485_get_bus_pan(),  PAN_PD42_ADDR,  1.0f, +1, 300, 10);
    pd42_axis_config(&g_gimbal.tilt, rs485_get_bus_tilt(), TILT_PD42_ADDR, 1.0f, +1, 300, 10);
    pd42_gimbal_init(&g_gimbal, 16);                        /* 使能 + 设细分 */
    pd42_set_mode(&g_gimbal.pan.drv,  PD42_MODE_SPEED_COMM);
    pd42_set_mode(&g_gimbal.tilt.drv, PD42_MODE_SPEED_COMM);
    pd42_joy_speed_config_default(&g_speed_cfg);
    /*
     * 调速接口：可在按键/串口菜单中直接修改 g_speed_cfg.pan_max_rpm
     * （PA8/PA9 下方电机上限）、tilt_max_rpm、deadband、acceleration 或 curve；
     * 下一帧遥控数据即按新参数生效。
     */
    delay_ms(50);
    pd42_gimbal_speed_stop(&g_gimbal, &g_speed_cfg);  /* 0 RPM 软停，绝不下发 0xFC */

    /*
     * PA18 的 GPIO 中断在 SysConfig 中默认关闭。待 OLED 和电机均初始化完成后，
     * 先清除复位期间累积的边沿标志，再依次开启 GPIO 外设中断与 NVIC。
     */
    DL_GPIO_clearInterruptStatus(GPIO_IR_PORT, GPIO_IR_PIN_REC_PIN);
    DL_GPIO_enableInterrupt(GPIO_IR_PORT, GPIO_IR_PIN_REC_PIN);
    NVIC_ClearPendingIRQ(GPIOA_INT_IRQn);
    NVIC_EnableIRQ(GPIOA_INT_IRQn);

    OLED_setCursor(0, 1); OLED_writeString("Ready   ");
    OLED_display();

    uint32_t no_frame_tick = 0;
    uint16_t last_x = g_speed_cfg.center, last_y = g_speed_cfg.center;
    uint32_t oled_tick = 0;

    for (;;) {
        uint16_t x, y;
        if (ir_decode_get(&x, &y)) {
            last_x = x;
            last_y = y;
            no_frame_tick = 0;

            /*
             * 死区内输出 0 RPM 的速度命令，而非 pd42_stop()/0xFC 立即刹车。
             * 默认 280 死区；下方 Pan 上限 50 RPM，上方 Tilt 上限 80 RPM，
             * 均采用立方曲线，中心附近更不灵敏。
             */
            pd42_gimbal_speed_from_joystick(&g_gimbal, &g_speed_cfg, x, y);
        } else {
            ++no_frame_tick;
            if (no_frame_tick == IR_TIMEOUT_TICK) {
                /* 遥控失联同样平滑停车；紧急刹车仅保留 pd42_gimbal_stop()。 */
                pd42_gimbal_speed_stop(&g_gimbal, &g_speed_cfg);
            }
        }

        /* OLED 约每 100 ms 刷新：显示原始 IR 值和偏移 */
        if (++oled_tick >= 5u) {
            oled_tick = 0;
            char buf[17];
            OLED_setCursor(0, 2);
            snprintf(buf, sizeof(buf), "X:%4u Y:%4u", last_x, last_y);
            OLED_writeString(buf);
            OLED_setCursor(0, 3);
            snprintf(buf, sizeof(buf), "dX:%+5d", (int)last_x - (int)g_speed_cfg.center);
            OLED_writeString(buf);
            OLED_setCursor(0, 4);
            snprintf(buf, sizeof(buf), "dY:%+5d", (int)last_y - (int)g_speed_cfg.center);
            OLED_writeString(buf);
            OLED_display();
        }

        delay_ms(LOOP_MS);
    }
}
