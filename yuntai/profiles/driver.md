# 双路步进电机驱动板技术手册 (MSPM0G3507 适配版)

## 一、 核心技术特性
1. **电源输入**：DC 5.5V ~ 17V (5.08mm接线端子)。支持电源并联输出（级联）。
2. **逻辑供电**：自带稳压电路，无需MCU外接逻辑供电。
3. **对外输出能力**：
   * **5V 输出**：最大 **5A**（带保护，可给MCU板/树莓派供电）。
   * **3.3V 输出**：最大 **0.5A**（带保护）。
4. **电压监测**：内置 **1/11 分压电阻**，输出至 `ADC` 引脚用于检测供电电压。
5. **控制配置**：拨码开关用于设置细分（Microstepping）和电流。

---

## 二、 控制接口引脚定义 (2x5 双排针)

| 引脚名称 | 功能描述 | MSPM0G3507 推荐外设映射 | 驱动逻辑 |
| :--- | :--- | :--- | :--- |
| **GND** | 电源地 | 系统 GND | 共同参考地 |
| **5V** | 5V 电源输出 | - | 可用作 MCU 板供电输入 |
| **ST1 / ST2** | 步进电机1/2 脉冲输入 | **TIMGx (PWM Mode)** | 每一个**上升沿**转动一步，频率决定速度 |
| **DIR1 / DIR2**| 步进电机1/2 方向控制 | **GPIO Output** | 高/低电平切换方向 |
| **EN1 / EN2** | 步进电机1/2 使能控制 | **GPIO Output** | **高电平使能**，低电平休眠 |
| **ADC** | 输入电压检测输出 | **ADC12 通道** | 输出值为 $V_{in} \times \frac{1}{11}$ |

---

## 三、 MSPM0G3507 DriverLib 开发指南

### 1. 脉冲输出 (ST1/ST2) — 定时器 PWM 配置
使用 `DL_TimerG` 配置 PWM 输出控制电机转速和步数。

```c
// 初始化 PWM (以 TIMG0 为例)
DL_TimerG_PWMConfig pwmConfig = {
    .period     = 2000, // 决定脉冲频率 (速度)
    .dutyCycle  = 1000, // 50% 占空比即可
    .startTimer = DL_TIMER_STOP,
};
DL_TimerG_initPWMMode(TIMG0, &pwmConfig);
DL_TimerG_enableClock(TIMG0);
// 启动发送脉冲
DL_TimerG_startCounter(TIMG0);