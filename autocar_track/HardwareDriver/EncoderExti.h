#ifndef __ENCODEREXTI_H
#define __ENCODEREXTI_H

#include "datatype.h"

/*
 * Quadrature encoder edge counting via GPIO interrupt (HardwareDriver layer).
 *
 * Wiring (see CLAUDE.md pin map):
 *   Left  wheel: A = PB12 (input), B = PB13 (interrupt, rising edge)
 *   Right wheel: A = PB20 (input), B = PB7  (interrupt, rising edge)
 *
 * All Port-B pin interrupts share one GPIOB_INT_IRQn vector -> a single
 * GROUP1_IRQHandler dispatches both encoders. On each rising B edge we read the
 * A-phase level to decide direction and accumulate Car.Motors->EncoderX->EncoderCount.
 * The ISR is deliberately minimal (two reads + a counter tick per event).
 *
 * Direction sign is validated on hardware in Phase 4; flip the A-level test here
 * if a wheel counts the wrong way.
 */

void EncoderExtiInit(void);   /* enable GPIOB interrupt in NVIC */

#endif /* __ENCODEREXTI_H */
