# mcu_yuntai 接口说明

## 硬件配置

| 外设 | SysConfig 实例 | 引脚 | 连接 |
|------|---------------|------|------|
| UART1 | `UART1_PAN` | PA8 TX / PA9 RX | Pan PD42S1 A 端子 |
| UART0 | `UART0_TILT` | PA28 TX / PA31 RX | Tilt PD42S1 A 端子 |
| I2C1 | `I2C_0` | PB2 SCL / PB3 SDA | OLED SSD1306 |
| GND | — | GND | 两台电机 B 端子各接 GND |

> PA9 对应 BoosterPack pin 3，LP-MSPM0G3507 的 J14 默认接 PB23，需拨到 **PA9** 侧。  
> 两台电机均使用出厂默认地址 **0x01**，各自在独立串口上无需区分地址。

---

## 模块层次

```
empty.c  (应用层 demo)
   └─ Motor/PD42S1_Gimbal.h/.c  (双轴角度层)
         └─ Motor/PD42S1.h/.c   (协议层，零 TI 依赖)
               └─ Motor/rs485_bus.h/.c  (MSPM0 UART transport)
   └─ oled.h/.c                 (SSD1306 I2C 显示)
```

---

## rs485_bus

**文件**：`Motor/rs485_bus.h` / `Motor/rs485_bus.c`

UART transport 层，将 MSPM0 两路 UART 封装为 `pd42_bus_t` 回调对象。

```c
void rs485_bus_init(void);
```
初始化两路 UART 的 RX 环形缓冲并使能 NVIC 中断。必须在 `SYSCFG_DL_init()` 之后调用。

```c
const pd42_bus_t *rs485_get_bus_pan(void);
const pd42_bus_t *rs485_get_bus_tilt(void);
```
分别返回 Pan 轴（UART1，PA8/PA9）和 Tilt 轴（UART0，PA28/PA31）的总线句柄，传给 `pd42_axis_config`。

---

## PD42S1（协议层）

**文件**：`Motor/PD42S1.h` / `Motor/PD42S1.c`

零 MCU 依赖的 PD42S1 串口帧协议实现，收发经 `pd42_bus_t` 回调解耦，可移植到任意平台。

### pd42_bus_t

```c
typedef struct {
    int  (*write)(void *ctx, const uint8_t *buf, uint16_t len);
    int  (*read )(void *ctx, uint8_t *buf, uint16_t max, uint32_t timeout_ms);
    void  *ctx;
} pd42_bus_t;
```

- `write`：阻塞发出整帧，返回发送字节数。RS485 需在其中翻转 DE 引脚。
- `read`：在 `timeout_ms` 内读取应答字节到 `buf`，返回字节数。仅读取类 API 使用；只发不收可传 `NULL`。
- `ctx`：透传给回调的用户上下文（本工程传 `0` 不用）。

### 初始化

```c
void pd42_init(pd42_t *m, const pd42_bus_t *bus, uint8_t addr);
```

### 运动控制（只发，不等应答）

| 函数 | 说明 |
|------|------|
| `pd42_enable(m, true/false)` | 使能 / 失能电机 |
| `pd42_stop(m)` | 立即刹车 |
| `pd42_zero_angle(m)` | 当前位置清零（协议清零，不移动） |
| `pd42_clear_state(m)` | 清除堵转 / 刹车 / 失能状态 |
| `pd42_speed(m, dir, acc, rpm)` | 速度模式：方向、加减速、转速 |
| `pd42_pos_abs(m, dir, acc, rpm, counts)` | 绝对位置（counts，51200=一圈） |
| `pd42_pos_rel(m, dir, acc, rpm, counts)` | 相对位置 |
| `pd42_torque(m, dir, ma)` | 力矩模式（mA） |

### 配置（开机时下发一次）

| 函数 | 说明 |
|------|------|
| `pd42_set_mode(m, mode)` | 工作模式，见 `pd42_mode_t` |
| `pd42_set_microstep(m, step)` | 细分 1~256（51200/圈与细分无关） |
| `pd42_set_pos_torque(m, ma)` | 位置环最大力矩 |
| `pd42_param_save(m)` | 保存参数到驱动器 flash |

### 读取（阻塞，需 bus->read 有效）

```c
bool pd42_read_pos(pd42_t *m, int32_t *counts, uint32_t timeout_ms);
bool pd42_read_speed(pd42_t *m, int16_t *rpm, uint32_t timeout_ms);
bool pd42_read_run_state(pd42_t *m, uint8_t *state, uint32_t timeout_ms);
bool pd42_read_arrived(pd42_t *m, bool *arrived, uint32_t timeout_ms);
```

成功返回 `true` 并填写输出参数；超时或校验失败返回 `false`。

---

## PD42S1_Gimbal（双轴角度层）

**文件**：`Motor/PD42S1_Gimbal.h` / `Motor/PD42S1_Gimbal.c`

在协议层之上封装"度"↔"counts"换算和 Pan/Tilt 双轴管理。

> counts = angle_deg / 360 × 51200 × gear

### 典型初始化流程

```c
pd42_gimbal_t gimbal;

SYSCFG_DL_init();
rs485_bus_init();

// 参数: 总线句柄, 地址, 减速比(直驱=1.0), 方向(±1), 最大RPM, 加减速(0~200)
pd42_axis_config(&gimbal.pan,  rs485_get_bus_pan(),  0x01, 1.0f, +1, 300, 10);
pd42_axis_config(&gimbal.tilt, rs485_get_bus_tilt(), 0x01, 1.0f, +1, 300, 10);

pd42_gimbal_init(&gimbal, 16);   // 设通信位置模式 + 细分16 + 使能
pd42_gimbal_set_origin(&gimbal); // 当前姿态定为 0/0
```

### pd42_axis_config

```c
void pd42_axis_config(pd42_axis_t *ax, const pd42_bus_t *bus, uint8_t addr,
                      float gear, int8_t sign, uint16_t rpm, uint8_t acc);
```

| 参数 | 说明 |
|------|------|
| `bus` | `rs485_get_bus_pan()` 或 `rs485_get_bus_tilt()` |
| `addr` | 电机从机地址（本工程两台均为 `0x01`） |
| `gear` | 减速比，直驱填 `1.0f` |
| `sign` | 正方向翻转：`+1` 或 `-1`，上板后按实际方向标定 |
| `rpm` | 位置模式最大转速（RPM） |
| `acc` | 加减速 `0`~`200`，`0` 为直接启动 |

### 运动接口

```c
// 绝对角度（度），非阻塞
void pd42_gimbal_move_pan(pd42_gimbal_t *g, float deg);
void pd42_gimbal_move_tilt(pd42_gimbal_t *g, float deg);

// 相对角度（度），非阻塞
void pd42_gimbal_move_by_pan(pd42_gimbal_t *g, float deg);
void pd42_gimbal_move_by_tilt(pd42_gimbal_t *g, float deg);

void pd42_gimbal_home(pd42_gimbal_t *g);         // 回到 0/0
void pd42_gimbal_stop(pd42_gimbal_t *g);         // 两轴立即刹车
void pd42_gimbal_enable(pd42_gimbal_t *g, bool); // 使能 / 失能
void pd42_gimbal_set_origin(pd42_gimbal_t *g);   // 当前位置定为 0/0
```

### 读取接口

```c
// 从驱动器回读实时角度，成功返回 true
bool pd42_gimbal_read_pan(pd42_gimbal_t *g, float *deg, uint32_t timeout_ms);
bool pd42_gimbal_read_tilt(pd42_gimbal_t *g, float *deg, uint32_t timeout_ms);

// 软件记账的目标角度（无通信）
float pd42_gimbal_get_pan(const pd42_gimbal_t *g);
float pd42_gimbal_get_tilt(const pd42_gimbal_t *g);

// 两轴是否都到位（需通信）
bool pd42_gimbal_arrived(pd42_gimbal_t *g, uint32_t timeout_ms);
```

### 方向 / 减速比标定

1. **sign**：调用 `pd42_gimbal_move_by_pan(&g, +10.0f)`，若实际转向与期望相反，将该轴 `sign` 改为 `-1`。
2. **gear**：给定一个已知机械角，实测实际转角，`gear = 目标角 / 实测角`。

---

## OLED

**文件**：`oled.h` / `oled.c`

SSD1306 128×64 I2C 驱动，双缓冲，只刷脏页。

```c
void OLED_init(void);              // 初始化，SYSCFG_DL_init() 后调用
void OLED_clear(void);             // 清空帧缓冲（不立即刷屏）
void OLED_display(void);           // 将帧缓冲脏页写入屏幕
void OLED_markDirty(void);         // 强制标记全部页为脏（重绘用）
```

#### 文本接口（8×8 点阵，一页 = 8 像素高）

```c
void OLED_setCursor(uint8_t col, uint8_t page);  // col: 0~127, page: 0~7
void OLED_writeChar(char c);
void OLED_writeString(const char *str);
void OLED_printf(const char *fmt, ...);          // 格式化输出，不自动换行
```

#### 绘图接口（写入帧缓冲，需 `OLED_display()` 刷屏）

```c
void OLED_setPixel(uint8_t x, uint8_t y, bool on);
void OLED_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on);
void OLED_drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on);
void OLED_fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on);
void OLED_drawCircle(uint8_t cx, uint8_t cy, uint8_t r, bool on);
void OLED_fillCircle(uint8_t cx, uint8_t cy, uint8_t r, bool on);
void OLED_drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                     const uint8_t *bmp);  // SSD1306 竖排格式位图
```

> OLED 地址固定 `0x3C`（SA0 接 GND）。若模块 SA0 接 VCC，修改 `oled.c` 中 `OLED_ADDR` 为 `0x3D`。
