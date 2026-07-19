/*
 * rs485_bus.c — 双路独立 UART transport (PD42S1 直连，无收发器).
 *   UART1_PAN  PA8/PA9    → Pan  轴
 *   UART0_TILT PA28/PA31  → Tilt 轴
 * 每路有独立 RX 环形缓冲，ISR 分别填充，互不干扰.
 */
#include "rs485_bus.h"
#include "ti_msp_dl_config.h"

#define RX_RING_SZ   128
#define CPU_HZ       32000000u
#define CYCLES_PER_MS (CPU_HZ / 1000u)
/* 115200 Bd 下 10 字节命令不足 1 ms；超过约 3 ms 视为 UART 故障。 */
#define UART_TX_TIMEOUT_LOOPS 100000u

/* ---- Pan 缓冲 ---- */
static volatile uint8_t  s_rx_pan[RX_RING_SZ];
static volatile uint16_t s_pan_head, s_pan_tail;

/* ---- Tilt 缓冲 ---- */
static volatile uint8_t  s_rx_tilt[RX_RING_SZ];
static volatile uint16_t s_tilt_head, s_tilt_tail;

/* ---- ISR ---- */
void UART1_PAN_INST_IRQHandler(void)
{
    if (DL_UART_Main_getPendingInterrupt(UART1_PAN_INST) ==
            DL_UART_MAIN_IIDX_RX) {
        uint8_t b = DL_UART_Main_receiveData(UART1_PAN_INST);
        uint16_t nh = (uint16_t)((s_pan_head + 1) % RX_RING_SZ);
        if (nh != s_pan_tail) { s_rx_pan[s_pan_head] = b; s_pan_head = nh; }
    }
}

void UART0_TILT_INST_IRQHandler(void)
{
    if (DL_UART_Main_getPendingInterrupt(UART0_TILT_INST) ==
            DL_UART_MAIN_IIDX_RX) {
        uint8_t b = DL_UART_Main_receiveData(UART0_TILT_INST);
        uint16_t nh = (uint16_t)((s_tilt_head + 1) % RX_RING_SZ);
        if (nh != s_tilt_tail) { s_rx_tilt[s_tilt_head] = b; s_tilt_head = nh; }
    }
}

/* ---- 通用 bus 回调实现 ---- */
static int _write(UART_Regs *inst, const uint8_t *buf, uint16_t len)
{
    uint32_t timeout = UART_TX_TIMEOUT_LOOPS;
    for (uint16_t i = 0; i < len; i++) {
        while (!DL_UART_transmitDataCheck(inst, buf[i])) {
            if (--timeout == 0U) return -1;
        }
    }
    while (DL_UART_isBusy(inst)) {
        if (--timeout == 0U) return -1;
    }
    return (int)len;
}

static int _read(volatile uint8_t *ring, volatile uint16_t *head,
                 volatile uint16_t *tail, uint8_t *buf,
                 uint16_t max, uint32_t timeout_ms)
{
    uint16_t n = 0;
    uint32_t idle = 0;
    uint32_t budget = timeout_ms ? timeout_ms : 1;
    while (budget) {
        if (*tail != *head) {
            uint8_t b = ring[*tail];
            *tail = (uint16_t)((*tail + 1) % RX_RING_SZ);
            if (n < max) buf[n++] = b;
            idle = 0;
            if (b == PD42_FRAME_TAIL && n >= 6) return (int)n;
        } else {
            DL_Common_delayCycles(CYCLES_PER_MS / 10);
            if (n > 0 && ++idle >= 20) return (int)n;
            if (n == 0) { if (--budget == 0) break; }
            else        { --budget; if (budget == 0) break; }
        }
    }
    return (int)n;
}

/* ---- Pan bus 回调 ---- */
static int pan_write(void *ctx, const uint8_t *buf, uint16_t len)
{
    (void)ctx;
    s_pan_tail = s_pan_head;   /* 清空 RX 缓冲 */
    return _write(UART1_PAN_INST, buf, len);
}
static int pan_read(void *ctx, uint8_t *buf, uint16_t max, uint32_t tms)
{
    (void)ctx;
    return _read(s_rx_pan, &s_pan_head, &s_pan_tail, buf, max, tms);
}

/* ---- Tilt bus 回调 ---- */
static int tilt_write(void *ctx, const uint8_t *buf, uint16_t len)
{
    (void)ctx;
    s_tilt_tail = s_tilt_head;
    return _write(UART0_TILT_INST, buf, len);
}
static int tilt_read(void *ctx, uint8_t *buf, uint16_t max, uint32_t tms)
{
    (void)ctx;
    return _read(s_rx_tilt, &s_tilt_head, &s_tilt_tail, buf, max, tms);
}

static const pd42_bus_t s_bus_pan  = { pan_write,  pan_read,  0 };
static const pd42_bus_t s_bus_tilt = { tilt_write, tilt_read, 0 };

const pd42_bus_t *rs485_get_bus_pan(void)  { return &s_bus_pan;  }
const pd42_bus_t *rs485_get_bus_tilt(void) { return &s_bus_tilt; }

void rs485_bus_init(void)
{
    s_pan_head  = s_pan_tail  = 0;
    s_tilt_head = s_tilt_tail = 0;
    NVIC_EnableIRQ(UART1_PAN_INST_INT_IRQN);
    NVIC_EnableIRQ(UART0_TILT_INST_INT_IRQN);
}
