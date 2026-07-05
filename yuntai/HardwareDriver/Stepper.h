#ifndef __STEPPER_H
#define __STEPPER_H

#include <stdint.h>
#include <stdbool.h>

/*
 * D36A dual-channel stepper driver (STEP / DIR / EN) for two MS42C motors.
 *
 * Each axis owns one TimerA PWM channel that emits the STEP pulse train
 * (PAN=TIMA1/PB4, TILT=TIMA0/PB14, both at a 4 MHz timer clock). The timer's
 * ZERO-event interrupt fires once per completed pulse; the ISR counts steps and
 * stops the timer when the requested move finishes — so Stepper_Move is
 * non-blocking and position is always known.
 *
 * D36A logic: STEP = one step per rising edge, DIR = level selects direction,
 * EN = HIGH enables the driver (LOW = sleep, motor free to turn by hand).
 */

typedef enum {
    STEPPER_PAN  = 0,
    STEPPER_TILT = 1,
    STEPPER_COUNT
} StepperId;

void    Stepper_Init(void);                          /* stop timers, disable, STEP idle low, zero position */

void    Stepper_Enable(StepperId id, bool on);       /* EN pin: true = drive enabled */
bool    Stepper_IsEnabled(StepperId id);

void    Stepper_SetSpeed(StepperId id, uint16_t sps);/* steps/sec; clamped; takes effect on next Move */
uint16_t Stepper_GetSpeed(StepperId id);

/* Start a relative move of |steps| in the sign's direction (non-blocking).
 * A new Move while busy is queued as absolute-additive: the remaining count is
 * replaced, so call Stepper_Stop first if you need a clean re-target. */
void    Stepper_Move(StepperId id, int32_t steps);

bool    Stepper_Busy(StepperId id);                  /* true while a move is running */
int32_t Stepper_Position(StepperId id);              /* signed absolute step position */
void    Stepper_ZeroPosition(StepperId id);          /* define current position as 0 */
void    Stepper_Stop(StepperId id);                  /* immediate stop, STEP driven low */

#endif /* __STEPPER_H */
