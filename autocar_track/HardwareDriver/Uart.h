#ifndef __UART_H
#define __UART_H

#include "datatype.h"

void UsartInit(void);
void uart0_send_char(char ch);
void uart_printf(int data);
void usart0_send_string(const char* s);

#endif
