#include "board.h"
#include "oled.h"
#include "linewalk.h"
#include <stdio.h>

int main(void)
{
    board_init();
    OLED_init();
    LineWalk_init();

    OLED_clear();
    OLED_setCursor(0, 0);
    OLED_writeString("8-Way LineSensor");
    OLED_setCursor(0, 2);
    OLED_writeString("CH 1 2 3 4 5 6 7 8");
    OLED_display();

    char uartBuf[40];

    while (1) {
        uint8_t val = LineWalk_read();

        /* Row 3: one character per channel, O=white, X=black */
        OLED_setCursor(0, 3);
        OLED_writeString("   ");
        int i;
        for (i = 0; i < 8; i++) {
            OLED_writeChar((val >> i) & 1u ? 'O' : 'X');
            OLED_writeChar(' ');
        }

        /* Row 5: hex value */
        OLED_setCursor(0, 5);
        OLED_printf("Val: 0x%02X  ", val);

        OLED_display();

        /* UART debug output */
        sprintf(uartBuf, "LW=0x%02X  b%d%d%d%d%d%d%d%d\r\n", val,
                (val >> 7) & 1, (val >> 6) & 1,
                (val >> 5) & 1, (val >> 4) & 1,
                (val >> 3) & 1, (val >> 2) & 1,
                (val >> 1) & 1, (val >> 0) & 1);
        uart0_send_string(uartBuf);

        delay_ms(50);
    }
}
