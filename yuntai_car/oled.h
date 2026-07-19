#ifndef OLED_H_
#define OLED_H_

#include <stdint.h>
#include <stdbool.h>

#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_PAGES   8

/* ---- Init / buffer control ---- */
void OLED_init(void);
void OLED_clear(void);
void OLED_display(void);   /* flush only dirty pages */
void OLED_markDirty(void); /* force-mark all pages dirty */

/* ---- Pixel ---- */
void OLED_setPixel(uint8_t x, uint8_t y, bool on);
bool OLED_getPixel(uint8_t x, uint8_t y);

/* ---- Primitives (write to frame buffer only, call display() to flush) ---- */
void OLED_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on);
void OLED_drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on);
void OLED_fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on);
void OLED_drawCircle(uint8_t cx, uint8_t cy, uint8_t r, bool on);
void OLED_fillCircle(uint8_t cx, uint8_t cy, uint8_t r, bool on);

/* Bitmap in SSD1306 native vertical format: bmp[page * w + col] */
void OLED_drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                     const uint8_t *bmp);

/* ---- Text (8x8 font) ---- */
void OLED_setCursor(uint8_t col, uint8_t page);
void OLED_writeChar(char c);
void OLED_writeString(const char *str);
void OLED_printf(const char *fmt, ...);

#endif /* OLED_H_ */
