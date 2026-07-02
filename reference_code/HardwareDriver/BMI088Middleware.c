#include "ti_msp_dl_config.h"
#include "BMI088Middleware.h"

/**
************************************************************************
* @brief:      	BMI088_GPIO_init(void)
* @param:       void
* @retval:     	void
* @details:    	BMI088传感器GPIO初始化函数
************************************************************************
**/
void BMI088_GPIO_init(void)
{

}
/**
************************************************************************
* @brief:      	BMI088_com_init(void)
* @param:       void
* @retval:     	void
* @details:    	BMI088传感器通信初始化函数
************************************************************************
**/
void BMI088_com_init(void)
{


}
/**
************************************************************************
* @brief:      	BMI088_delay_ms(uint16_t ms)
* @param:       ms - 要延迟的毫秒数
* @retval:     	void
* @details:    	延迟指定毫秒数的函数，基于微秒延迟实现
************************************************************************
**/
void BMI088_delay_ms(uint16_t ms)
{
    while(ms--)
    {
        BMI088_delay_us(1000);
    }
}
/**
************************************************************************
* @brief:      	BMI088_delay_us(uint16_t us)
* @param:       us - 要延迟的微秒数
* @retval:     	void
* @details:    	微秒级延迟函数，使用SysTick定时器实现
************************************************************************
**/
void BMI088_delay_us(uint16_t us)
{

    uint32_t ticks = 0;
    uint32_t told = 0;
    uint32_t tnow = 0;
    uint32_t tcnt = 0;
    uint32_t reload = 0;
    reload = SysTick->LOAD;
    ticks = us * 480;
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}
/**
************************************************************************
* @brief:      	BMI088_ACCEL_NS_L(void)
* @param:       void
* @retval:     	void
* @details:    	将BMI088加速度计片选信号置低，使其处于选中状态
************************************************************************
**/
void BMI088_ACCEL_NS_L(void)
{
 //   HAL_GPIO_WritePin(ACC_CS_GPIO_Port, ACC_CS_Pin, GPIO_PIN_RESET);
		DL_GPIO_clearPins(ACC_CS_GPIO_Port,ACC_CS_Pin);
}
/**
************************************************************************
* @brief:      	BMI088_ACCEL_NS_H(void)
* @param:       void
* @retval:     	void
* @details:    	将BMI088加速度计片选信号置高，使其处于非选中状态
************************************************************************
**/
void BMI088_ACCEL_NS_H(void)
{
//    HAL_GPIO_WritePin(ACC_CS_GPIO_Port, ACC_CS_Pin, GPIO_PIN_SET);
		DL_GPIO_setPins(ACC_CS_GPIO_Port,ACC_CS_Pin);
}
/**
************************************************************************
* @brief:      	BMI088_GYRO_NS_L(void)
* @param:       void
* @retval:     	void
* @details:    	将BMI088陀螺仪片选信号置低，使其处于选中状态
************************************************************************
**/
void BMI088_GYRO_NS_L(void)
{
//    HAL_GPIO_WritePin(GYRO_CS_GPIO_Port, GYRO_CS_Pin, GPIO_PIN_RESET);
		DL_GPIO_clearPins(GYRO_CS_GPIO_Port,GYRO_CS_Pin);
}
/**
************************************************************************
* @brief:      	BMI088_GYRO_NS_H(void)
* @param:       void
* @retval:     	void
* @details:    	将BMI088陀螺仪片选信号置高，使其处于非选中状态
************************************************************************
**/
void BMI088_GYRO_NS_H(void)
{
//    HAL_GPIO_WritePin(GYRO_CS_GPIO_Port, GYRO_CS_Pin, GPIO_PIN_SET);
		DL_GPIO_setPins(GYRO_CS_GPIO_Port,GYRO_CS_Pin);
}
/**
************************************************************************
* @brief:      	BMI088_read_write_byte(uint8_t txdata)
* @param:       txdata - 要发送的数据
* @retval:     	uint8_t - 接收到的数据
* @details:    	通过BMI088使用的SPI总线进行单字节的读写操作
************************************************************************
**/
uint8_t BMI088_read_write_byte(uint8_t txdata)
{
        uint8_t rxdata = 0;

        //发送数据
        DL_SPI_transmitData8(SPI_Bmi088_INST,txdata);
        //等待SPI总线空闲
        while(DL_SPI_isBusy(SPI_Bmi088_INST));
        //接收数据
        rxdata = DL_SPI_receiveData8(SPI_Bmi088_INST);
        //等待SPI总线空闲
        while(DL_SPI_isBusy(SPI_Bmi088_INST));

        return rxdata;
}

