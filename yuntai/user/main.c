#include "headfile.h"
#include "oled.h"
#include <stdlib.h>

/*
 * Milestone 2: vision closed-loop gimbal control.
 *
 * Main loop polls for new K230 frames (by FrameCount change) and calls
 * Gimbal_AimVision every frame (~100 Hz). Laser is on at all times.
 * OLED refreshes at 5 Hz to show live state without blocking the control loop.
 *
 * OLED layout (8 cols × 8 rows, 8×8 font):
 *   Row 0: flag + coarse/fine indicator   "BB FINE " / "BB CORS " / "CC ---- "
 *   Row 1: center error                   "Cx:+008 Cy:-012"
 *   Row 2: base-point error               "Bx:+003 By:+001"
 *   Row 3: base index                     "Base: 07"
 *   Row 4: PAN  step position             "Pan:+01234"
 *   Row 5: TILT step position             "Tlt: -0567"
 *   Row 6: frame count                    "N:001234"
 *   Row 7: 1 Hz blink dot                 "*"
 */

static void oled_refresh(Vision_t *v)
{
    bool fresh  = Vision_IsFresh();
    bool track  = fresh && (v->Flag == VISION_FLAG_TRACK);
    bool coarse = track && (abs((int)v->DxCenter) > VIS_COARSE_THRESH ||
                            abs((int)v->DyCenter) > VIS_COARSE_THRESH);

    OLED_clear();

    /* Row 0: status */
    OLED_setCursor(0, 0);
    if (!fresh)       OLED_writeString("NO SIGNAL  ");
    else if (!track)  OLED_writeString("CC LOST    ");
    else if (coarse)  OLED_writeString("BB COARSE  ");
    else              OLED_writeString("BB FINE    ");

    /* Row 1: center error */
    OLED_setCursor(0, 1);
    OLED_printf("Cx:%c%03d Cy:%c%03d",
                v->DxCenter >= 0 ? '+' : '-', abs((int)v->DxCenter),
                v->DyCenter >= 0 ? '+' : '-', abs((int)v->DyCenter));

    /* Row 2: base-point error */
    OLED_setCursor(0, 2);
    OLED_printf("Bx:%c%03d By:%c%03d",
                v->DxBase >= 0 ? '+' : '-', abs((int)v->DxBase),
                v->DyBase >= 0 ? '+' : '-', abs((int)v->DyBase));

    /* Row 3: base index */
    OLED_setCursor(0, 3);
    OLED_printf("Base: %02u", (unsigned)v->BaseIndex);

    /* Row 4/5: step positions */
    OLED_setCursor(0, 4);
    OLED_printf("Pan:%+06ld", (long)Stepper_Position(STEPPER_PAN));
    OLED_setCursor(0, 5);
    OLED_printf("Tlt:%+06ld", (long)Stepper_Position(STEPPER_TILT));

    /* Row 6: frame count */
    OLED_setCursor(0, 6);
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

    Laser_Set(true);

    /* splash */
    OLED_clear();
    OLED_setCursor(0, 0); OLED_writeString("YUNTAI  v2");
    OLED_setCursor(0, 2); OLED_writeString("waiting K230");
    OLED_display();

    uint32_t last_frame   = 0;
    uint32_t t_oled       = millis();
    uint32_t t_led        = millis();
    bool     led_state    = false;

    while (1) {
        uint32_t now = millis();

        /* --- Closed-loop control: run on every new vision frame --- */
        if (vision->FrameCount != last_frame) {
            last_frame = vision->FrameCount;
            Gimbal_AimVision(vision);
        }

        /* --- OLED refresh at 5 Hz (200 ms) --- */
        if (now - t_oled >= 200) {
            t_oled = now;
            oled_refresh(vision);
        }

        /* --- LED2 1 Hz blink: proof of life --- */
        if (now - t_led >= 500) {
            t_led     = now;
            led_state = !led_state;
            if (led_state) DL_GPIO_setPins(GPIO_LED_PORT,   GPIO_LED_STATUS_PIN);
            else            DL_GPIO_clearPins(GPIO_LED_PORT, GPIO_LED_STATUS_PIN);
        }
    }
}
