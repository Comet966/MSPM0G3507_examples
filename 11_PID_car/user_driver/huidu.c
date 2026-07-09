#include "huidu.h"

uint8_t huidu_value[] = {0, 0, 0, 0, 0};

uint8_t get_gpio_state(GPIO_Regs *gpio_port, uint32_t gpio) {
    uint32_t high_bits = DL_GPIO_readPins(gpio_port, gpio);
    // 传感器检测到黑线输出低电平，取反使 1=检测到黑线
    if((high_bits & gpio) != 0) return 0;
    else return 1;
}

void huidu_get_value()
{
    huidu_value[0] = get_gpio_state(HUIDU_L2_PORT, HUIDU_L2_PIN);
    huidu_value[1] = get_gpio_state(HUIDU_L1_PORT, HUIDU_L1_PIN);
    huidu_value[2] = get_gpio_state(HUIDU_M_PORT, HUIDU_M_PIN);
    huidu_value[3] = get_gpio_state(HUIDU_R1_PORT, HUIDU_R1_PIN);
    huidu_value[4] = get_gpio_state(HUIDU_R2_PORT, HUIDU_R2_PIN);
}
extern float target_speed_1;
extern float target_speed_2;

// ---- 循迹 PD 参数 (可调) ----
#define LINE_BASE     300   // 直行基础速度 mm/s
#define LINE_KP       20    // 比例增益: 差速 = KP*误差
#define LINE_KD       15    // 微分增益: 抑制回摆
#define LINE_DIFF_MAX 150   // 差速上限, 防止内轮被压进堵转区
// 传感器权重: 外侧远大于内侧 -> 中间修正温柔, 边缘转向凌厉
#define W_L2 (-3)
#define W_L1 (-1)
#define W_R1 (1)
#define W_R2 (3)

static float last_line_error = 0;

// 电机1=左轮，电机2=右轮
void adjust_motor()
{
    huidu_get_value();
    uint8_t L2=huidu_value[0], L1=huidu_value[1], M=huidu_value[2], R1=huidu_value[3], R2=huidu_value[4];
    uint8_t cnt = L2 + L1 + M + R1 + R2;

    motor_set_direction(1, 1);
    motor_set_direction(2, 1);

    // 全1: 十字/终点线, 停车
    if(cnt == 5) {
        target_speed_1 = 0;
        target_speed_2 = 0;
        return;
    }
    // 全0: 丢线, 保持上一次的目标继续找线(不修改 target)
    if(cnt == 0) {
        return;
    }

    // 加权位置误差: 正=偏左需右转, 负=偏右需左转
    float error = (float)(W_L2*L2 + W_L1*L1 + W_R1*R1 + W_R2*R2) / cnt;

    // PD 输出差速
    float diff = LINE_KP * error + LINE_KD * (error - last_line_error);
    last_line_error = error;

    // 差速限幅
    if(diff >  LINE_DIFF_MAX) diff =  LINE_DIFF_MAX;
    if(diff < -LINE_DIFF_MAX) diff = -LINE_DIFF_MAX;

    // error>0(偏左) -> 左轮加速/右轮减速 -> 右转纠偏
    target_speed_1 = LINE_BASE + diff;
    target_speed_2 = LINE_BASE - diff;
    if(target_speed_1 < 0) target_speed_1 = 0;
    if(target_speed_2 < 0) target_speed_2 = 0;
}
