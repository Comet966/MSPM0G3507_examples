# PD42S1 双轴云台驱动 (可移植)

正点原子 **PD42S1 闭环步进驱动器** 的串口/RS485 控制接口。核心协议层零 MCU 依赖，
收发经 `pd42_bus_t` 回调解耦，可拖进任意 MSPM0 / STM32 工程。

## 文件

| 文件 | 作用 |
|------|------|
| `PD42S1.h/.c` | 协议层：组帧、校验和、大端打包、运动/配置/读取命令 |
| `PD42S1_Gimbal.h/.c` | 双轴 Pan/Tilt 角度层：度 ↔ 51200/圈 脉冲换算、方向/减速比 |

## 协议要点（已对官方 STM32 例程逐字节核实）

- 帧：`0xC5 | addr | code | data… | checksum | 0x5C`
- `checksum = (帧头 + 地址 + 功能码 + 全部数据字节) & 0xFF`
- 多字节整数**大端**；float 亦**大端** 4 字节（`b[3] b[2] b[1] b[0]`）
- **51200 counts = 电机轴一圈**（与细分无关，协议固定）
- 回复：`0xC5 addr code err data… cs 0x5C`，`err=0x01` 成功
- 广播/分组地址（0x00）从机**不应答**——读取命令必须用真实地址

## 双机 RS485 接法

两台 PD42S1 用**不同从机地址**（如 0x01=Pan、0x02=Tilt）挂同一条 A/B 总线，
共用一个 `pd42_bus_t`。改地址：先单独接一台，`pd42`（协议层）用 `0x60` 功能码
或直接用驱动器按键屏设好并 `pd42_param_save`。

MCU 侧需一个 **DE/RE 方向 GPIO**：发送前拉高、发完拉低回接收。

## 移植：实现 bus 回调

```c
/* --- MSPM0 参考实现（RS485 半双工，DE = 某 GPIO） --- */
#include "ti_msp_dl_config.h"

static int rs485_write(void *ctx, const uint8_t *buf, uint16_t len) {
    (void)ctx;
    DL_GPIO_setPins(RS485_PORT, RS485_DE_PIN);        /* DE=发送 */
    for (uint16_t i = 0; i < len; i++) {
        DL_UART_Main_transmitDataBlocking(UART_MOTOR_INST, buf[i]);
    }
    while (DL_UART_isBusy(UART_MOTOR_INST)) { }       /* 等移位寄存器清空 */
    DL_GPIO_clearPins(RS485_PORT, RS485_DE_PIN);      /* DE=接收 */
    return len;
}

/* read：从 UART RX 环形缓冲里取应答（需你的 UART RX ISR 填充）。
 * 只发不收可传 NULL，读取类 API 即不可用。 */
static int rs485_read(void *ctx, uint8_t *buf, uint16_t max, uint32_t timeout_ms) {
    (void)ctx;
    return uart_motor_read(buf, max, timeout_ms);     /* 你的实现 */
}

static const pd42_bus_t g_bus = { rs485_write, rs485_read, 0 };
```

## 用法

```c
pd42_gimbal_t gimbal;

void gimbal_setup(void) {
    /* addr, 减速比(直驱=1), 方向符号(±1), 位置模式最大RPM, 加减速(0~200) */
    pd42_axis_config(&gimbal.pan,  &g_bus, 0x01, 1.0f, +1, 300, 10);
    pd42_axis_config(&gimbal.tilt, &g_bus, 0x02, 1.0f, +1, 300, 10);
    pd42_gimbal_init(&gimbal, 16);          /* 通信位置模式 + 细分16 + 使能 */
    pd42_gimbal_set_origin(&gimbal);        /* 当前姿态定为 0/0 */
}

void loop(void) {
    pd42_gimbal_move_pan(&gimbal, 45.0f);   /* Pan 转到 +45° */
    pd42_gimbal_move_tilt(&gimbal, -20.0f); /* Tilt 转到 -20° */

    float a;
    if (pd42_gimbal_read_pan(&gimbal, &a, 50)) { /* 回读真实角度 */ }
    if (pd42_gimbal_arrived(&gimbal, 50))      { /* 两轴到位 */ }
}
```

## 视觉闭环接入

要接 K230 像素误差闭环，在收到一帧视觉数据时用相对移动喂增量即可：

```c
pd42_gimbal_move_by_pan(&gimbal,  -kp_pan  * pixel_err_x);
pd42_gimbal_move_by_tilt(&gimbal, -kp_tilt * pixel_err_y);
```

`kp_*` 为像素→度增益，符号按实际方向用 `pd42_axis_config` 的 `sign` 或增益正负校准。

## 方向/减速比标定

1. `sign`：先给 +10° 相对移动，若转反了把该轴 `sign` 取反。
2. `gear`：给云台轴一个已知机械角，测实际转角，`gear = 目标角 / 实测角` 修正。
3. 直驱（电机轴=云台轴）`gear=1.0`。

## 硬件接线

### 方案 A：直连串口（无 RS485 收发器）

**适用场景**：线长 < 30 cm，手边无 RS485 收发器芯片，快速调试。

| 连接 | MCU 引脚 | PD42S1 端子 |
|------|----------|------------|
| 发送 | **PA8**（UART1 TX）| A 端子 |
| 接收 | **PA9**（UART1 RX）| A 端子（与 TX 共线）|
| 地线 | **GND** | B 端子 → GND |

```text
LP-MSPM0G3507          PD42S1 #1 (Pan  0x01)   PD42S1 #2 (Tilt 0x02)
──────────────────────────────────────────────────────────────────────
 PA8 TX ────┬────────── A 端子 ────────────────── A 端子
 PA9 RX ────┘
 GND ─────────────────── B 端子 ────────────────── B 端子 → GND
```

- PA8 与 PA9 **短接后**一起接到 PD42S1 的 A 端子（单端不平衡方式）。
- B 端子接 GND，构成信号参考。
- 两台驱动器 A/B 并联在同一对线，不同地址（0x01 / 0x02）靠协议区分。
- 线长 > 30 cm 或通信不稳时，升级为方案 B。

### 方案 B：经 RS485 收发器（推荐，长线稳定）

所需器件：MAX485 / SP3485 / SN65HVD3082（任选，3.3V 兼容）

| MCU 信号 | MCU 引脚 | MAX485 引脚 | 说明 |
|----------|----------|------------|------|
| UART1_TX | **PA8** | DI（pin 4）| 串行数据发送 |
| UART1_RX | **PA9** | RO（pin 1）| 串行数据接收 |
| RS485_DE | 任一 GPIO | DE（pin 3）+ /RE（pin 2）并接 | 方向控制 |
| GND | GND | GND（pin 5）| 共地 |
| 3.3V | 3.3V | VCC（pin 8）| 收发器供电 |

> 使用方案 B 时需在 SysConfig 重新添加 GPIO 实例（PB16 或其他引脚），并在 `rs485_bus.c` 的 `bus_write` 中恢复 DE 引脚翻转。

### LP-MSPM0G3507 LaunchPad 引脚位置

| MCU 引脚 | LP 头排位置 | 说明 |
|----------|------------|------|
| PA8 | J3 pin 4（左侧内排）| UART1 TX |
| PA9 | J3 pin 3（左侧内排）| UART1 RX |
| GND | J3 pin 2 / J4 pin 22 | 任一接地引出 |

> **J14 跳线（PA9 / PB23 切换）**：出厂默认 J14 将该排针连到 **PB23**；  
> 使用 UART1 RX（PA9）时**必须将 J14 拨到 PA9 一侧**，否则接收不通。  
> 详见 [LP-MSPM0G3507 用户指南](https://www.ti.com/lit/slau873) Figure 2-1。

### 供电注意

1. **MCU 3.3V** 与 **PD42S1 主电源（24~36V）** 独立供电，不共电源。
2. **必须共地**：MCU GND 与 PD42S1 GND 连在一起（统一参考电位）。
3. 先上 MCU 电，再上驱动器主电。
