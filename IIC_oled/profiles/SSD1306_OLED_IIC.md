# 中景园电子 0.96寸 OLED 显示屏 IIC 接口原理图说明

## 模块概述

| 参数 | 值 |
|------|-----|
| 屏幕尺寸 | 0.96 英寸 |
| 分辨率 | 128 × 64 像素 |
| 驱动芯片 | SSD1306 |
| 接口 | I2C (IIC) |
| 工作电压 | 3.3V / 5V（板载 662K 稳压管保护） |

---

## 引脚定义（J1 接口，4 Pin）

| Pin | 名称 | 说明 |
|-----|------|------|
| 1 | GND | 电源地 |
| 2 | VCC\_IN | 电源输入（3.3V–5V） |
| 3 | SCL | I2C 时钟线 |
| 4 | SDA | I2C 数据线 |

---

## I2C 地址

| D/C# (Pin 15) 接法 | 7-bit 地址 | 写地址（含R/W=0） |
|-------------------|-----------|----------------|
| 接 GND（默认）     | 0x3C      | 0x78           |
| 接 VCC            | 0x3D      | 0x7A           |

> 原理图注释：*"The I2C slave address is 0x78. If the customer ties D/C# (pin 15) to VCC, the I2C slave address will be 0x7A."*
> 
> **代码中使用 7-bit 地址 0x3C（MSPM0 driverlib 使用 7-bit）。**

---

## 总线接口模式选择（BS0/BS1/BS2）

原理图中 BS0=1, BS1=0, BS2=0 → **I2C 模式**。

| BS2 | BS1 | BS0 | 接口模式 |
|-----|-----|-----|---------|
| 0   | 0   | 1   | I2C     |
| 0   | 0   | 0   | SPI 4-wire |
| 0   | 1   | 0   | SPI 3-wire |

---

## 电源设计要点

- **VCC_IN → U2（662K 稳压管）→ VCC（3.3V）**：当外部供电高于 3.3V 时起保护作用；直接供 3.3V 则无需此管。
- **升压电路**：SSD1306 内置 charge pump，通过外接电容（C1/C2 各 1uF）自举产生 VBAT/VOUT。
- **复位电路**：硬件 RC 复位（D1=IN4148，C9=10uF，R10=4.7K），上电瞬间自动拉低 RES# 完成复位。

---

## I2C 上拉电阻

原理图中 R6/R7 = 4.7kΩ，接 VCC，分别上拉 SCL 和 SDA。

LP-MSPM0G3507 板载已有内部弱上拉，但建议使用模块自带的 4.7kΩ 外部上拉（Fast Mode 400kHz 适用）。

---

## SSD1306 关键寄存器（初始化参考）

| 命令 | 值 | 说明 |
|------|-----|------|
| 0xAE / 0xAF | — | 关闭 / 开启显示 |
| 0x20 | 0x00 | 水平寻址模式 |
| 0x81 | 0xFF | 对比度（亮度） |
| 0x8D | 0x14 | 使能 Charge Pump |
| 0xA1 | — | 列地址映射翻转 |
| 0xA8 | 0x3F | 复用比（64 行） |
| 0xC8 | — | COM 扫描方向翻转 |
| 0xDA | 0x12 | COM 引脚配置（128×64） |

---

## 与 MSPM0G3507 的连接（LP-MSPM0G3507）

| OLED Pin | MCU Pin | BoosterPack Pin |
|----------|---------|-----------------|
| GND      | GND     | Pin 20/22 |
| VCC_IN   | 3.3V    | Pin 1 |
| SCL      | PB2     | Pin 9 |
| SDA      | PB3     | Pin 10 |

SysConfig 实例名：`I2C_0`，外设分配：**I2C1**，速率：**400kHz Fast Mode**。

宏命名规则（来自 SysConfig 实例名 `I2C_0`）：
- 外设指针：`I2C_0_INST`
- 中断号：`I2C_0_INST_INT_IRQN`
- ISR 函数：`I2C_0_INST_IRQHandler`
