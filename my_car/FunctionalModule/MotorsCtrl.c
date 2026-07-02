#include "ti_msp_dl_config.h"
#include "MotorsCtrl.h"
#include "Pwm.h"

/*
 * Phase 3: open-loop two-wheel control. The Motors_t aggregate holds left/right
 * BrushMotor_t whose signed Output (-1000..1000) is pushed to the TB6612.
 * Encoder pointers and PIDs are left NULL here; Phase 4 wires them up.
 */

void MotorInit(Motors_t** Motors)
{
    static Motors_t   motors;
    static BrushMotor_t motorLeft;
    static BrushMotor_t motorRight;

    *Motors = &motors;

    motors.MotorLeft  = &motorLeft;
    motors.MotorRight = &motorRight;
    /* EncoderLeft/Right, PidLeft/Right, PidSelfTurn stay NULL until Phase 4. */

    motorLeft.state  = Disable;  motorLeft.ExpectOutput  = 0;  motorLeft.Output  = 0;
    motorRight.state = Disable;  motorRight.ExpectOutput = 0;  motorRight.Output = 0;

    PWMInit();   /* STBY high, TIMA0 running, duty 0 -> wheels stopped but driver live */
}

void MotorStop(Motors_t* Motors)
{
    Motors->MotorLeft->Output  = 0;
    Motors->MotorRight->Output = 0;
    MotorDataUpdate(Motors);
}

void MotorSetOpenLoop(Motors_t* Motors, int16_t left, int16_t right)
{
    Motors->MotorLeft->Output  = left;
    Motors->MotorRight->Output = right;
    MotorDataUpdate(Motors);
}

void MotorDataUpdate(Motors_t* Motors)
{
    PWM_SetDuty((int16_t)Motors->MotorLeft->Output,
                (int16_t)Motors->MotorRight->Output);
}
