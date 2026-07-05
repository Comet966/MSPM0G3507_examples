#include "ti_msp_dl_config.h"
#include "Uart.h"
#include "Vision.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/*
 * UART0 debug console + UART2 K230 link.
 *
 * UART0 RX ISR pushes bytes into a power-of-two ring buffer. Uart_ReadLine drains
 * the ring in the main loop and assembles one line at a time (terminated by CR or
 * LF), so the command parser never blocks in the ISR. TX is blocking — fine for a
 * human-facing console, and there is no control loop it can starve on this board.
 *
 * UART2 RX ISR forwards each byte straight to the Vision frame assembler.
 */

/*--------------------------- UART0 RX ring buffer ---------------------------*/
#define RX_RING_SIZE   128U                 /* must be a power of two */
#define RX_RING_MASK   (RX_RING_SIZE - 1U)

static volatile uint8_t  s_rx[RX_RING_SIZE];
static volatile uint16_t s_rxHead = 0;      /* ISR writes */
static volatile uint16_t s_rxTail = 0;      /* main reads */

/* Line-assembly state for Uart_ReadLine (main-loop context only). */
static char     s_line[96];
static uint16_t s_lineLen = 0;

void Uart_Init(void)
{
    NVIC_ClearPendingIRQ(UART0_Debug_INST_INT_IRQN);
    NVIC_EnableIRQ(UART0_Debug_INST_INT_IRQN);
    DL_UART_clearInterruptStatus(UART0_Debug_INST, DL_UART_INTERRUPT_RX);

    NVIC_ClearPendingIRQ(UART2_K230_INST_INT_IRQN);
    NVIC_EnableIRQ(UART2_K230_INST_INT_IRQN);
    DL_UART_clearInterruptStatus(UART2_K230_INST, DL_UART_INTERRUPT_RX);
}

/*------------------------------- UART0 TX --------------------------------*/
void Uart_SendChar(char c)
{
    while (DL_UART_isBusy(UART0_Debug_INST)) { /* wait for FIFO/shifter */ }
    DL_UART_Main_transmitData(UART0_Debug_INST, (uint8_t)c);
}

void Uart_SendString(const char *s)
{
    while (*s) Uart_SendChar(*s++);
}

void Uart_Printf(const char *fmt, ...)
{
    char buf[128];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if (n > (int)sizeof(buf)) n = sizeof(buf);
    for (int i = 0; i < n; i++) Uart_SendChar(buf[i]);
}

/*------------------------------- UART0 RX --------------------------------*/
bool Uart_ReadLine(char *buf, uint16_t bufLen)
{
    while (s_rxTail != s_rxHead) {
        char c = (char)s_rx[s_rxTail];
        s_rxTail = (s_rxTail + 1U) & RX_RING_MASK;

        if (c == '\n' || c == '\r') {
            if (s_lineLen == 0) continue;         /* skip empty lines / CRLF pairs */
            uint16_t n = (s_lineLen < bufLen - 1U) ? s_lineLen : (bufLen - 1U);
            memcpy(buf, s_line, n);
            buf[n] = '\0';
            s_lineLen = 0;
            return true;
        }

        if (s_lineLen < sizeof(s_line) - 1U) {
            s_line[s_lineLen++] = c;
        } else {
            s_lineLen = 0;                         /* overflow: drop the runaway line */
        }
    }
    return false;
}

/*------------------------------- ISRs --------------------------------*/
void UART0_Debug_INST_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART0_Debug_INST)) {
        case DL_UART_IIDX_RX: {
            uint8_t rx = DL_UART_Main_receiveData(UART0_Debug_INST);
            uint16_t next = (s_rxHead + 1U) & RX_RING_MASK;
            if (next != s_rxTail) {                 /* drop on overflow, never block */
                s_rx[s_rxHead] = rx;
                s_rxHead = next;
            }
            break;
        }
        default:
            break;
    }
}

void UART2_K230_INST_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART2_K230_INST)) {
        case DL_UART_IIDX_RX: {
            uint8_t rx = DL_UART_Main_receiveData(UART2_K230_INST);
            Vision_Feed(rx);
            break;
        }
        default:
            break;
    }
}
