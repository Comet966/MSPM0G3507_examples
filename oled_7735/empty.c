/*
 * ST7735S 1.8" 128x160 color TFT demo for LP-MSPM0G3507.
 *
 * Cycles through text, graphics, and image demos using the st7735 driver.
 * See st7735.h / profiles/ST7735_API.md for the full API.
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
#include "tools/st_image.h"

#define PAUSE()  delay_cycles(96000000)   /* ~3 s @ 32 MHz */

static void demo_text(void)
{
    ST7735_fillScreen(ST7735_BLACK);
    ST7735_setTextColor(ST7735_WHITE, ST7735_BLACK);
    ST7735_setTextSize(1);
    ST7735_setCursor(0, 0);  ST7735_writeString("== Text Demo ==");

    ST7735_setTextColor(ST7735_YELLOW, ST7735_BLACK);
    ST7735_setCursor(0, 16); ST7735_writeString("Hello, ST7735S!");
    ST7735_setTextColor(ST7735_CYAN, ST7735_BLACK);
    ST7735_setCursor(0, 28); ST7735_writeString("MSPM0G3507 SPI");

    ST7735_setTextColor(ST7735_GREEN, ST7735_BLACK);
    ST7735_setCursor(0, 44); ST7735_printf("printf: %d+%d=%d", 12, 34, 46);

    ST7735_setTextColor(ST7735_ORANGE, ST7735_BLACK);
    ST7735_setTextSize(2);
    ST7735_setCursor(0, 64); ST7735_writeString("Size 2");
    ST7735_setTextSize(3);
    ST7735_setTextColor(ST7735_MAGENTA, ST7735_BLACK);
    ST7735_setCursor(0, 90); ST7735_writeString("BIG");

    ST7735_setTextSize(1);
    ST7735_setTextColor(ST7735_WHITE, ST7735_BLACK);
    ST7735_setCursor(0, 130); ST7735_writeString("ABCDEFGHIJKLMNOPQRS");
    ST7735_setCursor(0, 142); ST7735_writeString("abcdefg 0123456789");
    PAUSE();
}

static void demo_lines(void)
{
    ST7735_fillScreen(ST7735_BLACK);
    ST7735_setTextColor(ST7735_WHITE, ST7735_BLACK);
    ST7735_setTextSize(1);
    ST7735_setCursor(0, 0); ST7735_writeString("== Lines ==");

    uint16_t colors[] = { ST7735_RED, ST7735_GREEN, ST7735_BLUE,
                          ST7735_YELLOW, ST7735_CYAN, ST7735_MAGENTA };
    for (int16_t i = 0; i <= 120; i += 8)
        ST7735_drawLine(0, 16, i, 159, colors[(i / 8) % 6]);
    for (int16_t i = 0; i <= 120; i += 8)
        ST7735_drawLine(127, 16, i, 159, colors[(i / 8) % 6]);
    PAUSE();
}

static void demo_rects(void)
{
    ST7735_fillScreen(ST7735_BLACK);
    ST7735_setTextColor(ST7735_WHITE, ST7735_BLACK);
    ST7735_setCursor(0, 0); ST7735_writeString("== Rectangles ==");

    for (int16_t i = 0; i < 6; i++)
        ST7735_drawRect(i * 4, 16 + i * 4, 128 - i * 8, 60 - i * 8,
                        ST7735_RGB565(255 - i * 40, i * 40, 128));
    ST7735_fillRect(10, 84, 50, 30, ST7735_RED);
    ST7735_fillRect(68, 84, 50, 30, ST7735_BLUE);
    ST7735_drawRoundRect(10, 122, 108, 32, 8, ST7735_YELLOW);
    ST7735_fillRoundRect(16, 128, 96, 20, 6, ST7735_DARKGREEN);
    PAUSE();
}

static void demo_circles(void)
{
    ST7735_fillScreen(ST7735_BLACK);
    ST7735_setTextColor(ST7735_WHITE, ST7735_BLACK);
    ST7735_setCursor(0, 0); ST7735_writeString("== Circles ==");

    for (int16_t r = 6; r <= 30; r += 6)
        ST7735_drawCircle(64, 56, r, ST7735_RGB565(0, r * 8, 255 - r * 6));
    ST7735_fillCircle(34, 120, 26, ST7735_RED);
    ST7735_fillCircle(94, 120, 26, ST7735_CYAN);
    PAUSE();
}

static void demo_triangles(void)
{
    ST7735_fillScreen(ST7735_BLACK);
    ST7735_setTextColor(ST7735_WHITE, ST7735_BLACK);
    ST7735_setCursor(0, 0); ST7735_writeString("== Triangles ==");

    ST7735_fillTriangle(64, 18, 14, 90, 114, 90, ST7735_GREEN);
    ST7735_drawTriangle(64, 18, 14, 90, 114, 90, ST7735_WHITE);
    ST7735_fillTriangle(20, 150, 64, 100, 108, 150, ST7735_ORANGE);
    PAUSE();
}

static void demo_image(void)
{
    ST7735_fillScreen(ST7735_BLACK);
    ST7735_setTextColor(ST7735_WHITE, ST7735_BLACK);
    ST7735_setCursor(0, 0); ST7735_writeString("== Image (RGB565) ==");
    /* Center the 80x64 sample image. */
    ST7735_drawRGBBitmap((128 - GIMAGE_WIDTH) / 2, 40,
                         GIMAGE_WIDTH, GIMAGE_HEIGHT, gImage);
    PAUSE();
}

static void demo_colorbars(void)
{
    uint16_t bars[] = { ST7735_RED, ST7735_GREEN, ST7735_BLUE,
                        ST7735_YELLOW, ST7735_CYAN, ST7735_MAGENTA,
                        ST7735_WHITE, ST7735_BLACK };
    int16_t bh = 160 / 8;
    for (int16_t i = 0; i < 8; i++)
        ST7735_fillRect(0, i * bh, 128, bh, bars[i]);
    ST7735_setTextColor(ST7735_BLACK, ST7735_WHITE);
    ST7735_setCursor(4, 4); ST7735_writeString("Color Bars");
    PAUSE();
}

int main(void)
{
    SYSCFG_DL_init();
    ST7735_init();

    while (1) {
        demo_text();
        demo_colorbars();
        demo_lines();
        demo_rects();
        demo_circles();
        demo_triangles();
        demo_image();
    }
}
