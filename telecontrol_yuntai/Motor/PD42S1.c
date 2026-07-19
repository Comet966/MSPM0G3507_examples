/*
 * PD42S1.c — 正点原子 PD42S1 闭环步进驱动器协议实现.
 * 字节序/组帧/校验和均对照官方 STM32 例程 smd.c / process_frame.c 复核一致.
 */
#include "PD42S1.h"

/* ---------------- 底层组帧 / 校验 / 解析 ---------------- */

uint8_t pd42_checksum(const uint8_t *data, uint8_t len)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) sum += data[i];
    return sum;   /* & 0xFF 隐含于 uint8_t 溢出 */
}

uint16_t pd42_build(uint8_t *out, uint8_t addr, uint8_t code,
                    const uint8_t *payload, uint8_t payload_len)
{
    uint16_t n = 0;
    out[n++] = PD42_FRAME_HEAD;
    out[n++] = addr;
    out[n++] = code;
    for (uint8_t i = 0; i < payload_len; i++) out[n++] = payload[i];
    out[n]   = pd42_checksum(out, n);   /* 校验和覆盖帧头起到最后数据字节 */
    n++;
    out[n++] = PD42_FRAME_TAIL;
    return n;
}

bool pd42_parse(const uint8_t *buf, uint16_t len, pd42_reply_t *r)
{
    if (len < 6) return false;                       /* 最小帧: head+addr+code+err+cs+tail */
    if (buf[0] != PD42_FRAME_HEAD) return false;
    if (buf[len - 1] != PD42_FRAME_TAIL) return false;
    if (buf[len - 2] != pd42_checksum(buf, (uint8_t)(len - 2))) return false;

    r->addr = buf[1];
    r->code = buf[2];
    r->err  = buf[3];
    uint16_t dlen = len - 6;                         /* 去掉 head/addr/code/err/cs/tail */
    if (dlen > PD42_MAX_FRAME) dlen = PD42_MAX_FRAME;
    r->data_len = (uint8_t)dlen;
    for (uint8_t i = 0; i < r->data_len; i++) r->data[i] = buf[4 + i];
    return (r->err == PD42_ACK_OK);
}

/* ---------------- 内部发送辅助 ---------------- */

static void pd42_send(pd42_t *m, uint8_t code,
                      const uint8_t *payload, uint8_t payload_len)
{
    uint8_t frame[PD42_MAX_FRAME];
    uint16_t n = pd42_build(frame, m->addr, code, payload, payload_len);
    if (m->bus && m->bus->write) m->bus->write(m->bus->ctx, frame, n);
}

/* 无数据域命令 */
static void pd42_send_bare(pd42_t *m, uint8_t code)
{
    pd42_send(m, code, 0, 0);
}

/* 大端打包 float(4B): 对照 smd.c 发 b[3]..b[0] (高字节在前) */
static void pack_f32_be(uint8_t *p, float v)
{
    union { float f; uint32_t u; } cv;
    cv.f = v;
    p[0] = (uint8_t)((cv.u >> 24) & 0xFF);
    p[1] = (uint8_t)((cv.u >> 16) & 0xFF);
    p[2] = (uint8_t)((cv.u >> 8)  & 0xFF);
    p[3] = (uint8_t)((cv.u >> 0)  & 0xFF);
}

static void pack_u32_be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)((v >> 24) & 0xFF);
    p[1] = (uint8_t)((v >> 16) & 0xFF);
    p[2] = (uint8_t)((v >> 8)  & 0xFF);
    p[3] = (uint8_t)((v >> 0)  & 0xFF);
}

static void pack_u16_be(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)((v >> 8) & 0xFF);
    p[1] = (uint8_t)((v >> 0) & 0xFF);
}

void pd42_init(pd42_t *m, const pd42_bus_t *bus, uint8_t addr)
{
    m->bus  = bus;
    m->addr = addr;
}

/* ---------------- 运动控制 ---------------- */

void pd42_enable(pd42_t *m, bool on)
{
    uint8_t d = on ? PD42_EN_ON : PD42_EN_OFF;   /* 0xFA: 0 使能, 1 失能 */
    pd42_send(m, 0xFA, &d, 1);
}

void pd42_stop(pd42_t *m)        { pd42_send_bare(m, 0xFC); }
void pd42_zero_angle(pd42_t *m)  { pd42_send_bare(m, 0xF8); }
void pd42_clear_state(pd42_t *m) { pd42_send_bare(m, 0xFB); }

void pd42_torque(pd42_t *m, uint8_t dir, uint16_t ma)
{
    uint8_t d[3];
    d[0] = dir;
    pack_u16_be(&d[1], ma);
    pd42_send(m, 0xF0, d, 3);
}

void pd42_speed(pd42_t *m, uint8_t dir, uint8_t acc, float rpm)
{
    uint8_t d[6];
    d[0] = dir;
    d[1] = acc;
    pack_f32_be(&d[2], rpm);
    pd42_send(m, 0xF1, d, 6);
}

void pd42_pos_abs(pd42_t *m, uint8_t dir, uint8_t acc, uint16_t rpm, uint32_t counts)
{
    uint8_t d[8];
    d[0] = dir;
    d[1] = acc;
    pack_u16_be(&d[2], rpm);
    pack_u32_be(&d[4], counts);
    pd42_send(m, 0xF2, d, 8);
}

void pd42_pos_rel(pd42_t *m, uint8_t dir, uint8_t acc, uint16_t rpm, uint32_t counts)
{
    uint8_t d[8];
    d[0] = dir;
    d[1] = acc;
    pack_u16_be(&d[2], rpm);
    pack_u32_be(&d[4], counts);
    pd42_send(m, 0xF3, d, 8);
}

/* ---------------- 配置 ---------------- */

void pd42_set_mode(pd42_t *m, pd42_mode_t mode)
{
    uint8_t d = (uint8_t)mode;
    pd42_send(m, 0x62, &d, 1);
}

void pd42_set_microstep(pd42_t *m, uint16_t step)
{
    uint8_t d[2];
    pack_u16_be(d, step);
    pd42_send(m, 0x65, d, 2);
}

void pd42_set_pos_torque(pd42_t *m, int16_t ma)
{
    uint8_t d[2];
    pack_u16_be(d, (uint16_t)ma);
    pd42_send(m, 0x64, d, 2);
}

void pd42_param_save(pd42_t *m) { pd42_send_bare(m, 0x04); }

/* ---------------- 读取类 ---------------- */

/* 发送读命令(无数据域) 并等待应答, 校验功能码匹配. 成功填 *r 返回 true. */
static bool pd42_query(pd42_t *m, uint8_t code, pd42_reply_t *r, uint32_t timeout_ms)
{
    if (!m->bus || !m->bus->read) return false;
    pd42_send_bare(m, code);

    uint8_t buf[PD42_MAX_FRAME];
    int got = m->bus->read(m->bus->ctx, buf, sizeof(buf), timeout_ms);
    if (got < 6) return false;
    if (!pd42_parse(buf, (uint16_t)got, r)) return false;
    return (r->code == code);
}

static int32_t be_to_i32(const uint8_t *p)
{
    return (int32_t)(((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
                     ((uint32_t)p[2] << 8)  | ((uint32_t)p[3] << 0));
}

bool pd42_read_pos(pd42_t *m, int32_t *counts, uint32_t timeout_ms)
{
    pd42_reply_t r;
    if (!pd42_query(m, 0x2A, &r, timeout_ms) || r.data_len < 4) return false;
    *counts = be_to_i32(r.data);      /* 51200 = 一圈 */
    return true;
}

bool pd42_read_speed(pd42_t *m, int16_t *rpm, uint32_t timeout_ms)
{
    pd42_reply_t r;
    if (!pd42_query(m, 0x29, &r, timeout_ms) || r.data_len < 2) return false;
    *rpm = (int16_t)(((uint16_t)r.data[0] << 8) | r.data[1]);
    return true;
}

bool pd42_read_run_state(pd42_t *m, uint8_t *state, uint32_t timeout_ms)
{
    pd42_reply_t r;
    if (!pd42_query(m, 0x2C, &r, timeout_ms) || r.data_len < 1) return false;
    *state = r.data[0];
    return true;
}

bool pd42_read_arrived(pd42_t *m, bool *arrived, uint32_t timeout_ms)
{
    pd42_reply_t r;
    if (!pd42_query(m, 0x30, &r, timeout_ms) || r.data_len < 1) return false;
    *arrived = (r.data[0] != 0);
    return true;
}
