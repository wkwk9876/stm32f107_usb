#include "test_usbh.h"
#include "cmsis_os.h"
#include "usbh_ce20.h"

#define RECV_BUFF_SIZE		(64)



typedef enum {
	CE20_APPLICATION_IDLE = 0,
	CE20_APPLICATION_START,
	CE20_APPLICATION_READY,
	CE20_APPLICATION_READY_CHECK,
	CE20_APPLICATION_SETTING_LINECODE,
	CE20_APPLICATION_RUNNING,
	CE20_APPLICATION_DISCONNECT,
}ApplicationTypeDef;

typedef struct _LOGBuffState {
	//sdram_buffer_map	*buf;
	__IO uint32_t     	p_read;
	__IO uint32_t     	p_write;
}CE20_LOG_BUFF_STATE;


typedef struct _ce20_app
{
	osMessageQId 			AppliEvent;
	ApplicationTypeDef 		Appli_state;
	//CE20_LOG_BUFF_STATE 		log_buf;
	char 					recv_buf[RECV_BUFF_SIZE + 1];
	volatile unsigned char	g_stop_flag;
	unsigned int 			rx_total_num;
	unsigned int 			tx_total_num;
	CE20_LineCodingTypeDef 	LineCoding;
	CE20_LineCodingTypeDef 	DefaultLineCoding;
	osThreadId				CE20_Send_Thread_id;
	osThreadId				LOG_Task_id;
}ce20_app;

static void Start_CE20_Application_Thread(void const *argument);

