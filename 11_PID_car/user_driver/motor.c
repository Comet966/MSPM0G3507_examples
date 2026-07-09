#include "motor.h"

void motor_init(uint8_t motor_id)
{
    DL_GPIO_setPins(DC_MOTOR_STBY_PORT, DC_MOTOR_STBY_PIN);
    if(motor_id == 1){
        // DL_Timer_startCounter(PWMAB_INST);
        DL_GPIO_setPins(DC_MOTOR_AIN1_PORT, DC_MOTOR_AIN1_PIN);
        DL_GPIO_setPins(DC_MOTOR_AIN2_PORT, DC_MOTOR_AIN2_PIN);
        DL_Timer_setCaptureCompareValue(PWMAB_INST, 0, GPIO_PWMAB_C0_IDX);
    }
    else if(motor_id == 2){
        
        DL_GPIO_setPins(DC_MOTOR_BIN1_PORT, DC_MOTOR_BIN1_PIN);
        DL_GPIO_setPins(DC_MOTOR_BIN2_PORT, DC_MOTOR_BIN2_PIN);
        DL_Timer_setCaptureCompareValue(PWMAB_INST, 0, GPIO_PWMAB_C1_IDX);
    }
    DL_Timer_startCounter(PWMAB_INST);
    DL_Timer_startCounter(MOTOR_PID_INST);
    NVIC_EnableIRQ(MOTOR_PID_INST_INT_IRQN);
}

// 限幅函数
uint32_t limit_duty(uint32_t duty)
{
    if(duty > 4000){
        duty = 4000;
    }
    return duty;
}

void motor_set_duty(uint8_t motor_id, uint32_t duty)
{
    duty = limit_duty(duty);
    if(motor_id == 1){
        DL_Timer_setCaptureCompareValue(PWMAB_INST, duty, GPIO_PWMAB_C0_IDX);
    }
    else if(motor_id == 2){
        DL_Timer_setCaptureCompareValue(PWMAB_INST, duty, GPIO_PWMAB_C1_IDX);
    }
}

// direction: 0 停止，1 正转，2 反转
void motor_set_direction(uint8_t motor_id, uint8_t direction)
{
    if(motor_id == 1){
        if(direction == 0){
            DL_GPIO_setPins(DC_MOTOR_AIN1_PORT, DC_MOTOR_AIN1_PIN);
            DL_GPIO_setPins(DC_MOTOR_AIN2_PORT, DC_MOTOR_AIN2_PIN);
        }
        else if(direction == 1){
            DL_GPIO_setPins(DC_MOTOR_AIN1_PORT, DC_MOTOR_AIN1_PIN);
            DL_GPIO_clearPins(DC_MOTOR_AIN2_PORT, DC_MOTOR_AIN2_PIN);
        }
        else if(direction == 2){
            DL_GPIO_clearPins(DC_MOTOR_AIN1_PORT, DC_MOTOR_AIN1_PIN);
            DL_GPIO_setPins(DC_MOTOR_AIN2_PORT, DC_MOTOR_AIN2_PIN);
        }
    }
    else if(motor_id == 2){
        if(direction == 0){
            DL_GPIO_setPins(DC_MOTOR_BIN1_PORT, DC_MOTOR_BIN1_PIN);
            DL_GPIO_setPins(DC_MOTOR_BIN2_PORT, DC_MOTOR_BIN2_PIN);
        }
        else if(direction == 1){
            DL_GPIO_setPins(DC_MOTOR_BIN1_PORT, DC_MOTOR_BIN1_PIN);
            DL_GPIO_clearPins(DC_MOTOR_BIN2_PORT, DC_MOTOR_BIN2_PIN);
        }
        else if(direction == 2){
            DL_GPIO_clearPins(DC_MOTOR_BIN1_PORT, DC_MOTOR_BIN1_PIN);
            DL_GPIO_setPins(DC_MOTOR_BIN2_PORT, DC_MOTOR_BIN2_PIN);
        }
    }
}


extern uint32_t counter_1_A;
float speed_1 = 0;

extern uint32_t counter_2_A;
float speed_2 = 0;

// 一阶低通滤波系数，0~1，越小越平滑但越滞后
#define SPEED_FILTER_ALPHA 0.4f

void calculate_speed(uint8_t motor_id)
{
    if (motor_id == 1) {
        // 每个脉冲约 58 mm/s，量化较粗，用低通滤波平滑
        float raw = (float)counter_1_A / MOTOR_BIANMAQI * PI * MOTOR_WHEEL_D * 100; // 轮速 mm/s
        speed_1 += SPEED_FILTER_ALPHA * (raw - speed_1);
        counter_1_A = 0; // 计算完速度后清零计数器
    }
    if (motor_id == 2) {
        float raw = (float)counter_2_A / MOTOR_BIANMAQI * PI * MOTOR_WHEEL_D * 100; // 轮速 mm/s
        speed_2 += SPEED_FILTER_ALPHA * (raw - speed_2);
        counter_2_A = 0; // 计算完速度后清零计数器
    }
}

float kp = 0.4; // 比例系数
float ki = 0.2; // 积分系数

// PWM 累加器改为 float，避免无符号下溢回绕导致瞬间钳到满油门
float PWM_1_duty = 0;
float target_speed_1 = 0; // 目标速度 mm/s
float last_error_1 = 0;
float current_error_1 = 0;

float PWM_2_duty = 0;
float target_speed_2 = 0; // 目标速度 mm/s
float last_error_2 = 0;
float current_error_2 = 0;

// 累加器双向限幅 [0, 4000]，用 float 做，负增量正常回落而不会回绕
static float clamp_duty_f(float d)
{
    if (d > 4000.0f) return 4000.0f;
    if (d < 0.0f)    return 0.0f;
    return d;
}

// 增量式 PI：Δduty = kp*(e-e_last) + ki*e
void DC_MOTOR_PID(uint8_t motor_id)
{
    if (motor_id == 1) {
        current_error_1 = target_speed_1 - speed_1;
        PWM_1_duty += kp * (current_error_1 - last_error_1) + ki * current_error_1;
        last_error_1 = current_error_1;
        PWM_1_duty = clamp_duty_f(PWM_1_duty);
        motor_set_duty(1, (uint32_t)PWM_1_duty);
    }
    if (motor_id == 2) {
        current_error_2 = target_speed_2 - speed_2;
        PWM_2_duty += kp * (current_error_2 - last_error_2) + ki * current_error_2;
        last_error_2 = current_error_2;
        PWM_2_duty = clamp_duty_f(PWM_2_duty);
        motor_set_duty(2, (uint32_t)PWM_2_duty);
    }
}

// 1 = 开环测试：跳过 adjust_motor 和 PID，占空比由 main 直接给
volatile uint8_t open_loop_test = 0;

void MOTOR_PID_INST_IRQHandler()
{
    switch (DL_Timer_getPendingInterrupt(MOTOR_PID_INST))
    {
    case DL_TIMER_IIDX_LOAD:
        // 无论什么模式都算速度，保证 OLED 能看到编码器读数
        calculate_speed(1);
        calculate_speed(2);
        if (!open_loop_test) {
            adjust_motor();
            DC_MOTOR_PID(1);
            DC_MOTOR_PID(2);
        }
        break;
    // case DL_TIMER_IIDX_COMPARE_0:
    //     status = (status + 3 -1) % 3;
    //     /* code */
    //     break;
    
    default:
        break;
    }
}



