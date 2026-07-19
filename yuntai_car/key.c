#include "key.h"
#include "System.h"

/*
 * 按键 + 编码器中断服务。
 * KEY9 = 循环切换运行模式；KEY10 = 启动/执行（运行中=急停）。
 * 两个按键在 GROUP1_IRQHandler 内以 millis() 去抖，仅置"请求标志"，
 * 由主循环消费，绝不在中断里做业务逻辑。编码器 AA/BA 计数逻辑保持不变。
 */

uint8_t get_key_state(uint32_t key) {
    uint32_t high_bits = DL_GPIO_readPins(KEY_PORT, key);
    if((high_bits & key) != 0) return 1;
    else return 0;
}

uint32_t counter_1_A = 0;
uint32_t counter_2_A = 0;

/* 主循环消费后清零；volatile 因为跨中断/主循环共享 */
volatile uint8_t key_mode_req   = 0;   /* KEY9 按下请求：切模式 */
volatile uint8_t key_action_req = 0;   /* KEY10 按下请求：启动/急停 */

#define KEY_DEBOUNCE_MS  200U
static uint32_t s_last_key9_ms  = 0;
static uint32_t s_last_key10_ms = 0;

void GROUP1_IRQHandler()
{
    switch (DL_GPIO_getPendingInterrupt(GPIOB))
    {
    case KEY_KEY9_IIDX: {
        uint32_t now = millis();
        if (now - s_last_key9_ms >= KEY_DEBOUNCE_MS) {
            s_last_key9_ms = now;
            key_mode_req = 1;
        }
        break;
    }
    case KEY_KEY10_IIDX: {
        uint32_t now = millis();
        if (now - s_last_key10_ms >= KEY_DEBOUNCE_MS) {
            s_last_key10_ms = now;
            key_action_req = 1;
        }
        break;
    }
    case DC_MOTOR_BA_IIDX:
        counter_2_A++;
        break;
    default:
        break;
    }

    switch (DL_GPIO_getPendingInterrupt(GPIOA))
    {
    case DC_MOTOR_AA_IIDX:
        counter_1_A++;
        break;
    default:
        break;
    }
}
