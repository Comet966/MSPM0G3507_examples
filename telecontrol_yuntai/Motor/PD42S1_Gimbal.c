/*
 * PD42S1_Gimbal.c — 双轴云台角度层实现.
 */
#include "PD42S1_Gimbal.h"

/* 角度(度) -> 电机脉冲数(带符号) */
static int32_t deg_to_counts(const pd42_axis_t *ax, float deg)
{
    float c = deg / 360.0f * (float)PD42_COUNTS_PER_REV * ax->gear;
    return (int32_t)(c >= 0 ? c + 0.5f : c - 0.5f);
}

/* 电机脉冲数 -> 角度(度) */
static float counts_to_deg(const pd42_axis_t *ax, int32_t counts)
{
    return (float)counts * 360.0f / ((float)PD42_COUNTS_PER_REV * ax->gear);
}

/* 下发一条绝对位置指令: 把带符号脉冲数拆成 方向+幅值 */
static void axis_move_abs(pd42_axis_t *ax, float deg)
{
    ax->target_deg = deg;
    int32_t counts = deg_to_counts(ax, deg - ax->origin_deg) * ax->sign;
    uint8_t dir = (counts >= 0) ? PD42_DIR_CW : PD42_DIR_CCW;
    uint32_t mag = (uint32_t)(counts >= 0 ? counts : -counts);
    pd42_pos_abs(&ax->drv, dir, ax->acc, ax->rpm, mag);
}

static void axis_move_rel(pd42_axis_t *ax, float deg)
{
    ax->target_deg += deg;
    int32_t counts = deg_to_counts(ax, deg) * ax->sign;
    uint8_t dir = (counts >= 0) ? PD42_DIR_CW : PD42_DIR_CCW;
    uint32_t mag = (uint32_t)(counts >= 0 ? counts : -counts);
    pd42_pos_rel(&ax->drv, dir, ax->acc, ax->rpm, mag);
}

void pd42_axis_config(pd42_axis_t *ax, const pd42_bus_t *bus, uint8_t addr,
                      float gear, int8_t sign, uint16_t rpm, uint8_t acc)
{
    pd42_init(&ax->drv, bus, addr);
    ax->gear       = (gear != 0.0f) ? gear : 1.0f;
    ax->sign       = (sign >= 0) ? 1 : -1;
    ax->rpm        = rpm;
    ax->acc        = acc;
    ax->origin_deg = 0.0f;
    ax->target_deg = 0.0f;
}

static void axis_init(pd42_axis_t *ax, uint16_t microstep)
{
    pd42_set_mode(&ax->drv, PD42_MODE_POS_COMM);   /* 通信绝对位置模式 */
    if (microstep) pd42_set_microstep(&ax->drv, microstep);
    pd42_enable(&ax->drv, true);
}

void pd42_gimbal_init(pd42_gimbal_t *g, uint16_t microstep)
{
    axis_init(&g->pan,  microstep);
    axis_init(&g->tilt, microstep);
}

void pd42_gimbal_move_pan(pd42_gimbal_t *g, float deg)  { axis_move_abs(&g->pan,  deg); }
void pd42_gimbal_move_tilt(pd42_gimbal_t *g, float deg) { axis_move_abs(&g->tilt, deg); }
void pd42_gimbal_move_by_pan(pd42_gimbal_t *g, float deg)  { axis_move_rel(&g->pan,  deg); }
void pd42_gimbal_move_by_tilt(pd42_gimbal_t *g, float deg) { axis_move_rel(&g->tilt, deg); }

float pd42_gimbal_get_pan(const pd42_gimbal_t *g)  { return g->pan.target_deg; }
float pd42_gimbal_get_tilt(const pd42_gimbal_t *g) { return g->tilt.target_deg; }

static bool axis_read_angle(pd42_axis_t *ax, float *deg, uint32_t timeout_ms)
{
    int32_t counts;
    if (!pd42_read_pos(&ax->drv, &counts, timeout_ms)) return false;
    *deg = counts_to_deg(ax, counts) * ax->sign + ax->origin_deg;
    return true;
}

bool pd42_gimbal_read_pan(pd42_gimbal_t *g, float *deg, uint32_t timeout_ms)
{
    return axis_read_angle(&g->pan, deg, timeout_ms);
}

bool pd42_gimbal_read_tilt(pd42_gimbal_t *g, float *deg, uint32_t timeout_ms)
{
    return axis_read_angle(&g->tilt, deg, timeout_ms);
}

bool pd42_gimbal_arrived(pd42_gimbal_t *g, uint32_t timeout_ms)
{
    bool a1 = false, a2 = false;
    if (!pd42_read_arrived(&g->pan.drv,  &a1, timeout_ms)) return false;
    if (!pd42_read_arrived(&g->tilt.drv, &a2, timeout_ms)) return false;
    return a1 && a2;
}

void pd42_gimbal_home(pd42_gimbal_t *g)
{
    axis_move_abs(&g->pan,  g->pan.origin_deg);
    axis_move_abs(&g->tilt, g->tilt.origin_deg);
}

void pd42_gimbal_set_origin(pd42_gimbal_t *g)
{
    pd42_zero_angle(&g->pan.drv);    /* 0xF8 驱动器内部位置清零 */
    pd42_zero_angle(&g->tilt.drv);
    g->pan.origin_deg  = 0.0f;  g->pan.target_deg  = 0.0f;
    g->tilt.origin_deg = 0.0f;  g->tilt.target_deg = 0.0f;
}

void pd42_gimbal_enable(pd42_gimbal_t *g, bool on)
{
    pd42_enable(&g->pan.drv,  on);
    pd42_enable(&g->tilt.drv, on);
}

void pd42_gimbal_stop(pd42_gimbal_t *g)
{
    pd42_stop(&g->pan.drv);
    pd42_stop(&g->tilt.drv);
}

void pd42_joy_speed_config_default(pd42_joy_speed_config_t *cfg)
{
    cfg->center       = 2048U;
    cfg->deadband     = 280U;
    cfg->pan_max_rpm  = 50.0f;
    cfg->tilt_max_rpm = 80.0f;
    cfg->acceleration = 50U;
    cfg->curve        = 3U;
}

/* 把 ADC 偏移映射为方向和转速。死区外从 0 重新起算，避免死区边缘跳变。 */
static float joystick_to_rpm(int32_t offset, const pd42_joy_speed_config_t *cfg,
                             float max_rpm)
{
    uint32_t magnitude = (uint32_t)(offset >= 0 ? offset : -offset);
    if (magnitude <= cfg->deadband || max_rpm <= 0.0f) return 0.0f;

    uint32_t usable = (cfg->center > cfg->deadband) ?
                      (uint32_t)(cfg->center - cfg->deadband) : 1U;
    float normalized = (float)(magnitude - cfg->deadband) / (float)usable;
    if (normalized > 1.0f) normalized = 1.0f;

    if (cfg->curve >= 3U) normalized = normalized * normalized * normalized;
    else if (cfg->curve == 2U) normalized = normalized * normalized;
    return normalized * max_rpm;
}

static void axis_speed_from_joystick(pd42_axis_t *ax,
                                     const pd42_joy_speed_config_t *cfg,
                                     uint16_t sample, float max_rpm)
{
    int32_t offset = (int32_t)sample - (int32_t)cfg->center;
    float rpm = joystick_to_rpm(offset, cfg, max_rpm);
    uint8_t dir = (offset >= 0) ? PD42_DIR_CW : PD42_DIR_CCW;
    if (ax->sign < 0) dir = (dir == PD42_DIR_CW) ? PD42_DIR_CCW : PD42_DIR_CW;

    /* rpm=0 是速度模式的正常停车，不能用 0xFC，否则会进入刹车状态。 */
    pd42_speed(&ax->drv, dir, cfg->acceleration, rpm);
}

void pd42_gimbal_speed_from_joystick(pd42_gimbal_t *g,
                                     const pd42_joy_speed_config_t *cfg,
                                     uint16_t x, uint16_t y)
{
    /* X -> 下方 Pan (PA8/PA9)，Y -> 上方 Tilt (PA28/PA31)。 */
    axis_speed_from_joystick(&g->pan,  cfg, x, cfg->pan_max_rpm);
    axis_speed_from_joystick(&g->tilt, cfg, y, cfg->tilt_max_rpm);
}

void pd42_gimbal_speed_stop(pd42_gimbal_t *g,
                            const pd42_joy_speed_config_t *cfg)
{
    /* 方向在 0 RPM 时无意义，固定 CW 可使帧内容确定。 */
    pd42_speed(&g->pan.drv, PD42_DIR_CW, cfg->acceleration, 0.0f);
    pd42_speed(&g->tilt.drv, PD42_DIR_CW, cfg->acceleration, 0.0f);
}
