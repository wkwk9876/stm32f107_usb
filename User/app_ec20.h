#include "test_usbh.h"
#include "cmsis_os.h"
#include "usbh_ec20.h"

#define RECV_BUFF_SIZE		(64)

typedef enum {
	EC20_APPLICATION_IDLE = 0,
	EC20_APPLICATION_START,
	EC20_APPLICATION_READY,	
	EC20_APPLICATION_SMS_DONE,
	//EC20_APPLICATION_PB_DONE,	
	EC20_APPLICATION_QUERY_CARD,				//AT+CPIN?
	EC20_APPLICATION_QUERY_CS,					//AT+CREG?
	EC20_APPLICATION_QUERY_PS,					//AT+CGREG?/AT+CEREG?
	EC20_APPLICATION_CONFIG_PDP,				//AT+QICSGP/AT+CGQREQ/AT+CGEQREQ/AT+CGQMIN/AT+CGEQMIN
	EC20_APPLICATION_ACTIVATE_PDP,				//AT+QIACT=<contextID>
	EC20_APPLICATION_RUNNING,					//AT+QPING=1,"www.baidu.com"
	EC20_APPLICATION_DISCONNECT,
	EC20_APPLICATION_MAX_NUM
}ApplicationTypeDef;

typedef struct _ec20_app
{
	osMessageQId 			AppliEvent;
	ApplicationTypeDef 		Appli_state;
	char 					recv_buf[RECV_BUFF_SIZE + 1];
	volatile unsigned char	g_stop_flag;
	unsigned int 			rx_total_num;
	unsigned int 			tx_total_num;
	//EC20_LineCodingTypeDef 	LineCoding;
	//EC20_LineCodingTypeDef 	DefaultLineCoding;
	osThreadId				EC20_Send_Thread_id;
	//osThreadId				LOG_Task_id;
	int						(*ec20_recvdata)(USBH_HandleTypeDef *phost);
	int						cmd_index;
	char					*result_buff;
	int						result_flag;
}ec20_app;

typedef struct _ec20_cmd
{
	char					*name;
	char					*result;
	int						(*ec20_recvdata)(USBH_HandleTypeDef *phost);
}ec20_cmd;

static void Start_EC20_Application_Thread(void const *argument);

