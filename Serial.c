#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "MN316.h"

//char Serial_RxPacket[100];
//uint8_t Serial_RxFlag;

uint16_t             uartxRxstate    = 0;										//串口读取状态
uint16_t             uartxTxstate    = 0;										//串口发送状态
uint8_t              uartxRxBuf[512];												//串口读取缓冲区
uint8_t              uartxTxBuf[512];												//串口发送缓冲区

void Serial_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( GPIOB, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode =USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity =USART_Parity_No;
	USART_InitStructure.USART_StopBits =USART_StopBits_1;
	USART_InitStructure.USART_WordLength =USART_WordLength_8b;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);											//使能USART3的USART_IT_RXNE中断
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_Initstructure;
	NVIC_Initstructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_Initstructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Initstructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Initstructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_Initstructure);
	
	
	USART_Cmd(USART3,ENABLE);
	
}

void Tim_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	//5ms 定时
	TIM_InternalClockConfig(TIM3);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 50000-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720-1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;			//抢占优先级可处理串口
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;						//响应优先级
	NVIC_Init(&NVIC_InitStructure);
}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART3,Byte);
	while(USART_GetFlagStatus(USART3,USART_FLAG_TXE) == RESET);//等待字节发送完成
}

void Serial_SendArray(uint8_t *Array,uint16_t Length)
{
	uint16_t i;
	for(i = 0;i < Length; i ++)
	{
		Serial_SendByte(Array[i]);
	}
}

void Serial_SendString(char *String)
{
	uint8_t i;
	for(i = 0; String[i] !='\0'; i ++)
	{
		Serial_SendByte(String[i]);
	}
}

uint32_t Serial_Pow(uint32_t x, uint32_t y)
{
	uint32_t Result = 1;
	while(y --)
	{
		Result *=x;
	}
	return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for(i = 0;i < Length; i ++)
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
	}
}

int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}

void Serial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial_SendString(String);
}

void USART3_IRQHandler(void)
{
	if(USART_GetFlagStatus(USART3,USART_IT_RXNE) == SET)
	{
		uint8_t RxData = USART_ReceiveData(USART3);
		if ( (uartxRxstate & UARTX_RECV_END) == 0 )			//读状态为0时，可接收状态
            {
							if ( uartxRxstate == 0x00 )						//读状态为0和接收缓冲区数据为0
                {
									memset( (uint8_t *) uartxRxBuf, 0x00, sizeof(uartxRxBuf) );//清空接收缓冲区
                }
                if ( uartxRxstate < sizeof(uartxRxBuf) )//判断是否超出接收缓冲区
                {
									TIM2->CNT = 0;                       	//定时器重装载
									/*（定时器为3ms，即接收3毫秒时长的数据）波特率为9600，接收一个字节约为0.84ms*/
                    TIM_Cmd(TIM3, ENABLE);								//使能定时器		
                    uartxRxBuf[uartxRxstate++] = RxData;  //开始接收数据     
                }
                else
                {
									uartxRxstate |= UARTX_RECV_END;  	//读状态置1，禁止接收状态      
                }
								USART_ClearITPendingBit(USART3,USART_IT_RXNE);
						}
	}
}
	
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		if ( (uartxRxstate & 0x7FFF) != 0 )							//判断接收缓冲区是否有数据
    {				
			if ( uartxTxstate == 1 )											//有数据发送
         uartxRxstate |= UARTX_RECV_END; 						//读状态置1，禁止接收状态  
      else if ( uartxTxstate == 0 )									//无数据发送
      {  
         NBAT_checkUrc( uartxRxBuf, uartxRxstate );	//检测URC数据，并处理
      }
    }
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);			//清除定时器标志位
		TIM_Cmd(TIM3, DISABLE);													//关闭定时器
	}
}

