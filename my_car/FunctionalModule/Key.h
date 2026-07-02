#ifndef __KEY_H
#define __KEY_H

#include "datatype.h"

/* Start/stop button on PB21 (board S2). Polled + debounced at 200 Hz.
 * Short press toggles Car->Tasks->CarStartFlag and beeps once.
 * (Full lap/start semantics are added with the Task state machine in Phase 6.) */

void KeyDataUpdate(Car_t* Car);        /* call at 200 Hz */
void KeyShortPressProcess(Car_t* Car);
void KeyLongPressProcess(Car_t* Car);

#endif /* __KEY_H */
