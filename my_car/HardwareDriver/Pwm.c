#include "ti_msp_dl_config.h"
#include "Pwm.h"

/*
 * TB6612FNG dual-wheel driver over TIMA0 (2-channel, 20 kHz edge-aligned).
 *
 * Duty mapping (SDK TIMER.md, verified LP-MSPM0G3507):
 *   duty% = compare_value / period  ->  CC = duty counts (0..1000), DIRECT.
 *
 * Zero-duty caveat (SDK TIMER.md CRITICAL): CC = 0 does NOT reliably force the
 * PWM pin low in edge-aligned mode — it can latch HIGH. Relying on CC=0 for
 * "stop" left the pin high, and with a direction pin asserted the TB6612 drove
 * the motor at full speed on reset. So we do NOT depend on the PWM pin for stop:
 * a zero command puts the channel into COAST (both IN pins low), which the
 * TB6612 truth table halts regardless of the PWM line.
 *
 * TB6612 truth table (per motor):
 *   IN1=1 IN2=0 -> forward     IN1=0 IN2=1 -> reverse
 *   IN1=0 IN2=0 -> coast/stop  IN1=1 IN2=1 -> brake
 */

static inline void motor_left_dir(int16_t signed_duty)
{
    if (signed_duty > 0) {                                    /* forward */
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_AIN1_PIN);
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_AIN2_PIN);
    } else if (signed_duty < 0) {                             /* reverse */
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_AIN1_PIN);
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_AIN2_PIN);
    } else {                                                  /* coast (both low) */
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_AIN1_PIN | GPIO_Motor_AIN2_PIN);
    }
}

static inline void motor_right_dir(int16_t signed_duty)
{
    if (signed_duty > 0) {                                    /* forward */
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_BIN1_PIN);
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_BIN2_PIN);
    } else if (signed_duty < 0) {                             /* reverse */
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_BIN1_PIN);
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_BIN2_PIN);
    } else {                                                  /* coast (both low) */
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_BIN1_PIN | GPIO_Motor_BIN2_PIN);
    }
}

void PWMInit(void)
{
    DL_TimerA_startCounter(PWM_Motors_INST);
    PWM_SetDuty(0, 0);                                        /* coast + CC 0 */
    DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_STBY_PIN);    /* release TB6612 from standby */
}

void PWMStop(void)
{
    PWM_SetDuty(0, 0);                                        /* coast both wheels */
    DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_STBY_PIN);  /* standby: outputs Hi-Z */
}

void PWM_SetDuty(int16_t left, int16_t right)
{
    /* Clamp to [-MAX, +MAX]. */
    if (left  >  PWM_DUTY_MAX) left  =  PWM_DUTY_MAX;
    if (left  < -PWM_DUTY_MAX) left  = -PWM_DUTY_MAX;
    if (right >  PWM_DUTY_MAX) right =  PWM_DUTY_MAX;
    if (right < -PWM_DUTY_MAX) right = -PWM_DUTY_MAX;

    motor_left_dir(left);
    motor_right_dir(right);

    uint16_t magL = (left  >= 0) ? (uint16_t)left  : (uint16_t)(-left);
    uint16_t magR = (right >= 0) ? (uint16_t)right : (uint16_t)(-right);

    /* Direct compare: duty% = CC / period, so CC = magnitude. Stop is handled by
     * coasting the direction pins above, not by trusting CC=0 to zero the pin. */
    DL_TimerA_setCaptureCompareValue(PWM_Motors_INST, magL, GPIO_PWM_Motors_C0_IDX);
    DL_TimerA_setCaptureCompareValue(PWM_Motors_INST, magR, GPIO_PWM_Motors_C1_IDX);
}
