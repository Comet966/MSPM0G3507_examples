#include "board.h"
#include "stdio.h"

char buf[80] = {'\0'};

int main(void)
{
	unsigned int LineL1, LineL2, LineL3, LineL4;
	unsigned int LineR1, LineR2, LineR3, LineR4;
	board_init();
	uart0_send_string("Eight_way patrol line\n");
    while (1)
    {
		LineL1 = DL_GPIO_readPins(LineWalk_L1_PORT, LineWalk_L1_PIN_27_PIN) > 0 ? 1 : 0;
		LineL2 = DL_GPIO_readPins(LineWalk_L2_PORT, LineWalk_L2_PIN_26_PIN) > 0 ? 1 : 0;
		LineL3 = DL_GPIO_readPins(LineWalk_L3_PORT, LineWalk_L3_PIN_15_PIN) > 0 ? 1 : 0;
		LineL4 = DL_GPIO_readPins(LineWalk_L4_PORT, LineWalk_L4_PIN_16_PIN) > 0 ? 1 : 0;
		LineR1 = DL_GPIO_readPins(LineWalk_R1_PORT, LineWalk_R1_PIN_24_PIN) > 0 ? 1 : 0;
		LineR2 = DL_GPIO_readPins(LineWalk_R2_PORT, LineWalk_R2_PIN_25_PIN) > 0 ? 1 : 0;
		LineR3 = DL_GPIO_readPins(LineWalk_R3_PORT, LineWalk_R3_PIN_17_PIN) > 0 ? 1 : 0;
		LineR4 = DL_GPIO_readPins(LineWalk_R4_PORT, LineWalk_R4_PIN_20_PIN) > 0 ? 1 : 0;
		sprintf(buf, "L=%d%d%d%d R=%d%d%d%d\n",
		        LineL1, LineL2, LineL3, LineL4,
		        LineR1, LineR2, LineR3, LineR4);
		uart0_send_string(buf);
		delay_ms(300);
    }
}
