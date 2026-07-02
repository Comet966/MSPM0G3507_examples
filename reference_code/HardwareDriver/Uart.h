#ifndef __UART_H
#define __UART_H

#include "headfile.h"


void UART_SendBytes(UART_Regs *port,uint8_t *ubuf, uint32_t len);
void UART_SendByte(UART_Regs *port,uint8_t data);
void uart_printf(int data);
void uart0_send_char(char ch);
void usart0_send_string(uint8_t *ucstr);
void uart1_restart(void);
void UsartInit(void);
void process_data(void);
	
extern uint8_t ucTemp;
extern uint8_t data1;
extern uint8_t data2;
extern  int  z;


#endif



