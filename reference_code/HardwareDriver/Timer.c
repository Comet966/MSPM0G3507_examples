// 头文件包含
#include "ti_msp_dl_config.h"   // TI MSPM0驱动库配置文件             // 系统级定义和函数
#include "timer.h"             // 时间相关函数和定义


//systime timer_t1a,timer_t5a,timer_t6a,timer_t7a;
extern void duty_200hz(void);  // 200Hz周期任务函数
extern void duty_1000hz(void);       // 1000Hz周期任务函数
extern void duty_100hz(void);        // 100Hz周期任务函数
extern void duty_10hz(void);         // 10Hz周期任务函数

void TimerInit(void)
{
    // 启动PWM定时器（TimerA和TimerG类型）
    // 配置TIMER1中断并启动定时器
    NVIC_EnableIRQ(TIMERA1_10hz_INST_INT_IRQN);  // 使能TIMER1的NVIC中断
    DL_TimerG_startCounter(TIMERA1_10hz_INST);    // 启动TIMER1对应的TimerG

    // 配置TIMER2中断并启动定时器
    NVIC_EnableIRQ(TIMERG7_200hz_INST_INT_IRQN);  // 使能TIMER2的NVIC中断
    DL_TimerG_startCounter(TIMERG7_200hz_INST);    // 启动TIMER2对应的TimerG

    // 配置TIMER_G8中断并启动定时器
    NVIC_EnableIRQ(TIMERG8_100hz_INST_INT_IRQN); // 使能TIMER_G8的NVIC中断
    DL_TimerG_startCounter(TIMERG8_100hz_INST);   // 启动TIMER_G8对应的TimerG

    // 配置TIMER_G12中断并启动定时器
    NVIC_EnableIRQ(TIMERG6_1000hz_INST_INT_IRQN); // 使能TIMER_G6的NVIC中断
    DL_TimerG_startCounter(TIMERG6_1000hz_INST);   // 启动TIMER_G6对应的TimerG
}

void timer_pwm_config(void)
{
  //pwm
  DL_TimerA_startCounter(PWM_Motors_INST);//A0
}



void TIMERG6_1000hz_INST_IRQHandler(void)
{
  switch (DL_TimerG_getPendingInterrupt(TIMERG6_1000hz_INST))
  {
    case DL_TIMERG_IIDX_ZERO:
    {
			duty_1000hz();
    }
    break;
    default:
      break;
  }
}


void TIMERG7_200hz_INST_IRQHandler(void)
{
  switch (DL_TimerG_getPendingInterrupt(TIMERG7_200hz_INST))
  {
    case DL_TIMERG_IIDX_ZERO:
    {
      duty_200hz();
      static uint32_t _cnt = 0; _cnt++;
//      if(_cnt % 50 == 0)	
//			DL_GPIO_togglePins(LED1_PORT, LED1_PIN_14_PIN);
    }
    break;

    default:
      break;
  }
}



void TIMERG8_100hz_INST_IRQHandler(void)//地面站数据发送中断函数
{
  switch (DL_TimerG_getPendingInterrupt(TIMERG8_100hz_INST))
  {
    case DL_TIMERG_IIDX_ZERO:
    {
   		duty_100hz();
    }
    break;

    default:
      break;
  }
}


void TIMERA1_10hz_INST_IRQHandler(void)//地面站数据发送中断函数
{
  switch (DL_TimerG_getPendingInterrupt(TIMERA1_10hz_INST))
  {
    case DL_TIMERG_IIDX_ZERO:
    {
			duty_10hz();
    }
    break;
    default:
      break;
  }
}






