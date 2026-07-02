#include "ti_msp_dl_config.h"
#include "Task.h"

void TaskInit(Task_t** Task)
{
	static Task_t task;
	*Task = &task;
	task.AverageSpeed = 10 ;
	task.Deltayaw = 0 ;
	
	
	task.CarStartFlag = 0 ;
	task.KeyPressFlag = 0 ;
	task.LastKeyPressFlag = 0 ;
	task.KeyPressCount = 0 ;
	task.CircleCount = 0 ;
	
	task.CarBeginLengthCountFlag = 0;
	task.CarBeginLengthCount = 0;
	
}

// 左转控制相关的静态状态变量
static uint8_t LeftAngleDetectedFlag = 0;      // 左直角检测标志
static uint16_t ForceLeftTurnCount = 0;    // 强制左转计数器
static float StartMileage = 0.0f;          // 检测到左转时的起始里程
static uint8_t WaitingForTurn = 0;         // 等待转向状态标志

static uint8_t LeftAngleProtectFlag = 0 ;
static uint8_t LeftAngleProtectCount = 0 ;

void Task(Car_t Car)
{
		// 检测左边三个传感器（bit0, bit1, bit2）是否都为0，判断左直角
	if((Car.GraySensor->BinaryData & 0x07) == 0x00 && LeftAngleDetectedFlag == 0&&LeftAngleProtectFlag == 0) //进入左直角
	{
		LeftAngleDetectedFlag = 1;
		if(Car.Tasks->CarBeginLengthCountFlag)//记录第一个直道长度，方便最后一圈回来
		{
			Car.Tasks->CarBeginLengthCount = StraightWay_Length - EncoderTotalLengthGet(Car.Motors);
			Car.Tasks->CarBeginLengthCountFlag = 0;
			uart_printf(Car.Tasks->CarBeginLengthCount);
		}
		EncoderLengthClear(Car.Motors);
		uart0_send_char(0x01);
	}
	
	else if(LeftAngleDetectedFlag ==1)
	{
		if(EncoderTotalLengthGet(Car.Motors)>CAR_LENGTH)
			LeftAngleDetectedFlag = 2 ;
		else
		{
			Car.Tasks->Deltayaw = 0;
			return;
		}
	}
	
	else if(LeftAngleDetectedFlag ==2)
	{
		if(((Car.GraySensor->BinaryData & 0x10) !=0x10) && EncoderDeltaLengthGet(Car.Motors)>Turn_LENGTH)
		{
			// 检测到黑线，使用滤波后的巡线逻辑
			Car.Tasks->Deltayaw = GraySensorToTurnAngle(Car.GraySensor);
			
			LeftAngleProtectFlag = 1;
			LeftAngleProtectCount = 200 ;
			
			LeftAngleDetectedFlag = 0 ;
			EncoderLengthClear(Car.Motors);
			
			Car.Tasks->RightAngleCount++;
			
			if(Car.Tasks->RightAngleCount == 4)
			{
				Car.Tasks->RightAngleCount = 0;
				Car.Tasks->CircleCount --;
				BuzzerBee(Car.Buzzer,2);
				if(Car.Tasks->CircleCount <= 0)
				{
					LeftAngleDetectedFlag = 3 ;
					EncoderLengthClear(Car.Motors);
				}
			}
			else
				BuzzerBee(Car.Buzzer,1);
			return;
		}
		else
		{
			// 未检测到黑线，强制左转
			Car.Tasks->Deltayaw = Turn_Speed; // 强制左转角度
			Car.Tasks->AverageSpeed = 30 ;
			return;
		}
	}
	else if(LeftAngleDetectedFlag ==3)
	{
		if(Car.Tasks->CarBeginLengthCount<EncoderTotalLengthGet(Car.Motors))
		{
			CarStop(Car);
			Car.Tasks->CarBeginLengthCount = 0;
			LeftAngleDetectedFlag = 0;
		}
	}
	

	if(LeftAngleProtectFlag)
	{
		LeftAngleProtectCount--;
		if(LeftAngleProtectCount <=0)
			LeftAngleProtectFlag = 0;
	}
	
	// 调用滤波后的灰度传感器转向角度计算函数
if(EncoderTotalLengthGet(Car.Motors)>5.0f)//&&EncoderTotalLengthGet(Car.Motors)<63.0f)
	{if(Car.Tasks->RightAngleCount == 2)
			Car.Tasks->AverageSpeed = 30 ;
		else if(Car.Tasks->RightAngleCount == 4|| Car.Tasks->RightAngleCount == 0)
			Car.Tasks->AverageSpeed = 35 ;
		else if(Car.Tasks->RightAngleCount == 1 || Car.Tasks->RightAngleCount == 3)
			Car.Tasks->AverageSpeed = 40 ;
		
		uart_printf(Car.Tasks->RightAngleCount);
	}
	else
		Car.Tasks->AverageSpeed = 30;
	
	
	Car.Tasks->Deltayaw = GraySensorToTurnAngle(Car.GraySensor);
}



void CarStart(Car_t car)
{
	car.Tasks->CarStartFlag = !car.Tasks->CarStartFlag;
	BuzzerBee(car.Buzzer,1);
}
void CarStop(Car_t car)
{
	car.Tasks->CarStartFlag = 0;
	BuzzerBee(car.Buzzer,1);
}



