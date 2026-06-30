#include "ti_msp_dl_config.h"
#include "oled.h"
#include "tools/oled_image.h"

#define PAUSE()  delay_cycles(64000000)

static void demo_text(void)
{
    OLED_clear();
    OLED_setCursor(0, 0); OLED_writeString("== Text Demo ==");
    OLED_setCursor(0, 2); OLED_writeString("Hello, MSPM0G3507!");
    OLED_setCursor(0, 3); OLED_writeString("ABCDEFGHIJKLMNOP");
    OLED_setCursor(0, 4); OLED_writeString("abcdefghijklmnop");
    OLED_setCursor(0, 5); OLED_writeString("0123456789!@#$%^");
    OLED_setCursor(0, 6); OLED_printf("printf: %d+%d=%d", 12, 34, 46);
    OLED_display();
    PAUSE();
}

static void demo_pixel(void)
{
    OLED_clear();
    OLED_setCursor(0, 0); OLED_writeString("== Pixel Demo ==");
    for (uint8_t y = 16; y < 64; y += 2)
        for (uint8_t x = 0; x < 128; x += 2)
            OLED_setPixel(x, y, true);
    OLED_display();
    PAUSE();
    for (uint8_t y = 16; y < 64; y++)
        for (uint8_t x = 0; x < 128; x++)
            OLED_setPixel(x, y, !OLED_getPixel(x, y));
    OLED_display();
    PAUSE();
}

static void demo_line(void)
{
    OLED_clear();
    OLED_setCursor(0, 0); OLED_writeString("== Line Demo ==");
    for (uint8_t i = 0; i < 8; i++)
        OLED_drawLine(0, 16, 127, 16 + i * 6, true);
    OLED_drawLine(0, 63, 127, 16, true);
    OLED_display();
    PAUSE();
}

static void demo_rect(void)
{
    OLED_clear();
    OLED_setCursor(0, 0); OLED_writeString("== Rect Demo ==");
    OLED_drawRect(0,  16, 128, 48, true);
    OLED_drawRect(8,  22, 112, 36, true);
    OLED_drawRect(16, 28,  96, 24, true);
    OLED_drawRect(24, 34,  80, 12, true);
    OLED_display();
    PAUSE();
    OLED_clear();
    OLED_setCursor(0, 0); OLED_writeString("== FillRect ==");
    for (uint8_t i = 0; i < 4; i++)
        OLED_fillRect(i * 32, 16, 28, 48, true);
    OLED_display();
    PAUSE();
}

static void demo_circle(void)
{
    OLED_clear();
    OLED_setCursor(0, 0); OLED_writeString("== Circle Demo ==");
    for (uint8_t r = 4; r <= 22; r += 6)
        OLED_drawCircle(32, 40, r, true);
    OLED_fillCircle(96, 40, 20, true);
    OLED_display();
    PAUSE();
}

static const uint8_t gDiamond[4 * 32] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF,0xFF,
    0xFF,0xFF,0xFE,0xFC,0xF8,0xF0,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x80,0xF0,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0xF0,0x80,0x00,0x00,0x00,
    0x00,0x00,0x01,0x0F,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3F,0x0F,0x01,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF,0xFF,0xFF,
    0xFF,0xFF,0x7F,0x3F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

static void demo_bitmap(void)
{
    OLED_clear();
    OLED_setCursor(0, 0); OLED_writeString("== Bitmap Demo ==");
    OLED_drawBitmap(48, 16, 32, 32, gDiamond);
    OLED_display();
    PAUSE();
}

static void demo_image(void)
{
    OLED_clear();
    OLED_drawBitmap(0, 0, GIMAGE_WIDTH, GIMAGE_HEIGHT, gImage);
    OLED_display();
    PAUSE();
}

static void demo_combined(void)
{
    OLED_clear();
    OLED_drawRect(0, 0, 128, 64, true);
    OLED_setCursor(2, 0); OLED_writeString("MSPM0 OLED");
    OLED_drawLine(0, 10, 127, 10, true);
    OLED_drawCircle(24, 37, 20, true);
    OLED_fillCircle(24, 37,  8, true);
    OLED_drawRect(52, 18, 24, 38, true);
    OLED_fillRect (56, 22, 16, 10, true);
    OLED_setCursor(10, 3); OLED_writeString("SSD1306");
    OLED_setCursor(10, 4); OLED_writeString("128x64");
    OLED_setCursor(10, 5); OLED_printf("I2C 400k");
    OLED_display();
    PAUSE();
}

int main(void)
{
    SYSCFG_DL_init();
    OLED_init();

    while (1) {
        demo_text();
        demo_pixel();
        demo_line();
        demo_rect();
        demo_circle();
        demo_bitmap();
        demo_image();
        demo_combined();
    }
}
