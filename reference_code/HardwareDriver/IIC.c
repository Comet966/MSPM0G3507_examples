#include "IIC.h"

void IIC_delay(void)
{
		uint32_t told;
    told = SysTick->VAL;
		told = SysTick->VAL;
    told = SysTick->VAL;
		told = SysTick->VAL;
}

void MYIIC_Start()//IIC Start
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	IIC_delay();
	OLED_W_SDA(0);
	IIC_delay();
	OLED_W_SCL(0);
	IIC_delay();
}
void MYIIC_Stop(void)//IIC Stop
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	IIC_delay();
	OLED_W_SDA(1);
}

void MYIIC_Wait_Ack(void)
{
	OLED_W_SDA(1);
	IIC_delay();
	OLED_W_SCL(1);
	IIC_delay();
	OLED_W_SCL(0);
//	IIC_delay();
}
void MYIIC_Write_Byte(u8 Byte)// IIC Write byte
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(!!(Byte & (0x80 >> i)));
		IIC_delay();
		OLED_W_SCL(1);
		IIC_delay();
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

void MYIIC_InitGPIO(void)
{
//	GPIO_InitTypeDef  GPIO_InitStructure;

//	RCC_APB2PeriphClockCmd(MYIIC_SCL_GPIO_RCC | MYIIC_SDA_GPIO_RCC, ENABLE);
//	GPIO_InitStructure.GPIO_Pin   = MYIIC_SCL_GPIO_PIN;
// 	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(MYIIC_SCL_GPIO, &GPIO_InitStructure);
// 	GPIO_SetBits(MYIIC_SCL_GPIO,MYIIC_SCL_GPIO_PIN);
//	
//	GPIO_InitStructure.GPIO_Pin   = MYIIC_SDA_GPIO_PIN;
// 	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(MYIIC_SDA_GPIO, &GPIO_InitStructure);
// 	GPIO_SetBits(MYIIC_SDA_GPIO,MYIIC_SDA_GPIO_PIN);
}


