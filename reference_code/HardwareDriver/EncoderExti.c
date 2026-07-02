#include "ti_msp_dl_config.h"
#include "EncoderExti.h"

extern Car_t Car;

void GROUP1_IRQHandler(void)//Group1的中断服务函数
{
    //读取Group1的中断寄存器并清除中断标志位
    switch( DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1) )
    {
        //检查是否是KEY的GPIOA端口中断，注意是INT_IIDX，不是PIN_18_IIDX
        case Encoder1_INT_IIDX:
            //如果按键按下变为高电平
            if( DL_GPIO_readPins(Encoder1_PORT, Encoder1_Encoder1A_PIN) > 0 )
            {
                //设置LED引脚状态翻转
                Car.Motors->EncoderLeftFront->EncoderCount -- ;
            }
						else
								Car.Motors->EncoderLeftFront->EncoderCount ++ ;
        break;
				
				case Encoder2_INT_IIDX:
            //如果按键按下变为高电平
            if( DL_GPIO_readPins(Encoder2_PORT, Encoder2_Encoder2A_PIN) > 0 )
            {
                //设置LED引脚状态翻转
                Car.Motors->EncoderRightFront->EncoderCount ++ ;
            }
						else
								Car.Motors->EncoderRightFront->EncoderCount -- ;
        break;
    }
}
