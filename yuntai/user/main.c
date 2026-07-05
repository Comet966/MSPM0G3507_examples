#include "headfile.h"
#include "oled.h"

/*
 * Vision decode verification via OLED (SSD1306 128x64, I2C1, PB2/PB3).
 *
 * Display layout (8 rows × 16 chars, 8×8 font):
 *   Row 0: "VISION TEST"
 *   Row 1: flag + fresh indicator   e.g. "FL:BB  Fr:1"
 *   Row 2: center dx/dy             e.g. "Cx:+012 Cy:-034"
 *   Row 3: base index               e.g. "Base:07"
 *   Row 4: base dx/dy               e.g. "Bx:+100 By:+002"
 *   Row 5: frame count              e.g. "N:000123"
 *   Row 6: (spare)
 *   Row 7: LED blink indicator      e.g. "* "  (toggles each refresh)
 *
 * LED2 (PB27) blinks at 1 Hz always — confirms main loop is alive even if
 * OLED wiring is wrong.
 */

static void oled_show(Vision_t *v)
{
    bool fresh = Vision_IsFresh();

    OLED_clear();

    OLED_setCursor(0, 0);
    OLED_writeString("VISION  TEST");

    OLED_setCursor(0, 1);
    OLED_printf("FL:%02X  Fr:%c", v->Flag, fresh ? '1' : '0');

    OLED_setCursor(0, 2);
    /* dx/dy are signed int16; print sign explicitly to fit in 16 chars */
    OLED_printf("Cx:%c%03d Cy:%c%03d",
                 v->DxCenter >= 0 ? '+' : '-', (int)(v->DxCenter < 0 ? -v->DxCenter : v->DxCenter),
                 v->DyCenter >= 0 ? '+' : '-', (int)(v->DyCenter < 0 ? -v->DyCenter : v->DyCenter));

    OLED_setCursor(0, 3);
    OLED_printf("Base: %02u", (unsigned)v->BaseIndex);

    OLED_setCursor(0, 4);
    OLED_printf("Bx:%c%03d By:%c%03d",
                 v->DxBase >= 0 ? '+' : '-', (int)(v->DxBase < 0 ? -v->DxBase : v->DxBase),
                 v->DyBase >= 0 ? '+' : '-', (int)(v->DyBase < 0 ? -v->DyBase : v->DyBase));

    OLED_setCursor(0, 5);
    OLED_printf("N:%06lu", (unsigned long)v->FrameCount);

    OLED_display();
}

int main(void)
{
    SYSCFG_DL_init();
    SystemInit_Timebase();
    Uart_Init();

    OLED_init();

    Laser_Init();
    Vision_t *vision;
    Vision_Init(&vision);
    Gimbal_t *gimbal;
    Gimbal_Init(&gimbal);

    /* splash */
    OLED_clear();
    OLED_setCursor(0, 0);
    OLED_writeString("VISION TEST");
    OLED_setCursor(0, 2);
    OLED_writeString("waiting K230...");
    OLED_display();

    uint32_t t_display = millis();
    uint32_t t_led     = millis();
    bool     led_state = false;

    while (1) {
        uint32_t now = millis();

        /* refresh OLED every 100 ms */
        if (now - t_display >= 100) {
            t_display = now;
            oled_show(vision);
        }

        /* 1 Hz LED blink — proof of life */
        if (now - t_led >= 500) {
            t_led     = now;
            led_state = !led_state;
            if (led_state) DL_GPIO_setPins(GPIO_LED_PORT,   GPIO_LED_STATUS_PIN);
            else            DL_GPIO_clearPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);
        }
    }
}
