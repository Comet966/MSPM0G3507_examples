/*
 * mcu_yuntai — PD42S1 双轴云台演示 (OLED 调试版)
 *
 * 硬件 (LP-MSPM0G3507, SysConfig):
 *   UART1_MOTOR  PA8(TX)/PA9(RX) 115200  -> 两台 PD42S1 (A端子并联)
 *   I2C_0        PB2(SCL)/PB3(SDA) 400kHz -> OLED SSD1306
 *   从机地址: Pan=0x01  Tilt=0x02
 */
#include "ti_msp_dl_config.h"
#include "Motor/PD42S1_Gimbal.h"
#include "Motor/rs485_bus.h"
#include "oled.h"
#include <stdio.h>

static pd42_gimbal_t g_gimbal;

static void delay_ms(uint32_t ms)
{
    while (ms--) DL_Common_delayCycles(32000u);
}

/* 将浮点角度格式化为 "±XXX.X" 写入 buf (至少7字节) */
static void fmt_angle(char *buf, float a)
{
    int sign = (a < 0.0f) ? -1 : 1;
    int whole = (int)(a < 0 ? -a : a);
    int frac  = (int)(((a < 0 ? -a : a) - whole) * 10.0f + 0.5f);
    if (frac >= 10) { frac = 0; whole++; }
    snprintf(buf, 8, "%c%3d.%d", sign < 0 ? '-' : '+', whole, frac);
}

static void show_angles(void)
{
    float pan = 0.0f, tilt = 0.0f;
    bool ok_p = pd42_gimbal_read_pan (&g_gimbal, &pan,  60);
    bool ok_t = pd42_gimbal_read_tilt(&g_gimbal, &tilt, 60);

    char ap[8], at[8];
    fmt_angle(ap, pan);
    fmt_angle(at, tilt);

    OLED_setCursor(0, 2);
    OLED_printf("Pan :%s%c", ap, ok_p ? ' ' : '?');
    OLED_setCursor(0, 3);
    OLED_printf("Tilt:%s%c", at, ok_t ? ' ' : '?');
    OLED_display();
}

int main(void)
{
    SYSCFG_DL_init();
    rs485_bus_init();

    OLED_init();
    OLED_clear();
    OLED_setCursor(0, 0);
    OLED_writeString("PD42S1 Gimbal");
    OLED_setCursor(0, 1);
    OLED_writeString("Init...");
    OLED_display();

    pd42_axis_config(&g_gimbal.pan,  rs485_get_bus_pan(),  0x01, 1.0f, +1, 300, 10);
    pd42_axis_config(&g_gimbal.tilt, rs485_get_bus_tilt(), 0x01, 1.0f, +1, 300, 10);
    pd42_gimbal_init(&g_gimbal, 16);
    delay_ms(50);
    pd42_gimbal_set_origin(&g_gimbal);

    OLED_setCursor(0, 1);
    OLED_writeString("Ready.  ");
    OLED_display();

    uint32_t step = 0;
    for (;;) {
        if (step % 2 == 0) {
            OLED_setCursor(0, 0);
            OLED_writeString("->+45/-20  ");
            OLED_display();
            pd42_gimbal_move_pan (&g_gimbal,  45.0f);
            pd42_gimbal_move_tilt(&g_gimbal, -20.0f);
        } else {
            OLED_setCursor(0, 0);
            OLED_writeString("->Home     ");
            OLED_display();
            pd42_gimbal_home(&g_gimbal);
        }
        delay_ms(1500);
        show_angles();
        delay_ms(500);
        step++;
    }
}
