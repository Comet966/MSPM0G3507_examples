#include "headfile.h"
#include "MotorsCtrl.h"

static fp32 BrushMotor_PID[3] = {BrushMotor_PID_KP, BrushMotor_PID_KI, BrushMotor_PID_KD};
static fp32 SelfTurn_PID[3]   = {SelfTurn_PID_KP,   SelfTurn_PID_KI,   SelfTurn_PID_KD};

void MotorInit(Motors_t** Motors)
{
    static Motors_t     motors;
    static BrushMotor_t motorLeft, motorRight;
    static Pid_t        pidLeft, pidRight, pidSelfTurn;

    *Motors = &motors;

    motors.MotorLeft   = &motorLeft;
    motors.MotorRight  = &motorRight;
    motors.PidLeft     = &pidLeft;
    motors.PidRight    = &pidRight;
    motors.PidSelfTurn = &pidSelfTurn;

    motorLeft.state  = Disable;
    motorLeft.ExpectOutput  = 0;
    motorLeft.Output = 0;
    motorRight.state = Disable;
    motorRight.ExpectOutput = 0;
    motorRight.Output = 0;

    PID_init(&pidLeft,     BrushMotor_PID_mode, BrushMotor_PID, BrushMotor_PID_Maxout, BrushMotor_PID_MaxIout);
    PID_init(&pidRight,    BrushMotor_PID_mode, BrushMotor_PID, BrushMotor_PID_Maxout, BrushMotor_PID_MaxIout);
    PID_init(&pidSelfTurn, SelfTurn_PID_mode,   SelfTurn_PID,   SelfTurn_PID_Maxout,   SelfTurn_PID_MaxIout);

    /* STBY high — enable TB6612 outputs */
    DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_STBY_PIN);

    PWMStart(PWM_Motors_INST);
    PWM_Output(0, 0);
}

void MotorDirectionSet(Motors_t* Motors)
{
    /* Left: AIN1/AIN2. Forward (Output>0): AIN1=0, AIN2=1 */
    if (Motors->MotorLeft->Output > 0) {
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_AIN1_PIN);
        DL_GPIO_setPins  (GPIO_Motor_PORT, GPIO_Motor_AIN2_PIN);
    } else {
        DL_GPIO_setPins  (GPIO_Motor_PORT, GPIO_Motor_AIN1_PIN);
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_AIN2_PIN);
    }

    /* Right: BIN1/BIN2. Forward (Output>0): BIN1=0, BIN2=1 */
    if (Motors->MotorRight->Output > 0) {
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_BIN1_PIN);
        DL_GPIO_setPins  (GPIO_Motor_PORT, GPIO_Motor_BIN2_PIN);
    } else {
        DL_GPIO_setPins  (GPIO_Motor_PORT, GPIO_Motor_BIN1_PIN);
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_BIN2_PIN);
    }
}

void MotorsPWMSet(Motors_t* Motors)
{
    PWM_Output((uint16_t)ABS(Motors->MotorLeft->Output),
               (uint16_t)ABS(Motors->MotorRight->Output));
}

void MotorPidCtrl(Motors_t* Motors, fp32 TurnAngleSet, fp32 AverageSpeedSet)
{
    float diff = PID_Calc(Motors->PidSelfTurn, TurnAngleSet, 0);

    Motors->MotorLeft->ExpectOutput  = (int)(AverageSpeedSet - diff);
    Motors->MotorRight->ExpectOutput = (int)(AverageSpeedSet + diff);

    Motors->MotorLeft->Output  = (int)PID_Calc(Motors->PidLeft,  Motors->EncoderLeft->V,  (float)Motors->MotorLeft->ExpectOutput);
    Motors->MotorRight->Output = (int)PID_Calc(Motors->PidRight, Motors->EncoderRight->V, (float)Motors->MotorRight->ExpectOutput);
}

void MotorStop(Motors_t* Motors)
{
    Motors->MotorLeft->Output  = 0;
    Motors->MotorRight->Output = 0;
    MotorDataUpdate(Motors);
}

void MotorDataUpdate(Motors_t* Motors)
{
    MotorDirectionSet(Motors);
    MotorsPWMSet(Motors);
}
