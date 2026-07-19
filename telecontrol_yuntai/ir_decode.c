/*
 * ir_decode.c — 自定义 NEC 风格 32 位红外帧解码.
 *
 * 帧格式 (LSB first):
 *   引导码: 9000 µs mark + 4500 µs space
 *   数据0 : 560 µs mark + 560 µs space
 *   数据1 : 560 µs mark + 1690 µs space
 *   结束码: 560 µs mark
 *   Byte0=X[7:0], Byte1=X[11:8]|(Y[3:0]<<4), Byte2=Y[11:4], Byte3=CRC8
 *
 * 时间测量: TIMG_IR 以 1 MHz 下计，16-bit 差值 (s_prev - now) = elapsed µs.
 * 边沿方向: 读 PA18 电平 (HIGH=sing, LOW=falling).
 */
#include "ir_decode.h"
#include "ti_msp_dl_config.h"

static volatile uint16_t s_rx_x;
static volatile uint16_t s_rx_y;
static volatile uint8_t  s_rx_flag;

static uint8_t ir_crc8(const uint8_t *data, uint8_t length)
{
    uint8_t crc = 0;
    while (length--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; ++i)
            crc = (crc & 0x80U) ? (uint8_t)((crc << 1) ^ 0x07U) : (uint8_t)(crc << 1);
    }
    return crc;
}

void ir_decode_init(void)
{
    DL_TimerG_startCounter(TIMG_IR_INST);
}

void ir_decode_tick(void)
{
    static uint16_t s_prev;
    static uint8_t  s_state;
    static uint8_t  s_bit_count;
    static uint32_t s_frame;
    static bool     s_init = false;

    uint16_t now = (uint16_t)DL_TimerG_getTimerCount(TIMG_IR_INST);
    if (!s_init) {
        s_prev = now;
        s_init = true;
        return;
    }

    /* 下计计数器：elapsed = prev - now（uint16_t 自然处理溢出） */
    uint16_t width = (uint16_t)(s_prev - now);
    s_prev = now;

    bool rising = DL_GPIO_readPins(GPIO_IR_PORT, GPIO_IR_PIN_REC_PIN) != 0U;

    if (s_state == 0U) {
        /* 等待 9 ms 引导低电平结束（上升沿） */
        if (rising && width >= 8500U && width <= 9500U)
            s_state = 1U;
        return;
    }

    if (s_state == 1U) {
        /* 等待 4.5 ms 引导高电平结束（下降沿） */
        if (!rising && width >= 4000U && width <= 5000U) {
            s_state     = 2U;
            s_bit_count = 0U;
            s_frame     = 0U;
        } else {
            s_state = 0U;
        }
        return;
    }

    if (s_state == 2U) {
        /* 等待 560 µs 数据 mark 结束（上升沿） */
        if (rising && width >= 350U && width <= 800U) {
            s_state = 3U;
        } else {
            s_state = 0U;
        }
        return;
    }

    /* state 3: 下降沿结束 space，决定位值 */
    if (!rising) {
        if (width >= 1200U && width <= 2000U) {
            s_frame |= 1UL << s_bit_count;
        } else if (width < 350U || width > 800U) {
            s_state = 0U;
            return;
        }
        ++s_bit_count;
        if (s_bit_count == 32U) {
            uint8_t b[4];
            b[0] = (uint8_t)s_frame;
            b[1] = (uint8_t)(s_frame >> 8);
            b[2] = (uint8_t)(s_frame >> 16);
            b[3] = (uint8_t)(s_frame >> 24);
            if (ir_crc8(b, 3U) == b[3]) {
                s_rx_x    = (uint16_t)b[0] | ((uint16_t)(b[1] & 0x0FU) << 8);
                s_rx_y    = (uint16_t)(b[1] >> 4) | ((uint16_t)b[2] << 4);
                s_rx_flag = 1U;
            }
            s_state = 0U;
        } else {
            s_state = 2U;
        }
    } else {
        s_state = 0U;
    }
}

bool ir_decode_get(uint16_t *x, uint16_t *y)
{
    if (!s_rx_flag) return false;
    __disable_irq();
    *x        = s_rx_x;
    *y        = s_rx_y;
    s_rx_flag = 0U;
    __enable_irq();
    return true;
}
