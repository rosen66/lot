#include "stm32f10x.h"                  // Device header
#include "Serial.h"
#include "OLED.h"
#include "string.h"
#include "MN316.h"
#include "Delay.h"
#include "myRTC.h"

int Day;
int Month;
int Year;
int Second;
int Minute;
int Hour;

char    rbuf[AT_DATA_LEN];
char    wbuf[AT_DATA_LEN];

char tmpbuf[AT_DATA_LEN] = { 0 };

char    sendData[256];

//static uint8_t              aRxBuffer[128];													//AT指令读取缓冲区
static uint8_t              gUrcTableSize;
static const struct at_urc  *gUrcTable;

//typedef union
//{
//	uint8_t TimeArray[6];
//	struct
//	{
//		uint8_t Day;
//    uint8_t Month;
//    uint8_t Year;
//    uint8_t Second;
//    uint8_t Minute;
//    uint8_t Hour;  
//	}my_time;
//}TimeTypeDef;

//TimeTypeDef MyRTC_Time;

uint32_t g_socketRecvLen = 0;

uint8_t g_connetOkFlage         = 0;
uint8_t g_mqttConnetOkFlage     = 0;


at_task at;

char* NBAT_checkCmd( char *str )
{
    char *strx = 0;
    if ( uartxRxstate & UARTX_RECV_END )
    {                                               
        uartxRxBuf[uartxRxstate & 0x7FFF] = 0;   //设置结束标志，strstr() 要求输入的字符串必须以 '\0' 结尾
        strx = (char *) strstr( (const char *) uartxRxBuf, (const char *) str );//在接收的数据里面找字符串，并返回字符串的首地址
    }
    return (strx);
}

uint8_t NBAT_checkUrc( uint8_t *outResponse, uint32_t len )
{
    uint8_t idx;
    uint8_t *strPtr     = outResponse;
    uint8_t strLen      = 0;
    char    * strStart  = NULL;
    char    * strEnd    = NULL;
	
    for ( idx = 0; idx < gUrcTableSize; idx++ )
    {
        strStart = strstr( (char *) strPtr, gUrcTable[idx].cmd_prefix );
        if ( strStart != 0 )
        {
            strLen  = strlen( gUrcTable[idx].cmd_prefix );
            strEnd  = strstr( (char *) strPtr + strLen, gUrcTable[idx].cmd_suffix );
            strLen  = strEnd - strStart + strlen( gUrcTable[idx].cmd_suffix );
            if ( strEnd != 0 )
                gUrcTable[idx].func( (const char *) strPtr, strLen );
            strPtr += strLen;
        }
    }
    uartxRxstate = 0;   //检查完URC后清零
    return (1);
}

uint8_t NBAT_sendCmd( char *cmd, char *ack, uint32_t waittime, uint8_t *outResponse )
{
    uint8_t     res     = 0;
    uint16_t    sendLen = 0;
    if ( strlen( cmd ) > sizeof(uartxTxBuf) )
    {
        OLED_ShowString( 0, 0, cmd ,OLED_8X16);
				OLED_Update();
        return (2);
    }
		//不为0表示有URC数据接收
    while ( uartxRxstate != 0 )
        ;
    uartxTxstate = 1;                                           

    sendLen = sprintf( (char *) uartxTxBuf, "%s", cmd );				//将cmd命令字符存入发送缓冲区中，返回值为存入的数据长度
    Serial_SendArray( uartxTxBuf, sendLen );           					//发送命令	              
    Serial_Printf(AT_NB_LINE_END); 															//发送结束符号
		OLED_ShowString( 0, 0, "SE:" ,OLED_8X16);
		OLED_ShowString( 24, 0, cmd ,OLED_8X16);
		OLED_Update();

    waittime *= 10 * 100;
    if ( ack && waittime )                                      
    {
        while ( --waittime )                                    
        {
            Delay_us( 10 );
            if ( uartxRxstate & UARTX_RECV_END )                
            {
                if ( NBAT_checkCmd( ack ) )
                {
                    if ( outResponse != NULL )
                    {
                        memcpy( outResponse, uartxRxBuf, uartxRxstate & 0x7FFF );
                        outResponse[uartxRxstate & 0x7FFF] = 0;
                    }
                    uartxRxstate = 0;
                    break;
                }
            }
        }
        if ( waittime == 0 )
            res = 1;						//指令接收失败
    }
    uartxRxstate    = 0;
    uartxTxstate    = 0;
    return (res);
}

int32_t at_cmd( int8_t *cmd, int32_t len, const char *suffix, char *resp_buf, int* resp_len )
{
    int         result;
    result = NBAT_sendCmd( (char *) cmd, (char *) suffix, 100, (uint8_t *) resp_buf );
    return (result);
}

int NBAT_sendBuf( char *buf, char *ack, uint32_t waittime, uint8_t *outResponse )
{
    uint8_t     res     = 0;
    uint16_t    sendLen = 0;
    if ( strlen( buf ) > sizeof(uartxTxBuf) )
    {
        printf( "NBAT_sendCmd Len is %d\r\n", strlen( buf ) );
        return (2);
    }
		//不为0表示有URC数据接收
    while ( uartxRxstate != 0 )
        ; 
    uartxTxstate = 1;			//发送标致位置位
		
    sendLen = sprintf( (char *) uartxTxBuf, "%s", buf );
    Serial_SendArray( uartxTxBuf, sendLen );

    waittime *= 10 * 100;
    if ( ack && waittime )
    {
        while ( --waittime )
        {
            Delay_us( 10 );
            if ( uartxRxstate & UARTX_RECV_END )
            {
                if ( NBAT_checkCmd( ack ) )
                {
                    if ( outResponse != NULL )
                    {
                        memcpy( outResponse, uartxRxBuf, uartxRxstate & 0x7FFF );
                        outResponse[uartxRxstate & 0x7FFF] = 0;
                    }
                    //得到有效数据
                    uartxRxstate = 0;
                    break;
                }
            }
        }
        if ( waittime == 0 )
            res = 1;
    }
    uartxRxstate    = 0;
    uartxTxstate    = 0;
    return (res);
}

int32_t at_buf( int8_t *buf, int32_t len, const char *suffix, char *resp_buf, int* resp_len )
{
    uint32_t    recvLen = 0;
    int         result;
    result = NBAT_sendBuf( (char *) buf, (char *) suffix, 100, (uint8_t *) resp_buf );
    return (result);
}


uint8_t NBAT_recvData( char *ack, uint16_t waittime, uint8_t *outResponse )
{
    uint8_t res = 0;
    if ( (uartxRxstate & UARTX_RECV_END) == 0 )
        return (2);
    if ( ack && waittime )      //等待应答                        
    {
        //等待倒计时
        do
        {
            Delay_ms( 10 );
            if ( uartxRxstate & UARTX_RECV_END )     //接收到期待的应答结果   
            {
                if ( NBAT_checkCmd( ack ) )
                {
                    memcpy( outResponse, uartxRxBuf, uartxRxstate & 0x7FFF );
                    outResponse[uartxRxstate & 0x7FFF]  = 0;
                    uartxRxstate                        = 0;
                    break;                              //得到有效数据
                }
            }
        }
        while ( --waittime );
        if ( waittime == 0 )
            res = 1;
    }
    return (res);
}


int32_t NB_checkDevice( void ) 						//检测模块是否通电
{
		char    *cmd = "AT";
    int32_t res = 0;
    res = at.cmd( (int8_t *) cmd, strlen( cmd ), "OK", NULL, NULL );
    if ( res == 0 )
    {
        OLED_ShowString(0,48,"BOOT SUCCESS",OLED_8X16);
				OLED_UpdateArea(0,48,126,63);//第4行显示
    }
    return (res);
}

//查询IMEI号
int32_t NB_getCGSN()
{
    char    *cmd = "AT+CGSN=1";
    char    recvBuf[64];
    int     recvLen;
    int     ret;
    ret = at.cmd( (int8_t *) cmd, strlen( cmd ), "OK", recvBuf, &recvLen );
    if ( recvLen < 64 )
    {
        recvBuf[recvLen] = '\0';
				OLED_Printf(0, 17, OLED_8X16, "CGSN:%s", recvBuf);
        OLED_UpdateArea(0,17,126,33);//第2行显示
    }
    return (ret);
}

//查询信号
int32_t NB_checkCsq( void )
{
    char *cmd = "AT+CSQ";
		char    recvBuf[64];
    int     recvLen = 12;
		int     ret;
		//do{
    ret = at.cmd( (int8_t *) cmd, strlen( cmd ), "+CSQ:", recvBuf, &recvLen );
		if ( recvLen < 64 )
    {
        recvBuf[recvLen] = '\0';
				OLED_Printf(0, 32, OLED_8X16, "%d", recvBuf + 3);
        OLED_UpdateArea(0,32,126,47);//第2行显示
    }
	//}while( recvBuf[9] == "9");
		return (ret);
}

//查询特定PDP上下文编号
int32_t NB_CIMI( void )
{
    char *cmd = "AT+CGPADDR";
    return (at.cmd( (int8_t *) cmd, strlen( cmd ), "+CGPADDR:0,", NULL, NULL ) );
}

//查询PDP激活状态（即是否驻网成功）回复1表示驻网成功
int32_t NB_netstat( void )
{
		int32_t res = 0;
    char *cmd = "AT+CGATT?";
    res = at.cmd( (int8_t *) cmd, strlen( cmd ), "CGATT: 1", NULL, NULL );
		if ( res == 0 )
		{
				OLED_ShowString(0,48,"Networking SUCCESS",OLED_8X16);
				OLED_UpdateArea(0,48,126,63);//第4行显示
		}
		return (res);
}

//查询特定PDP上下文的IP地址
int32_t NB_queryIp( void )
{
    char *cmd = "AT+CGPADDR=0";
    return (at.cmd( (int8_t *) cmd, strlen( cmd ), "+CGPADDR", NULL, NULL ) );
}

//查询网络注册状态配置
int32_t NB_getCereg( void )
{
    char    *cmd = "AT+CEREG?";
    char    recvBuf[64];
    int     recvLen;
    int     ret;
    ret = at.cmd( (int8_t *) cmd, strlen( cmd ), "0,1", recvBuf, &recvLen );
    if ( recvLen < 64 )
    {
        recvBuf[recvLen] = '\0';
        printf( "[INFO]cereg:%s\r\n", recvBuf );
    }
    return (ret);
}

//获取时间
int32_t NB_GetTime( void )					
{
    char *cmd = "AT+CMNTP";
		char    recvBuf[42];
    int     recvLen = 42;
		int     ret;
    ret = at.cmd( (int8_t *) cmd, strlen( cmd ), "+CMNTP", recvBuf, &recvLen );
		if ( recvLen < 64 )
    {
        recvBuf[recvLen] = '\0';
				OLED_Printf(0, 16, OLED_8X16, "%s", recvBuf + 8);
        OLED_UpdateArea(0,16,126,31);//第2行显示
				sscanf( (char *) (recvBuf  + 8), "\r\n+CMNTP:0,\"%02d/%02d/%02d,%02d:%02d:%02d+32\"\r\n",&Year ,&Month ,&Day ,&Hour ,&Minute ,&Second);
				MyRTC_Time.my_time.Year = MyRTC_Time.my_time.Year + 2000;
    }
		return (ret);
}


void nb_urc_dataIoctl( const char *buf, unsigned long size )							//检测模块是否驻网成功，驻网成功获取时间，并检测是否连接到平台
{
    char    recvData[128];
    memset( recvData, 0, 128 );																								//将数组全部设置为0
	if ( strstr( (char *) buf, "+CMNTP" ) != NULL )														//在缓冲区查找是否出现“ME PDN ACT”字符串
    {
        sscanf( (char *) buf, "\r\n+CMNTP:0,\"%d/%d/%d,%d:%d:%d+32\"\r\n",MyRTC_Time.my_time.Year ,MyRTC_Time.my_time.Month ,MyRTC_Time.my_time.Day ,MyRTC_Time.my_time.Hour ,MyRTC_Time.my_time.Minute ,MyRTC_Time.my_time.Second);	//将数据存入变量中
    }
  else if ( strstr( (char *) buf, "+MQTTOPEN:OK" ) != NULL )									//在缓冲区查找是否出现“+MQTTOPEN:OK”字符串
    {
        g_connetOkFlage = 1;
    }
}

static struct at_urc    nb_urc_table[] = {
    { "\r\n+CMNTP", "\r\n", nb_urc_dataIoctl },
    { "\r\n+MQTTOPEN:OK:", "\r\n", nb_urc_dataIoctl }
};


void nb_mqtt_dataIoctl( const char *buf, unsigned long size )
{
    char    recvData[256];
//    int     recvLen;
//    int     socketId;
    printf( "[INFO]This is nb_mqtt_dataIoctl!\r\n" );
    memset( recvData, 0, 256 );
    memcpy( recvData, buf, size );

    if ( strstr( (char *) recvData, "+MQTTOPEN:OK" ) != NULL )
    {
        g_mqttConnetOkFlage = 1;
    }
    else if ( strstr( (char *) recvData, "+MQTTPUBLISH:" ) != NULL )
    {
        printf( "[INFO]MQTT RECV success!\r\n" );
        printf( "%s", recvData );
    }
}


static struct at_urc mqtt_urc_table[] = {
    { "+MQTTOPEN:",    "\r\n", nb_mqtt_dataIoctl },
    { "+MQTTPUBLISH:", "\r\n", nb_mqtt_dataIoctl },
};


void NB_Init(void)
{
	int32_t ret;										//返回值
	int32_t timecnt = 0;
	while ( 1 )											//等待上电检测
    {
        ret = NB_checkDevice();
        if ( ret == AT_OK )
            break;
        Delay_ms( 500 );
    }
	while ( timecnt < 120 )					//等待驻网检测
    {
        ret = NB_checkCsq();			//查询网络信号
				//ret |= NB_netstat();			//查询是否驻网，
		if ( ret == AT_OK )				//两个返回值同时为0，即驻网成功
        {	
						OLED_ClearArea(0,48,126,63);
						OLED_UpdateArea(0,48,126,63);//第4行更新
						OLED_ShowString(0,48,"Networking",OLED_8X16);
						OLED_UpdateArea(0,48,126,63);//第4行显示
            break;
        }
        Delay_ms( 500 );
        timecnt++;
    }
}

void Set_sendData(uint32_t angle,uint32_t direction,uint8_t led,uint8_t temperature )
{
	
	sprintf( sendData, "{\
\"id\":\"1225\", \
\"version\":\"1.0\",\
\"params\":{\
\"angle\":{\"value\":%d},\
\"direction\":{\"value\":%d},\
\"led\":{\"value\":%d},\
\"temperature\":{\"value\":%d}}\
}", angle, direction, led,temperature );
}

int32_t NB_MN316_mqttConnet( char *brokerAddress, uint16_t port,
                             char *clientID, char * userName, char * password )
{
    int     ret;
    char    buf[256];

    /*AT+MQTTCFG="183.230.40.96",1883,"test",60,"84QSXilFVc","version=2018-10-31&res=products%2F84QSXilFVc%2Fdevices%2Ftest&et=2537256630&method=md5&sign=FBtMbCLP6Hjonj0TUZQlXA%3D%3D",1,0 */
    if ( userName != NULL && password != NULL )
    {
			//将格式化的字符串写入缓冲区buf
        snprintf( buf, sizeof(buf), "AT+MQTTCFG=\"%s\",%d,\"%s\",60,\"%s\",\"%s\",1,0",
                  brokerAddress, port,
                  clientID, userName, password );
    }
    else
    {
        snprintf( buf, sizeof(buf), "AT+MQTTCFG=\"%s\",%d,\"%s\",60,\"\",\"\",1,0",
                  brokerAddress, port, clientID );
    }
    ret = at.cmd( (int8_t *) buf, strlen( buf ), "OK", NULL, NULL );
    if ( ret < 0 )
    {
				OLED_ShowString(0,16,"MQTTCON ERROR1",OLED_8X16);
				OLED_UpdateArea(0,16,126,31);//第2行显示
        return (ret);
    }

    if ( userName != NULL && password != NULL )
    {
        snprintf( buf, sizeof(buf), "AT+MQTTOPEN=1,1,0,0,0,\"\",\"\"" );
    }
    else
    {
        snprintf( buf, sizeof(buf), "AT+MQTTOPEN=0,0,0,0,0,\"\",\"\"" );
    }
    ret = at.cmd( (int8_t *) buf, strlen( buf ), "MQTTOPEN:OK", NULL, NULL );
    if ( ret < 0 )
    {
				OLED_ShowString(0,16,"MQTTCON ERROR2",OLED_8X16);
				OLED_UpdateArea(0,16,126,31);//第2行显示
        return (ret);
    }
    return (AT_OK);
}

int nb_mqtt_con( char * brokerAddress, uint16_t port, char *clientID, char *userName, char *password )
{
    int ret;

    ret = NB_MN316_mqttConnet( brokerAddress, port, clientID, userName, password );
    if ( ret == AT_OK )
		{
				OLED_ClearArea(0,16,126,31);
				OLED_UpdateArea(0,16,126,31);//第2行更新
				OLED_ShowString(0,16,"MQTTCON SUCCESS",OLED_8X16);
				OLED_UpdateArea(0,16,126,31);//第2行显示
		}
    return (ret);
}

int32_t NB_MN316_mqttSub( char* topic )
{
    int ret;

    snprintf( wbuf, sizeof(wbuf), "AT+MQTTSUB=\"%s\",0", topic );
    ret = at.cmd( (int8_t *) wbuf, strlen( wbuf ), "OK", NULL, NULL );
    if ( ret < 0 )
    {
        return (ret);
    }
    return (AT_OK);
}

int nb_mqtt_sub( char* topic )
{
    return (NB_MN316_mqttSub( topic ) );
}

int hex_to_hexStr( const char *bufin, int len, char *bufout )
{
    int i = 0;
    if ( NULL == bufin || len <= 0 || NULL == bufout )
    {
        return (-1);
    }
    for ( i = 0; i < len; i++ )
    {
        sprintf( bufout + i * 2, "%02X", bufin[i] );
    }
    return (0);
}

int32_t NB_MN316_mqttPub( char* topic,
                          char* msg,
                          uint32_t msgLen )
{
    int ret;

    hex_to_hexStr( msg, msgLen, tmpbuf );
    snprintf( wbuf, sizeof(wbuf), "AT+MQTTPUB=\"%s\",0,0,0,%d,\"%s\"", topic, msgLen, tmpbuf );
    ret = at.cmd( (int8_t *) wbuf, strlen( wbuf ), "OK", NULL, NULL );
    if ( ret < 0 )
    {
        return (ret);
    }
    return (AT_OK);
}

int nb_mqtt_pub( char* topic, char* buf )
{
    return (NB_MN316_mqttPub( topic, buf, strlen( buf ) ) );
}


int32_t at_set_urc_table( const struct at_urc *urc_table, uint32_t table_sz )
{
    uint8_t idx;

    if ( table_sz > 5 )
        return (-1);
    for ( idx = 0; idx < table_sz; idx++ )
    {
        if ( urc_table[idx].cmd_prefix == NULL || urc_table[idx].cmd_suffix == NULL )
        {
           // printf( "[ERROR]at_set_urc_table fail\r\n" );
            return (-2);
        }
    }
    gUrcTable       = urc_table;
    gUrcTableSize   = table_sz;
    return (0);
}

void nb_urc_set()
{
   at_set_urc_table( nb_urc_table, sizeof(nb_urc_table) / sizeof(nb_urc_table[0]));
}


at_task at =
{
    
    .cmd        = at_cmd,
		.sendBuf    = at_buf,
};
