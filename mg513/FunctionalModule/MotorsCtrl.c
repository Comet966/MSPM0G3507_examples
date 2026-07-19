#include "headfile.h"
#include "MotorsCtrl.h"
#include "Pwm.h"
#include "Encoder.h"
#include "pid.h"

/*
 * Phase 4: two-wheel closed-loop control.
 *   MotorPidCtrl(turnAngle, avgSpeed):
 *     1. self-turn PID: turnAngle setpoint -> wheel-speed differential (cm/s)
 *     2. per-wheel speed PID: (avgSpeed +/- diff) vs encoder V -> PWM duty
 * Open-loop helpers from Phase 3 remain for bring-up / testing.
 */

static const fp32 BrushMotor_PID[3] = { BrushMotor_PID_KP, BrushMotor_PID_KI, BrushMotor_PID_KD };
static const fp32 SelfTurn_PID[3]   = { SelfTurn_PID_KP,   SelfTurn_PID_KI,   SelfTurn_PID_KD   };

void MotorInit(Motors_t** Motors)
{
    static Motors_t    motors;
    static BrushMotor_t motorLeft;
    static BrushMotor_t motorRight;
    static Pid_t        pidLeft;
    static Pid_t        pidRight;
    static Pid_t        pidSelfTurn;

    *Motors = &motors;

    motors.MotorLeft   = &motorLeft;
    motors.MotorRight  = &motorRight;
    motors.PidLeft     = &pidLeft;
    motors.PidRight    = &pidRight;
    motors.PidSelfTurn = &pidSelfTurn;

    motorLeft.state  = Disable;  motorLeft.ExpectOutput  = 0;  motorLeft.Output  = 0;
    motorRight.state = Disable;  motorRight.ExpectOutput = 0;  motorRight.Output = 0;

    PID_init(motors.PidLeft,     BrushMotor_PID_mode, BrushMotor_PID, BrushMotor_PID_Maxout, BrushMotor_PID_MaxIout);
    PID_init(motors.PidRight,    BrushMotor_PID_mode, BrushMotor_PID, BrushMotor_PID_Maxout, BrushMotor_PID_MaxIout);
    PID_init(motors.PidSelfTurn, SelfTurn_PID_mode,   SelfTurn_PID,   SelfTurn_PID_Maxout,   SelfTurn_PID_MaxIout);

    EncodersInit(&motors);   /* allocate encoders + enable EXTI */

    PWMInit();               /* STBY on, TIMA0 running, wheels stopped */
}

void MotorStop(Motors_t* Motors)
{
    Motors->MotorLeft->Output  = 0;
    Motors->MotorRight->Output = 0;
    PID_clear(Motors->PidLeft);
    PID_clear(Motors->PidRight);
    PID_clear(Motors->PidSelfTurn);
    MotorDataUpdate(Motors);
}

void MotorSetOpenLoop(Motors_t* Motors, int16_t left, int16_t right)
{
    Motors->MotorLeft->Output  = left;
    Motors->MotorRight->Output = right;
    MotorDataUpdate(Motors);
}

void MotorPidCtrl(Motors_t* Motors, fp32 turnAngle, fp32 avgSpeed)
{
    /* Self-turn: drive turnAngle toward 0 -> speed differential (cm/s). */
    fp32 diff = PID_Calc(Motors->PidSelfTurn, turnAngle, 0.0f);

    Motors->MotorLeft->ExpectOutput  = (int)(avgSpeed - diff);
    Motors->MotorRight->ExpectOutput = (int)(avgSpeed + diff);

    /* Per-wheel speed PID: feedback = encoder linear velocity (cm/s).
     * Only computes each motor's Output; the caller pushes it to hardware via
     * MotorDataUpdate() (kept separate to match the reference control chain). */
    Motors->MotorLeft->Output  = (int)PID_Calc(Motors->PidLeft,
                                    Motors->EncoderLeft->V,  (fp32)Motors->MotorLeft->ExpectOutput);
    Motors->MotorRight->Output = (int)PID_Calc(Motors->PidRight,
                                    Motors->EncoderRight->V, (fp32)Motors->MotorRight->ExpectOutput);
}

void MotorDataUpdate(Motors_t* Motors)
{
    /* Single hardware choke point — direction/swap fixups applied here so every
     * drive path (line-follow, PID, open-loop, stop) stays coherent. */
    int16_t l = (int16_t)Motors->MotorLeft->Output;
    int16_t r = (int16_t)Motors->MotorRight->Output;

#if MOTOR_INVERT_DIR
    l = (int16_t)(-l);   /* flip forward/reverse (both wheels equally → steering preserved) */
    r = (int16_t)(-r);
#endif

#if MOTOR_SWAP_LR
    PWM_SetDuty(r, l);   /* physical L/R wheels swapped */
#else
    PWM_SetDuty(l, r);
#endif
}
