#include "ti_msp_dl_config.h"
#include "Uart.h"
#include <stdio.h>
#include <string.h>

/* UART0 debug backchannel. SysConfig instance name = UART0_Debug -> UART0_Debug_INST. */

void UsartInit(void)
{
    NVIC_ClearPendingIRQ(UART0_Debug_INST_INT_IRQN);
    NVIC_EnableIRQ(UART0_Debug_INST_INT_IRQN);
    DL_UART_clearInterruptStatus(UART0_Debug_INST, DL_UART_INTERRUPT_RX);
}

/* UART0 RX ISR. For now, echo received bytes (loopback sanity for the console).
 * The 0xA5/0xA6 remote key-press protocol is wired in Phase 2 with Key.c. */
void UART0_Debug_INST_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART0_Debug_INST)) {
        case DL_UART_IIDX_RX: {
            uint8_t rx = DL_UART_Main_receiveData(UART0_Debug_INST);
            uart0_send_char((char)rx);   /* echo */
            break;
        }
        default:
            break;
    }
}

void uart0_send_char(char ch)
{
    while (DL_UART_isBusy(UART0_Debug_INST)) { /* wait */ }
    DL_UART_Main_transmitData(UART0_Debug_INST, (uint8_t)ch);
}

void uart0_send_string(const char *s)
{
    while (*s) {
        uart0_send_char(*s++);
    }
}

void uart_printf(int data)
{
    char temp[16];
    int n = snprintf(temp, sizeof(temp), "%d\n", data);
    for (int i = 0; i < n; i++) {
        uart0_send_char(temp[i]);
    }
}
