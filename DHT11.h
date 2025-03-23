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
///*****************���絥Ƭ�����******************
//				    STM32
// * �ļ�			:	DHT11�¶�ʪ�ȴ�����h�ļ�                   
// * �汾			:   V1.0
// * ����			:   2024.8.4
// * MCU			:	STM32F103C8T6
// * �ӿ�			:	������							
// * BILIBILI	    :	���絥Ƭ�����
// * CSDN			:	���絥Ƭ�����
// * ����			:	����
//**********************BEGIN***********************
//***************�����Լ��������****************/
////DHT11���ź궨��
//#define DHT11_GPIO_PORT  GPIOB
//#define DHT11_GPIO_PIN   GPIO_Pin_12
//#define DHT11_GPIO_CLK   RCC_APB2Periph_GPIOB
///*********************END**********************/
// 
////���״̬����
//#define OUT 1
//#define IN  0
// 
////����DHT11��������ߵ͵�ƽ
//#define DHT11_Low  GPIO_ResetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)
//#define DHT11_High GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)
// 
//u8 DHT11_Init(void);//��ʼ��DHT11
//u8 DHT11_Read_Data(u8 *temp,u8 *humi);//��ȡ��ʪ������
//u8 DHT11_Read_Byte(void);//��ȡһ���ֽڵ�����
//u8 DHT11_Read_Bit(void);//��ȡһλ������
//void DHT11_Mode(u8 mode);//DHT11�������ģʽ����
//u8 DHT11_Check(void);//���DHT11
//void DHT11_Rst(void);//��λDHT11   
// 



#endif

