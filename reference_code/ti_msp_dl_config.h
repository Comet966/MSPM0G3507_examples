/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define CPUCLK_FREQ                                                     80000000



/* Defines for PWM_Motors */
#define PWM_Motors_INST                                                    TIMA0
#define PWM_Motors_INST_IRQHandler                              TIMA0_IRQHandler
#define PWM_Motors_INST_INT_IRQN                                (TIMA0_INT_IRQn)
#define PWM_Motors_INST_CLK_FREQ                                        10000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_Motors_C0_PORT                                            GPIOA
#define GPIO_PWM_Motors_C0_PIN                                     DL_GPIO_PIN_8
#define GPIO_PWM_Motors_C0_IOMUX                                 (IOMUX_PINCM19)
#define GPIO_PWM_Motors_C0_IOMUX_FUNC                IOMUX_PINCM19_PF_TIMA0_CCP0
#define GPIO_PWM_Motors_C0_IDX                               DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_Motors_C1_PORT                                            GPIOA
#define GPIO_PWM_Motors_C1_PIN                                     DL_GPIO_PIN_9
#define GPIO_PWM_Motors_C1_IOMUX                                 (IOMUX_PINCM20)
#define GPIO_PWM_Motors_C1_IOMUX_FUNC                IOMUX_PINCM20_PF_TIMA0_CCP1
#define GPIO_PWM_Motors_C1_IDX                               DL_TIMER_CC_1_INDEX
/* GPIO defines for channel 2 */
#define GPIO_PWM_Motors_C2_PORT                                            GPIOA
#define GPIO_PWM_Motors_C2_PIN                                     DL_GPIO_PIN_7
#define GPIO_PWM_Motors_C2_IOMUX                                 (IOMUX_PINCM14)
#define GPIO_PWM_Motors_C2_IOMUX_FUNC                IOMUX_PINCM14_PF_TIMA0_CCP2
#define GPIO_PWM_Motors_C2_IDX                               DL_TIMER_CC_2_INDEX
/* GPIO defines for channel 3 */
#define GPIO_PWM_Motors_C3_PORT                                            GPIOB
#define GPIO_PWM_Motors_C3_PIN                                     DL_GPIO_PIN_2
#define GPIO_PWM_Motors_C3_IOMUX                                 (IOMUX_PINCM15)
#define GPIO_PWM_Motors_C3_IOMUX_FUNC                IOMUX_PINCM15_PF_TIMA0_CCP3
#define GPIO_PWM_Motors_C3_IDX                               DL_TIMER_CC_3_INDEX

/* Defines for PWM_Buzzer */
#define PWM_Buzzer_INST                                                   TIMG12
#define PWM_Buzzer_INST_IRQHandler                             TIMG12_IRQHandler
#define PWM_Buzzer_INST_INT_IRQN                               (TIMG12_INT_IRQn)
#define PWM_Buzzer_INST_CLK_FREQ                                        10000000
/* GPIO defines for channel 1 */
#define GPIO_PWM_Buzzer_C1_PORT                                            GPIOA
#define GPIO_PWM_Buzzer_C1_PIN                                    DL_GPIO_PIN_25
#define GPIO_PWM_Buzzer_C1_IOMUX                                 (IOMUX_PINCM55)
#define GPIO_PWM_Buzzer_C1_IOMUX_FUNC               IOMUX_PINCM55_PF_TIMG12_CCP1
#define GPIO_PWM_Buzzer_C1_IDX                               DL_TIMER_CC_1_INDEX



/* Defines for TIMERA1_10hz */
#define TIMERA1_10hz_INST                                                (TIMA1)
#define TIMERA1_10hz_INST_IRQHandler                            TIMA1_IRQHandler
#define TIMERA1_10hz_INST_INT_IRQN                              (TIMA1_INT_IRQn)
#define TIMERA1_10hz_INST_LOAD_VALUE                                     (9999U)
/* Defines for TIMERG7_200hz */
#define TIMERG7_200hz_INST                                               (TIMG7)
#define TIMERG7_200hz_INST_IRQHandler                           TIMG7_IRQHandler
#define TIMERG7_200hz_INST_INT_IRQN                             (TIMG7_INT_IRQn)
#define TIMERG7_200hz_INST_LOAD_VALUE                                     (499U)
/* Defines for TIMERG8_100hz */
#define TIMERG8_100hz_INST                                               (TIMG8)
#define TIMERG8_100hz_INST_IRQHandler                           TIMG8_IRQHandler
#define TIMERG8_100hz_INST_INT_IRQN                             (TIMG8_INT_IRQn)
#define TIMERG8_100hz_INST_LOAD_VALUE                                     (499U)
/* Defines for TIMERG6_1000hz */
#define TIMERG6_1000hz_INST                                              (TIMG6)
#define TIMERG6_1000hz_INST_IRQHandler                          TIMG6_IRQHandler
#define TIMERG6_1000hz_INST_INT_IRQN                            (TIMG6_INT_IRQn)
#define TIMERG6_1000hz_INST_LOAD_VALUE                                   (9999U)



/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                           40000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                        DL_GPIO_PIN_11
#define GPIO_UART_0_TX_PIN                                        DL_GPIO_PIN_10
#define GPIO_UART_0_IOMUX_RX                                     (IOMUX_PINCM22)
#define GPIO_UART_0_IOMUX_TX                                     (IOMUX_PINCM21)
#define GPIO_UART_0_IOMUX_RX_FUNC                      IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                      IOMUX_PINCM21_PF_UART0_TX
#define UART_0_BAUD_RATE                                                (115200)
#define UART_0_IBRD_40_MHZ_115200_BAUD                                      (21)
#define UART_0_FBRD_40_MHZ_115200_BAUD                                      (45)
/* Defines for UART_1 */
#define UART_1_INST                                                        UART1
#define UART_1_INST_FREQUENCY                                           40000000
#define UART_1_INST_IRQHandler                                  UART1_IRQHandler
#define UART_1_INST_INT_IRQN                                      UART1_INT_IRQn
#define GPIO_UART_1_RX_PORT                                                GPIOB
#define GPIO_UART_1_TX_PORT                                                GPIOB
#define GPIO_UART_1_RX_PIN                                         DL_GPIO_PIN_7
#define GPIO_UART_1_TX_PIN                                         DL_GPIO_PIN_6
#define GPIO_UART_1_IOMUX_RX                                     (IOMUX_PINCM24)
#define GPIO_UART_1_IOMUX_TX                                     (IOMUX_PINCM23)
#define GPIO_UART_1_IOMUX_RX_FUNC                      IOMUX_PINCM24_PF_UART1_RX
#define GPIO_UART_1_IOMUX_TX_FUNC                      IOMUX_PINCM23_PF_UART1_TX
#define UART_1_BAUD_RATE                                                (256000)
#define UART_1_IBRD_40_MHZ_256000_BAUD                                       (9)
#define UART_1_FBRD_40_MHZ_256000_BAUD                                      (49)




/* Defines for SPI_Bmi088 */
#define SPI_Bmi088_INST                                                    SPI1
#define SPI_Bmi088_INST_IRQHandler                              SPI1_IRQHandler
#define SPI_Bmi088_INST_INT_IRQN                                  SPI1_INT_IRQn
#define GPIO_SPI_Bmi088_PICO_PORT                                         GPIOA
#define GPIO_SPI_Bmi088_PICO_PIN                                 DL_GPIO_PIN_18
#define GPIO_SPI_Bmi088_IOMUX_PICO                              (IOMUX_PINCM40)
#define GPIO_SPI_Bmi088_IOMUX_PICO_FUNC              IOMUX_PINCM40_PF_SPI1_PICO
#define GPIO_SPI_Bmi088_POCI_PORT                                         GPIOA
#define GPIO_SPI_Bmi088_POCI_PIN                                 DL_GPIO_PIN_16
#define GPIO_SPI_Bmi088_IOMUX_POCI                              (IOMUX_PINCM38)
#define GPIO_SPI_Bmi088_IOMUX_POCI_FUNC              IOMUX_PINCM38_PF_SPI1_POCI
/* GPIO configuration for SPI_Bmi088 */
#define GPIO_SPI_Bmi088_SCLK_PORT                                         GPIOA
#define GPIO_SPI_Bmi088_SCLK_PIN                                 DL_GPIO_PIN_17
#define GPIO_SPI_Bmi088_IOMUX_SCLK                              (IOMUX_PINCM39)
#define GPIO_SPI_Bmi088_IOMUX_SCLK_FUNC              IOMUX_PINCM39_PF_SPI1_SCLK



/* Defines for ADCGraySensor */
#define ADCGraySensor_INST                                                  ADC0
#define ADCGraySensor_INST_IRQHandler                            ADC0_IRQHandler
#define ADCGraySensor_INST_INT_IRQN                              (ADC0_INT_IRQn)
#define ADCGraySensor_ADCMEM_C5                               DL_ADC12_MEM_IDX_0
#define ADCGraySensor_ADCMEM_C5_REF              DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADCGraySensor_ADCMEM_C5_REF_VOLTAGE_V                                     3.3
#define GPIO_ADCGraySensor_C6_PORT                                         GPIOB
#define GPIO_ADCGraySensor_C6_PIN                                 DL_GPIO_PIN_20



/* Port definition for Pin Group Key */
#define Key_PORT                                                         (GPIOA)

/* Defines for KeyPutIN: GPIOA.23 with pinCMx 53 on package pin 24 */
#define Key_KeyPutIN_PIN                                        (DL_GPIO_PIN_23)
#define Key_KeyPutIN_IOMUX                                       (IOMUX_PINCM53)
/* Port definition for Pin Group MotorDirection */
#define MotorDirection_PORT                                              (GPIOB)

/* Defines for LeftFront: GPIOB.9 with pinCMx 26 on package pin 61 */
#define MotorDirection_LeftFront_PIN                             (DL_GPIO_PIN_9)
#define MotorDirection_LeftFront_IOMUX                           (IOMUX_PINCM26)
/* Defines for LeftRear: GPIOB.16 with pinCMx 33 on package pin 4 */
#define MotorDirection_LeftRear_PIN                             (DL_GPIO_PIN_16)
#define MotorDirection_LeftRear_IOMUX                            (IOMUX_PINCM33)
/* Defines for RightFront: GPIOB.14 with pinCMx 31 on package pin 2 */
#define MotorDirection_RightFront_PIN                           (DL_GPIO_PIN_14)
#define MotorDirection_RightFront_IOMUX                          (IOMUX_PINCM31)
/* Defines for RightRear: GPIOB.15 with pinCMx 32 on package pin 3 */
#define MotorDirection_RightRear_PIN                            (DL_GPIO_PIN_15)
#define MotorDirection_RightRear_IOMUX                           (IOMUX_PINCM32)
/* Port definition for Pin Group GPIO_OLED */
#define GPIO_OLED_PORT                                                   (GPIOA)

/* Defines for PIN_SCL: GPIOA.21 with pinCMx 46 on package pin 17 */
#define GPIO_OLED_PIN_SCL_PIN                                   (DL_GPIO_PIN_21)
#define GPIO_OLED_PIN_SCL_IOMUX                                  (IOMUX_PINCM46)
/* Defines for PIN_SDA: GPIOA.22 with pinCMx 47 on package pin 18 */
#define GPIO_OLED_PIN_SDA_PIN                                   (DL_GPIO_PIN_22)
#define GPIO_OLED_PIN_SDA_IOMUX                                  (IOMUX_PINCM47)
/* Port definition for Pin Group Encoder1 */
#define Encoder1_PORT                                                    (GPIOB)

/* Defines for Encoder1B: GPIOB.3 with pinCMx 16 on package pin 51 */
// pins affected by this interrupt request:["Encoder1B"]
#define Encoder1_INT_IRQN                                       (GPIOB_INT_IRQn)
#define Encoder1_INT_IIDX                       (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define Encoder1_Encoder1B_IIDX                              (DL_GPIO_IIDX_DIO3)
#define Encoder1_Encoder1B_PIN                                   (DL_GPIO_PIN_3)
#define Encoder1_Encoder1B_IOMUX                                 (IOMUX_PINCM16)
/* Defines for Encoder1A: GPIOB.8 with pinCMx 25 on package pin 60 */
#define Encoder1_Encoder1A_PIN                                   (DL_GPIO_PIN_8)
#define Encoder1_Encoder1A_IOMUX                                 (IOMUX_PINCM25)
/* Port definition for Pin Group Encoder2 */
#define Encoder2_PORT                                                    (GPIOA)

/* Defines for Encoder2B: GPIOA.13 with pinCMx 35 on package pin 6 */
// pins affected by this interrupt request:["Encoder2B"]
#define Encoder2_INT_IRQN                                       (GPIOA_INT_IRQn)
#define Encoder2_INT_IIDX                       (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define Encoder2_Encoder2B_IIDX                             (DL_GPIO_IIDX_DIO13)
#define Encoder2_Encoder2B_PIN                                  (DL_GPIO_PIN_13)
#define Encoder2_Encoder2B_IOMUX                                 (IOMUX_PINCM35)
/* Defines for Encoder2A: GPIOA.12 with pinCMx 34 on package pin 5 */
#define Encoder2_Encoder2A_PIN                                  (DL_GPIO_PIN_12)
#define Encoder2_Encoder2A_IOMUX                                 (IOMUX_PINCM34)
/* Defines for PIN_0: GPIOB.24 with pinCMx 52 on package pin 23 */
#define GraySensor_PIN_0_PORT                                            (GPIOB)
#define GraySensor_PIN_0_PIN                                    (DL_GPIO_PIN_24)
#define GraySensor_PIN_0_IOMUX                                   (IOMUX_PINCM52)
/* Defines for PIN_1: GPIOB.19 with pinCMx 45 on package pin 16 */
#define GraySensor_PIN_1_PORT                                            (GPIOB)
#define GraySensor_PIN_1_PIN                                    (DL_GPIO_PIN_19)
#define GraySensor_PIN_1_IOMUX                                   (IOMUX_PINCM45)
/* Defines for PIN_2: GPIOA.31 with pinCMx 6 on package pin 39 */
#define GraySensor_PIN_2_PORT                                            (GPIOA)
#define GraySensor_PIN_2_PIN                                    (DL_GPIO_PIN_31)
#define GraySensor_PIN_2_IOMUX                                    (IOMUX_PINCM6)
/* Port definition for Pin Group CS */
#define CS_PORT                                                          (GPIOA)

/* Defines for ACC_CS_Pin: GPIOA.14 with pinCMx 36 on package pin 7 */
#define CS_ACC_CS_Pin_PIN                                       (DL_GPIO_PIN_14)
#define CS_ACC_CS_Pin_IOMUX                                      (IOMUX_PINCM36)
/* Defines for GYRO_CS_Pin: GPIOA.15 with pinCMx 37 on package pin 8 */
#define CS_GYRO_CS_Pin_PIN                                      (DL_GPIO_PIN_15)
#define CS_GYRO_CS_Pin_IOMUX                                     (IOMUX_PINCM37)




/* Defines for MCAN0 */
#define MCAN0_INST                                                        CANFD0
#define GPIO_MCAN0_CAN_TX_PORT                                             GPIOA
#define GPIO_MCAN0_CAN_TX_PIN                                     DL_GPIO_PIN_26
#define GPIO_MCAN0_IOMUX_CAN_TX                                  (IOMUX_PINCM59)
#define GPIO_MCAN0_IOMUX_CAN_TX_FUNC               IOMUX_PINCM59_PF_CANFD0_CANTX
#define GPIO_MCAN0_CAN_RX_PORT                                             GPIOA
#define GPIO_MCAN0_CAN_RX_PIN                                     DL_GPIO_PIN_27
#define GPIO_MCAN0_IOMUX_CAN_RX                                  (IOMUX_PINCM60)
#define GPIO_MCAN0_IOMUX_CAN_RX_FUNC               IOMUX_PINCM60_PF_CANFD0_CANRX


/* Defines for MCAN0 MCAN RAM configuration */
#define MCAN0_INST_MCAN_STD_ID_FILT_START_ADDR     (0)
#define MCAN0_INST_MCAN_STD_ID_FILTER_NUM          (1)
#define MCAN0_INST_MCAN_EXT_ID_FILT_START_ADDR     (48)
#define MCAN0_INST_MCAN_EXT_ID_FILTER_NUM          (1)
#define MCAN0_INST_MCAN_TX_BUFF_START_ADDR         (148)
#define MCAN0_INST_MCAN_TX_BUFF_SIZE               (2)
#define MCAN0_INST_MCAN_FIFO_1_START_ADDR          (192)
#define MCAN0_INST_MCAN_FIFO_1_NUM                 (2)
#define MCAN0_INST_MCAN_TX_EVENT_START_ADDR        (164)
#define MCAN0_INST_MCAN_TX_EVENT_SIZE              (2)
#define MCAN0_INST_MCAN_EXT_ID_AND_MASK            (0x1FFFFFFFU)
#define MCAN0_INST_MCAN_RX_BUFF_START_ADDR         (208)
#define MCAN0_INST_MCAN_FIFO_0_START_ADDR          (172)
#define MCAN0_INST_MCAN_FIFO_0_NUM                 (3)




/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_SYSCTL_CLK_init(void);
void SYSCFG_DL_PWM_Motors_init(void);
void SYSCFG_DL_PWM_Buzzer_init(void);
void SYSCFG_DL_TIMERA1_10hz_init(void);
void SYSCFG_DL_TIMERG7_200hz_init(void);
void SYSCFG_DL_TIMERG8_100hz_init(void);
void SYSCFG_DL_TIMERG6_1000hz_init(void);
void SYSCFG_DL_UART_0_init(void);
void SYSCFG_DL_UART_1_init(void);
void SYSCFG_DL_SPI_Bmi088_init(void);
void SYSCFG_DL_ADCGraySensor_init(void);

void SYSCFG_DL_SYSTICK_init(void);
void SYSCFG_DL_MCAN0_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
