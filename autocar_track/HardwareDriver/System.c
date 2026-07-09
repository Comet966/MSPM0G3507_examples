#include "ti_msp_dl_config.h"
#include "system.h"


volatile uint32_t sysTickUptime = 0;
//void SysTick_Handler(void)
//{
//  sysTickUptime++;
//}




//返回 us
uint32_t micros(void)
{
  uint32_t systick_period = CPUCLK_FREQ / 1000U;
  return sysTickUptime * 1000 + (1000 * (systick_period - SysTick->VAL)) / systick_period;
}

//返回 ms
uint32_t millis(void) {
	return micros() / 1000UL;
}

//延时us
void delayMicroseconds(uint32_t us) {
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 38;

    // 计算需要的时钟数 = 延迟微秒数 * 每微秒的时钟数
    ticks = us * (80000000 / 1000000);

    // 获取当前的SysTick值
    told = SysTick->VAL;

    while (1)
    {
        // 重复刷新获取当前的SysTick值
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += SysTick->LOAD - tnow + told;

            told = tnow;

            // 如果达到了需要的时钟数，就退出循环
            if (tcnt >= ticks)
                break;
        }
    }
}

void delay(uint32_t ms) {
  delayMicroseconds(ms * 1000UL);
}


void delay_ms(uint32_t x)
{
  delay(x);
}

void delay_us(uint32_t x)
{
  delayMicroseconds(x);
}

void Delay_Ms(uint32_t time)  //延时函数  
{   
	delay_ms(time);
}  

void Delay_Us(uint32_t time)  //延时函数  
{   
	delay_us(time);
}  


float get_systime_ms(void)
{
  return millis();//单位ms
}

uint32_t get_systick_ms(void)
{
  return (uint32_t)(sysTickUptime);//单位ms
}




