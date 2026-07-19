#ifndef __PWM_H
#define __PWM_H

#include "datatype.h"

/*
 * TB6612FNG dual-channel motor PWM (HardwareDriver layer).
 *
 * Wiring (11_PID_car pin map):
 *   PWMA = PA12 (TIMG0.CCP0) -> left  motor speed
 *   PWMB = PA13 (TIMG0.CCP1) -> right motor speed
 *   AIN1/AIN2 = PA9/PA8      -> left  motor direction  (GPIOA)
 *   BIN1      = PA7          -> right motor direction   (GPIOA)
 *   BIN2      = PB18         -> right motor direction   (GPIOB)
 *   STBY      = PB24         -> chip enable (high = active, low = standby/coast)  (GPIOB)
 *
 * PWM: TIMG0 @ 20 kHz, period count 1000 -> duty resolution 0.1 %.
 * Signed duty: sign selects direction via IN1/IN2, magnitude (0..1000) sets speed.
 */

#define PWM_DUTY_MAX  1000   /* full scale = 100 % duty (matches SysConfig timerCount) */

void PWMInit(void);                              /* enable TB6612 (STBY high) + start TIMG0, duty 0 */
void PWMStop(void);                              /* standby (STBY low), counter stopped, duty 0 */
void PWM_SetDuty(int16_t left, int16_t right);   /* -1000..1000 per wheel; sign = direction */

#endif /* __PWM_H */
