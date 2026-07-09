#include "headfile.h"
#include "Uart.h"

extern Car_t Car;

void UsartInit(void)
{
    NVIC_ClearPendingIRQ(UART0_Debug_INST_INT_IRQN);
    NVIC_EnableIRQ(UART0_Debug_INST_INT_IRQN);
    DL_UART_clearInterruptStatus(UART0_Debug_INST, DL_UART_INTERRUPT_RX);
}

void UART0_Debug_INST_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART0_Debug_INST)) {
        case DL_UART_IIDX_RX: {
            uint8_t ch = DL_UART_Main_receiveData(UART0_Debug_INST);
            /* Serial remote start/stop: 0xA5 = start, 0xA6 = stop. */
            if (ch == 0xA5) CarStart(Car);
            if (ch == 0xA6) CarStop(Car);
            uart0_send_char((char)ch);
            break;
        }
        default:
            break;
    }
}

void uart0_send_char(char ch)
{
    while (DL_UART_isBusy(UART0_Debug_INST) == true);
    DL_UART_Main_transmitData(UART0_Debug_INST, (uint8_t)ch);
}

void uart_printf(int data)
{
    char temp[64];
    int len = sprintf(temp, "%d\n", data);
    for (int i = 0; i < len; i++)
        uart0_send_char(temp[i]);
}

void usart0_send_string(const char* s)
{
    while (*s)
        uart0_send_char(*s++);
}
