#pragma once
#include <stdint.h>
#include <stdbool.h>

void ir_decode_init(void);  /* 启动 TIMG_IR 计数器 */
void ir_decode_tick(void);  /* 在 GROUP1_IRQHandler 中每次 PA18 边沿调用 */
bool ir_decode_get(uint16_t *x, uint16_t *y); /* 有完整帧时返回 true，清 flag */
