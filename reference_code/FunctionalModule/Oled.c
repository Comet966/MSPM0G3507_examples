//////////////////////////////////////////////////////////////////////////////////	 
//
//  最近修改   : 2022-11-12
//  功能描述   : 0.96寸OLED 接口演示例程(STM32F103C8系列IIC)
//              说明: 
//              ----------------------------------------------------------------
//              GND   电源地
//              VCC   接3.3v电源
//              SCL   接PA5（SCL）
//              SDA   接PA7（SDA）            
//              ----------------------------------------------------------------
//
//////////////////////////////////////////////////////////////////////////////////

#include "oled.h"
#include "oledfont.h"
//#include "delay.h"
#include "mymath.h"
#include "IIC.h"

//OLED的显存
u8 Map[8][128] = { 0 };
//存放格式如下.
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127

//旋转参数，默认旋转中心为显示区域坐标，对应贴图上的坐标(0,0)点(左上角)，角度为顺时针旋转
struct SpanSet{
	POINT spanCenter;	//字旋转中心
	POINT spanZero;		//句旋转中心
	float ang;				//旋转角
	float argCos;			//旋转角余弦值
	float argSin;			//旋转角正弦值
}	spanSet;

/**********************************************/
/****************** IIC驱动函数 ***************/
/**********************************************/
// IIC Write Command
void OLED_Write_IIC_Command(u8 IIC_Command)
{
	MYIIC_Start();
	MYIIC_Write_Byte(0x78);            //Slave address,SA0=0
	MYIIC_Wait_Ack();	
	MYIIC_Write_Byte(0x00);			//write command
	MYIIC_Wait_Ack();	
	MYIIC_Write_Byte(IIC_Command); 
	MYIIC_Wait_Ack();	
	MYIIC_Stop();
}
// IIC Write Data
void OLED_Write_IIC_Data(u8 IIC_Data)
{
	MYIIC_Start();
	MYIIC_Write_Byte(0x78);			//D/C#=0; R/W#=0
	MYIIC_Wait_Ack();	
	MYIIC_Write_Byte(0x40);			//write data
	MYIIC_Wait_Ack();	
	MYIIC_Write_Byte(IIC_Data);
	MYIIC_Wait_Ack();	
	MYIIC_Stop();
}
void OLED_WR_Byte(u8 dat, u8 cmd)
{
	if(cmd)	OLED_Write_IIC_Data(dat);
	else		OLED_Write_IIC_Command(dat);
}


/**********************************************/
/****************** OLED驱动函数 **************/
/**********************************************/
//坐标设置
void OLED_Set_Pos(u8 x, u8 y)
{
	OLED_WR_Byte(0xb0 + y, OLED_CMD);										//设置页地址（0~7）
	OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);	  //设置显示位置—列低地址
	OLED_WR_Byte((x & 0x0f), OLED_CMD); 								//设置显示位置—列高地址
}
//开启OLED显示    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X14, OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF, OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X10, OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE, OLED_CMD);  //DISPLAY OFF
}
//清页函数
void OLED_Clear_page(u8 i)
{
	u8  n;
	OLED_WR_Byte (0xb0 + i - 1, OLED_CMD);  //设置页地址（0~7）
	OLED_WR_Byte (0x00, OLED_CMD);          //设置显示位置—列低地址
	OLED_WR_Byte (0x10, OLED_CMD);          //设置显示位置—列高地址   
	for(n = 0; n < 128; n++)
			OLED_WR_Byte(0, OLED_DATA);
}
//清屏函数
void OLED_Clear(void)  
{
	u8  i, n;
	for(i = 0; i < 8; i++)  
	{  
		OLED_WR_Byte (0xb0 + i, OLED_CMD);  //设置页地址（0~7）
		OLED_WR_Byte (0x00, OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10, OLED_CMD);      //设置显示位置—列高地址   
		for(n = 0; n < 128; n++)
			OLED_WR_Byte(0, OLED_DATA); 
	} //更新显示
}
void OLED_On(void)  
{
	u8  i, n;		    
	for(i = 0; i < 8; i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n = 0; n < 128; n++)
            OLED_WR_Byte(1, OLED_DATA); 
	} //更新显示
}
//设置旋转角度
void OLED_SetSpanAng(float ang)
{
	spanSet.ang = ang;
	spanSet.argSin = MYMATH_Sin(ang);
	spanSet.argCos = MYMATH_Cos(ang);
}
//设置字旋转中心
void OLED_SetSpan(float x, float y)
{
	spanSet.spanCenter.x = x;
	spanSet.spanCenter.y = y;
}
//设置句旋转中心
void OLED_SetSpanZero(float x, float y)
{
	spanSet.spanZero.x = x;
	spanSet.spanZero.y = y;
}
//设置旋转中心，(原偏移坐标，设置的坐标)返回设置后的偏移坐标(贴图左上角坐标)
POINT OLED_SetSpanCenter(float xBef, float yBef, float xSet, float ySet)
{
	POINT point;
	point.x = xSet - spanSet.argCos * (xSet - xBef) + spanSet.argSin * (ySet - yBef);
	point.y = ySet - spanSet.argSin * (xSet - xBef) - spanSet.argCos * (ySet - yBef);
	return point;
}
//向Map中写入数据
void OLED_WriteMap(u8 bit, int xpose, int ypose)
{
	u8 y = 0;
	u8 i = 0;
	u8 bit_ = 0;
	if(xpose >=0 && xpose < 128 && ypose >= 0 && ypose < 64)
	{
		y = ypose / 8;
		bit_ = Map[y][xpose];
		Map[y][xpose] = 0;
		for(i = 0; i < 8; i++)
		{
			Map[y][xpose] <<= 1;
			if(i != 7 - ypose + 8 * y)
				Map[y][xpose] += (bit_ & 0x80) >> 7;
			else if(bit > 0)
				Map[y][xpose] += 0x01;
			bit_ <<= 1;
		}
	}
}
//Map载入显存
void OLED_ShowMap(void)
{
	u8 i = 0;
	u8 page = 0;
	for(page = 0; page < 8; page++)
	{
		OLED_Set_Pos(0, page);
		MYIIC_Start();
		MYIIC_Write_Byte(0x78);			//D/C#=0; R/W#=0
		MYIIC_Wait_Ack();	
		MYIIC_Write_Byte(0x40);			//write data
		MYIIC_Wait_Ack();
		for(i = 0; i < 128; i++)
		{
			MYIIC_Write_Byte(Map[page][i]);
			MYIIC_Wait_Ack();	
		}
		MYIIC_Stop();
	}
}
//清除Map数据
void OLED_ClearMap(u8 data)
{
	u8 i = 0, n = 0;
	for(i = 0; i < 8; i++)
	{
		for(n = 0; n < 128; n++)
		{
			Map[i][n] = data;
		}
	}
}
/***************************************************
//OLED_Reverse: 将选定的区域取反
//xpose,ypose :	左上角坐标
//width,height:	区域宽高
****************************************************/
void OLED_Reverse(int xpose, int ypose, u8 width, u8 height)
{
	u8 xend = 0, yend = 0, ypagestart = 0, ypageend = 0;
	u8 i = 0, n = 0;
	u8 y1 = 0, y2 = 0;	//第一个、最后一个字节需要取反的位数
	u8 len = 0;					//记录取反的总位数
	
	if(xpose < 0)	xpose = 0;
	if(ypose < 0)	ypose = 0;
	xend = xpose + width;
	yend = ypose + height;
	if(xend >= X_WIDTH)	xend = X_WIDTH;
	if(yend >= Y_WIDTH)	yend = Y_WIDTH;
	ypagestart = ypose / 8;
	ypageend = yend / 8;
	y1 = ypose - 8 * ypagestart;
	y2 = yend - 8 * ypageend;
	
	for(i = xpose; i < xend; i++)
	{
		len = 0;
		/*对开始一个字节存在的部分位取反*/
		for(n = (y1 > 0 ? y1 : 8); n < 8 && len < height; n++)
		{
			len++;
			Map[ypagestart][i] ^= (0x01 << n);
		}
		/*对中间存在的整个字节取反*/
		for(n = ypagestart + (y1 > 0 ? 1 : 0); n < ypageend && n < Y_PAGE && len < height; n++)
		{
			len += 8;
			Map[n][i] = ~Map[n][i];
		}
		/*对最后一个字节存在的部分位取反*/
		for(n = (y2 > 0 ? (y2 - 1) : 0); n < 8 && len < height; n--)
		{
			len++;
			Map[ypageend][i] ^= (0x01 << n);
		}
	}
}
//二进制数反转
u8 reversedata(u8 data)
{
	u8 i = 0;
	u8 tem = 0;
	u8 ans = 0;
	for(i=0; i<8;i++)
	{
		tem = data & 0x01;
		ans <<= 1;
		ans += tem;
		data >>= 1;
	}
	return ans;
}

/**********************************************/
/****************** 显示函数 ******************/
/**********************************************/

/***************************************************
//OLED_ShowChar: 在指定位置显示一个字符,包括部分字符
//x,y :左上角坐标
//ch  :待显示的字符
//Size:字体大小
//mode:显示模式，透明、反色、旋转、翻转
****************************************************/
void OLED_ShowChar(int x, int y, u8 ch, u8 Size, u8 mode)
{
	u8 i = 0, n = 0;
	u8 o = 0, p = 0;
	float xpose = 0, ypose = 0;
	u8 temp = 0;
	ch -= ' ';  //得到偏移后的值
	
	for(i = 0; i < 8; i++)
	{
		if(Size == SIZE16)
		{
			if(mode&L2R)	o = 8 - i;
			else o = i;
		}
		else
		{
			if(i >= 6)		break;
			if(mode&L2R)	o = 6 - i;
			else o = i;
		}
		
		for(n = 0; n < 16; n++)
		{
			if(Size == SIZE16)
			{
				if(mode&U2D)	p = 16 - n;
				else p = n;
				temp = F8X16[ch*16+i+n/8*8]&(0x01<<(n%8));
			}
			else
			{
				if(n >= 8)		break;
				if(mode&U2D)	p = 8 - n;
				else p = n;
				temp = F6x8[ch][i]&(0x01<<n);
			}
			
			if((mode&Transparent) && temp == 0) continue;	//透明贴图
			if(mode&Reverse)			temp = !temp;						//反色
			if((mode&Span) || (mode&SpanSingle))					//旋转
			{
				xpose = spanSet.spanCenter.x + spanSet.argCos * o - spanSet.argSin * p;
				ypose = spanSet.spanCenter.y + spanSet.argSin * o + spanSet.argCos * p;
				OLED_WriteMap(temp, (int)xpose, (int)ypose);
			}
			else
				OLED_WriteMap(temp, o+x, p+y);
		}
	}
}
/***************************************************
//OLED_ShowNum: 显示数字
//x,y :左上角坐标
//length :指定显示数字长度（从个位开始），为0自动确定长度
//num:数值(0~4294967295)
//Size:字体大小
//if0 :是否将不足位上补零
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void OLED_ShowNum(int x,int y,u32 num, u8 length, u8 size, u8 if0, u8 mode)
{
	int i = 0;
	u32 temp = num;
	u8 data = 0;
	
	if(size != SIZE16)	size = SIZE12;
//	if(mode&Span || mode&SpanSingle)	OLED_SetSpan(x,y);
	
	if(length == 0)
	{
		do{
			temp /= 10;
			length++;
		}while(temp > 0);
	}
	for(i = 0; i < length; i++)
	{
		data = num % 10 + '0';
		num /= 10;
		if(num == 0 && data == '0')
		{
			if(if0 == Add0)	data = '0';
			else					data = ' ';
		}
		if(mode&Span)	OLED_SetSpan(x+(size/2)*(length-i-1),y);
		else if(mode&SpanSingle)
		{
			OLED_SetSpan(spanSet.spanZero.x+(x+(size/2)*(length-i-1)-spanSet.spanZero.x)*spanSet.argCos,
										spanSet.spanZero.y+(x+(size/2)*(length-i-1)-spanSet.spanZero.x)*spanSet.argSin);
		}
		OLED_ShowChar(x+(size/2)*(length-i-1),y,data,size,mode);
	}
}

/***************************************************
//OLED_ShowInt: 显示整型数
//x,y :左上角坐标
//num :整型数值
//size:字体大小
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void OLED_ShowInt(int x, int y, int num, u8 size, u8 mode)
{
	int temp = 0;
	u8 length = 0;
	
	if(size != SIZE16)	size = SIZE12;
	
	if(mode&Span || mode&SpanSingle)
	{
		OLED_SetSpan(x,y);
		OLED_SetSpanZero(x,y);
	}
	
	if(num < 0)
	{
		OLED_ShowChar(x,y,'-',size,mode);
		x += size / 2;
		num = -num;
	}
	
	temp = num;
	
	do{
		temp /= 10;
		length++;
	}while(temp > 0);
	if(mode&Span)	OLED_SetSpan(x+size/2*length,y);
	else if(mode&SpanSingle)
	{
		OLED_SetSpan(spanSet.spanZero.x+(x-spanSet.spanZero.x)*spanSet.argCos,
									spanSet.spanZero.y+(x-spanSet.spanZero.x)*spanSet.argSin);
	}
	OLED_ShowNum(x,y,num,length,size,Add0,mode);
}

void OLED_ShowBinNum(int x, int y, int num, u8 size, u8 mode)
{
	int temp = 0;
	int Binnumb = 0;
	u8 length = 0;
	
	if(size != SIZE16)	size = SIZE12;
	
	if(mode&Span || mode&SpanSingle)
	{
		OLED_SetSpan(x,y);
		OLED_SetSpanZero(x,y);
	}
	
	if(num < 0)
	{
		OLED_ShowChar(x,y,'-',size,mode);
		x += size / 2;
		num = -num;
	}
		temp = num;
		Binnumb = 0;
do {
    Binnumb += (temp % 2)*pow(10,length); // 正确获取二进制位并累加
    temp /= 2;
    length++;
} while (temp > 0);
	if(length<8)length = 8;
	if(mode&Span)	OLED_SetSpan(x+size/2*length,y);
	else if(mode&SpanSingle)
	{
		OLED_SetSpan(spanSet.spanZero.x+(x-spanSet.spanZero.x)*spanSet.argCos,
									spanSet.spanZero.y+(x-spanSet.spanZero.x)*spanSet.argSin);
	}
	OLED_ShowNum(x,y,Binnumb,length,size,Add0,mode);
}
/***************************************************
//OLED_ShowFloat: 显示浮点数，当len = 0时，可显示整数
//x,y :左上角坐标
//num :浮点型数值
//len :保留的小数位数
//size:字体大小
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void OLED_ShowFloat(int x, int y, float num, u8 len, u8 size, u8 mode)
{
	int temp = 0;
	int tempInt = 0;
	int tempFloat2Int = 0;
	float tempFloat = 0;
	u8 length = 0;
	
	if(size != SIZE16)	size = SIZE12;
	
	if(mode&Span || mode&SpanSingle)
	{
		OLED_SetSpan(x,y);
		OLED_SetSpanZero(x,y);
	}
	
	if(num < 0)
	{
		OLED_ShowChar(x,y,'-',size,mode);
		x += size / 2;
		num = -num;
	}
	
	tempInt = (int)num;
	tempFloat = num - tempInt;
	length = len;
	
	while(length > 0)
	{
		tempFloat *= 10;
		length--;
	}
	tempFloat2Int = (int)tempFloat;
	temp = tempInt;
	length = 0;
	do{
		temp /= 10;
		length++;
	}while(temp > 0);
	if(mode&Span)	OLED_SetSpan(x+size/2*length,y);
	else if(mode&SpanSingle)
	{
		OLED_SetSpan(spanSet.spanZero.x+(x-spanSet.spanZero.x)*spanSet.argCos,
									spanSet.spanZero.y+(x-spanSet.spanZero.x)*spanSet.argSin);
	}
	OLED_ShowNum(x,y,tempInt,length,size,Add0,mode);
	if(len > 0)
	{
		if(mode&Span)	OLED_SetSpan(x+size/2*length,y);
		else if(mode&SpanSingle)
		{
			OLED_SetSpan(spanSet.spanZero.x+(x+size/2*length-spanSet.spanZero.x)*spanSet.argCos,
										spanSet.spanZero.y+(x+size/2*length-spanSet.spanZero.x)*spanSet.argSin);
		}
		OLED_ShowChar(x+size/2*length,y,'.',size,mode);
		if(mode&Span)	OLED_SetSpan(x+size/2*length+size/2,y);
		else if(mode&SpanSingle)
		{
			OLED_SetSpan(spanSet.spanZero.x+(x+size/2*length+size/2-spanSet.spanZero.x)*spanSet.argCos,
										spanSet.spanZero.y+(x+size/2*length+size/2-spanSet.spanZero.x)*spanSet.argSin);
		}
		OLED_ShowNum(x+size/2*length+size/2,y,tempFloat2Int,len,size,Add0,mode);
	}
}


/***************************************************
//OLED_ShowString: 显示一个字符号串
//x,y :左上角坐标
//chr :字符串
//size:字体大小
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void OLED_ShowString(int x, int y, u8 *chr, u8 size, u8 mode)
{
	u8 i = 0;
	if(size != SIZE16)	size = SIZE12;
	if(mode&SpanSingle)	OLED_SetSpanZero(x,y);
	
	while(chr[i] != '\0')
	{
		if(mode&Span)	OLED_SetSpan(x,y);
		else if(mode&SpanSingle)
		{
			OLED_SetSpan(spanSet.spanZero.x+(x-spanSet.spanZero.x)*spanSet.argCos,
										spanSet.spanZero.y+(x-spanSet.spanZero.x)*spanSet.argSin);
		}
		OLED_ShowChar(x,y,chr[i],size,mode);
		x += size / 2;
		i++;
	}
}
/***************************************************
//OLED_ShowChinese: 显示汉字
//x,y :左上角坐标
//num :待显示汉字字符编号，字体默认16号
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void OLED_ShowChinese(int x, int y, u8 num, u8 mode)
{
	int i = 0, n = 0;
	u8 o = 0, p = 0;
	float xpose = 0, ypose = 0;
	u8 temp = 0;
	
//	if(mode&Span || mode&SpanSingle)	OLED_SetSpan(x,y);
	
	for(i = 0; i < 16; i++)
	{
		if(mode&L2R)	o = 16 - i;
		else o = i;
		
		for(n = 0; n < 16; n++)
		{
			if(mode&U2D)	p = 16 - n;
			else p = n;
			
			temp = Hzk[2*num+n/8][i]&(0x01<<(n%8));
			if((mode&Transparent) && temp == 0) continue;
			if(mode&Reverse)			temp = !temp;
			if((mode&Span) || (mode&SpanSingle))					//旋转
			{
				xpose = spanSet.spanCenter.x + spanSet.argCos * o - spanSet.argSin * p;
				ypose = spanSet.spanCenter.y + spanSet.argSin * o + spanSet.argCos * p;
				OLED_WriteMap(temp, (int)xpose, (int)ypose);
			}
			else
				OLED_WriteMap(temp, o+x, p+y);
		}
	}
}
/***************************************************
//OLED_ShowChineseStr: 显示一句话
//x,y :左上角坐标
//nums:由待显示汉字字符编号组成的数组，字体默认16号
//len :nums长度
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void OLED_ShowChineseStr(int x, int y, u8* nums, u8 len, u8 mode)
{
	if(mode&SpanSingle)	OLED_SetSpanZero(x,y);
	
	while(len > 0)
	{
		if(mode&Span)	OLED_SetSpan(x,y);
		else if(mode&SpanSingle)
		{
			OLED_SetSpan(spanSet.spanZero.x+(x-spanSet.spanZero.x)*spanSet.argCos,
										spanSet.spanZero.y+(x-spanSet.spanZero.x)*spanSet.argSin);
		}
		OLED_ShowChinese(x,y,*nums,mode);
		x += 16;
		len--;
		nums++;
	}
}


/**********************************************/
/****************** 绘图函数 ******************/
/**********************************************/
//绘制直线
void OLED_line(int xpose1, int ypose1, int xpose2, int ypose2, u8 width)
{
	int i = 0, n = 0;
	float k = 0;
	float y = 0;
	if(xpose1 < xpose2)
	{
		if(ypose1 < ypose2)//从左上向右下画线
		{
			k = (float)(ypose1 - ypose2) / (float)(xpose1 - xpose2);
			for(i = xpose1; i < xpose2; i++)
				for(n = ypose1; n < ypose2; n++)
					if(i <128 &&  n < 64)
					{
						y = k * (i - xpose2) + ypose2;
						if(y < (float)n + (float)width / 2 && y > (float)n - (float)width / 2)
							OLED_WriteMap(1, i, n);
					}
		}
		else if(ypose1 > ypose2)//从左下向右上画线
		{
			k = (float)(ypose1 - ypose2) / (float)(xpose1 - xpose2);
			for(i = xpose1; i < xpose2; i++)
				for(n = ypose1; n > ypose2; n--)
					if(i <128 &&  n < 64)
					{
						y = k * (i - xpose2) + ypose2;
						if(y < (float)n + (float)width / 2 && y > (float)n - (float)width / 2)
							OLED_WriteMap(1, i, n);
					}
		}
		else	//从左向右画线
		{
			for(i = xpose1; i < xpose2; i++)
				for(n = ypose1 - width / 2; n <= ypose1 + width / 2; n++)
					if(i <128 &&  n < 64)
						OLED_WriteMap(1, i, n);
		}
	}
	else if(xpose1 > xpose2)
	{
		if(ypose1 < ypose2)//从右下向左上画线
		{
			k = (float)(ypose1 - ypose2) / (float)(xpose1 - xpose2);
			for(i = xpose1; i > xpose2; i--)
				for(n = ypose1; n < ypose2; n++)
					if(i <128 &&  n < 64)
					{
						y = k * (i - xpose2) + ypose2;
						if(y < (float)n + (float)width / 2 && y > (float)n - (float)width / 2)
							OLED_WriteMap(1, i, n);
					}
		}
		else if(ypose1 > ypose2)//从右上向左下画线
		{
			k = (float)(ypose1 - ypose2) / (float)(xpose1 - xpose2);
			for(i = xpose1; i > xpose2; i--)
				for(n = ypose1; n > ypose2; n--)
					if(i <128 &&  n < 64)
					{
						y = k * (i - xpose2) + ypose2;
						if(y < (float)n + (float)width / 2 && y > (float)n - (float)width / 2)
							OLED_WriteMap(1, i, n);
					}
		}
		else	//从右向左画线
		{
			for(i = xpose1; i > xpose2; i--)
				for(n = ypose1 - width / 2; n <= ypose1 + width / 2; n++)
					if(i <128 &&  n < 64)
						OLED_WriteMap(1, i, n);
		}
	}
	else
	{
		if(ypose1 > ypose2)	//从下向上画线
		{
			for(i = xpose1 - width / 2; i <= xpose1 + width / 2; i++)
				for(n = ypose1; n > ypose2; n--)
					if(i <128 &&  n < 64)
						OLED_WriteMap(1, i, n);
		}
		else if(ypose1 < ypose2)	//从上向下画线
		{
			for(i = xpose1 - width / 2; i <= xpose1 + width / 2; i++)
				for(n = ypose1; n < ypose2; n++)
					if(i <128 &&  n < 64)
						OLED_WriteMap(1, i, n);
		}
		else	//画点
			OLED_rectangle(xpose1 - width / 2, ypose1 - width / 2, width,width);
	}
}
//绘制直线
void OLED_DrawLine(int x, int y, float ang, u8 length, u8 width)
{
	u8 i = 0;
	float n = 0;
	float xpose = 0, ypose = 0;
	float cosarg = MYMATH_Cos(ang), sinarg = MYMATH_Sin(ang);
	for(i = 0; i < length; i++)
	{
		for(n = -width/2.0; n < width/2.0; n++)
		{
			xpose = x + cosarg * i - sinarg * n;
			ypose = y + sinarg * i + cosarg * n;
			OLED_WriteMap(1, (int)xpose, (int)ypose);
		}
	}
}
//绘制圆形
void OLED_circle(int xpose, int ypose, u8 r)
{
	int i = 0, n = 0;
	for(i = xpose - r; i <= xpose + r; i++)
	{
		for(n = ypose - r; n <= ypose + r; n++)
		{
			if((i - xpose) * (i - xpose) + (n - ypose) * (n - ypose) <= r * r + 1)
				OLED_WriteMap(1, i, n);
		}
	}
}
//绘制圆形不填充
void OLED_circleunfilled(int xpose, int ypose, u8 r, u8 width)
{
	int i = 0, n = 0;
	width /= 2;
	for(i = xpose - r - width; i <= xpose + r + width; i++)
	{
		for(n = ypose - r - width; n <= ypose + r + width; n++)
		{
			if((i - xpose) * (i - xpose) + (n - ypose) * (n - ypose) <= (r + width) * (r + width) + 1
					&& (i - xpose) * (i - xpose) + (n - ypose) * (n - ypose) >= (r - width) * (r - width) - 1)
				OLED_WriteMap(1, i, n);
		}
	}
}
//绘制矩形
void OLED_rectangle(int xpose, int ypose, u8 width, u8 height)
{
	int i = 0, n = 0;
	for(i = xpose; i <= xpose + width; i++)
	{
		for(n = ypose; n <= ypose + height; n++)
		{
			OLED_WriteMap(1, i, n);
		}
	}
}
//绘制矩形不填充
void OLED_rectangleunfilled(int xpose, int ypose, u8 width, u8 height, u8 linewidth)
{
	int i = 0, n = 0;
	linewidth /= 2;
	for(i = xpose - linewidth; i <= xpose + width + linewidth; i++)
	{
		for(n = ypose - linewidth; n <= ypose + height + linewidth; n++)
		{
			if((i < xpose + linewidth + 1 || i >= xpose + width - linewidth)
					|| (n < ypose + linewidth + 1 || n >= ypose + height - linewidth))
				OLED_WriteMap(1, i, n);
		}
	}
}
/*
//绘制平行四边形
//最左边(最上边(先左后上))的顶点为1,四个顶点标号依次为顺时针方向1,2,3,4
//void OLED_rectangle(u8 xpose1, u8 ypose1, u8 xpose2, u8 ypose2,u8 xpose3, u8 ypose3)
//{
	
//}
//绘制三角形
//void OLED_triangle(u8 xpose1, u8 ypose1, u8 xpose2, u8 ypose2,u8 xpose3, u8 ypose3)
//{
	
//}
*/
/***************************************************
//OLED_ShowBMP: 显示位图
//x0、y0、x1、y1:分别为左上角坐标右下角坐标
//BMP :图片
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void OLED_ShowBMP(int x0, int y0, int x1, int y1, const u8* BMP, u8 mode)
{
	u32  j = 0;
	u8 index = 0;
	u8 temp = 0;
	int i = 0, n = 0;
	int o = 0, p = 0;
	float xpose, ypose;
	
	for(i = x0; i < x1; i++)
	{
    for(n = y0; n < y1; n += 8)
	  {
			if(mode&L2R)	o = x1 + x0 - i;
			else o = i;
			for(index = 0; index < 8 && index < y1-n; index++)
			{
				if(mode&U2D)	p = y1 + y0 - n - index;
				else p = n+index;
				temp = (BMP[j]<<index)&0x80;
				if((mode&Transparent) && temp == 0) continue;	//透明贴图
				if(mode&Reverse)			temp = !temp;						//反色
				if((mode&Span) || (mode&SpanSingle))					//旋转
				{
					xpose = x0 + spanSet.argCos * (o - x0) - spanSet.argSin * (p - y0);
					ypose = y0 + spanSet.argSin * (o - x0) + spanSet.argCos * (p - y0);
					OLED_WriteMap(temp, (int)xpose, (int)ypose);
				}
				else OLED_WriteMap(temp,o,p);
			}
			j++;
	  }
	}
}
/*
//原BMP贴图函数
void OLED_ShowBMP(int x0, int y0, int x1, int y1, const u8* BMP, u8 mode)
{
	u32  j = 0;
	u8 index = 0;
	u8 temp = 0;
	int i = 0, n = 0;
	int o = 0, p = 0;
	float xpose, ypose;
	
	for(n = y0; n < y1; n += 8)
	{
    for(i = x0; i < x1; i++)
	  {
			if(mode&L2R)	o = x1 + x0 - i;
			else o = i;
			for(index = 0; index < 8 && index < y1-n; index++)
			{
				if(mode&U2D)	p = y1 + y0 - n - index;
				else p = n+index;
				temp = (BMP[j]>>index)&0x01;
				if((mode&Transparent) && temp == 0) continue;	//透明贴图
				if(mode&Reverse)			temp = !temp;						//反色
				if((mode&Span) || (mode&SpanSingle))					//旋转
				{
					xpose = x0 + spanSet.argCos * (o - x0) - spanSet.argSin * (p - y0);
					ypose = y0 + spanSet.argSin * (o - x0) + spanSet.argCos * (p - y0);
					OLED_WriteMap(temp, (int)xpose, (int)ypose);
				}
				else OLED_WriteMap(temp,o,p);
			}
			j++;
	  }
	}
}
*/
/***************************************************
//OLED_ShowPNG: 显示透明贴图
//x0、y0、x1、y1:分别为左上角坐标右下角坐标
//Mask:图片掩码（决定在原图中抠出的形状）
//PNG :图片
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void OLED_ShowPNG(int x0, int y0, int x1, int y1, const u8* Mask, const u8* PNG, u8 mode)
{
	u32  j = 0;
	u8 index = 0;
	u8 temp = 0;
	int i = 0, n = 0;
	int o = 0, p = 0;
	float xpose, ypose;
	
	for(i = x0; i < x1; i++)
	{
    for(n = y0; n < y1; n += 8)
	  {
			if(mode&L2R)	o = x1 + x0 - i;
			else o = i;
			for(index = 0; index < 8 && index < y1-n; index++)
			{
				if(mode&U2D)	p = y1 + y0 - n - index;
				else p = n+index;
				temp = (PNG[j]<<index)&0x80;
				if(((Mask[j]<<index)&0x80) == 0)			continue;	//判断掩码值
				if((mode&Transparent) && temp == 0) continue;	//透明贴图
				if(mode&Reverse)			temp = !temp;						//反色
				if((mode&Span) || (mode&SpanSingle))					//旋转
				{
					xpose = x0 + spanSet.argCos * (o - x0) - spanSet.argSin * (p - y0);
					ypose = y0 + spanSet.argSin * (o - x0) + spanSet.argCos * (p - y0);
					OLED_WriteMap(temp, (int)xpose, (int)ypose);
				}
				else OLED_WriteMap(temp,o,p);
			}
			j++;
	  }
	}
}
/*
void OLED_ShowPNG(int x0, int y0, int x1, int y1, const u8* Mask, const u8* PNG, u8 mode)
{
	u32  j = 0;
	u8 index = 0;
	u8 temp = 0;
	int i = 0, n = 0;
	int o = 0, p = 0;
	float xpose, ypose;
	
	for(n = y0; n < y1; n += 8)
	{
    for(i = x0; i < x1; i++)
	  {
			if(mode&L2R)	o = x1 + x0 - i;
			else o = i;
			for(index = 0; index < 8 && index < y1-n; index++)
			{
				if(mode&U2D)	p = y1 + y0 - n - index;
				else p = n+index;
				temp = (PNG[j]>>index)&0x01;
				if(((Mask[j]>>index)&0x01) == 0)			continue;	//判断掩码值
				if((mode&Transparent) && temp == 0) continue;	//透明贴图
				if(mode&Reverse)			temp = !temp;						//反色
				if((mode&Span) || (mode&SpanSingle))					//旋转
				{
					xpose = x0 + spanSet.argCos * (o - x0) - spanSet.argSin * (p - y0);
					ypose = y0 + spanSet.argSin * (o - x0) + spanSet.argCos * (p - y0);
					OLED_WriteMap(temp, (int)xpose, (int)ypose);
				}
				else OLED_WriteMap(temp,o,p);
			}
			j++;
	  }
	}
}
*/
/***************************************************
//初始化SSD1306
***************************************************/
#include "bmp.h"
void OLED_Init(void)
{
	MYIIC_InitGPIO();
	
  delay_ms(800);

  OLED_WR_Byte(0xAE,OLED_CMD);//关闭显示
	OLED_WR_Byte(0x00,OLED_CMD);//设置低列地址
	OLED_WR_Byte(0x10,OLED_CMD);//设置高列地址
	OLED_WR_Byte(0x40,OLED_CMD);//设置起始行地址  
	OLED_WR_Byte(0xB0,OLED_CMD);//设置页面地址
	OLED_WR_Byte(0x81,OLED_CMD);//控制管理
	OLED_WR_Byte(0xFF,OLED_CMD);//128
	OLED_WR_Byte(0xA1,OLED_CMD);//设置部分重映射
	OLED_WR_Byte(0xA6,OLED_CMD);//正常 / 镜像
	OLED_WR_Byte(0xA8,OLED_CMD);//设置多路复用率(1 to 64)
	OLED_WR_Byte(0x3F,OLED_CMD);//1/32 功率
	OLED_WR_Byte(0xC8,OLED_CMD);//Com扫描方向
	OLED_WR_Byte(0xD3,OLED_CMD);//设置显示offset
	OLED_WR_Byte(0x00,OLED_CMD);//
	
	OLED_WR_Byte(0xD5,OLED_CMD);//设置osc模块
	OLED_WR_Byte(0x80,OLED_CMD);//
	
	OLED_WR_Byte(0xD8,OLED_CMD);//将区域颜色模式设置为关闭
	OLED_WR_Byte(0x05,OLED_CMD);//
	
	OLED_WR_Byte(0xD9,OLED_CMD);//设置切换时间
	OLED_WR_Byte(0xF1,OLED_CMD);//
	
	OLED_WR_Byte(0xDA,OLED_CMD);//设置引脚配置
	OLED_WR_Byte(0x12,OLED_CMD);//
	
	OLED_WR_Byte(0xDB,OLED_CMD);//设置Vcomh
	OLED_WR_Byte(0x30,OLED_CMD);//
	
	OLED_WR_Byte(0x8D,OLED_CMD);//使能电荷泵
	OLED_WR_Byte(0x14,OLED_CMD);//
	
	OLED_WR_Byte(0xAF,OLED_CMD);//显示oled
	
	OLED_Clear();         //清屏
	OLED_ClearMap(0x00);
	OLED_SetSpanAng(0);
	OLED_ShowBMP(0, 0, 30, 32, PIG1, Normal);
	OLED_ShowMap();
}

extern Car_t Car;
void OledDataUpdate(void)
{
	OLED_ShowBinNum(25,0,Car.GraySensor->BinaryData,SIZE12,SpanSingle);						OLED_ShowString(0,0,"Gray",SIZE12,SpanSingle);
	OLED_ShowFloat(35,10,Car.HiPnuc->yaw,2,SIZE12,SpanSingle);										OLED_ShowString(0,10,"Yaw:",SIZE12,SpanSingle);
	OLED_ShowFloat(80,20,Car.Tasks->CircleCount,2,SIZE12,SpanSingle);								OLED_ShowString(0,20,"CircleCount:",SIZE12,SpanSingle);
	OLED_ShowFloat(70,30,Car.Tasks->Deltayaw,2,SIZE12,SpanSingle);								OLED_ShowString(0,30,"DeltaYAW:",SIZE12,SpanSingle);
	OLED_ShowFloat(35,40,Car.Motors->EncoderLeftFront->V,4,SIZE12,SpanSingle);		OLED_ShowString(0,40,"Lspeed:",SIZE12,SpanSingle);
	OLED_ShowFloat(35,50,Car.Motors->EncoderRightFront->V,4,SIZE12,SpanSingle);		OLED_ShowString(0,50,"Rspeed:",SIZE12,SpanSingle);
	OLED_ShowMap();
	OLED_ClearMap(0x00);
}