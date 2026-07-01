#include "linewalk.h"
#include "board.h"
#include "ti_msp_dl_config.h"

void LineWalk_init(void)
{
    /* CLK starts low; sensor counts rising edges */
    DL_GPIO_clearPins(LINEWALK_CLK_PORT, LINEWALK_CLK_PIN_13_PIN);
}

uint8_t LineWalk_read(void)
{
    uint8_t data = 0;
    int i;

    for (i = 0; i < 8; i++) {
        /* Rising edge: sensor drives DAT with current channel bit */
        DL_GPIO_setPins(LINEWALK_CLK_PORT, LINEWALK_CLK_PIN_13_PIN);
        delay_us(5);   /* hold CLK high >= 5 us for sensor to settle DAT */

        if (DL_GPIO_readPins(LINEWALK_DAT_PORT, LINEWALK_DAT_PIN_12_PIN))
            data |= (uint8_t)(1u << i);

        /* Falling edge: advance sensor's internal channel counter */
        DL_GPIO_clearPins(LINEWALK_CLK_PORT, LINEWALK_CLK_PIN_13_PIN);
        delay_us(5);   /* inter-clock gap must stay < 1 ms */
    }

    /*
     * Hold CLK low >= 1 ms to reset the sensor's internal bit counter,
     * so the next call starts cleanly from CH1.
     */
    delay_ms(1);

    return data;
}
