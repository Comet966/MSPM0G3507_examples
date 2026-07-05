#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include <stdbool.h>

/*
 * UART0 = debug/command console on the XDS110 backchannel (PA10/PA11, 115200 8N1).
 * UART2 = K230 vision link (PB16/PB15); its RX ISR feeds the Vision parser.
 *
 * UART0 RX is buffered in a ring; the main loop pulls whole '\n'/'\r'-terminated
 * lines with Uart_ReadLine (non-blocking). TX is simple blocking (console only).
 */

void Uart_Init(void);                       /* arm both RX interrupts */

/* UART0 console output */
void Uart_SendChar(char c);
void Uart_SendString(const char *s);
void Uart_Printf(const char *fmt, ...);     /* blocking, console only — never in a hot ISR */

/* UART0 console input: copies one line (without terminator) into buf, returns
 * true when a full line is ready. Non-blocking; returns false otherwise. */
bool Uart_ReadLine(char *buf, uint16_t bufLen);

#endif /* __UART_H */
