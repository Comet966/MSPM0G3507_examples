#ifndef ST7735_H_
#define ST7735_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * ST7735S 1.8" 128x160 65K-color TFT-LCD driver (SPI, 4-wire)
 * Target: MSPM0G3507 (LP-MSPM0G3507), DriverLib / SysConfig.
 *
 * Architecture: DIRECT-DRAW. A full 128x160 RGB565 frame buffer is 40 KB,
 * which exceeds the MSPM0G3507's 32 KB SRAM, so every primitive streams
 * pixels straight to the panel over SPI. There is no flush step: whatever
 * you draw appears immediately. Color is RGB565 (16 bits/pixel).
 *
 * SysConfig contract (created via the SysConfig MCP server):
 *   - SPI controller instance "SPI_0"  -> SPI1, SCLK=PB9, PICO=PB8,
 *                                         8-bit, mode 0, highest bit rate
 *   - GPIO output group     "GPIO_LCD" -> DC=PB16, RES=PB15, CS=PB17, BLK=PB12
 */

/* ---- Panel geometry (native, rotation 0) ---- */
#define ST7735_TFTWIDTH    128
#define ST7735_TFTHEIGHT   160

/* ---- 16-bit RGB565 color helper ---- */
#define ST7735_RGB565(r, g, b) \
    ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

/* Common named colors (RGB565) */
#define ST7735_BLACK       0x0000
#define ST7735_WHITE       0xFFFF
#define ST7735_RED         0xF800
#define ST7735_GREEN       0x07E0
#define ST7735_BLUE        0x001F
#define ST7735_YELLOW      0xFFE0
#define ST7735_CYAN        0x07FF
#define ST7735_MAGENTA     0xF81F
#define ST7735_ORANGE      0xFD20
#define ST7735_GRAY        0x8410
#define ST7735_NAVY        0x000F
#define ST7735_DARKGREEN   0x03E0
#define ST7735_MAROON      0x7800
#define ST7735_PURPLE      0x780F

/* ---- Init / global state ---- */

/*
 * Initialize the panel. Performs hardware reset, runs the ST7735S boot
 * sequence, sets color mode to RGB565, turns the display on and the
 * backlight on, then clears the screen to black.
 * Must be called once after SYSCFG_DL_init().
 */
void ST7735_init(void);

/*
 * Set display rotation (0-3). Rotation changes the logical width/height
 * and the origin used by all subsequent drawing calls:
 *   0 -> 128x160 (portrait, default)
 *   1 -> 160x128 (landscape)
 *   2 -> 128x160 (portrait flipped)
 *   3 -> 160x128 (landscape flipped)
 */
void ST7735_setRotation(uint8_t rotation);

/* Logical screen size for the current rotation. */
uint16_t ST7735_width(void);
uint16_t ST7735_height(void);

/* Backlight (BLK pin) on/off. Display is brought up with it on. */
void ST7735_backlight(bool on);

/* Invert display colors (ST7735 INVON/INVOFF). */
void ST7735_invertDisplay(bool invert);

/* ---- Fill / clear ---- */

/* Fill the entire screen with one color. */
void ST7735_fillScreen(uint16_t color);

/* Convenience: fill the screen with black. */
void ST7735_clear(void);

/* ---- Pixel ---- */

/* Draw a single pixel. Out-of-bounds coordinates are ignored. */
void ST7735_drawPixel(int16_t x, int16_t y, uint16_t color);

/* ---- Graphics primitives ---- */

void ST7735_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void ST7735_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void ST7735_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                     uint16_t color);
void ST7735_drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     uint16_t color);
void ST7735_fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     uint16_t color);
void ST7735_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                          int16_t r, uint16_t color);
void ST7735_fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                          int16_t r, uint16_t color);
void ST7735_drawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
void ST7735_fillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
void ST7735_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                         int16_t x2, int16_t y2, uint16_t color);
void ST7735_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                         int16_t x2, int16_t y2, uint16_t color);

/* ---- Text (built-in 5x7 font, rendered in a 6x8 cell) ---- */

/*
 * Set text cursor (pixel coordinates of the top-left of the next glyph),
 * foreground/background color and integer size multiplier.
 * If bg == fg the glyph background is transparent (only foreground pixels
 * are drawn, which is slower but preserves what is underneath).
 */
void ST7735_setCursor(int16_t x, int16_t y);
void ST7735_setTextColor(uint16_t fg, uint16_t bg);
void ST7735_setTextSize(uint8_t size);
void ST7735_setTextWrap(bool wrap);

/* Draw one glyph at an explicit position/size/color (does not move cursor). */
void ST7735_drawChar(int16_t x, int16_t y, char c,
                     uint16_t fg, uint16_t bg, uint8_t size);

/* Write using the cursor/color/size state. Handles '\n' and '\r'. */
void ST7735_writeChar(char c);
void ST7735_writeString(const char *str);
void ST7735_printf(const char *fmt, ...);

/* ---- Images ---- */

/*
 * Blit a full-color RGB565 image. Pixels are row-major, big-endian
 * (high byte first) so the array can be streamed straight to the panel:
 *   img[(row * w + col)] = RGB565 pixel
 * Use the img2st7735.py tool to generate the array. Off-screen regions
 * are clipped.
 */
void ST7735_drawRGBBitmap(int16_t x, int16_t y, int16_t w, int16_t h,
                          const uint16_t *img);

/*
 * Blit a 1-bpp monochrome bitmap (MSB-first, row-padded to whole bytes),
 * drawing set bits in fg and clear bits in bg. If bg == fg, clear bits are
 * left transparent. Row stride is ((w + 7) / 8) bytes.
 */
void ST7735_drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h,
                       const uint8_t *bmp, uint16_t fg, uint16_t bg);

/* ---- Low-level window access (advanced) ---- */

/*
 * Open a rectangular address window and stream raw RGB565 pixels into it.
 * ST7735_pushColor must be bracketed by ST7735_setAddrWindow and is used by
 * higher-level routines; exposed for custom rendering.
 */
void ST7735_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ST7735_pushColor(uint16_t color, uint32_t count);

#endif /* ST7735_H_ */
