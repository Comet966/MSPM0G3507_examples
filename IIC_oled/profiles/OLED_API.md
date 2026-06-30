# OLED 驱动接口文档

**目标硬件**: 中景园 0.96寸 SSD1306 128×64 单色 OLED（IIC 接口）  
**MCU**: MSPM0G3507（LP-MSPM0G3507 LaunchPad）  
**头文件**: `oled.h`  
**实现文件**: `oled.c`

---

## 硬件参数

| 参数 | 值 |
|------|-----|
| 分辨率 | 128 × 64 像素 |
| 驱动芯片 | SSD1306 |
| I2C 7-bit 地址 | 0x3C（D/C# 接 GND，默认） |
| I2C 速率 | 400 kHz Fast Mode |
| SCL | PB2（BoosterPack Pin 9） |
| SDA | PB3（BoosterPack Pin 10） |
| SysConfig 外设 | I2C1，实例名 `I2C_0` |

---

## 坐标系

```
(0,0) ──────────────────► x (0–127)
  │
  │
  ▼
  y (0–63)
```

帧缓冲按 SSD1306 原生格式存储：8 个 page，每 page 8 行，字节低位对应上方像素。

---

## 常量

```c
#define OLED_WIDTH   128   // 像素列数
#define OLED_HEIGHT   64   // 像素行数
#define OLED_PAGES     8   // 64 / 8
```

---

## 初始化与缓冲控制

### `OLED_init`
```c
void OLED_init(void);
```
初始化 I2C 中断、发送 SSD1306 启动序列、清空帧缓冲并全屏刷新。  
**必须在 `SYSCFG_DL_init()` 之后调用一次。**

---

### `OLED_clear`
```c
void OLED_clear(void);
```
清空帧缓冲（全 0），并将所有 page 标记为脏。  
需调用 `OLED_display()` 才会实际刷新屏幕。

---

### `OLED_display`
```c
void OLED_display(void);
```
将帧缓冲中**被标记为脏**的 page 通过 I2C 写入 OLED。  
未修改的 page 不发送，节省总线时间。

> 性能：最优情况（只改 1 page）= 7 + 129 = 136 字节 I2C 传输。  
> 最差情况（全屏）= 8 × 136 = 1088 字节。

---

### `OLED_markDirty`
```c
void OLED_markDirty(void);
```
强制将所有 8 个 page 标记为脏。下次 `OLED_display()` 将全屏刷新。  
用于恢复被外部破坏的显示状态。

---

## 像素操作

### `OLED_setPixel`
```c
void OLED_setPixel(uint8_t x, uint8_t y, bool on);
```
设置或清除单个像素。越界自动忽略。  
自动标记所在 page 为脏。

| 参数 | 范围 | 说明 |
|------|------|------|
| `x` | 0–127 | 列坐标 |
| `y` | 0–63 | 行坐标 |
| `on` | true/false | true = 亮，false = 灭 |

---

### `OLED_getPixel`
```c
bool OLED_getPixel(uint8_t x, uint8_t y);
```
读取帧缓冲中指定像素的状态。越界返回 `false`。

---

## 图形绘制

> 所有图形函数只操作帧缓冲，**不自动刷新屏幕**，绘制完成后需调用 `OLED_display()`。

### `OLED_drawLine`
```c
void OLED_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on);
```
Bresenham 直线算法，任意方向。

---

### `OLED_drawRect`
```c
void OLED_drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on);
```
空心矩形，左上角 `(x, y)`，宽 `w`，高 `h`。

---

### `OLED_fillRect`
```c
void OLED_fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on);
```
实心矩形。内部按字节操作帧缓冲（位掩码），**不走逐像素循环**，性能优于逐行调用 `setPixel`。

---

### `OLED_drawCircle`
```c
void OLED_drawCircle(uint8_t cx, uint8_t cy, uint8_t r, bool on);
```
中点画圆算法，空心圆，圆心 `(cx, cy)`，半径 `r`。

---

### `OLED_fillCircle`
```c
void OLED_fillCircle(uint8_t cx, uint8_t cy, uint8_t r, bool on);
```
实心圆，内部用 `fillRect` 按水平扫描线填充。

---

### `OLED_drawBitmap`
```c
void OLED_drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                     const uint8_t *bmp);
```
在 `(x, y)` 处绘制位图，以 OR 方式合并到帧缓冲（不清除背景）。

**位图格式**（SSD1306 原生垂直格式）：
```
bmp[page * w + col]
```
- page：图像高度方向的分页，每页 8 行
- col：图像宽度方向的列索引
- 每字节低位对应上方像素

支持非 8 的倍数 y 坐标（内部做位移合并）。

| 参数 | 说明 |
|------|------|
| `x` | 左上角列坐标 |
| `y` | 左上角行坐标 |
| `w` | 位图宽度（像素） |
| `h` | 位图高度（像素） |
| `bmp` | 位图数据指针（`const`，建议放 flash） |

---

## 文本输出（8×8 像素字体）

字体支持 ASCII 0x20–0x7E（共 95 个字符）。  
光标单位为**字符格**：列 0–15（每格 8 像素宽），page 0–7。

---

### `OLED_setCursor`
```c
void OLED_setCursor(uint8_t col, uint8_t page);
```
设置文本光标位置。越界自动归 0。

| 参数 | 范围 | 说明 |
|------|------|------|
| `col` | 0–15 | 字符列（0 = 最左） |
| `page` | 0–7 | 字符行（0 = 最上） |

---

### `OLED_writeChar`
```c
void OLED_writeChar(char c);
```
在当前光标处写入一个字符并将光标右移。  
支持 `\n`（换行）和 `\r`（回车）。不可打印字符显示为 `?`。  
到达行尾自动换行，到达屏幕底部回绕到第 0 页。

---

### `OLED_writeString`
```c
void OLED_writeString(const char *str);
```
逐字符调用 `OLED_writeChar`，直到字符串结束符。

---

### `OLED_printf`
```c
void OLED_printf(const char *fmt, ...);
```
格式化输出，内部使用 `vsnprintf`，缓冲区 64 字节。  
用法与标准 `printf` 相同，超出 64 字节部分被截断。

```c
OLED_printf("T=%d.%02d C", 25, 37);  // 输出 "T=25.37 C"
```

---

## 典型使用流程

```c
// 1. 系统初始化
SYSCFG_DL_init();
OLED_init();

// 2. 文本显示
OLED_setCursor(0, 0);
OLED_writeString("Hello OLED!");
OLED_display();

// 3. 图形绘制
OLED_clear();
OLED_drawRect(0, 0, 128, 64, true);   // 外框
OLED_fillCircle(64, 32, 20, true);    // 实心圆
OLED_setCursor(4, 6);
OLED_printf("r=%d", 20);
OLED_display();

// 4. 像素级操作
OLED_setPixel(10, 10, true);
OLED_setPixel(10, 11, true);
OLED_display();
```

---

## 内部架构说明

### 帧缓冲
```c
static uint8_t gFb[8][128];  // 1 KB SRAM
static uint8_t gDirty;       // 8-bit 脏页掩码，bit i = page i 有修改
```

### 脏页追踪
每次修改帧缓冲时对应 bit 置 1，`OLED_display()` 只发送 bit 为 1 的 page，发完后清除该 bit。

### I2C 传输
中断驱动，8 字节 FIFO 分批填充：
- 发送前预填充最多 8 字节
- `TXFIFO_TRIGGER` 中断中继续填充剩余字节
- `TX_DONE` 中断置完成标志
- 主函数 `__WFE()` 等待完成，不轮询 CPU

### SSD1306 寻址模式
初始化设为**水平寻址模式**（`0x20, 0x00`），每次刷新 page 时重新设置列/页地址窗口，确保定位准确。
