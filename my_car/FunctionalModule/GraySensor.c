#include "headfile.h"
#include "GraySensor.h"

/*
 * 感为 7-way digital line sensor driver — parallel mode.
 *
 * Hardware (all GPIOA, input pull-up):
 *   CH1=PA21  CH2=PA22  CH3=PA2   CH4=PA3
 *   CH5=PA14  CH6=PA15  CH7=PA16
 *
 * Sensor output: 1 = white/light (no line), 0 = black/dark (line present).
 * We invert to project convention on the way in:
 *   BinaryData bit N = 1  →  CH(N+1) sees the black line.
 *   bit0=CH1=leftmost, bit6=CH7=rightmost.
 *
 * Timing: single GPIO port read, ~0 µs — no CLK/DAT protocol overhead.
 */

static uint8_t parallel_read(void)
{
    uint32_t port = DL_GPIO_readPins(GPIO_GraySensor_PORT,
        GPIO_GraySensor_CH1_PIN | GPIO_GraySensor_CH2_PIN |
        GPIO_GraySensor_CH3_PIN | GPIO_GraySensor_CH4_PIN |
        GPIO_GraySensor_CH5_PIN | GPIO_GraySensor_CH6_PIN |
        GPIO_GraySensor_CH7_PIN);

    uint8_t data = 0;
    if (port & GPIO_GraySensor_CH1_PIN) data |= (1u << 0);
    if (port & GPIO_GraySensor_CH2_PIN) data |= (1u << 1);
    if (port & GPIO_GraySensor_CH3_PIN) data |= (1u << 2);
    if (port & GPIO_GraySensor_CH4_PIN) data |= (1u << 3);
    if (port & GPIO_GraySensor_CH5_PIN) data |= (1u << 4);
    if (port & GPIO_GraySensor_CH6_PIN) data |= (1u << 5);
    if (port & GPIO_GraySensor_CH7_PIN) data |= (1u << 6);
    return data;
}

void GraySensorInit(GraySensor_t** GraySensor)
{
    static GraySensor_t gs;
    *GraySensor = &gs;

    gs.BinaryData      = 0x00;
    gs.GraySensorNoData = 0;
    gs.bit0 = gs.bit1 = gs.bit2 = gs.bit3 = 0;
    gs.bit4 = gs.bit5 = gs.bit6 = 0;
    /* GPIO pins are configured as input pull-up by SysConfig; nothing to drive. */
}

void GraySensorDataUpdate(GraySensor_t* GraySensor)
{
    /* raw: 1=white/light, 0=black/dark.  Invert → 1=line. */
    uint8_t raw = parallel_read();
    uint8_t b   = (uint8_t)(~raw) & 0x7Fu;  /* mask to 7 bits */

    GraySensor->bit0 = (b >> 0) & 1;
    GraySensor->bit1 = (b >> 1) & 1;
    GraySensor->bit2 = (b >> 2) & 1;
    GraySensor->bit3 = (b >> 3) & 1;
    GraySensor->bit4 = (b >> 4) & 1;
    GraySensor->bit5 = (b >> 5) & 1;
    GraySensor->bit6 = (b >> 6) & 1;

    GraySensor->BinaryData = b;

    GraySensor->GraySensorNoData = (b == 0x00) ? 1 : 0;
}

float GraySensorToTurnAngle(GraySensor_t* GraySensor)
{
    /* Symmetric weights for 7 channels: left=positive, right=negative, centre=0. */
    static const float w[7] = {3.0f, 2.0f, 1.0f, 0.0f, -1.0f, -2.0f, -3.0f};

    float wsum  = 0.0f;
    float total = 0.0f;
    uint8_t b   = GraySensor->BinaryData;

    for (int i = 0; i < 7; i++) {
        if (b & (1u << i)) {
            wsum  += w[i];
            total += 1.0f;
        }
    }

    if (total == 0.0f)
        return 0.0f;

    return (wsum / total) * 10.0f;
}
