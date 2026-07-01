/*
 * ST7735S slideshow for LP-MSPM0G3507.
 * Cycles through all images in tools/slideshow.h, one per ~3 s.
 *
 * Wiring (SPI1 + control GPIOs, BoosterPack header):
 *   Display  Signal           MCU    BP pin
 *   SCL      SPI1_SCLK         PB9    7
 *   SDA      SPI1_PICO(MOSI)   PB8    15
 *   DC       GPIO out          PB16   11
 *   RES      GPIO out          PB15   17
 *   CS       GPIO out          PB17   18
 *   BLK      GPIO out          PB12   12
 *   VCC      3V3                      1
 *   GND      GND                      20
 */
#include "ti_msp_dl_config.h"
#include "st7735.h"
#include "tools/slideshow.h"

/* ~3 s @ 32 MHz */
#define PAUSE()  delay_cycles(96000000)

/* Center a SLIDESHOW_W x SLIDESHOW_H image on the 128x160 panel. */
#define IMG_X  ((ST7735_TFTWIDTH  - SLIDESHOW_W) / 2)
#define IMG_Y  ((ST7735_TFTHEIGHT - SLIDESHOW_H) / 2)

int main(void)
{
    SYSCFG_DL_init();
    ST7735_init();

    uint8_t idx = 0;
    while (1) {
        ST7735_clear();
        ST7735_drawRGBBitmap(IMG_X, IMG_Y,
                             SLIDESHOW_W, SLIDESHOW_H,
                             slideshow_images[idx]);
        PAUSE();
        idx = (idx + 1 >= SLIDESHOW_COUNT) ? 0 : idx + 1;
    }
}
