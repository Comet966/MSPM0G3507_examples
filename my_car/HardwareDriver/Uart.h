#ifndef __UART_H
#define __UART_H

#include <stdint.h>

/* UART0 = debug backchannel (PA10/PA11, 115200). UART1/UART2 added in later phases. */

void UsartInit(void);              /* enable UART RX interrupts */

void uart0_send_char(char ch);     /* blocking single-byte TX on UART0 */
void uart0_send_string(const char *s);
void uart_printf(int data);        /* print an int + newline over UART0 (debug) */

#endif /* __UART_H */
