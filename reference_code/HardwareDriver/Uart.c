/* 头文件包含 ----------------------------------------------------------------*/
#include "ti_msp_dl_config.h"    // TI MSP系列芯片底层驱动库
#include "uart.h"              // 系统级配置


/* 全局变量定义 --------------------------------------------------------------*/

volatile unsigned char uart_data = 0;

volatile uint8_t new_data_flag = 0;  

/* 自定义通信协议帧配置 -------------------------------------------------------*/


/* 函数定义 ----------------------------------------------------------------*/

/**
  * @brief  UART中断配置函数
  * @note   配置所有UART通道的中断使能和状态清除
  *         适用于TI MSPM0系列芯片的UART配置
  */
void UsartInit(void)
{
  // 清除所有UART通道的中断挂起状态
  NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
	NVIC_ClearPendingIRQ(UART_1_INST_INT_IRQN);
//  NVIC_ClearPendingIRQ(UART_2_INST_INT_IRQN);
//  NVIC_ClearPendingIRQ(UART_3_INST_INT_IRQN);

  // 使能所有UART通道的接收中断
  NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
  NVIC_EnableIRQ(UART_1_INST_INT_IRQN);
//  NVIC_EnableIRQ(UART_2_INST_INT_IRQN);
//  NVIC_EnableIRQ(UART_3_INST_INT_IRQN);

  // 清除所有UART接收中断标志（防止残留中断）
  DL_UART_clearInterruptStatus(UART_0_INST,DL_UART_INTERRUPT_RX);
  DL_UART_clearInterruptStatus(UART_1_INST,DL_UART_INTERRUPT_RX);
 // DL_UART_clearInterruptStatus(UART_2_INST,DL_UART_INTERRUPT_RX);
//  DL_UART_clearInterruptStatus(UART_3_INST,DL_UART_INTERRUPT_RX);
	

}
//串口发送单个字符
extern Car_t Car;
extern uint16_t BeginProtectFlag;
extern uint16_t BeginProtectCount;

void UART_0_INST_IRQHandler(void)
{
    //如果产生了串口中断
    switch( DL_UART_getPendingInterrupt(UART_0_INST) )
    {
        case DL_UART_IIDX_RX://如果是接收中断
					
          uart_data = DL_UART_Main_receiveData(UART_0_INST);
				if(uart_data==0xA5)
					KeyShortPressProcess(&Car);
				if(uart_data==0xA6)
					KeyLongPressProcess(&Car);
					uart0_send_char(uart_data);
					break;

        default://其他的串口中断
            break;
    }
}

void UART_1_INST_IRQHandler(void)
{

  if(DL_UART_getEnabledInterruptStatus(UART_1_INST,DL_UART_INTERRUPT_RX) == DL_UART_INTERRUPT_RX)
  {
		DL_UART_clearInterruptStatus(UART_1_INST,DL_UART_INTERRUPT_RX);
		uint8_t ch = DL_UART_receiveData(UART_1_INST);
  }

}



void uart1_restart(void)
{
	DL_UART_clearInterruptStatus(UART_1_INST,DL_UART_INTERRUPT_RX);
	DL_UART_reset(UART_1_INST);
	DL_UART_disable(UART_1_INST);
	DL_UART_disablePower(UART_1_INST);
	DL_UART_enablePower(UART_1_INST);
	SYSCFG_DL_UART_1_init();
}


void uart0_send_char(char ch)
{
    //当串口0忙的时候等待，不忙的时候再发送传进来的字符
    while( DL_UART_isBusy(UART_0_INST) == true );
    //发送单个字符
    DL_UART_Main_transmitData(UART_0_INST, ch);
}

void uart_printf(int data)
{
	char temp[64];
	sprintf(temp, "%d\n", data);
	int a ,b = strlen((const char*)temp) ;
	for(a = 0;a<b;a++)
	uart0_send_char(temp[a]);
}
