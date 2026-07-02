#include "headfile.h"
#include "GraySensor.h"

/*
 * 感为 8-way digital line sensor driver.
 *
 * Hardware: PA2=CLK (output, active-high), PA3=DAT (input, pull-up).
 * Protocol (from linewalk.c / sensor datasheet):
 *   - CLK rising edge: sensor drives DAT with current channel bit.
 *   - Hold CLK high >= 5 µs to let DAT settle; sample; pull CLK low; wait >= 5 µs.
 *   - After 8 clocks, hold CLK low >= 1 ms to reset the sensor's internal counter.
 *   - Sensor output: 1 = white/light (no line), 0 = black/dark (line present).
 *
 * We invert to our project convention on the way in:
 *   BinaryData bit N = 1  →  CH(N+1) sees the black line.
 *   bit0=CH1=leftmost, bit7=CH8=rightmost.
 *
 * Timing budget: 8×10 µs + 1 ms ≈ 1.1 ms per call; fits inside 5 ms control period.
 * delay_us/ms use pure SysTick VAL counting and are safe to call from ISR context.
 */

/* Read 8 bits from the sensor; returns raw value (1=white, 0=black). */
static uint8_t linewalk_read(void)
{
    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        DL_GPIO_setPins(GPIO_GraySensor_PORT, GPIO_GraySensor_CLK_PIN);
        delay_us(5);
        if (DL_GPIO_readPins(GPIO_GraySensor_PORT, GPIO_GraySensor_DAT_PIN))
            data |= (uint8_t)(1u << i);
        DL_GPIO_clearPins(GPIO_GraySensor_PORT, GPIO_GraySensor_CLK_PIN);
        delay_us(5);
    }
    delay_ms(1);   /* reset sensor's internal bit counter (ISR-safe: uses SysTick VAL) */
    return data;
}

void GraySensorInit(GraySensor_t** GraySensor)
{
    static GraySensor_t gs;
    *GraySensor = &gs;

    gs.BinaryData      = 0x00;
    gs.GraySensorNoData = 0;
    gs.bit0 = gs.bit1 = gs.bit2 = gs.bit3 = 0;
    gs.bit4 = gs.bit5 = gs.bit6 = gs.bit7 = 0;

    /* CLK starts low */
    DL_GPIO_clearPins(GPIO_GraySensor_PORT, GPIO_GraySensor_CLK_PIN);
}

void GraySensorDataUpdate(GraySensor_t* GraySensor)
{
    /* raw: 1=white/light, 0=black/dark.  Invert → 1=line. */
    uint8_t raw = linewalk_read();
    uint8_t b   = (uint8_t)(~raw);   /* b: 1 = line detected */

    GraySensor->bit0 = (b >> 0) & 1;
    GraySensor->bit1 = (b >> 1) & 1;
    GraySensor->bit2 = (b >> 2) & 1;
    GraySensor->bit3 = (b >> 3) & 1;
    GraySensor->bit4 = (b >> 4) & 1;
    GraySensor->bit5 = (b >> 5) & 1;
    GraySensor->bit6 = (b >> 6) & 1;
    GraySensor->bit7 = (b >> 7) & 1;

    GraySensor->BinaryData = b;

    /* Flag: no line in view when all bits are 0 (no channel sees black line). */
    GraySensor->GraySensorNoData = (b == 0x00) ? 1 : 0;
}

float GraySensorToTurnAngle(GraySensor_t* GraySensor)
{
    /*
     * Weighted centroid steering.
     * bit0 = leftmost (+3.0 weight → positive turnAngle → steer left).
     * bit7 = rightmost (-3.0 weight → negative turnAngle → steer right).
     * Matches SelfTurn PID convention: positive diff → left wheel slower, car turns left.
     */
    static const float w[8] = {3.0f, 1.5f, 0.5f, 0.10f, -0.10f, -0.5f, -1.5f, -3.0f};

    float wsum  = 0.0f;
    float total = 0.0f;
    uint8_t b   = GraySensor->BinaryData;

    for (int i = 0; i < 8; i++) {
        if (b & (1u << i)) {
            wsum  += w[i];
            total += 1.0f;
        }
    }

    if (total == 0.0f)
        return 0.0f;   /* no line: hold last command (caller's responsibility) */

    return (wsum / total) * 10.0f;
}
