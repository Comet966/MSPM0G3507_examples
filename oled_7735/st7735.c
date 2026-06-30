#include "st7735.h"
#include "ti_msp_dl_config.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/*
 * ST7735S driver implementation for MSPM0G3507.
 *
 * SysConfig instances expected (see st7735.h):
 *   SPI_0     : SPI1 controller, SCLK=PB9, PICO=PB8, 8-bit, mode 0
 *   GPIO_LCD  : output group on PORTB with pins DC=PB16, RES=PB15,
 *               CS=PB17, BLK=PB12
 *
 * Macro names follow the SysConfig convention:
 *   SPI_0_INST                        - SPI peripheral pointer
 *   GPIO_LCD_PORT                     - shared GPIO port
 *   GPIO_LCD_DC_PIN / _RES_ / _CS_ / _BLK_PIN
 */

/* ====================================================================== */
/* Panel command set                                                      */
/* ====================================================================== */
#define ST_NOP        0x00
#define ST_SWRESET    0x01
#define ST_SLPOUT     0x11
#define ST_INVOFF     0x20
#define ST_INVON      0x21
#define ST_DISPON     0x29
#define ST_CASET      0x2A
#define ST_RASET      0x2B
#define ST_RAMWR      0x2C
#define ST_MADCTL     0x36
#define ST_COLMOD     0x3A
#define ST_FRMCTR1    0xB1
#define ST_FRMCTR2    0xB2
#define ST_FRMCTR3    0xB3
#define ST_INVCTR     0xB4
#define ST_PWCTR1     0xC0
#define ST_PWCTR2     0xC1
#define ST_PWCTR3     0xC2
#define ST_PWCTR4     0xC3
#define ST_PWCTR5     0xC4
#define ST_VMCTR1     0xC5
#define ST_GMCTRP1    0xE0
#define ST_GMCTRN1    0xE1

/* MADCTL bits */
#define MADCTL_MY     0x80
#define MADCTL_MX     0x40
#define MADCTL_MV     0x20
#define MADCTL_ML     0x10
#define MADCTL_RGB    0x00
#define MADCTL_BGR    0x08

/*
 * Many 1.8" ST7735S "red tab" / RGB_TFT modules carry no row/col offset.
 * If your panel shows a few stray pixels shifted at an edge, adjust these.
 */
#define COLSTART      0
#define ROWSTART      0

/* ====================================================================== */
/* Module state                                                           */
/* ====================================================================== */
static uint8_t  gRotation = 0;
static uint16_t gW = ST7735_TFTWIDTH;
static uint16_t gH = ST7735_TFTHEIGHT;

static int16_t  gCurX = 0, gCurY = 0;
static uint16_t gTextFg = ST7735_WHITE, gTextBg = ST7735_BLACK;
static uint8_t  gTextSize = 1;
static bool     gWrap = true;

/* ====================================================================== */
/* Low-level SPI / control pins                                           */
/* ====================================================================== */
static inline void cs_low(void)  { DL_GPIO_clearPins(GPIO_LCD_PORT, GPIO_LCD_CS_PIN); }
static inline void cs_high(void) { DL_GPIO_setPins(GPIO_LCD_PORT, GPIO_LCD_CS_PIN); }
static inline void dc_cmd(void)  { DL_GPIO_clearPins(GPIO_LCD_PORT, GPIO_LCD_DC_PIN); }
static inline void dc_data(void) { DL_GPIO_setPins(GPIO_LCD_PORT, GPIO_LCD_DC_PIN); }

/* Push one byte, keeping the 4-deep TX FIFO fed without overflow. */
static inline void spi_tx(uint8_t b)
{
    while (DL_SPI_isTXFIFOFull(SPI_0_INST)) { /* wait for space */ }
    DL_SPI_transmitData8(SPI_0_INST, b);
}

/* Block until all queued bytes have been clocked out. */
static inline void spi_flush(void)
{
    while (DL_SPI_isBusy(SPI_0_INST)) { /* wait */ }
}

static void wr_cmd(uint8_t cmd)
{
    spi_flush();
    dc_cmd();
    spi_tx(cmd);
    spi_flush();
    dc_data();
}

static void wr_data(uint8_t d)
{
    spi_tx(d);
}

static void wr_data_buf(const uint8_t *buf, uint16_t len)
{
    while (len--) spi_tx(*buf++);
}

/* ====================================================================== */
/* Address window + raw color push                                        */
/* ====================================================================== */
void ST7735_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    x0 += COLSTART; x1 += COLSTART;
    y0 += ROWSTART; y1 += ROWSTART;

    wr_cmd(ST_CASET);
    wr_data(x0 >> 8); wr_data(x0 & 0xFF);
    wr_data(x1 >> 8); wr_data(x1 & 0xFF);

    wr_cmd(ST_RASET);
    wr_data(y0 >> 8); wr_data(y0 & 0xFF);
    wr_data(y1 >> 8); wr_data(y1 & 0xFF);

    wr_cmd(ST_RAMWR);
}

void ST7735_pushColor(uint16_t color, uint32_t count)
{
    uint8_t hi = color >> 8, lo = color & 0xFF;
    while (count--) {
        spi_tx(hi);
        spi_tx(lo);
    }
}

/* ====================================================================== */
/* Init sequence                                                          */
/* ====================================================================== */
static void hw_reset(void)
{
    DL_GPIO_setPins(GPIO_LCD_PORT, GPIO_LCD_RES_PIN);
    delay_cycles(3200000);     /* ~100 ms @ 32 MHz */
    DL_GPIO_clearPins(GPIO_LCD_PORT, GPIO_LCD_RES_PIN);
    delay_cycles(3200000);
    DL_GPIO_setPins(GPIO_LCD_PORT, GPIO_LCD_RES_PIN);
    delay_cycles(3200000);
}

void ST7735_init(void)
{
    cs_high();
    ST7735_backlight(true);
    hw_reset();

    cs_low();

    wr_cmd(ST_SWRESET);
    delay_cycles(4800000);     /* ~150 ms */

    wr_cmd(ST_SLPOUT);
    delay_cycles(16000000);    /* ~500 ms */

    /* Frame rate control */
    wr_cmd(ST_FRMCTR1); wr_data(0x01); wr_data(0x2C); wr_data(0x2D);
    wr_cmd(ST_FRMCTR2); wr_data(0x01); wr_data(0x2C); wr_data(0x2D);
    wr_cmd(ST_FRMCTR3);
    wr_data(0x01); wr_data(0x2C); wr_data(0x2D);
    wr_data(0x01); wr_data(0x2C); wr_data(0x2D);

    wr_cmd(ST_INVCTR);  wr_data(0x07);

    /* Power control */
    wr_cmd(ST_PWCTR1); wr_data(0xA2); wr_data(0x02); wr_data(0x84);
    wr_cmd(ST_PWCTR2); wr_data(0xC5);
    wr_cmd(ST_PWCTR3); wr_data(0x0A); wr_data(0x00);
    wr_cmd(ST_PWCTR4); wr_data(0x8A); wr_data(0x2A);
    wr_cmd(ST_PWCTR5); wr_data(0x8A); wr_data(0xEE);
    wr_cmd(ST_VMCTR1); wr_data(0x0E);

    wr_cmd(ST_INVOFF);

    /* Memory access / color order */
    wr_cmd(ST_MADCTL); wr_data(MADCTL_MX | MADCTL_MY | MADCTL_BGR);

    /* 16-bit/pixel (RGB565) */
    wr_cmd(ST_COLMOD); wr_data(0x05);

    /* Gamma */
    {
        static const uint8_t gp[] = {
            0x02,0x1c,0x07,0x12,0x37,0x32,0x29,0x2d,
            0x29,0x25,0x2B,0x39,0x00,0x01,0x03,0x10 };
        static const uint8_t gn[] = {
            0x03,0x1d,0x07,0x06,0x2E,0x2C,0x29,0x2D,
            0x2E,0x2E,0x37,0x3F,0x00,0x00,0x02,0x10 };
        wr_cmd(ST_GMCTRP1); wr_data_buf(gp, sizeof(gp));
        wr_cmd(ST_GMCTRN1); wr_data_buf(gn, sizeof(gn));
    }

    wr_cmd(ST_DISPON);
    delay_cycles(3200000);     /* ~100 ms */

    spi_flush();
    cs_high();

    gRotation = 0;
    gW = ST7735_TFTWIDTH;
    gH = ST7735_TFTHEIGHT;

    ST7735_fillScreen(ST7735_BLACK);
}

void ST7735_setRotation(uint8_t rotation)
{
    uint8_t madctl;
    gRotation = rotation & 3;
    switch (gRotation) {
        case 0:
            madctl = MADCTL_MX | MADCTL_MY | MADCTL_BGR;
            gW = ST7735_TFTWIDTH;  gH = ST7735_TFTHEIGHT;
            break;
        case 1:
            madctl = MADCTL_MY | MADCTL_MV | MADCTL_BGR;
            gW = ST7735_TFTHEIGHT; gH = ST7735_TFTWIDTH;
            break;
        case 2:
            madctl = MADCTL_BGR;
            gW = ST7735_TFTWIDTH;  gH = ST7735_TFTHEIGHT;
            break;
        default: /* 3 */
            madctl = MADCTL_MX | MADCTL_MV | MADCTL_BGR;
            gW = ST7735_TFTHEIGHT; gH = ST7735_TFTWIDTH;
            break;
    }
    cs_low();
    wr_cmd(ST_MADCTL); wr_data(madctl);
    spi_flush();
    cs_high();
}

uint16_t ST7735_width(void)  { return gW; }
uint16_t ST7735_height(void) { return gH; }

void ST7735_backlight(bool on)
{
    if (on) DL_GPIO_setPins(GPIO_LCD_PORT, GPIO_LCD_BLK_PIN);
    else    DL_GPIO_clearPins(GPIO_LCD_PORT, GPIO_LCD_BLK_PIN);
}

void ST7735_invertDisplay(bool invert)
{
    cs_low();
    wr_cmd(invert ? ST_INVON : ST_INVOFF);
    spi_flush();
    cs_high();
}

/* ====================================================================== */
/* Fill primitives                                                        */
/* ====================================================================== */
void ST7735_fillScreen(uint16_t color)
{
    ST7735_fillRect(0, 0, gW, gH, color);
}

void ST7735_clear(void)
{
    ST7735_fillScreen(ST7735_BLACK);
}

void ST7735_drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || y < 0 || x >= (int16_t)gW || y >= (int16_t)gH) return;
    cs_low();
    ST7735_setAddrWindow((uint16_t)x, (uint16_t)y, (uint16_t)x, (uint16_t)y);
    ST7735_pushColor(color, 1);
    spi_flush();
    cs_high();
}

void ST7735_fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     uint16_t color)
{
    /* Clip to screen */
    if (w <= 0 || h <= 0) return;
    if (x >= (int16_t)gW || y >= (int16_t)gH) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > (int16_t)gW) w = gW - x;
    if (y + h > (int16_t)gH) h = gH - y;
    if (w <= 0 || h <= 0) return;

    cs_low();
    ST7735_setAddrWindow((uint16_t)x, (uint16_t)y,
                         (uint16_t)(x + w - 1), (uint16_t)(y + h - 1));
    ST7735_pushColor(color, (uint32_t)w * (uint32_t)h);
    spi_flush();
    cs_high();
}

void ST7735_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    ST7735_fillRect(x, y, w, 1, color);
}

void ST7735_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    ST7735_fillRect(x, y, 1, h, color);
}

/* ====================================================================== */
/* Line / shape primitives                                                */
/* ====================================================================== */
void ST7735_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                     uint16_t color)
{
    int16_t steep = (y1 > y0 ? y1 - y0 : y0 - y1) >
                    (x1 > x0 ? x1 - x0 : x0 - x1);
    if (steep) { int16_t t; t=x0;x0=y0;y0=t; t=x1;x1=y1;y1=t; }
    if (x0 > x1) { int16_t t; t=x0;x0=x1;x1=t; t=y0;y0=y1;y1=t; }

    int16_t dx = x1 - x0;
    int16_t dy = (y1 > y0 ? y1 - y0 : y0 - y1);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;

    for (; x0 <= x1; x0++) {
        if (steep) ST7735_drawPixel(y0, x0, color);
        else       ST7735_drawPixel(x0, y0, color);
        err -= dy;
        if (err < 0) { y0 += ystep; err += dx; }
    }
}

void ST7735_drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     uint16_t color)
{
    if (w <= 0 || h <= 0) return;
    ST7735_drawFastHLine(x, y, w, color);
    ST7735_drawFastHLine(x, y + h - 1, w, color);
    ST7735_drawFastVLine(x, y, h, color);
    ST7735_drawFastVLine(x + w - 1, y, h, color);
}

/* Midpoint circle, 8-way symmetry. */
void ST7735_drawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color)
{
    if (r < 0) return;
    int16_t x = 0, y = r, d = 1 - r;
    ST7735_drawPixel(cx, cy + r, color);
    ST7735_drawPixel(cx, cy - r, color);
    ST7735_drawPixel(cx + r, cy, color);
    ST7735_drawPixel(cx - r, cy, color);
    while (x < y) {
        if (d < 0) d += 2 * x + 3;
        else { d += 2 * (x - y) + 5; y--; }
        x++;
        ST7735_drawPixel(cx + x, cy + y, color);
        ST7735_drawPixel(cx - x, cy + y, color);
        ST7735_drawPixel(cx + x, cy - y, color);
        ST7735_drawPixel(cx - x, cy - y, color);
        ST7735_drawPixel(cx + y, cy + x, color);
        ST7735_drawPixel(cx - y, cy + x, color);
        ST7735_drawPixel(cx + y, cy - x, color);
        ST7735_drawPixel(cx - y, cy - x, color);
    }
}

void ST7735_fillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color)
{
    if (r < 0) return;
    int16_t x = 0, y = r, d = 1 - r;
    ST7735_drawFastVLine(cx, cy - r, 2 * r + 1, color);
    while (x < y) {
        if (d < 0) d += 2 * x + 3;
        else { d += 2 * (x - y) + 5; y--; }
        x++;
        ST7735_drawFastVLine(cx + x, cy - y, 2 * y + 1, color);
        ST7735_drawFastVLine(cx - x, cy - y, 2 * y + 1, color);
        ST7735_drawFastVLine(cx + y, cy - x, 2 * x + 1, color);
        ST7735_drawFastVLine(cx - y, cy - x, 2 * x + 1, color);
    }
}

static void round_helper(int16_t cx, int16_t cy, int16_t r,
                         uint8_t corner, int16_t delta, uint16_t color,
                         bool fill)
{
    int16_t x = 0, y = r, d = 1 - r;
    while (x < y) {
        if (d < 0) d += 2 * x + 3;
        else { d += 2 * (x - y) + 5; y--; }
        x++;
        if (fill) {
            if (corner & 0x1) {
                ST7735_drawFastVLine(cx + x, cy - y, 2*y + 1 + delta, color);
                ST7735_drawFastVLine(cx + y, cy - x, 2*x + 1 + delta, color);
            }
            if (corner & 0x2) {
                ST7735_drawFastVLine(cx - x, cy - y, 2*y + 1 + delta, color);
                ST7735_drawFastVLine(cx - y, cy - x, 2*x + 1 + delta, color);
            }
        } else {
            if (corner & 0x1) { /* top-right */
                ST7735_drawPixel(cx + x, cy - y, color);
                ST7735_drawPixel(cx + y, cy - x, color);
            }
            if (corner & 0x2) { /* top-left */
                ST7735_drawPixel(cx - y, cy - x, color);
                ST7735_drawPixel(cx - x, cy - y, color);
            }
            if (corner & 0x4) { /* bottom-right */
                ST7735_drawPixel(cx + x, cy + y, color);
                ST7735_drawPixel(cx + y, cy + x, color);
            }
            if (corner & 0x8) { /* bottom-left */
                ST7735_drawPixel(cx - y, cy + x, color);
                ST7735_drawPixel(cx - x, cy + y, color);
            }
        }
    }
}

void ST7735_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                          int16_t r, uint16_t color)
{
    if (w <= 0 || h <= 0) return;
    int16_t maxr = ((w < h) ? w : h) / 2;
    if (r > maxr) r = maxr;
    ST7735_drawFastHLine(x + r,     y,         w - 2 * r, color);
    ST7735_drawFastHLine(x + r,     y + h - 1, w - 2 * r, color);
    ST7735_drawFastVLine(x,         y + r,     h - 2 * r, color);
    ST7735_drawFastVLine(x + w - 1, y + r,     h - 2 * r, color);
    round_helper(x + r,         y + r,         r, 0x2, 0, color, false);
    round_helper(x + w - r - 1, y + r,         r, 0x1, 0, color, false);
    round_helper(x + w - r - 1, y + h - r - 1, r, 0x4, 0, color, false);
    round_helper(x + r,         y + h - r - 1, r, 0x8, 0, color, false);
}

void ST7735_fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                          int16_t r, uint16_t color)
{
    if (w <= 0 || h <= 0) return;
    int16_t maxr = ((w < h) ? w : h) / 2;
    if (r > maxr) r = maxr;
    ST7735_fillRect(x + r, y, w - 2 * r, h, color);
    round_helper(x + w - r - 1, y + r, r, 0x1, h - 2 * r - 1, color, true);
    round_helper(x + r,         y + r, r, 0x2, h - 2 * r - 1, color, true);
}

void ST7735_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                         int16_t x2, int16_t y2, uint16_t color)
{
    ST7735_drawLine(x0, y0, x1, y1, color);
    ST7735_drawLine(x1, y1, x2, y2, color);
    ST7735_drawLine(x2, y2, x0, y0, color);
}

void ST7735_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                         int16_t x2, int16_t y2, uint16_t color)
{
    int16_t a, b, ya, t;
    /* Sort vertices by y (y0 <= y1 <= y2) */
    if (y0 > y1) { t=y0;y0=y1;y1=t; t=x0;x0=x1;x1=t; }
    if (y1 > y2) { t=y1;y1=y2;y2=t; t=x1;x1=x2;x2=t; }
    if (y0 > y1) { t=y0;y0=y1;y1=t; t=x0;x0=x1;x1=t; }

    if (y0 == y2) { /* degenerate: a single horizontal line */
        a = b = x0;
        if (x1 < a) a = x1; else if (x1 > b) b = x1;
        if (x2 < a) a = x2; else if (x2 > b) b = x2;
        ST7735_drawFastHLine(a, y0, b - a + 1, color);
        return;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0;
    int16_t dx02 = x2 - x0, dy02 = y2 - y0;
    int16_t dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    int16_t last = (y1 == y2) ? y1 : y1 - 1;
    for (ya = y0; ya <= last; ya++) {
        a = x0 + (int16_t)(sa / dy01);
        b = x0 + (int16_t)(sb / dy02);
        sa += dx01; sb += dx02;
        if (a > b) { t = a; a = b; b = t; }
        ST7735_drawFastHLine(a, ya, b - a + 1, color);
    }

    sa = (int32_t)dx12 * (ya - y1);
    sb = (int32_t)dx02 * (ya - y0);
    for (; ya <= y2; ya++) {
        a = x1 + (int16_t)(sa / dy12);
        b = x0 + (int16_t)(sb / dy02);
        sa += dx12; sb += dx02;
        if (a > b) { t = a; a = b; b = t; }
        ST7735_drawFastHLine(a, ya, b - a + 1, color);
    }
}

/* ====================================================================== */
/* Built-in 5x7 font (ASCII 0x20-0x7E), 5 column bytes per glyph          */
/* ====================================================================== */
static const uint8_t gFont5x7[95][5] = {
{0x00,0x00,0x00,0x00,0x00}, /*   */ {0x00,0x00,0x5F,0x00,0x00}, /* ! */
{0x00,0x07,0x00,0x07,0x00}, /* " */ {0x14,0x7F,0x14,0x7F,0x14}, /* # */
{0x24,0x2A,0x7F,0x2A,0x12}, /* $ */ {0x23,0x13,0x08,0x64,0x62}, /* % */
{0x36,0x49,0x55,0x22,0x50}, /* & */ {0x00,0x05,0x03,0x00,0x00}, /* ' */
{0x00,0x1C,0x22,0x41,0x00}, /* ( */ {0x00,0x41,0x22,0x1C,0x00}, /* ) */
{0x14,0x08,0x3E,0x08,0x14}, /* * */ {0x08,0x08,0x3E,0x08,0x08}, /* + */
{0x00,0x50,0x30,0x00,0x00}, /* , */ {0x08,0x08,0x08,0x08,0x08}, /* - */
{0x00,0x60,0x60,0x00,0x00}, /* . */ {0x20,0x10,0x08,0x04,0x02}, /* / */
{0x3E,0x51,0x49,0x45,0x3E}, /* 0 */ {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
{0x42,0x61,0x51,0x49,0x46}, /* 2 */ {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
{0x18,0x14,0x12,0x7F,0x10}, /* 4 */ {0x27,0x45,0x45,0x45,0x39}, /* 5 */
{0x3C,0x4A,0x49,0x49,0x30}, /* 6 */ {0x01,0x71,0x09,0x05,0x03}, /* 7 */
{0x36,0x49,0x49,0x49,0x36}, /* 8 */ {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
{0x00,0x36,0x36,0x00,0x00}, /* : */ {0x00,0x56,0x36,0x00,0x00}, /* ; */
{0x08,0x14,0x22,0x41,0x00}, /* < */ {0x14,0x14,0x14,0x14,0x14}, /* = */
{0x00,0x41,0x22,0x14,0x08}, /* > */ {0x02,0x01,0x51,0x09,0x06}, /* ? */
{0x32,0x49,0x79,0x41,0x3E}, /* @ */ {0x7E,0x11,0x11,0x11,0x7E}, /* A */
{0x7F,0x49,0x49,0x49,0x36}, /* B */ {0x3E,0x41,0x41,0x41,0x22}, /* C */
{0x7F,0x41,0x41,0x22,0x1C}, /* D */ {0x7F,0x49,0x49,0x49,0x41}, /* E */
{0x7F,0x09,0x09,0x09,0x01}, /* F */ {0x3E,0x41,0x49,0x49,0x7A}, /* G */
{0x7F,0x08,0x08,0x08,0x7F}, /* H */ {0x00,0x41,0x7F,0x41,0x00}, /* I */
{0x20,0x40,0x41,0x3F,0x01}, /* J */ {0x7F,0x08,0x14,0x22,0x41}, /* K */
{0x7F,0x40,0x40,0x40,0x40}, /* L */ {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
{0x7F,0x04,0x08,0x10,0x7F}, /* N */ {0x3E,0x41,0x41,0x41,0x3E}, /* O */
{0x7F,0x09,0x09,0x09,0x06}, /* P */ {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
{0x7F,0x09,0x19,0x29,0x46}, /* R */ {0x46,0x49,0x49,0x49,0x31}, /* S */
{0x01,0x01,0x7F,0x01,0x01}, /* T */ {0x3F,0x40,0x40,0x40,0x3F}, /* U */
{0x1F,0x20,0x40,0x20,0x1F}, /* V */ {0x7F,0x20,0x18,0x20,0x7F}, /* W */
{0x63,0x14,0x08,0x14,0x63}, /* X */ {0x03,0x04,0x78,0x04,0x03}, /* Y */
{0x61,0x51,0x49,0x45,0x43}, /* Z */ {0x00,0x7F,0x41,0x41,0x00}, /* [ */
{0x02,0x04,0x08,0x10,0x20}, /* \ */ {0x00,0x41,0x41,0x7F,0x00}, /* ] */
{0x04,0x02,0x01,0x02,0x04}, /* ^ */ {0x40,0x40,0x40,0x40,0x40}, /* _ */
{0x00,0x01,0x02,0x04,0x00}, /* ` */ {0x20,0x54,0x54,0x54,0x78}, /* a */
{0x7F,0x48,0x44,0x44,0x38}, /* b */ {0x38,0x44,0x44,0x44,0x20}, /* c */
{0x38,0x44,0x44,0x48,0x7F}, /* d */ {0x38,0x54,0x54,0x54,0x18}, /* e */
{0x08,0x7E,0x09,0x01,0x02}, /* f */ {0x0C,0x52,0x52,0x52,0x3E}, /* g */
{0x7F,0x08,0x04,0x04,0x78}, /* h */ {0x00,0x44,0x7D,0x40,0x00}, /* i */
{0x20,0x40,0x44,0x3D,0x00}, /* j */ {0x7F,0x10,0x28,0x44,0x00}, /* k */
{0x00,0x41,0x7F,0x40,0x00}, /* l */ {0x7C,0x04,0x18,0x04,0x78}, /* m */
{0x7C,0x08,0x04,0x04,0x78}, /* n */ {0x38,0x44,0x44,0x44,0x38}, /* o */
{0x7C,0x14,0x14,0x14,0x08}, /* p */ {0x08,0x14,0x14,0x18,0x7C}, /* q */
{0x7C,0x08,0x04,0x04,0x08}, /* r */ {0x48,0x54,0x54,0x54,0x20}, /* s */
{0x04,0x3F,0x44,0x40,0x20}, /* t */ {0x3C,0x40,0x40,0x20,0x7C}, /* u */
{0x1C,0x20,0x40,0x20,0x1C}, /* v */ {0x3C,0x40,0x30,0x40,0x3C}, /* w */
{0x44,0x28,0x10,0x28,0x44}, /* x */ {0x0C,0x50,0x50,0x50,0x3C}, /* y */
{0x44,0x64,0x54,0x4C,0x44}, /* z */ {0x00,0x08,0x36,0x41,0x00}, /* { */
{0x00,0x00,0x7F,0x00,0x00}, /* | */ {0x00,0x41,0x36,0x08,0x00}, /* } */
{0x08,0x04,0x08,0x10,0x08}, /* ~ */
};

void ST7735_drawChar(int16_t x, int16_t y, char c,
                     uint16_t fg, uint16_t bg, uint8_t size)
{
    if (c < 0x20 || c > 0x7E) c = '?';
    if (size < 1) size = 1;
    const uint8_t *glyph = gFont5x7[(uint8_t)(c - 0x20)];
    bool opaque = (fg != bg);

    /* Render the 6x8 cell (5 glyph columns + 1 spacing column). */
    for (uint8_t col = 0; col < 6; col++) {
        uint8_t bits = (col < 5) ? glyph[col] : 0x00;
        for (uint8_t row = 0; row < 8; row++) {
            bool on = (bits >> row) & 1;
            if (on) {
                if (size == 1) ST7735_drawPixel(x + col, y + row, fg);
                else ST7735_fillRect(x + col * size, y + row * size,
                                     size, size, fg);
            } else if (opaque) {
                if (size == 1) ST7735_drawPixel(x + col, y + row, bg);
                else ST7735_fillRect(x + col * size, y + row * size,
                                     size, size, bg);
            }
        }
    }
}

void ST7735_setCursor(int16_t x, int16_t y)       { gCurX = x; gCurY = y; }
void ST7735_setTextColor(uint16_t fg, uint16_t bg){ gTextFg = fg; gTextBg = bg; }
void ST7735_setTextSize(uint8_t size)             { gTextSize = size ? size : 1; }
void ST7735_setTextWrap(bool wrap)                { gWrap = wrap; }

void ST7735_writeChar(char c)
{
    uint8_t cw = 6 * gTextSize;
    uint8_t ch = 8 * gTextSize;
    if (c == '\n') { gCurX = 0; gCurY += ch; return; }
    if (c == '\r') { gCurX = 0; return; }
    if (gWrap && gCurX + cw > (int16_t)gW) { gCurX = 0; gCurY += ch; }
    ST7735_drawChar(gCurX, gCurY, c, gTextFg, gTextBg, gTextSize);
    gCurX += cw;
}

void ST7735_writeString(const char *str)
{
    while (*str) ST7735_writeChar(*str++);
}

void ST7735_printf(const char *fmt, ...)
{
    char buf[80];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    ST7735_writeString(buf);
}

/* ====================================================================== */
/* Images                                                                 */
/* ====================================================================== */
void ST7735_drawRGBBitmap(int16_t x, int16_t y, int16_t w, int16_t h,
                          const uint16_t *img)
{
    if (w <= 0 || h <= 0) return;

    /* Fast path: fully on-screen -> single window, stream straight through. */
    if (x >= 0 && y >= 0 &&
        x + w <= (int16_t)gW && y + h <= (int16_t)gH) {
        cs_low();
        ST7735_setAddrWindow((uint16_t)x, (uint16_t)y,
                             (uint16_t)(x + w - 1), (uint16_t)(y + h - 1));
        uint32_t n = (uint32_t)w * (uint32_t)h;
        for (uint32_t i = 0; i < n; i++) {
            uint16_t px = img[i];
            spi_tx(px >> 8);
            spi_tx(px & 0xFF);
        }
        spi_flush();
        cs_high();
        return;
    }

    /* Clipped path: per-pixel (handles partially off-screen images). */
    for (int16_t row = 0; row < h; row++) {
        int16_t py = y + row;
        if (py < 0 || py >= (int16_t)gH) continue;
        for (int16_t col = 0; col < w; col++) {
            int16_t px = x + col;
            if (px < 0 || px >= (int16_t)gW) continue;
            ST7735_drawPixel(px, py, img[row * w + col]);
        }
    }
}

void ST7735_drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h,
                       const uint8_t *bmp, uint16_t fg, uint16_t bg)
{
    if (w <= 0 || h <= 0) return;
    uint16_t stride = (w + 7) >> 3;
    bool opaque = (fg != bg);
    for (int16_t row = 0; row < h; row++) {
        for (int16_t col = 0; col < w; col++) {
            uint8_t byte = bmp[row * stride + (col >> 3)];
            bool on = (byte >> (7 - (col & 7))) & 1;
            if (on)        ST7735_drawPixel(x + col, y + row, fg);
            else if (opaque) ST7735_drawPixel(x + col, y + row, bg);
        }
    }
}
