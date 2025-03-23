#include "stm32f10x.h"  // Device header
//#include "stdio.h"
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "string.h"
#include "MN316.h"
#include "DHT11.h"
#include "myRTC.h"

uint32_t AngleHL,AngleRL;
uint8_t  led,Temp;


int main()
{
	OLED_Init();
	Serial_Init();
	Tim_Init();
	myRTC_Init();
	

	
	NB_Init();
	nb_mqtt_con( "183.230.40.96", 1883, "test", "84QSXilFVc", "version=2018-10-31&res=products%2F84QSXilFVc%2Fdevices%2Ftest&et=2537256630&method=md5&sign=FBtMbCLP6Hjonj0TUZQlXA%3D%3D");
	nb_mqtt_sub(	DEMO_ONNET_repubtopic);
//	nb_mqtt_pub( DEMO_ONNET_pubtopic, Property_Init );						//固定参数上报			
	
	Set_sendData( AngleHL, AngleRL, led, Temp );									//可变参数上报
	nb_mqtt_pub( DEMO_ONNET_pubtopic, sendData);

	NB_GetTime();		//时间获取
//	nb_urc_set();
	myRTC_SetTime();
	
	while(1)
	{
//			DHT11_REC_Data();
			//nb_urc_set();
	//	NB_GetTime();
			//myRTC_ReadTime();
//		OLED_Printf(0, 0, OLED_8X16, "Time: %d:%d:%d",Hour,Minute,Second);
//	OLED_Update();
	}

}
