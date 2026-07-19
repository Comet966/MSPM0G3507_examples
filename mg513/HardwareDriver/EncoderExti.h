#ifndef __ENCODEREXTI_H
#define __ENCODEREXTI_H

#include "datatype.h"

/*
 * Quadrature encoder edge counting via GPIO interrupt (HardwareDriver layer).
 *
 * Wiring (11_PID_car pin map, split across two ports):
 *   Left  wheel: A = PA21 (interrupt, rising edge), B = PA22 (dir level)  -> GPIOA
 *   Right wheel: A = PB19 (interrupt, rising edge), B = PB20 (dir level)  -> GPIOB
 *
 * GPIOA and GPIOB both feed the shared INT_GROUP1 vector -> a single
 * GROUP1_IRQHandler dispatches both encoders. On each rising A edge we read the
 * B-phase level to decide direction and accumulate Car.Motors->EncoderX->EncoderCount.
 * The ISR is deliberately minimal (two reads + a counter tick per event).
 *
 * Direction sign is validated on hardware; flip the B-level test in the ISR
 * if a wheel counts the wrong way.
 */

void EncoderExtiInit(void);   /* enable GPIOA + GPIOB interrupts in NVIC */

#endif /* __ENCODEREXTI_H */
