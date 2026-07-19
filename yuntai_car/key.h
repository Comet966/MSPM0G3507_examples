#ifndef KEY_H
#define KEY_H

#include "ti_msp_dl_config.h"

uint8_t get_key_state(uint32_t key);

/* 主循环消费的按键请求标志（在 key.c 的 GROUP1_IRQHandler 里置位，去抖后）。
 * key_mode_req   : KEY9 —— 循环切换运行模式
 * key_action_req : KEY10 —— 启动/执行（运行中按下=急停） */
extern volatile uint8_t key_mode_req;
extern volatile uint8_t key_action_req;

#endif


