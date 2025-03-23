#ifndef __MYRTC_H
#define __MYRTC_H

#include "stdint.h"

typedef union
{
	uint32_t TimeArray[6];
	struct
	{
		uint32_t Year;
    uint32_t Month;
    uint32_t Day;
    uint32_t Hour;
    uint32_t Minute;
    uint32_t Second;  
	}my_time;
}TimeTypeDef;

extern TimeTypeDef MyRTC_Time;

void myRTC_Init(void);
void myRTC_SetTime(void);
void myRTC_ReadTime(void);

#endif
