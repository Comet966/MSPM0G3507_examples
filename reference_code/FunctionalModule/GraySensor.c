#include "ti_msp_dl_config.h"
#include "GraySensor.h"
extern Car_t Car ;


void GraySensorInit(GraySensor_t** GraySensor)
{
	 static GraySensor_t graysensor;
		*GraySensor = &graysensor;
		graysensor.bit0 = 0;
		graysensor.bit1 = 0;
		graysensor.bit2 = 0;
		graysensor.bit3 = 0;
		graysensor.bit4 = 0;
		graysensor.bit5 = 0;
		graysensor.bit6 = 0;
		graysensor.bit7 = 0;
		graysensor.BinaryData = 0xFF;
		graysensor.GraySensorNoData = 0xFF;
		
		// 初始化ADC原始值
		graysensor.adc0_raw = 0;
		graysensor.adc1_raw = 0;
		graysensor.adc2_raw = 0;
		graysensor.adc3_raw = 0;
		graysensor.adc4_raw = 0;
		graysensor.adc5_raw = 0;
		graysensor.adc6_raw = 0;
		graysensor.adc7_raw = 0;
		
		// 设置ADC位数对应的满量程值（12位ADC）
		graysensor.adc_bits = 4096;
		
		// 手动设置校准数据 - 白色值（传感器在白色表面上的ADC读数）
		graysensor.calibrated_white[0] = 3234;  // bit0白色校准值
		graysensor.calibrated_white[1] = 3230;  // bit1白色校准值
		graysensor.calibrated_white[2] = 3246;  // bit2白色校准值
		graysensor.calibrated_white[3] = 3177;  // bit3白色校准值
		graysensor.calibrated_white[4] = 3178;  // bit4白色校准值
		graysensor.calibrated_white[5] = 3163;  // bit5白色校准值
		graysensor.calibrated_white[6] = 3198;  // bit6白色校准值
		graysensor.calibrated_white[7] = 3140;  // bit7白色校准值
		
		// 手动设置校准数据 - 黑色值（传感器在黑色线上的ADC读数）
		graysensor.calibrated_black[0] = 2584;   // bit0黑色校准值
		graysensor.calibrated_black[1] = 1328;   // bit1黑色校准值
		graysensor.calibrated_black[2] = 2749;   // bit2黑色校准值
		graysensor.calibrated_black[3] = 2758;   // bit3黑色校准值
		graysensor.calibrated_black[4] = 1903;   // bit4黑色校准值
		graysensor.calibrated_black[5] = 3062;   // bit5黑色校准值
		graysensor.calibrated_black[6] = 2964;   // bit6黑色校准值
		graysensor.calibrated_black[7] = 2029;   // bit7黑色校准值
		
		// 计算归一化系数
		for(int i = 0; i < 8; i++)
		{
			// 确保白值 > 黑值（必要时交换）
			if(graysensor.calibrated_black[i] >= graysensor.calibrated_white[i])
			{
				uint16_t temp = graysensor.calibrated_white[i];
				graysensor.calibrated_white[i] = graysensor.calibrated_black[i];
				graysensor.calibrated_black[i] = temp;
			}
			
			// 处理无效校准数据（全黑/全白/相等情况）
			if ((graysensor.calibrated_white[i] == 0 && graysensor.calibrated_black[i] == 0) ||
				  (graysensor.calibrated_white[i] == graysensor.calibrated_black[i]))
			{
				graysensor.normal_factor[i] = 0.0f;  // 无效通道
			}
			else
			{
				// 计算动态范围
				float dynamic_range = (float)(graysensor.calibrated_white[i] - graysensor.calibrated_black[i]);
				// 计算归一化系数
				graysensor.normal_factor[i] = (float)graysensor.adc_bits / dynamic_range;
			}
		}
}
// ADC读取灰度传感器值的宏定义
#define READ_GRAY_ADC() ({ \
    unsigned int gAdcResult = 0; \
    DL_ADC12_enableConversions(ADCGraySensor_INST); \
    DL_ADC12_startConversion(ADCGraySensor_INST); \
    while (DL_ADC12_getStatus(ADCGraySensor_INST) != DL_ADC12_STATUS_CONVERSION_IDLE); \
    DL_ADC12_stopConversion(ADCGraySensor_INST); \
    DL_ADC12_disableConversions(ADCGraySensor_INST); \
    gAdcResult = DL_ADC12_getMemResult(ADCGraySensor_INST, ADCGraySensor_ADCMEM_C5 ); \
    gAdcResult; \
})

// 将ADC值转换为数字量（0或1）的宏定义
// 阈值可根据实际情况调整
#define ADC_THRESHOLD 3950  // ADC阈值，假设12位ADC，中间值为2048
#define SAMPLE_COUNT 4      // 每个传感器采样次数（总共4次，舍去第1次，保留后3次）

// 读取一路ADC原始值（舍去第一次采样，保留后三次并取平均）
#define READ_AND_STORE_ADC() ({ \
    unsigned int sum = 0; \
    for(int sample = 0; sample < SAMPLE_COUNT; sample++) { \
        unsigned int current_sample = READ_GRAY_ADC(); \
        if(sample > 0) { /* 舍去第一次采样（sample=0），从第二次开始累加 */ \
            sum += current_sample; \
        } \
    } \
    sum / (SAMPLE_COUNT - 1); /* 除以有效采样次数（3次） */ \
})

// 选择器控制宏定义 - 设置3个引脚的组合来选择传感器
//#define SET_SELECTOR(pin2, pin1, pin0) do { \
//    (pin2) ? DL_GPIO_setPins(GraySensor_PORT,GraySensor_PIN_2_PIN) : DL_GPIO_clearPins(GraySensor_PORT, GraySensor_PIN_2_PIN); \
//    (pin1) ? DL_GPIO_setPins(GraySensor_PORT,GraySensor_PIN_1_PIN) : DL_GPIO_clearPins(GraySensor_PORT, GraySensor_PIN_1_PIN); \
//    (pin0) ? DL_GPIO_setPins(GraySensor_PORT,GraySensor_PIN_0_PIN) : DL_GPIO_clearPins(GraySensor_PORT, GraySensor_PIN_0_PIN); \
//} while(0)

#define DELAY_CYCLES(cycles) do { \
    volatile uint32_t delay_count = (cycles); \
    while(delay_count--); \
} while(0)

void SET_SELECTOR(uint8_t pin2, uint8_t pin1, uint8_t pin0)
{
	if(pin2) DL_GPIO_setPins(GraySensor_PIN_2_PORT,GraySensor_PIN_2_PIN);
	else DL_GPIO_clearPins(GraySensor_PIN_2_PORT ,GraySensor_PIN_2_PIN);
	if(pin1) DL_GPIO_setPins(GraySensor_PIN_1_PORT,GraySensor_PIN_1_PIN);
	else DL_GPIO_clearPins(GraySensor_PIN_1_PORT, GraySensor_PIN_1_PIN);
	if(pin0) DL_GPIO_setPins(GraySensor_PIN_0_PORT,GraySensor_PIN_0_PIN);
	else DL_GPIO_clearPins(GraySensor_PIN_0_PORT, GraySensor_PIN_0_PIN);
}

void GraySensorDataUpdate (GraySensor_t* GraySensor)
{
	// 第一步：依次读取8个传感器的ADC原始值
	
	// 读取bit0的ADC值
	SET_SELECTOR(0,0,0);
	GraySensor->adc0_raw = READ_AND_STORE_ADC();
	
	// 读取bit1的ADC值
	SET_SELECTOR(0,0,1);
	GraySensor->adc1_raw = READ_AND_STORE_ADC();
	
	// 读取bit2的ADC值
	SET_SELECTOR(0,1,0);
	GraySensor->adc2_raw = READ_AND_STORE_ADC();
	
	// 读取bit3的ADC值
	SET_SELECTOR(0,1,1);
	GraySensor->adc3_raw = READ_AND_STORE_ADC();
	
	// 读取bit4的ADC值
	SET_SELECTOR(1,0,0);
	GraySensor->adc4_raw = READ_AND_STORE_ADC();
	
	// 读取bit5的ADC值
	SET_SELECTOR(1,0,1);
	GraySensor->adc5_raw = READ_AND_STORE_ADC();
	
	// 读取bit6的ADC值
	SET_SELECTOR(1,1,0);
	GraySensor->adc6_raw = READ_AND_STORE_ADC();
	
	// 读取bit7的ADC值
	SET_SELECTOR(1,1,1);
	GraySensor->adc7_raw = READ_AND_STORE_ADC();
	SET_SELECTOR(0,0,0);
	
	// 第二步：使用归一化系数处理ADC原始值并与阈值比较
	uint16_t normalized_adc[8];
	
	// 对每个传感器的ADC值进行归一化处理
	for(int i = 0; i < 8; i++)
	{
		uint16_t* adc_raw_ptr[] = {
			&GraySensor->adc0_raw, &GraySensor->adc1_raw, &GraySensor->adc2_raw, &GraySensor->adc3_raw,
			&GraySensor->adc4_raw, &GraySensor->adc5_raw, &GraySensor->adc6_raw, &GraySensor->adc7_raw
		};
		
		if(GraySensor->normal_factor[i] > 0.0f)  // 有效通道
		{
			// 归一化处理：(原始值 - 黑色值) * 归一化系数
			float temp = ((float)(*adc_raw_ptr[i]) - (float)GraySensor->calibrated_black[i]) * GraySensor->normal_factor[i];
			
			// 限制在有效范围内
			if(temp < 0) temp = 0;
			if(temp > GraySensor->adc_bits) temp = GraySensor->adc_bits;
			
			normalized_adc[i] = (uint16_t)temp;
		}
		else
		{
			normalized_adc[i] = *adc_raw_ptr[i];  // 无效通道直接使用原始值
		}
	}
	
//	uart_printf(GraySensor->adc0_raw);
//	uart_printf(GraySensor->adc1_raw);
//	uart_printf(GraySensor->adc2_raw);
//	uart_printf(GraySensor->adc3_raw);
//	uart_printf(GraySensor->adc4_raw);
//	uart_printf(GraySensor->adc5_raw);
//	uart_printf(GraySensor->adc6_raw);
//	uart_printf(GraySensor->adc7_raw);
//	uart_printf(normalized_adc[0]);
//	uart_printf(normalized_adc[1]);
//	uart_printf(normalized_adc[2]);
//	uart_printf(normalized_adc[3]);
//	uart_printf(normalized_adc[4]);
//	uart_printf(normalized_adc[5]);
//	uart_printf(normalized_adc[6]);
//	uart_printf(normalized_adc[7]);
//	uart_printf(321);
		//uart_printf(normalized_adc[0]);
	// 使用归一化后的值与阈值比较，获得数字值
	uint16_t normalized_threshold = GraySensor->adc_bits / 2;  // 使用满量程的一半作为阈值
	
	GraySensor->bit0 = (normalized_adc[0] > normalized_threshold) ? 1 : 0;
	GraySensor->bit1 = (normalized_adc[1] > normalized_threshold) ? 1 : 0;
	GraySensor->bit2 = (normalized_adc[2] > normalized_threshold) ? 1 : 0;
	GraySensor->bit3 = (normalized_adc[3] > normalized_threshold) ? 1 : 0;
	GraySensor->bit4 = (normalized_adc[4] > normalized_threshold) ? 1 : 0;
	GraySensor->bit5 = (normalized_adc[5] > normalized_threshold) ? 1 : 0;
	GraySensor->bit6 = (normalized_adc[6] > normalized_threshold) ? 1 : 0;
	GraySensor->bit7 = (normalized_adc[7] > normalized_threshold) ? 1 : 0;
	
	GraySensor->BinaryData = 0x00 ;
	
	GraySensor->BinaryData |= GraySensor->bit0 << 0 ;
	GraySensor->BinaryData |= GraySensor->bit1 << 1 ;
	GraySensor->BinaryData |= GraySensor->bit2 << 2 ;
	GraySensor->BinaryData |= GraySensor->bit3 << 3 ;
	GraySensor->BinaryData |= GraySensor->bit4 << 4 ;
	GraySensor->BinaryData |= GraySensor->bit5 << 5 ;
	GraySensor->BinaryData |= GraySensor->bit6 << 6 ;
	GraySensor->BinaryData |= GraySensor->bit7 << 7 ;
//	uart_printf(GraySensor->BinaryData);
	if((GraySensor->BinaryData == 0xFF||GraySensor->BinaryData == 0x00)&&GraySensor->GraySensorNoData!= 0xFF) GraySensor->GraySensorNoData++;
	else if(GraySensor->GraySensorNoData!= 0x00)GraySensor->GraySensorNoData = GraySensor->GraySensorNoData/2;
}

float GraySensorToTurnAngle(GraySensor_t* GraySensor)
{
    // 传感器位置权重 (从左到右: bit0到bit7)
    // 负值表示左偏，正值表示右偏
    const float sensorWeights[8] = {3.0f, 1.5f, 0.5f, 0.10f, -0.10f, -0.5f, -1.5f, -3.0f};
    
    float weightedSum = 0.0f;    // 加权位置和
    float totalWeight = 0.0f;    // 总权重
    uint8_t lineCount = 0;       // 检测到黑线的传感器数量
    
    // 计算加权位置
    for(int i = 0; i < 8; i++)
    {
        // 检测到黑线时(传感器值为0)
        if(!(GraySensor->BinaryData & (1 << i)))
        {
            weightedSum += sensorWeights[i];
            totalWeight += 1.0f;
            lineCount++;
        }
    }
    
    // 如果没有检测到任何黑线，返回0（直行）
    if(lineCount == 0)
    {
        return 0.0f;
    }
    
    // 计算平均位置偏差
    float positionError = weightedSum / totalWeight;
    
    // 根据检测到的传感器数量和位置偏差计算转向角度
    float turnAngle = 0.0f;
    

    turnAngle = positionError * 10.0f;
        
    return turnAngle;

/* 原始离散逻辑代码保留作为参考
//速度50时的参数
	if(!(GraySensor->BinaryData&0x01)) return +55;
	else if(!(GraySensor->BinaryData&0x80)) return -55;
	else if(!(GraySensor->BinaryData&0x02)) return +20;
	else if(!(GraySensor->BinaryData&0x40)) return -20;
	else if(!(GraySensor->BinaryData&0x04)) return +10;
	else if(!(GraySensor->BinaryData&0x20)) return -10;
	else if((!(GraySensor->BinaryData&0x08))&!(GraySensor->BinaryData&0x10)) return 0;
	else if(!(GraySensor->BinaryData&0x08)) return +5;
	else if(!(GraySensor->BinaryData&0x10)) return -5 ;
//	else return 0;
//	
//速度80时的参数
//	if(!(GraySensor->BinaryData&0x01)) return +60;
//	else if(!(GraySensor->BinaryData&0x80)) return -60;
//	else if(!(GraySensor->BinaryData&0x02)) return +40;
//	else if(!(GraySensor->BinaryData&0x40)) return -40;
//	else if(!(GraySensor->BinaryData&0x04)) return +20;
//	else if(!(GraySensor->BinaryData&0x20)) return -20;
//	else if((!(GraySensor->BinaryData&0x08))&!(GraySensor->BinaryData&0x10)) return 0;
//	else if(!(GraySensor->BinaryData&0x08)) return +10;
//	else if(!(GraySensor->BinaryData&0x10)) return -10 ;
//	else return 0;
//速度100时的参数
//	if(!(GraySensor->BinaryData&0x01)) return +90;
//	else if(!(GraySensor->BinaryData&0x80)) return -90;
//	else if(!(GraySensor->BinaryData&0x02)) return +60;
//	else if(!(GraySensor->BinaryData&0x40)) return -60;
//	else if(!(GraySensor->BinaryData&0x04)) return +35;
//	else if(!(GraySensor->BinaryData&0x20)) return -35;
//	else if((!(GraySensor->BinaryData&0x08))&!(GraySensor->BinaryData&0x10)) return 0;
//	else if(!(GraySensor->BinaryData&0x08)) return +15;
//	else if(!(GraySensor->BinaryData&0x10)) return -15 ;
//	else return 0;
//速度130时的参数
//	if(!(GraySensor->BinaryData&0x01)) return +40;
//	else if(!(GraySensor->BinaryData&0x80)) return -40;
//	else if(!(GraySensor->BinaryData&0x02)) return +25;
//	else if(!(GraySensor->BinaryData&0x40)) return -25;
//	else if(!(GraySensor->BinaryData&0x04)) return +15;
//	else if(!(GraySensor->BinaryData&0x20)) return -15;
//	else if((!(GraySensor->BinaryData&0x08))&!(GraySensor->BinaryData&0x10)) return 0;
//	else if(!(GraySensor->BinaryData&0x08)) return +5;
//	else if(!(GraySensor->BinaryData&0x10)) return -5 ;
//	else return 0;
*/
}
