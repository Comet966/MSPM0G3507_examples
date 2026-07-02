#ifndef _OLED_H_
#define _OLED_H_

#include "headfile.h"
#include "stdlib.h"
#include "IIC.h"

//字号
#define SIZE16	16
#define SIZE12	12

#define Add0				1	//不足位补0
#define NoAdd0			0	//不足位不补0

//图形字符显示模式
#define Normal						0x00	//正常显示
#define Transparent				0x01	//显示透明背景
#define Reverse						0x02	//颜色反转显示
#define L2R								0x04	//左右翻转
#define U2D								0x08	//上下翻转
#define Span							0x10	//一个句子中单个字符旋转
#define SpanSingle				0x20	//一个句子作为整体旋转

#define Max_Column	128     //最大列数
#define Max_Row			64      //最大行数
#define	Brightness	0xFF    //灰度（亮度）
#define X_WIDTH 		128     //列数
#define Y_WIDTH 		64	    //行数
#define Y_PAGE			8				//页数

//OLED指令
#define OLED_CMD  MYIIC_CMD		//写命令0
#define OLED_DATA MYIIC_DATA	//写数据1

//点
typedef struct{
	float x;
	float y;
}	POINT;

//IIC驱动函数
void OLED_Write_IIC_Command(u8 IIC_Command);
void OLED_Write_IIC_Data(u8 IIC_Data);
void OLED_WR_Byte(u8 dat,u8 cmd);           //写入数据
//OLED驱动函数
void OLED_SetSpanAng(float ang);												//设置旋转角度
void OLED_SetSpan(float x, float y);										//设置字旋转中心
void OLED_SetSpanZero(float x, float y);								//设置句旋转中心
POINT OLED_SetSpanCenter(float xBef, float yBef, float xSet, float ySet);	//设置旋转中心，(原贴图坐标，设置的旋转中心坐标)返回设置后的贴图坐标(贴图左上角坐标)
void OLED_WriteMap(u8 bit, int xpose, int ypose);				//向Map中写入数据
void OLED_ShowMap(void);																//Map载入显存
void OLED_ClearMap(u8 data);														//清除Map数据,全部替换为data
u8 	 reversedata(u8 data);															//二进制数反转
void OLED_Set_Pos(u8 x, u8 y);													//坐标设置
void OLED_Display_On(void);                             //开启OLED显示  
void OLED_Display_Off(void);	   												//关闭OLED显示  
void OLED_Init(void);                                   //初始化SSD1306
void OLED_Clear(void);                                  //清屏函数
void OLED_Clear_page(u8 i);                             //清页函数,清除第i(1<=i<=8)页
//显示函数
//void OLED_DrawPoint(u8 x,u8 y,u8 t);
//void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
void OLED_Reverse(int x, int y, u8 width, u8 height);													//将选择的区域反显
void OLED_ShowChar(int x, int y, u8 ch, u8 Size, u8 mode);      							//显示字符(x坐标,y坐标,ch字符,size字体大小,mode显示模式)
void OLED_ShowNum(int x,int y,u32 num, u8 length, u8 Size, u8 if0, u8 mode);	//显示数字(x坐标,y坐标,num数字,size字体大小,mode显示模式)
void OLED_ShowInt(int x, int y, int num, u8 size, u8 mode);										//显示整型数(x坐标,y坐标,num数字,size字体大小,mode显示模式)
void OLED_ShowBinNum(int x, int y, int num, u8 size, u8 mode);
void OLED_ShowFloat(int x, int y, float num, u8 len, u8 size, u8 mode);				//显示浮点数(x坐标,y坐标,num数字,len保留小数位数,size字体大小,mode显示模式)
void OLED_ShowString(int x,int y, u8 *ch,u8 Size, u8 mode);										//显示字符号串(x坐标,y坐标,ch字符指针,size字体大小,mode显示模式)
void OLED_ShowChinese(int x, int y, u8 num, u8 mode);                 				//显示汉字(x坐标,y坐标,num汉字编号,mode显示模式)
void OLED_ShowChineseStr(int x, int y, u8* nums, u8 len, u8 mode);						//显示一句话(x坐标,y坐标,nums汉字编号数组,mode显示模式)
void OLED_ShowBMP(int x0, int y0, int x1, int y1, const u8* BMP, u8 mode);		//显示位图(x0y0左上角坐标,x1y1右下角坐标,BMP图片数组,mode显示模式)
void OLED_ShowPNG(int x0, int y0, int x1, int y1, const u8* Mask, const u8* PNG, u8 mode);	//显示贴图(x0y0左上角坐标,x1y1右下角坐标,Mask图片掩码,PNG图片数组,mode显示模式)
//绘图函数
void OLED_line(int xpose1, int ypose1, int xpose2, int ypose2, u8 width);	//绘制直线(起点坐标,终点坐标,线宽)
void OLED_DrawLine(int x, int y, float ang, u8 length, u8 width);			//绘制直线(起点坐标,角度,长度,线宽)
void OLED_circle(int xpose, int ypose, u8 r);													//绘制圆形(圆心坐标,半径)
void OLED_circleunfilled(int xpose, int ypose, u8 r, u8 width);				//绘制圆形不填充(圆心坐标,半径,线宽)
void OLED_rectangle(int xpose, int ypose, u8 width, u8 height);				//绘制矩形(起点坐标,宽高)
void OLED_rectangleunfilled(int xpose, int ypose, u8 width, u8 height, u8 linewidth);		//绘制矩形不填充(起点坐标,宽高,线宽)
//void OLED_rectangle(u8 xpose1, u8 ypose1, u8 xpose2, u8 ypose2,u8 xpose3, u8 ypose3);	//绘制矩形
//void OLED_triangle(u8 xpose1, u8 ypose1, u8 xpose2, u8 ypose2,u8 xpose3, u8 ypose3);	//绘制三角形
void OledDataUpdate(void);

#endif
