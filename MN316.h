#ifndef __MN316_H
#define __MN316_H

#include "stdint.h"

#define AT_MODU_NAME    "MN316"
#define AT_NB_LINE_END  "\r\n"
#define AT_NB_at        "AT"
#define  AT_DATA_LEN    1024

#define  AT_OK          0

#define DEMO_ONNET_brokerAddress    "183.230.40.96"
#define DEMO_ONNET_port             (1883)
#define DEMO_ONNET_cientId          "test"
#define DEMO_ONNET_userName         "84QSXilFVc"
#define DEMO_ONNET_password         "version=2018-10-31&res=products%2F84QSXilFVc%2Fdevices%2Ftest&et=2537256630&method=md5&sign=FBtMbCLP6Hjonj0TUZQlXA%3D%3D"
#define DEMO_ONNET_repubtopic       "$sys/84QSXilFVc/test/thing/property/post/reply"
#define DEMO_ONNET_resubtopic       "$sys/84QSXilFVc/test/thing/event/post/reply"
#define DEMO_ONNET_pubtopic         "$sys/84QSXilFVc/test/thing/property/post"
#define DEMO_ONNET_subtopic         "$sys/84QSXilFVc/test/thing/property/set"

#define Property_Init 							"{\"id\":\"1225\", \"version\":\"1.0\",\"params\":{\"angle\":{\"value\":30},\"direction\":{\"value\":15},\"led\":{\"value\":0},\"temperature\":{\"value\":17}}}"

#define  UARTX_RECV_END (0x8000)

extern int Day;
extern int Month;
extern int Year;
extern int Second;
extern int Minute;
extern int Hour;

extern char    sendData[256];
 
extern uint8_t              gUrcTableSize;
extern const struct at_urc  *gUrcTable;
extern struct at_urc    nb_urc_table[2];

struct at_urc																												//定义urc数据结构体
{
    const char  *cmd_prefix;																				//URC 的前缀字符串
    const char  *cmd_suffix;																				//URC 的后缀字符串
    void        (*func)( const char *data, unsigned long size );		//函数指针，指向一个回调函数
};
typedef struct at_urc *at_urc_t;																		//定义at_urc别名为*at_urc_t,而at_urc_t是一个指向at_urc的指针

typedef struct __config {
    char        * name;
    uint32_t    usart_port;
    uint32_t    buardrate;
    uint32_t    user_buf_len;
    char        * cmd_begin;
    char        * line_end;
    uint32_t    mux_mode;
    uint32_t    timeout; /*command respond timeout */
}at_config;

typedef struct at_task 
{
    void (*step_callback)();
    int32_t (*init)( at_config *config );
    int32_t (*cmd)( int8_t * cmd, int32_t len, const char * suffix, char * resp_buf, int* resp_len );
    int32_t (*sendBuf)( int8_t * buf, int32_t len, const char * suffix, char * resp_buf, int* resp_len );
    int32_t (*deinit)( void );
}at_task;

extern at_task at;
extern uint8_t aRxBuffer[128];

void NB_Init(void);
int nb_mqtt_con( char * brokerAddress, uint16_t port, char *clientID, char *userName, char *password );
int nb_mqtt_sub( char* topic );
int nb_mqtt_pub( char* topic, char* buf );

void Set_sendData(uint32_t angle,uint32_t direction,uint8_t led,uint8_t temperature );

uint8_t NBAT_checkUrc( uint8_t *outResponse, uint32_t len );
void nb_urc_dataIoctl( const char *buf, unsigned long size );
int32_t NB_checkDevice( void );
int32_t NB_GetTime( void );
uint8_t NBAT_sendCmd( char *cmd, char *ack, uint32_t waittime, uint8_t *outResponse );

int32_t at_set_urc_table( const struct at_urc *urc_table, uint32_t table_sz );
void nb_urc_set();

#endif
