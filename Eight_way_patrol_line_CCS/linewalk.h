#ifndef LINEWALK_H_
#define LINEWALK_H_

#include <stdint.h>

/* Initialize CLK output (idle-low). */
void LineWalk_init(void);

/*
 * Read one 8-bit frame from the Ganwei sensor via CLK+DAT serial mode.
 * Returns: bit0=CH1 ... bit7=CH8.  1=white/light, 0=black/dark.
 * Blocks ~1 ms (8 clock cycles + 1 ms reset delay).
 */
uint8_t LineWalk_read(void);

#endif /* LINEWALK_H_ */
