/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "board.h"
#include "IOI2C.h"
#include "oled.h"
#define Pi 3.14159265
uint8_t Way_Angle=2;
float Angle_Balance,Gyro_Balance,Gyro_Turn;
float Acceleration_Z;
void Get_Angle(uint8_t way);
int main(void)
{
    SYSCFG_DL_init();
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    MPU6050_initialize();
    DMP_Init();
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);

    OLED_init();
    OLED_clear();
    OLED_setCursor(0, 0); OLED_writeString("  MPU6050 Data  ");
    OLED_display();

    while (1) {
        Get_Angle(1);

        OLED_clear();
        OLED_setCursor(0, 0); OLED_writeString("  MPU6050 Data  ");
        OLED_setCursor(0, 2); OLED_printf("Pitch: %7.2f", Pitch);
        OLED_setCursor(0, 3); OLED_printf("Roll : %7.2f", Roll);
        OLED_setCursor(0, 4); OLED_printf("Yaw  : %7.2f", Yaw);
        OLED_display();

        printf("Pitch=%.2f Roll=%.2f Yaw=%.2f\r\n", Pitch, Roll, Yaw);
        delay_ms(50);
    }
}


/**************************************************************************
Function: Get angle
Input   : way��The algorithm of getting angle 1��DMP  2��kalman  3��Complementary filtering
Output  : none
�������ܣ���ȡ�Ƕ�
��ڲ�����way����ȡ�Ƕȵ��㷨 1��DMP  2�������� 3�������˲�
����  ֵ����
**************************************************************************/
void Get_Angle(uint8_t way)
{
    float gyro_x,gyro_y,accel_x,accel_y,accel_z;
    float Accel_Y,Accel_Z,Accel_X,Accel_Angle_x,Accel_Angle_y,Gyro_X,Gyro_Z,Gyro_Y;
    if(way==1)                           //DMP�Ķ�ȡ�����ݲɼ��ж϶�ȡ���ϸ���ѭʱ��Ҫ��
    {
        Read_DMP();                          //��ȡ���ٶȡ����ٶȡ����
        Angle_Balance=Pitch;                 //����ƽ�����,ǰ��Ϊ��������Ϊ��
        Gyro_Balance=gyro[0];              //����ƽ����ٶ�,ǰ��Ϊ��������Ϊ��
        Gyro_Turn=gyro[2];                 //����ת����ٶ�
        Acceleration_Z=accel[2];           //����Z����ٶȼ�
    }
    else
    {
        Gyro_X=(I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_XOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_XOUT_L);    //��ȡX��������
        Gyro_Y=(I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_YOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_YOUT_L);    //��ȡY��������
        Gyro_Z=(I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_ZOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_ZOUT_L);    //��ȡZ��������
        Accel_X=(I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_XOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_XOUT_L); //��ȡX����ٶȼ�
        Accel_Y=(I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_YOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_YOUT_L); //��ȡX����ٶȼ�
        Accel_Z=(I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_ZOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_ZOUT_L); //��ȡZ����ٶȼ�
        if(Gyro_X>32768)  Gyro_X-=65536;                 //��������ת��  Ҳ��ͨ��shortǿ������ת��
        if(Gyro_Y>32768)  Gyro_Y-=65536;                 //��������ת��  Ҳ��ͨ��shortǿ������ת��
        if(Gyro_Z>32768)  Gyro_Z-=65536;                 //��������ת��
        if(Accel_X>32768) Accel_X-=65536;                //��������ת��
        if(Accel_Y>32768) Accel_Y-=65536;                //��������ת��
        if(Accel_Z>32768) Accel_Z-=65536;                //��������ת��
        Gyro_Balance=-Gyro_X;                            //����ƽ����ٶ�
        Accel_Angle_x=atan2(Accel_Y,Accel_Z)*180/Pi;     //������ǣ�ת����λΪ��
        Accel_Angle_y=atan2(Accel_X,Accel_Z)*180/Pi;     //������ǣ�ת����λΪ��
        accel_x=Accel_X/1671.84;
        accel_y=Accel_Y/1671.84;
        accel_z=Accel_Z/1671.84;
        gyro_x=Gyro_X/16.4;                              //����������ת��
        gyro_y=Gyro_Y/16.4;                              //����������ת��
        if(Way_Angle==2)
        {
             Pitch= -Kalman_Filter_x(Accel_Angle_x,gyro_x);//�������˲�
             Roll = -Kalman_Filter_y(Accel_Angle_y,gyro_y);
        }
        else if(Way_Angle==3)
        {
             Pitch = -Complementary_Filter_x(Accel_Angle_x,gyro_x);//�����˲�
             Roll = -Complementary_Filter_y(Accel_Angle_y,gyro_y);
        }
        Angle_Balance=Pitch;                              //����ƽ�����
        Gyro_Turn=Gyro_Z;                                 //����ת����ٶ�
        Acceleration_Z=Accel_Z;                           //����Z����ٶȼ�
    }

}



