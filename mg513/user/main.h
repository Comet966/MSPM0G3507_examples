#ifndef __MAIN_H
#define __MAIN_H

#include "ti_msp_dl_config.h"
#include "datatype.h"

/* Global car aggregator (defined in main.c). */
extern Car_t Car;

/* Fixed-rate scheduler callbacks, invoked from the Timer ISR (HardwareDriver/Timer.c).
 * duty_200hz is the master control tick; 100/10/1000 Hz are divided down from it. */
void duty_1000hz(void);
void duty_200hz(void);
void duty_100hz(void);
void duty_10hz(void);

#endif /* __MAIN_H */
