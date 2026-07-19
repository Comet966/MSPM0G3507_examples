# STM32F103C8T6 红外接收机迁移指南

本文说明如何将另一个 STM32F103C8T6 项目配置为本项目的红外信号接收机。

当前发送机使用的是 NEC 时序的自定义 32 位数据帧，用于传输两个 12 位 ADC 坐标值。接收机必须使用相同的载波频率、脉宽定义、位顺序和 CRC 算法。

## 1. 系统结构

发送机负责：

- 读取 ADC X/Y 轴；
- 生成 38 kHz 红外载波；
- 发送 9 ms 引导码、4.5 ms 空间和 32 位数据；
- 摇杆移动时约每 120 ms 发送一帧。

接收机负责：

- 红外接收头输出边沿捕获；
- 使用 TIM3 测量高低电平时间；
- 软件切换输入捕获极性，实现双边沿捕获；
- 解码 X/Y 和 CRC8；
- 将有效数据交给主循环或控制器。

接收机不需要配置 TIM2 PWM，也不需要连接红外发射 LED。

## 2. 硬件连接

### 2.1 红外接收头

常见三脚红外接收头连接如下：

| 接收头引脚 | STM32F103C8T6 | 说明 |
|---|---|---|
| VCC | 3.3 V | 不建议直接使用 5 V 输出到 MCU |
| GND | GND | 必须共地 |
| OUT | PA6 / TIM3_CH1 | 解调后的数字电平 |

多数红外接收头的 OUT 空闲为高电平、收到红外信号时为低电平。不同型号可能存在极性差异，若波形相反，需要调整初始捕获极性和解码状态机。

建议在 VCC 与 GND 之间靠近接收头放置 `0.1 uF` 去耦电容。

### 2.2 接收机 OLED（可选）

如果接收机需要显示接收到的坐标：

| OLED 信号 | STM32F103C8T6 |
|---|---|
| SCL | PB6 / I2C1_SCL |
| SDA | PB7 / I2C1_SDA |
| VCC | 3.3 V |
| GND | GND |

HAL 中 OLED 写地址使用 `0x78`，它对应 7 位地址 `0x3C` 左移一位。如果模块实际地址为 `0x3D`，HAL 地址应改为 `0x7A`。

## 3. CubeMX 配置

### 3.1 时钟

当前发送机和接收机均使用：

- HSE：8 MHz；
- PLL：×9；
- SYSCLK：72 MHz；
- APB1：36 MHz；
- TIM3 时钟：72 MHz。

如果使用相同的时钟树，TIM3 配置为：

```text
Prescaler = 71
Counter Period = 65535
Counter Mode = Up
```

因为 `72 MHz / (71 + 1) = 1 MHz`，所以 TIM3 计数器每增加 1 就代表约 1 us。

如果接收机的 TIM3 时钟不是 72 MHz，应按下面公式重新计算：

```text
Prescaler = TIM3_Clock_Hz / 1_000_000 - 1
```

### 3.2 PA6 输入捕获

在 CubeMX 中配置：

```text
PA6        = TIM3_CH1
Channel    = Input Capture direct mode
Polarity   = Rising Edge
Prescaler  = 71
Period     = 65535
Interrupt  = TIM3 global interrupt enabled
```

初始使用上升沿是因为常见接收头在 9 ms 低电平结束时产生第一个上升沿。代码收到一次捕获后，会直接修改 `TIM3->CCER` 的 `CC1P` 位捕获下一种边沿。

### 3.3 NVIC

启用：

```text
TIM3 global interrupt = Enabled
Priority              = 0 或其他较高优先级
```

不要在 TIM3 捕获回调中执行 OLED 刷新、`HAL_Delay()` 或大量计算。中断只负责测量和组帧，显示和控制逻辑放在主循环。

### 3.4 保留 SWD

不要在 `HAL_MspInit()` 中关闭 SWD。应使用：

```c
__HAL_AFIO_REMAP_SWJ_NOJTAG();
```

不要使用：

```c
__HAL_AFIO_REMAP_SWJ_DISABLE();
```

后者会同时关闭 JTAG 和 SWD，可能导致后续 ST-LINK 无法读取 Core ID。

## 4. 数据帧格式

红外载波为约 38 kHz，占空比约 33%。数据帧采用 NEC 风格的脉宽编码，但数据内容不是标准 NEC 的地址反码结构。

### 4.1 时序

```text
引导码：9,000 us mark + 4,500 us space
数据位：560 us mark + 560 us space       表示 0
数据位：560 us mark + 1,690 us space     表示 1
结束码：560 us mark
```

32 位数据按 LSB first 发送。

接收判断范围建议设置为：

```text
引导低电平：8,500 ~ 9,500 us
引导高电平：4,000 ~ 5,000 us
数据 mark：350 ~ 800 us
数据 0 space：350 ~ 800 us
数据 1 space：1,200 ~ 2,000 us
```

### 4.2 32 位内容

```text
Byte0 = X[7:0]
Byte1 = X[11:8] | (Y[3:0] << 4)
Byte2 = Y[11:4]
Byte3 = CRC8(Byte0, Byte1, Byte2)
```

还原公式：

```c
x = byte0 | ((byte1 & 0x0F) << 8);
y = (byte1 >> 4) | (byte2 << 4);
```

X/Y 均为 `0~4095` 的 12 位 ADC 值。

### 4.3 CRC8

当前使用 CRC8：

```text
Polynomial = 0x07
Initial    = 0x00
Input      = Byte0, Byte1, Byte2
```

参考实现：

```c
static uint8_t IR_CRC8(const uint8_t *data, uint8_t length)
{
    uint8_t crc = 0;

    while (length--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; ++i) {
            crc = (crc & 0x80U)
                ? (uint8_t)((crc << 1) ^ 0x07U)
                : (uint8_t)(crc << 1);
        }
    }
    return crc;
}
```

## 5. 接收机代码

### 5.1 全局变量

放在接收机应用源文件中：

```c
extern TIM_HandleTypeDef htim3;

volatile uint16_t rx_x;
volatile uint16_t rx_y;
volatile uint8_t rx_flag;
```

### 5.2 初始化

在 `MX_TIM3_Init()` 完成后启动输入捕获中断：

```c
HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
```

不要调用：

```c
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
```

接收机没有必要启动红外载波输出。

### 5.3 双边沿捕获和解码

以下代码可以放在接收机的应用源文件中。它与当前发送机的数据帧格式对应：

```c
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    static uint16_t previous;
    static uint8_t state;
    static uint8_t bit_count;
    static uint32_t frame;

    if (htim != &htim3 ||
        htim->Channel != HAL_TIM_ACTIVE_CHANNEL_1) {
        return;
    }

    uint16_t now = (uint16_t)HAL_TIM_ReadCapturedValue(
        htim, TIM_CHANNEL_1);
    uint16_t width = (uint16_t)(now - previous);
    previous = now;

    /* CC1P=0: current capture is rising edge; CC1P=1: falling edge. */
    uint8_t rising = (TIM3->CCER & TIM_CCER_CC1P) == 0U;

    /* Emulate double-edge capture. */
    TIM3->CCER ^= TIM_CCER_CC1P;

    if (state == 0U) {
        /* End of the 9 ms leading mark. */
        if (rising && width >= 8500U && width <= 9500U) {
            state = 1U;
        }
        return;
    }

    if (state == 1U) {
        /* End of the 4.5 ms leading space. */
        if (!rising && width >= 4000U && width <= 5000U) {
            state = 2U;
            bit_count = 0U;
            frame = 0U;
        } else {
            state = 0U;
        }
        return;
    }

    if (state == 2U) {
        /* End of a 560 us data mark. */
        if (rising && width >= 350U && width <= 800U) {
            state = 3U;
        } else {
            state = 0U;
        }
        return;
    }

    /* Falling edge ends the data space and determines the bit value. */
    if (!rising) {
        if (width >= 1200U && width <= 2000U) {
            frame |= 1UL << bit_count;
        } else if (width < 350U || width > 800U) {
            state = 0U;
            return;
        }

        ++bit_count;
        if (bit_count == 32U) {
            uint8_t data[4];
            data[0] = (uint8_t)frame;
            data[1] = (uint8_t)(frame >> 8);
            data[2] = (uint8_t)(frame >> 16);
            data[3] = (uint8_t)(frame >> 24);

            if (IR_CRC8(data, 3) == data[3]) {
                rx_x = (uint16_t)data[0]
                     | ((uint16_t)(data[1] & 0x0FU) << 8);
                rx_y = (uint16_t)(data[1] >> 4)
                     | ((uint16_t)data[2] << 4);
                rx_flag = 1U;
            }
            state = 0U;
        } else {
            state = 2U;
        }
    } else {
        state = 0U;
    }
}
```

`previous` 使用 `uint16_t` 差值，因此 TIM3 溢出时只要相邻边沿间隔小于 65.5 ms，差值仍能正确计算。9 ms 引导码不会超过该范围。

## 6. 主循环集成

主循环中只在 `rx_flag` 置位后读取数据。因为变量由中断更新，读取时应使用短临界区：

```c
if (rx_flag) {
    uint16_t x;
    uint16_t y;

    __disable_irq();
    x = rx_x;
    y = rx_y;
    rx_flag = 0U;
    __enable_irq();

    /* 将 x/y 交给控制器、OLED 或通信接口。 */
}
```

不要在 `HAL_TIM_IC_CaptureCallback()` 中直接刷新 OLED。OLED 一次全屏刷新需要较长时间，可能阻塞后续红外边沿捕获。

如果接收机使用 OLED，可在主循环中低频刷新，例如每 100 ms 一次：

```c
OLED_Clear();
OLED_ShowString(0, 0, "RX X:");
OLED_ShowNum(36, 0, x, 4);
OLED_ShowString(0, 2, "RX Y:");
OLED_ShowNum(36, 2, y, 4);
OLED_Update();
```

## 7. 验证顺序

建议按以下顺序排查：

1. 确认接收头供电为 3.3 V，且发送机与接收机共地。
2. 用示波器确认接收头 OUT 在收到信号时有约 9 ms、4.5 ms、560/1690 us 脉冲。
3. 确认 TIM3 计数频率为 1 MHz。
4. 在调试器中观察 `bit_count` 是否能到达 32。
5. 观察 CRC 校验是否通过。
6. 观察 `rx_flag` 是否置 1，以及 `rx_x/rx_y` 是否随摇杆变化。
7. 最后再启用 OLED 或电机控制逻辑。

常见问题：

| 现象 | 可能原因 |
|---|---|
| 完全没有捕获中断 | PA6 接线错误、TIM3 中断未启用、接收头无供电 |
| 只能测到固定宽度 | TIM3 计数频率不是 1 MHz |
| `bit_count` 到不了 32 | 初始边沿极性错误或脉宽容差过窄 |
| 能收到边沿但 CRC 全部失败 | 发送端和接收端帧格式不一致、位顺序错误 |
| X/Y 偶发跳变 | 红外干扰、接收头供电去耦不足或 CRC 未正确检查 |
| 加 OLED 后丢帧 | OLED 刷新放进了中断，或主循环阻塞时间过长 |

## 8. 与当前发送机的兼容性边界

接收机必须匹配以下参数：

```text
载波：约 38 kHz
引导码：9000 / 4500 us
数据 mark：560 us
数据 0 space：560 us
数据 1 space：1690 us
数据位序：LSB first
数据长度：32 bit
X/Y：各 12 bit
CRC8：多项式 0x07，初值 0x00
```

这套帧不是标准 NEC 遥控器帧，因此不能直接用只检查地址反码和命令反码的 NEC 解码器。若后续要增加设备地址、帧类型或序号，应重新规划 32 位字段，或者使用多帧协议，并同步修改发送端和接收端。
