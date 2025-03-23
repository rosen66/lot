#include "stm32f10x.h"                  // Device header
#include "Time.h"
#include "myRTC.h"
#include "OLED.h"

//uint32_t myRTC_time[] = {2025,2,23,15,42,55};



TimeTypeDef MyRTC_Time;

void myRTC_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP,ENABLE);
	
	PWR_BackupAccessCmd(ENABLE);
	
	RCC_LSEConfig(RCC_LSE_ON);
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);
	
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	RCC_RTCCLKCmd(ENABLE);
	
	RTC_WaitForSynchro();
	RTC_WaitForLastTask();
	
	RTC_SetPrescaler(32768 - 1);
	RTC_WaitForLastTask();
	
	RTC_SetCounter(1720856100);
	RTC_WaitForLastTask();
}

void myRTC_SetTime(void)
{
	time_t time_cnt;
	struct tm time_date;
	
	time_date.tm_year = MyRTC_Time.TimeArray[0] - 1900;
	time_date.tm_mon = MyRTC_Time.TimeArray[1] - 1;
	time_date.tm_mday = MyRTC_Time.TimeArray[2];
	time_date.tm_hour = MyRTC_Time.TimeArray[3];
	time_date.tm_min = MyRTC_Time.TimeArray[4];
	time_date.tm_sec = MyRTC_Time.TimeArray[5];
	
	time_cnt = mktime(&time_date);//转北京时区-8*60*60
	
	RTC_SetCounter(time_cnt);
	RTC_WaitForLastTask();
}

void myRTC_ReadTime(void)
{
	time_t time_cnt;
	struct tm time_date;
	
	time_cnt = RTC_GetCounter();//转北京时区+8*60*60
	
	time_date = *localtime(&time_cnt);
	
	MyRTC_Time.TimeArray[0] = time_date.tm_year + 1900;
	MyRTC_Time.TimeArray[1] = time_date.tm_mon + 1;
	MyRTC_Time.TimeArray[2] = time_date.tm_mday;
	MyRTC_Time.TimeArray[3] = time_date.tm_hour;
	MyRTC_Time.TimeArray[4] = time_date.tm_min;
	MyRTC_Time.TimeArray[5] = time_date.tm_sec;
	
	OLED_Printf(0, 0, OLED_8X16, "Time: %d:%d:%d",MyRTC_Time.my_time.Hour,MyRTC_Time.my_time.Minute,MyRTC_Time.my_time.Second);
	OLED_Update();
}
