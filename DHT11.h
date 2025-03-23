#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f10x.h"                  // Device header



#define dht11_high GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define dht11_low GPIO_ResetBits(GPIOB, GPIO_Pin_12)
#define Read_Data GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)

extern unsigned int R_H,R_L,T_H,T_L;
extern unsigned char RH,RL,TH,TL,CHECK;

void DHT11_GPIO_Init_OUT(void);
void DHT11_GPIO_Init_IN(void);
void DHT11_Start(void);
unsigned char DHT11_REC_Byte(void);
void DHT11_REC_Data(void);





//#include "delay.h"
// 
///*****************辰哥单片机设计******************
//				    STM32
// * 文件			:	DHT11温度湿度传感器h文件                   
// * 版本			:   V1.0
// * 日期			:   2024.8.4
// * MCU			:	STM32F103C8T6
// * 接口			:	见代码							
// * BILIBILI	    :	辰哥单片机设计
// * CSDN			:	辰哥单片机设计
// * 作者			:	辰哥
//**********************BEGIN***********************
//***************根据自己需求更改****************/
////DHT11引脚宏定义
//#define DHT11_GPIO_PORT  GPIOB
//#define DHT11_GPIO_PIN   GPIO_Pin_12
//#define DHT11_GPIO_CLK   RCC_APB2Periph_GPIOB
///*********************END**********************/
// 
////输出状态定义
//#define OUT 1
//#define IN  0
// 
////控制DHT11引脚输出高低电平
//#define DHT11_Low  GPIO_ResetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)
//#define DHT11_High GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)
// 
//u8 DHT11_Init(void);//初始化DHT11
//u8 DHT11_Read_Data(u8 *temp,u8 *humi);//读取温湿度数据
//u8 DHT11_Read_Byte(void);//读取一个字节的数据
//u8 DHT11_Read_Bit(void);//读取一位的数据
//void DHT11_Mode(u8 mode);//DHT11引脚输出模式控制
//u8 DHT11_Check(void);//检测DHT11
//void DHT11_Rst(void);//复位DHT11   
// 



#endif

