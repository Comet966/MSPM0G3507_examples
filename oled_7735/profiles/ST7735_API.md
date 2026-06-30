# ST7735S 彩屏驱动接口文档

**目标硬件**: 1.8 寸 ST7735S 128×160 65K 彩色 TFT-LCD（SPI 四线接口）
**MCU**: MSPM0G3507（LP-MSPM0G3507 LaunchPad）
**头文件**: `st7735.h`
**实现文件**: `st7735.c`

---

## 硬件参数

| 参数 | 值 |
|------|-----|
| 分辨率 | 128 × 160 像素 |
| 驱动芯片 | ST7735S |
| 颜色格式 | RGB565（16 位/像素，65K 色） |
| 接口 | SPI（4 线：CS / DC / SCL / SDA），只写 |
| SPI 模式 | Mode 0（CPOL=0, CPHA=0），MSB first |
| 显示区域 | 约 28 × 35 mm |

### 引脚连接（BoosterPack 排针）

| 屏幕引脚 | 功能 | MCU 引脚 | BP 引脚 | 说明 |
|---------|------|---------|--------|------|
| GND | 电源地 | GND | 20 | |
| VCC | 3.3–5V | 3V3 | 1 | 模块自带稳压，接 3V3 |
| SCL | SPI 时钟 | PB9 | 7 | SPI1_SCLK |
| SDA | SPI 数据 | PB8 | 15 | SPI1_PICO (MOSI) |
| RES | 复位 | PB15 | 17 | GPIO 输出 |
| DC | 数据/命令选择 | PB16 | 11 | GPIO 输出，低=命令 高=数据 |
| CS | 片选 | PB17 | 18 | GPIO 输出，低有效 |
| BLK | 背光 | PB12 | 12 | GPIO 输出，高=亮 |

### SysConfig 实例约定

| 实例名 | 模块 | 配置 |
|--------|------|------|
| `SPI_0` | `/ti/driverlib/SPI` | SPI1 控制器，SCLK=PB9，PICO=PB8，8 位帧，Motorola Mode 0，最高位速率 |
| `GPIO_LCD` | `/ti/driverlib/GPIO` | PORTB 输出组，引脚 `DC`=PB16，`RES`=PB15，`CS`=PB17，`BLK`=PB12 |

宏命名遵循 SysConfig 约定：`SPI_0_INST`、`GPIO_LCD_PORT`、`GPIO_LCD_DC_PIN` 等。

---

## 架构说明：直接绘制（Direct-Draw）

128×160 全彩帧缓冲需要 `128 × 160 × 2 = 40 KB`，超过 MSPM0G3507 的 **32 KB SRAM**。
因此本驱动**不使用帧缓冲**，所有绘制函数直接将像素通过 SPI 写入屏幕 GRAM。

- **无需刷新步骤**：绘制即显示，没有 `display()`/`flush()` 调用。
- **代价**：逐像素图形（如斜线、圆弧）会有较多 SPI 小事务；矩形/填充类用地址窗口批量推送，速度快。
- **读取**：SDA 单线只写，不支持回读像素，因此没有 `getPixel()`。

---

## 坐标与颜色

```
(0,0) ──────────────► x
  │
  ▼  y
```

旋转 0（默认）下逻辑尺寸 128(宽)×160(高)。可用 `ST7735_setRotation()` 改为横屏。

颜色为 RGB565，用宏构造：

```c
uint16_t c = ST7735_RGB565(255, 128, 0);   // 橙色
```

预定义颜色：`ST7735_BLACK / WHITE / RED / GREEN / BLUE / YELLOW / CYAN /
MAGENTA / ORANGE / GRAY / NAVY / DARKGREEN / MAROON / PURPLE`。

---

## 初始化与全局控制

### `ST7735_init`
```c
void ST7735_init(void);
```
硬件复位 → 运行 ST7735S 启动序列 → 设为 RGB565 → 开显示 → 开背光 → 清屏为黑。
**必须在 `SYSCFG_DL_init()` 之后调用一次。**

### `ST7735_setRotation`
```c
void ST7735_setRotation(uint8_t rotation);  // 0..3
```
| 值 | 方向 | 逻辑尺寸 |
|----|------|---------|
| 0 | 竖屏（默认） | 128×160 |
| 1 | 横屏 | 160×128 |
| 2 | 竖屏翻转 | 128×160 |
| 3 | 横屏翻转 | 160×128 |

### `ST7735_width` / `ST7735_height`
```c
uint16_t ST7735_width(void);
uint16_t ST7735_height(void);
```
返回当前旋转下的逻辑宽/高。

### `ST7735_backlight` / `ST7735_invertDisplay`
```c
void ST7735_backlight(bool on);      // 背光开关
void ST7735_invertDisplay(bool inv); // 反色显示
```

---

## 填充与清屏

```c
void ST7735_fillScreen(uint16_t color);  // 整屏填充
void ST7735_clear(void);                 // = fillScreen(BLACK)
```

---

## 像素

```c
void ST7735_drawPixel(int16_t x, int16_t y, uint16_t color);
```
越界自动忽略。

---

## 图形绘制

> 所有图形函数直接绘制到屏幕，无需额外刷新。越界部分自动裁剪。

```c
void ST7735_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void ST7735_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void ST7735_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void ST7735_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void ST7735_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void ST7735_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void ST7735_fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void ST7735_drawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
void ST7735_fillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
void ST7735_drawTriangle(int16_t x0,int16_t y0,int16_t x1,int16_t y1,int16_t x2,int16_t y2,uint16_t color);
void ST7735_fillTriangle(int16_t x0,int16_t y0,int16_t x1,int16_t y1,int16_t x2,int16_t y2,uint16_t color);
```

- `fillRect` / `fillScreen` / `drawFastHLine` / `drawFastVLine` 走地址窗口批量推送，性能最优。
- 直线为 Bresenham，圆为中点算法，填充圆/圆角矩形以水平/垂直扫描线实现。

---

## 文本输出（内置 5×7 字体，6×8 字格）

字体覆盖 ASCII 0x20–0x7E（95 个字符）。坐标单位为**像素**。

```c
void ST7735_setCursor(int16_t x, int16_t y);          // 下一个字符左上角像素坐标
void ST7735_setTextColor(uint16_t fg, uint16_t bg);   // 前景/背景色；fg==bg 时背景透明
void ST7735_setTextSize(uint8_t size);                // 整数倍放大（1,2,3,...）
void ST7735_setTextWrap(bool wrap);                   // 行尾是否自动换行
void ST7735_drawChar(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t size);
void ST7735_writeChar(char c);                        // 用光标状态绘制，支持 \n \r
void ST7735_writeString(const char *str);
void ST7735_printf(const char *fmt, ...);             // 内部 vsnprintf，缓冲 80 字节
```

> 透明背景（`fg == bg`）只绘制前景像素，保留底图，但速度较慢（逐像素）。
> 不透明背景会整格填充，速度更快。

```c
ST7735_setTextColor(ST7735_YELLOW, ST7735_BLACK);
ST7735_setTextSize(2);
ST7735_setCursor(0, 20);
ST7735_printf("T=%d.%02d C", 25, 37);   // 输出 "T=25.37 C"
```

---

## 图像显示

### `ST7735_drawRGBBitmap` — 全彩图像
```c
void ST7735_drawRGBBitmap(int16_t x, int16_t y, int16_t w, int16_t h,
                          const uint16_t *img);
```
绘制 RGB565 全彩图像。数据按**行优先**排列，每像素一个 `uint16_t`：
```
img[row * w + col] = RGB565 像素
```
完全在屏内时走单次地址窗口流式传输（最快）；部分越界时逐像素裁剪绘制。
使用 `tools/img2st7735.py` 生成数组。

### `ST7735_drawBitmap` — 单色位图
```c
void ST7735_drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h,
                       const uint8_t *bmp, uint16_t fg, uint16_t bg);
```
1bpp 单色位图，MSB 在前，每行按字节对齐（行跨度 `(w+7)/8` 字节）。
置位像素绘制为 `fg`，清零像素绘制为 `bg`；若 `fg == bg` 则清零像素透明。

---

## 底层窗口接口（高级）

```c
void ST7735_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ST7735_pushColor(uint16_t color, uint32_t count);
```
自定义渲染时使用：先用 `setAddrWindow` 打开矩形区域，再 `pushColor` 推送像素。
调用前后由上层负责 CS 时序（参考 `st7735.c` 内部用法）。

---

## 图像取模工具 `tools/img2st7735.py`

```bash
python3 tools/img2st7735.py <图片> [-o 输出名] [-n 数组名] \
        [-W 宽] [-H 高] [--fit contain|stretch|cover] [-p]
```
- 默认输出 128×160；`--fit cover` 裁剪铺满、`contain` 保持比例留黑边、`stretch` 拉伸。
- 生成 `<名>.h`，含 `static const uint16_t` 数组与 `_WIDTH/_HEIGHT` 宏。

```c
#include "tools/st_image.h"
ST7735_drawRGBBitmap(0, 0, GIMAGE_WIDTH, GIMAGE_HEIGHT, gImage);
```

> 注意 flash 占用：全屏 128×160 图像 = 40 KB（MSPM0G3507 有 128 KB flash，单张可放，多张需取舍）。

---

## 典型使用流程

```c
#include "ti_msp_dl_config.h"
#include "st7735.h"

int main(void)
{
    SYSCFG_DL_init();
    ST7735_init();                       // 初始化 + 清屏

    ST7735_setTextColor(ST7735_WHITE, ST7735_BLACK);
    ST7735_setCursor(0, 0);
    ST7735_writeString("Hello ST7735S!");

    ST7735_drawRect(0, 20, 128, 60, ST7735_RED);
    ST7735_fillCircle(64, 110, 30, ST7735_BLUE);
    ST7735_drawLine(0, 159, 127, 0, ST7735_GREEN);

    while (1) {}
}
```

---

## 调整说明

- **偏移**：部分模块存在行/列偏移。若边缘出现错位像素，调整 `st7735.c` 中
  `COLSTART` / `ROWSTART`。
- **颜色顺序**：若红蓝对调，将初始化中 `MADCTL` 的 `MADCTL_BGR` 改为 `MADCTL_RGB`。
- **镜像/方向**：用 `ST7735_setRotation()` 调整，或修改 `MADCTL` 的 MX/MY/MV 位。
