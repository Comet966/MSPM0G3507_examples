#ifndef __RS485_BUS_H
#define __RS485_BUS_H

/*
 * rs485_bus.h — 双路独立 UART transport，无 RS485 收发器.
 *   UART1_PAN  = UART1  PA8(TX) / PA9(RX)   115200 8N1  → Pan  PD42S1 A端子
 *   UART0_TILT = UART0  PA10(TX)/ PA11(RX)  115200 8N1  → Tilt PD42S1 A端子
 *   两路 B 端子各自接 GND.
 */

#include "PD42S1.h"

void rs485_bus_init(void);

const pd42_bus_t *rs485_get_bus_pan(void);
const pd42_bus_t *rs485_get_bus_tilt(void);

#endif /* __RS485_BUS_H */
