/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gPWM_MotorsBackup;
DL_TimerA_backupConfig gTIMERA1_10hzBackup;
DL_TimerG_backupConfig gTIMERG7_200hzBackup;
DL_TimerG_backupConfig gTIMERG6_1000hzBackup;
DL_SPI_backupConfig gSPI_Bmi088Backup;
DL_MCAN_backupConfig gMCAN0Backup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_PWM_Motors_init();
    SYSCFG_DL_PWM_Buzzer_init();
    SYSCFG_DL_TIMERA1_10hz_init();
    SYSCFG_DL_TIMERG7_200hz_init();
    SYSCFG_DL_TIMERG8_100hz_init();
    SYSCFG_DL_TIMERG6_1000hz_init();
    SYSCFG_DL_UART_0_init();
    SYSCFG_DL_UART_1_init();
    SYSCFG_DL_SPI_Bmi088_init();
    SYSCFG_DL_ADCGraySensor_init();
    SYSCFG_DL_SYSTICK_init();
    SYSCFG_DL_MCAN0_init();
    SYSCFG_DL_SYSCTL_CLK_init();
    /* Ensure backup structures have no valid state */
	gPWM_MotorsBackup.backupRdy 	= false;
	gTIMERA1_10hzBackup.backupRdy 	= false;
	gTIMERG7_200hzBackup.backupRdy 	= false;
	gTIMERG6_1000hzBackup.backupRdy 	= false;

	gSPI_Bmi088Backup.backupRdy 	= false;
	gMCAN0Backup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(PWM_Motors_INST, &gPWM_MotorsBackup);
	retStatus &= DL_TimerA_saveConfiguration(TIMERA1_10hz_INST, &gTIMERA1_10hzBackup);
	retStatus &= DL_TimerG_saveConfiguration(TIMERG7_200hz_INST, &gTIMERG7_200hzBackup);
	retStatus &= DL_TimerG_saveConfiguration(TIMERG6_1000hz_INST, &gTIMERG6_1000hzBackup);
	retStatus &= DL_SPI_saveConfiguration(SPI_Bmi088_INST, &gSPI_Bmi088Backup);
	retStatus &= DL_MCAN_saveConfiguration(MCAN0_INST, &gMCAN0Backup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(PWM_Motors_INST, &gPWM_MotorsBackup, false);
	retStatus &= DL_TimerA_restoreConfiguration(TIMERA1_10hz_INST, &gTIMERA1_10hzBackup, false);
	retStatus &= DL_TimerG_restoreConfiguration(TIMERG7_200hz_INST, &gTIMERG7_200hzBackup, false);
	retStatus &= DL_TimerG_restoreConfiguration(TIMERG6_1000hz_INST, &gTIMERG6_1000hzBackup, false);
	retStatus &= DL_SPI_restoreConfiguration(SPI_Bmi088_INST, &gSPI_Bmi088Backup);
	retStatus &= DL_MCAN_restoreConfiguration(MCAN0_INST, &gMCAN0Backup);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(PWM_Motors_INST);
    DL_TimerG_reset(PWM_Buzzer_INST);
    DL_TimerA_reset(TIMERA1_10hz_INST);
    DL_TimerG_reset(TIMERG7_200hz_INST);
    DL_TimerG_reset(TIMERG8_100hz_INST);
    DL_TimerG_reset(TIMERG6_1000hz_INST);
    DL_UART_Main_reset(UART_0_INST);
    DL_UART_Main_reset(UART_1_INST);
    DL_SPI_reset(SPI_Bmi088_INST);
    DL_ADC12_reset(ADCGraySensor_INST);

    DL_MCAN_reset(MCAN0_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(PWM_Motors_INST);
    DL_TimerG_enablePower(PWM_Buzzer_INST);
    DL_TimerA_enablePower(TIMERA1_10hz_INST);
    DL_TimerG_enablePower(TIMERG7_200hz_INST);
    DL_TimerG_enablePower(TIMERG8_100hz_INST);
    DL_TimerG_enablePower(TIMERG6_1000hz_INST);
    DL_UART_Main_enablePower(UART_0_INST);
    DL_UART_Main_enablePower(UART_1_INST);
    DL_SPI_enablePower(SPI_Bmi088_INST);
    DL_ADC12_enablePower(ADCGraySensor_INST);

    DL_MCAN_enablePower(MCAN0_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralAnalogFunction(GPIO_HFXIN_IOMUX);
    DL_GPIO_initPeripheralAnalogFunction(GPIO_HFXOUT_IOMUX);

    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_Motors_C0_IOMUX,GPIO_PWM_Motors_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_Motors_C0_PORT, GPIO_PWM_Motors_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_Motors_C1_IOMUX,GPIO_PWM_Motors_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_Motors_C1_PORT, GPIO_PWM_Motors_C1_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_Motors_C2_IOMUX,GPIO_PWM_Motors_C2_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_Motors_C2_PORT, GPIO_PWM_Motors_C2_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_Motors_C3_IOMUX,GPIO_PWM_Motors_C3_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_Motors_C3_PORT, GPIO_PWM_Motors_C3_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_Buzzer_C1_IOMUX,GPIO_PWM_Buzzer_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_Buzzer_C1_PORT, GPIO_PWM_Buzzer_C1_PIN);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_0_IOMUX_TX, GPIO_UART_0_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_0_IOMUX_RX, GPIO_UART_0_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_1_IOMUX_TX, GPIO_UART_1_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_1_IOMUX_RX, GPIO_UART_1_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_Bmi088_IOMUX_SCLK, GPIO_SPI_Bmi088_IOMUX_SCLK_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_Bmi088_IOMUX_PICO, GPIO_SPI_Bmi088_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_SPI_Bmi088_IOMUX_POCI, GPIO_SPI_Bmi088_IOMUX_POCI_FUNC);

    DL_GPIO_initDigitalInputFeatures(Key_KeyPutIN_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutputFeatures(MotorDirection_LeftFront_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    DL_GPIO_initDigitalOutputFeatures(MotorDirection_LeftRear_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    DL_GPIO_initDigitalOutputFeatures(MotorDirection_RightFront_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    DL_GPIO_initDigitalOutputFeatures(MotorDirection_RightRear_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    DL_GPIO_initDigitalOutput(GPIO_OLED_PIN_SCL_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_OLED_PIN_SDA_IOMUX);

    DL_GPIO_initDigitalInputFeatures(Encoder1_Encoder1B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(Encoder1_Encoder1A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(Encoder2_Encoder2B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(Encoder2_Encoder2A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutputFeatures(GraySensor_PIN_0_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    DL_GPIO_initDigitalOutputFeatures(GraySensor_PIN_1_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    DL_GPIO_initDigitalOutputFeatures(GraySensor_PIN_2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    DL_GPIO_initDigitalOutput(CS_ACC_CS_Pin_IOMUX);

    DL_GPIO_initDigitalOutput(CS_GYRO_CS_Pin_IOMUX);

    DL_GPIO_clearPins(GPIOA, GraySensor_PIN_2_PIN |
		CS_ACC_CS_Pin_PIN |
		CS_GYRO_CS_Pin_PIN);
    DL_GPIO_setPins(GPIOA, GPIO_OLED_PIN_SCL_PIN |
		GPIO_OLED_PIN_SDA_PIN);
    DL_GPIO_enableOutput(GPIOA, GPIO_OLED_PIN_SCL_PIN |
		GPIO_OLED_PIN_SDA_PIN |
		GraySensor_PIN_2_PIN |
		CS_ACC_CS_Pin_PIN |
		CS_GYRO_CS_Pin_PIN);
    DL_GPIO_setLowerPinsPolarity(GPIOA, DL_GPIO_PIN_13_EDGE_RISE);
    DL_GPIO_clearInterruptStatus(GPIOA, Encoder2_Encoder2B_PIN);
    DL_GPIO_enableInterrupt(GPIOA, Encoder2_Encoder2B_PIN);
    DL_GPIO_clearPins(GPIOB, MotorDirection_LeftFront_PIN |
		MotorDirection_LeftRear_PIN |
		MotorDirection_RightFront_PIN |
		MotorDirection_RightRear_PIN |
		GraySensor_PIN_0_PIN |
		GraySensor_PIN_1_PIN);
    DL_GPIO_enableOutput(GPIOB, MotorDirection_LeftFront_PIN |
		MotorDirection_LeftRear_PIN |
		MotorDirection_RightFront_PIN |
		MotorDirection_RightRear_PIN |
		GraySensor_PIN_0_PIN |
		GraySensor_PIN_1_PIN);
    DL_GPIO_setLowerPinsPolarity(GPIOB, DL_GPIO_PIN_3_EDGE_RISE);
    DL_GPIO_clearInterruptStatus(GPIOB, Encoder1_Encoder1B_PIN);
    DL_GPIO_enableInterrupt(GPIOB, Encoder1_Encoder1B_PIN);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_MCAN0_IOMUX_CAN_TX, GPIO_MCAN0_IOMUX_CAN_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_MCAN0_IOMUX_CAN_RX, GPIO_MCAN0_IOMUX_CAN_RX_FUNC);

}


static const DL_SYSCTL_SYSPLLConfig gSYSPLLConfig = {
    .inputFreq              = DL_SYSCTL_SYSPLL_INPUT_FREQ_32_48_MHZ,
	.rDivClk2x              = 3,
	.rDivClk1               = 0,
	.rDivClk0               = 0,
	.enableCLK2x            = DL_SYSCTL_SYSPLL_CLK2X_DISABLE,
	.enableCLK1             = DL_SYSCTL_SYSPLL_CLK1_ENABLE,
	.enableCLK0             = DL_SYSCTL_SYSPLL_CLK0_ENABLE,
	.sysPLLMCLK             = DL_SYSCTL_SYSPLL_MCLK_CLK0,
	.sysPLLRef              = DL_SYSCTL_SYSPLL_REF_HFCLK,
	.qDiv                   = 3,
	.pDiv                   = DL_SYSCTL_SYSPLL_PDIV_1
};
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_1);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
	/* Set default configuration */
	DL_SYSCTL_disableHFXT();
	DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setHFCLKSourceHFXTParams(DL_SYSCTL_HFXT_RANGE_32_48_MHZ,20, true);
    DL_SYSCTL_configSYSPLL((DL_SYSCTL_SYSPLLConfig *) &gSYSPLLConfig);
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_2);
    DL_SYSCTL_setHFCLKDividerForMFPCLK(DL_SYSCTL_HFCLK_MFPCLK_DIVIDER_10);
    DL_SYSCTL_enableMFCLK();
    DL_SYSCTL_enableMFPCLK();
	DL_SYSCTL_setMFPCLKSource(DL_SYSCTL_MFPCLK_SOURCE_HFCLK);
    DL_SYSCTL_setMCLKSource(SYSOSC, HSCLK, DL_SYSCTL_HSCLK_SOURCE_SYSPLL);
    /* INT_GROUP1 Priority */
    NVIC_SetPriority(GPIOA_INT_IRQn, 1);

}
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_CLK_init(void) {
    while ((DL_SYSCTL_getClockStatus() & (DL_SYSCTL_CLK_STATUS_SYSPLL_GOOD
		 | DL_SYSCTL_CLK_STATUS_HFCLK_GOOD
		 | DL_SYSCTL_CLK_STATUS_HSCLK_GOOD))
	       != (DL_SYSCTL_CLK_STATUS_SYSPLL_GOOD
		 | DL_SYSCTL_CLK_STATUS_HFCLK_GOOD
		 | DL_SYSCTL_CLK_STATUS_HSCLK_GOOD))
	{
		/* Ensure that clocks are in default POR configuration before initialization.
		* Additionally once LFXT is enabled, the internal LFOSC is disabled, and cannot
		* be re-enabled other than by executing a BOOTRST. */
		;
	}
}



/*
 * Timer clock configuration to be sourced by  / 8 (10000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   10000000 Hz = 10000000 Hz / (8 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gPWM_MotorsClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale = 0U
};

static const DL_TimerA_PWMConfig gPWM_MotorsConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 1001,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_Motors_init(void) {

    DL_TimerA_setClockConfig(
        PWM_Motors_INST, (DL_TimerA_ClockConfig *) &gPWM_MotorsClockConfig);

    DL_TimerA_initPWMMode(
        PWM_Motors_INST, (DL_TimerA_PWMConfig *) &gPWM_MotorsConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerA_setCounterControl(PWM_Motors_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerA_setCaptureCompareOutCtl(PWM_Motors_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_Motors_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_Motors_INST, 1001, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(PWM_Motors_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_Motors_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_Motors_INST, 1001, DL_TIMER_CC_1_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(PWM_Motors_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_2_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_Motors_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_2_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_Motors_INST, 1001, DL_TIMER_CC_2_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(PWM_Motors_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_3_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_Motors_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_3_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_Motors_INST, 1001, DL_TIMER_CC_3_INDEX);

    DL_TimerA_enableClock(PWM_Motors_INST);


    
    DL_TimerA_setCCPDirection(PWM_Motors_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT | DL_TIMER_CC2_OUTPUT | DL_TIMER_CC3_OUTPUT );


}
/*
 * Timer clock configuration to be sourced by  / 8 (10000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   10000000 Hz = 10000000 Hz / (8 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gPWM_BuzzerClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale = 0U
};

static const DL_TimerG_PWMConfig gPWM_BuzzerConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 1000,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_Buzzer_init(void) {

    DL_TimerG_setClockConfig(
        PWM_Buzzer_INST, (DL_TimerG_ClockConfig *) &gPWM_BuzzerClockConfig);

    DL_TimerG_initPWMMode(
        PWM_Buzzer_INST, (DL_TimerG_PWMConfig *) &gPWM_BuzzerConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerG_setCounterControl(PWM_Buzzer_INST,DL_TIMER_CZC_CCCTL1_ZCOND,DL_TIMER_CAC_CCCTL1_ACOND,DL_TIMER_CLC_CCCTL1_LCOND);

    DL_TimerG_setCaptureCompareOutCtl(PWM_Buzzer_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_1_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(PWM_Buzzer_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_Buzzer_INST, 1000, DL_TIMER_CC_1_INDEX);

    DL_TimerG_enableClock(PWM_Buzzer_INST);


    
    DL_TimerG_setCCPDirection(PWM_Buzzer_INST , DL_TIMER_CC1_OUTPUT );


}



/*
 * Timer clock configuration to be sourced by BUSCLK /  (10000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   100000 Hz = 10000000 Hz / (8 * (99 + 1))
 */
static const DL_TimerA_ClockConfig gTIMERA1_10hzClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale    = 99U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMERA1_10hz_INST_LOAD_VALUE = (100 ms * 100000 Hz) - 1
 */
static const DL_TimerA_TimerConfig gTIMERA1_10hzTimerConfig = {
    .period     = TIMERA1_10hz_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMERA1_10hz_init(void) {

    DL_TimerA_setClockConfig(TIMERA1_10hz_INST,
        (DL_TimerA_ClockConfig *) &gTIMERA1_10hzClockConfig);

    DL_TimerA_initTimerMode(TIMERA1_10hz_INST,
        (DL_TimerA_TimerConfig *) &gTIMERA1_10hzTimerConfig);
    DL_TimerA_enableInterrupt(TIMERA1_10hz_INST , DL_TIMERA_INTERRUPT_ZERO_EVENT);
    DL_TimerA_enableClock(TIMERA1_10hz_INST);





}

/*
 * Timer clock configuration to be sourced by BUSCLK /  (10000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   100000 Hz = 10000000 Hz / (8 * (99 + 1))
 */
static const DL_TimerG_ClockConfig gTIMERG7_200hzClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale    = 99U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMERG7_200hz_INST_LOAD_VALUE = (5 ms * 100000 Hz) - 1
 */
static const DL_TimerG_TimerConfig gTIMERG7_200hzTimerConfig = {
    .period     = TIMERG7_200hz_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMERG7_200hz_init(void) {

    DL_TimerG_setClockConfig(TIMERG7_200hz_INST,
        (DL_TimerG_ClockConfig *) &gTIMERG7_200hzClockConfig);

    DL_TimerG_initTimerMode(TIMERG7_200hz_INST,
        (DL_TimerG_TimerConfig *) &gTIMERG7_200hzTimerConfig);
    DL_TimerG_enableInterrupt(TIMERG7_200hz_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_enableClock(TIMERG7_200hz_INST);





}

/*
 * Timer clock configuration to be sourced by BUSCLK /  (5000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   50000 Hz = 5000000 Hz / (8 * (99 + 1))
 */
static const DL_TimerG_ClockConfig gTIMERG8_100hzClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale    = 99U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMERG8_100hz_INST_LOAD_VALUE = (10 ms * 50000 Hz) - 1
 */
static const DL_TimerG_TimerConfig gTIMERG8_100hzTimerConfig = {
    .period     = TIMERG8_100hz_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMERG8_100hz_init(void) {

    DL_TimerG_setClockConfig(TIMERG8_100hz_INST,
        (DL_TimerG_ClockConfig *) &gTIMERG8_100hzClockConfig);

    DL_TimerG_initTimerMode(TIMERG8_100hz_INST,
        (DL_TimerG_TimerConfig *) &gTIMERG8_100hzTimerConfig);
    DL_TimerG_enableInterrupt(TIMERG8_100hz_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_enableClock(TIMERG8_100hz_INST);





}

/*
 * Timer clock configuration to be sourced by BUSCLK /  (10000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   10000000 Hz = 10000000 Hz / (8 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gTIMERG6_1000hzClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMERG6_1000hz_INST_LOAD_VALUE = (1 ms * 10000000 Hz) - 1
 */
static const DL_TimerG_TimerConfig gTIMERG6_1000hzTimerConfig = {
    .period     = TIMERG6_1000hz_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMERG6_1000hz_init(void) {

    DL_TimerG_setClockConfig(TIMERG6_1000hz_INST,
        (DL_TimerG_ClockConfig *) &gTIMERG6_1000hzClockConfig);

    DL_TimerG_initTimerMode(TIMERG6_1000hz_INST,
        (DL_TimerG_TimerConfig *) &gTIMERG6_1000hzTimerConfig);
    DL_TimerG_enableInterrupt(TIMERG6_1000hz_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_enableClock(TIMERG6_1000hz_INST);





}



static const DL_UART_Main_ClockConfig gUART_0ClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_0Config = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_0_init(void)
{
    DL_UART_Main_setClockConfig(UART_0_INST, (DL_UART_Main_ClockConfig *) &gUART_0ClockConfig);

    DL_UART_Main_init(UART_0_INST, (DL_UART_Main_Config *) &gUART_0Config);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(UART_0_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_0_INST, UART_0_IBRD_40_MHZ_115200_BAUD, UART_0_FBRD_40_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_0_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);
    /* Setting the Interrupt Priority */
    NVIC_SetPriority(UART_0_INST_INT_IRQN, 1);


    DL_UART_Main_enable(UART_0_INST);
}

static const DL_UART_Main_ClockConfig gUART_1ClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_1Config = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_1_init(void)
{
    DL_UART_Main_setClockConfig(UART_1_INST, (DL_UART_Main_ClockConfig *) &gUART_1ClockConfig);

    DL_UART_Main_init(UART_1_INST, (DL_UART_Main_Config *) &gUART_1Config);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 256000
     *  Actual baud rate: 256000
     */
    DL_UART_Main_setOversampling(UART_1_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_1_INST, UART_1_IBRD_40_MHZ_256000_BAUD, UART_1_FBRD_40_MHZ_256000_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_1_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);
    /* Setting the Interrupt Priority */
    NVIC_SetPriority(UART_1_INST_INT_IRQN, 0);


    DL_UART_Main_enable(UART_1_INST);
}

static const DL_SPI_Config gSPI_Bmi088_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_8,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
};

static const DL_SPI_ClockConfig gSPI_Bmi088_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_SPI_Bmi088_init(void) {
    DL_SPI_setClockConfig(SPI_Bmi088_INST, (DL_SPI_ClockConfig *) &gSPI_Bmi088_clockConfig);

    DL_SPI_init(SPI_Bmi088_INST, (DL_SPI_Config *) &gSPI_Bmi088_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     10000000 = (80000000)/((1 + 3) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(SPI_Bmi088_INST, 3);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(SPI_Bmi088_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(SPI_Bmi088_INST);
}

/* ADCGraySensor Initialization */
static const DL_ADC12_ClockConfig gADCGraySensorClockConfig = {
    .clockSel       = DL_ADC12_CLOCK_SYSOSC,
    .divideRatio    = DL_ADC12_CLOCK_DIVIDE_8,
    .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,
};
SYSCONFIG_WEAK void SYSCFG_DL_ADCGraySensor_init(void)
{
    DL_ADC12_setClockConfig(ADCGraySensor_INST, (DL_ADC12_ClockConfig *) &gADCGraySensorClockConfig);
    DL_ADC12_initSingleSample(ADCGraySensor_INST,
        DL_ADC12_REPEAT_MODE_ENABLED, DL_ADC12_SAMPLING_SOURCE_AUTO, DL_ADC12_TRIG_SRC_SOFTWARE,
        DL_ADC12_SAMP_CONV_RES_12_BIT, DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);
    DL_ADC12_configConversionMem(ADCGraySensor_INST, ADCGraySensor_ADCMEM_C5,
        DL_ADC12_INPUT_CHAN_6, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_setPowerDownMode(ADCGraySensor_INST,DL_ADC12_POWER_DOWN_MODE_MANUAL);
    DL_ADC12_setSampleTime0(ADCGraySensor_INST,0);
    DL_ADC12_enableConversions(ADCGraySensor_INST);
}

SYSCONFIG_WEAK void SYSCFG_DL_SYSTICK_init(void)
{
    /*
     * Initializes the SysTick period to 1.00 ms,
     * enables the interrupt, and starts the SysTick Timer
     */
    DL_SYSTICK_config(80000);
}

static const DL_MCAN_ClockConfig gMCAN0ClockConf = {
    .clockSel = DL_MCAN_FCLK_SYSPLLCLK1,
    .divider  = DL_MCAN_FCLK_DIV_1,
};

static const DL_MCAN_InitParams gMCAN0InitParams= {

/* Initialize MCAN Init parameters.    */
    .fdMode            = true,
    .brsEnable         = false,
    .txpEnable         = false,
    .efbi              = false,
    .pxhddisable       = false,
    .darEnable         = false,
    .wkupReqEnable     = false,
    .autoWkupEnable    = false,
    .emulationEnable   = false,
    .tdcEnable         = false,
    .wdcPreload        = 255,

/* Transmitter Delay Compensation parameters. */
    .tdcConfig.tdcf    = 10,
    .tdcConfig.tdco    = 6,
};


static const DL_MCAN_MsgRAMConfigParams gMCAN0MsgRAMConfigParams ={

    /* Standard ID Filter List Start Address. */
    .flssa                = MCAN0_INST_MCAN_STD_ID_FILT_START_ADDR,
    /* List Size: Standard ID. */
    .lss                  = MCAN0_INST_MCAN_STD_ID_FILTER_NUM,
    /* Extended ID Filter List Start Address. */
    .flesa                = MCAN0_INST_MCAN_EXT_ID_FILT_START_ADDR,
    /* List Size: Extended ID. */
    .lse                  = MCAN0_INST_MCAN_EXT_ID_FILTER_NUM,
    /* Tx Buffers Start Address. */
    .txStartAddr          = MCAN0_INST_MCAN_TX_BUFF_START_ADDR,
    /* Number of Dedicated Transmit Buffers. */
    .txBufNum             = MCAN0_INST_MCAN_TX_BUFF_SIZE,
    .txFIFOSize           = 0,
    /* Tx Buffer Element Size. */
    .txBufMode            = 0,
    .txBufElemSize        = DL_MCAN_ELEM_SIZE_64BYTES,
    /* Tx Event FIFO Start Address. */
    .txEventFIFOStartAddr = MCAN0_INST_MCAN_TX_EVENT_START_ADDR,
    /* Event FIFO Size. */
    .txEventFIFOSize      = MCAN0_INST_MCAN_TX_EVENT_SIZE,
    /* Level for Tx Event FIFO watermark interrupt. */
    .txEventFIFOWaterMark = 3,
    /* Rx FIFO0 Start Address. */
    .rxFIFO0startAddr     = MCAN0_INST_MCAN_FIFO_0_START_ADDR,
    /* Number of Rx FIFO elements. */
    .rxFIFO0size          = MCAN0_INST_MCAN_FIFO_0_NUM,
    /* Rx FIFO0 Watermark. */
    .rxFIFO0waterMark     = 3,
    .rxFIFO0OpMode        = 0,
    /* Rx FIFO1 Start Address. */
    .rxFIFO1startAddr     = MCAN0_INST_MCAN_FIFO_1_START_ADDR,
    /* Number of Rx FIFO elements. */
    .rxFIFO1size          = MCAN0_INST_MCAN_FIFO_1_NUM,
    /* Level for Rx FIFO 1 watermark interrupt. */
    .rxFIFO1waterMark     = 3,
    /* FIFO blocking mode. */
    .rxFIFO1OpMode        = 0,
    /* Rx Buffer Start Address. */
    .rxBufStartAddr       = MCAN0_INST_MCAN_RX_BUFF_START_ADDR,
    /* Rx Buffer Element Size. */
    .rxBufElemSize        = DL_MCAN_ELEM_SIZE_64BYTES,
    /* Rx FIFO0 Element Size. */
    .rxFIFO0ElemSize      = DL_MCAN_ELEM_SIZE_64BYTES,
    /* Rx FIFO1 Element Size. */
    .rxFIFO1ElemSize      = DL_MCAN_ELEM_SIZE_64BYTES,
};



static const DL_MCAN_BitTimingParams   gMCAN0BitTimes = {
    /* Arbitration Baud Rate Pre-scaler. */
    .nomRatePrescalar   = 0,
    /* Arbitration Time segment before sample point. */
    .nomTimeSeg1        = 138,
    /* Arbitration Time segment after sample point. */
    .nomTimeSeg2        = 19,
    /* Arbitration (Re)Synchronization Jump Width Range. */
    .nomSynchJumpWidth  = 19,
    /* Data Baud Rate Pre-scaler. */
    .dataRatePrescalar  = 0,
    /* Data Time segment before sample point. */
    .dataTimeSeg1       = 0,
    /* Data Time segment after sample point. */
    .dataTimeSeg2       = 0,
    /* Data (Re)Synchronization Jump Width.   */
    .dataSynchJumpWidth = 0,
};


SYSCONFIG_WEAK void SYSCFG_DL_MCAN0_init(void) {
    DL_MCAN_RevisionId revid_MCAN0;

    DL_MCAN_enableModuleClock(MCAN0_INST);

    DL_MCAN_setClockConfig(MCAN0_INST, (DL_MCAN_ClockConfig *) &gMCAN0ClockConf);

    /* Get MCANSS Revision ID. */
    DL_MCAN_getRevisionId(MCAN0_INST, &revid_MCAN0);

    /* Wait for Memory initialization to be completed. */
    while(false == DL_MCAN_isMemInitDone(MCAN0_INST));

    /* Put MCAN in SW initialization mode. */

    DL_MCAN_setOpMode(MCAN0_INST, DL_MCAN_OPERATION_MODE_SW_INIT);

    /* Wait till MCAN is not initialized. */
    while (DL_MCAN_OPERATION_MODE_SW_INIT != DL_MCAN_getOpMode(MCAN0_INST));

    /* Initialize MCAN module. */
    DL_MCAN_init(MCAN0_INST, (DL_MCAN_InitParams *) &gMCAN0InitParams);


    /* Configure Bit timings. */
    DL_MCAN_setBitTime(MCAN0_INST, (DL_MCAN_BitTimingParams*) &gMCAN0BitTimes);

    /* Configure Message RAM Sections */
    DL_MCAN_msgRAMConfig(MCAN0_INST, (DL_MCAN_MsgRAMConfigParams*) &gMCAN0MsgRAMConfigParams);



    /* Set Extended ID Mask. */
    DL_MCAN_setExtIDAndMask(MCAN0_INST, MCAN0_INST_MCAN_EXT_ID_AND_MASK );

    /* Loopback mode */

    /* Take MCAN out of the SW initialization mode */
    DL_MCAN_setOpMode(MCAN0_INST, DL_MCAN_OPERATION_MODE_NORMAL);

    while (DL_MCAN_OPERATION_MODE_NORMAL != DL_MCAN_getOpMode(MCAN0_INST));


}

