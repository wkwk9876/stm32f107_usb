#include "test_usbh.h"
#include "cmsis_os.h"
#include "usbh_ec20.h"

#define RECV_BUFF_SIZE		(64)

typedef enum {
	EC20_APPLICATION_IDLE = 0,
	EC20_APPLICATION_START,
	EC20_APPLICATION_READY,
	//EC20_APPLICATION_READY_CHECK,
	//EC20_APPLICATION_SETTING_LINECODE,
	EC20_APPLICATION_RUNNING,
	EC20_APPLICATION_DISCONNECT,
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
	int						(*recvdata)(USBH_HandleTypeDef *phost);
}ec20_app;

static void Start_EC20_Application_Thread(void const *argument);

