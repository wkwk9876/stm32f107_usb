#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "usbh_ec20.h"

#include "app_ec20.h"

//#define __EC20_DEBUG__

#define NUM_OF_ARRAY(x)			(sizeof(x) / sizeof(x[0]))

#define _CMD_TESTING_			"ati;+csub"
#define _CMD_SMS_DONE_			"\r\n+QIND: SMS DONE"
#define _CMD_PB_DONE_			"\r\n+QIND: PB DONE"
#define _CMD_QUERY_CARD_		"AT+CPIN?"
#define _CMD_QUERY_CS_			"AT+CREG?"
#define _CMD_QUERY_PS_			"AT+CGREG?"
#define _CMD_CONFIG_PDP_		"AT+QICSGP=1"
#define _CMD_ACTIVATE_PDP_		"AT+QIACT=1"
#define _CMD_RUNNING_			"AT+QPING=1,\"www.baidu.com\""

#define MAX_TIME_OUT			(30 * 1000)
#define MAX_TIME_OUT_NUM		(5)


char * ec20_tx_cmd[EC20_APPLICATION_MAX_NUM] = 
{
	[EC20_APPLICATION_QUERY_CARD] 	= _CMD_QUERY_CARD_,
	[EC20_APPLICATION_QUERY_CS] 	= _CMD_QUERY_CS_,
	[EC20_APPLICATION_QUERY_PS] 	= _CMD_QUERY_PS_,
	[EC20_APPLICATION_CONFIG_PDP] 	= _CMD_CONFIG_PDP_,
	[EC20_APPLICATION_ACTIVATE_PDP] = _CMD_ACTIVATE_PDP_,
	[EC20_APPLICATION_RUNNING] 		= _CMD_RUNNING_,
};

ec20_cmd cmd[];

int	ec20_recv_cmd_select(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef delete_EC20_Application(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef new_EC20_Application(USBH_HandleTypeDef *phost)
{
	ec20_app * app_data = NULL;

	if(NULL == phost)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "phost is NULL!\r\n");
		return USBH_FAIL;
	}

	app_data = (ec20_app *)pvPortMalloc(sizeof(ec20_app));
	if(NULL == app_data)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "malloc failed!\r\n");
		return USBH_FAIL;
	}

	memset(app_data, 0, sizeof(ec20_app));

	/* Create Application Queue */
    osMessageQDef(EC20queue, 1, uint16_t);
    app_data->AppliEvent = osMessageCreate(osMessageQ(EC20queue), NULL);
	if(NULL == app_data->AppliEvent)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "Message queue create failed!\r\n");
		return USBH_FAIL;
	}

	app_data->ec20_recvdata = ec20_recv_cmd_select;
	app_data->Appli_state = EC20_APPLICATION_IDLE;

	phost->app_data = app_data;

	return USBH_OK;
}


void USBH_EC20_ReceiveCallback(USBH_HandleTypeDef *phost)
{
	ec20_app 				*app_data		= NULL;
	uint16_t 				i;
	int						ret_val			= -1;

	if(NULL == phost || NULL == phost->app_data)
		return;

	app_data = (ec20_app *)phost->app_data;

	if(NULL != app_data->ec20_recvdata)
	{
		ret_val = app_data->ec20_recvdata(phost);
	}
	
	if(-1 == ret_val)
	{
		for(i = 0; i < USBH_EC20_GetLastReceivedDataSize(phost); ++i)
		{
			putchar(app_data->recv_buf[i]);
		}
	}
	app_data->rx_total_num += USBH_EC20_GetLastReceivedDataSize(phost);
    memset(app_data->recv_buf, 0, RECV_BUFF_SIZE + 1);
	USBH_EC20_Receive(phost, (unsigned char *)app_data->recv_buf, RECV_BUFF_SIZE);
}

static USBH_StatusTypeDef EC20_send_data(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t buff_size, char *cmd)
{
	USBH_StatusTypeDef Status = USBH_BUSY;
	unsigned int len = snprintf((char *)pbuff, buff_size, "%s\r\n", cmd);
	ec20_app 	*app_data	= NULL;
	
	app_data = (ec20_app *)phost->app_data;
	
	Status = USBH_EC20_Transmit(phost, (unsigned char *)pbuff, strlen((char *)pbuff));
	if(USBH_OK == Status)
	{
		app_data->tx_total_num += len;
		printf("Send(%d)\r\n", app_data->tx_total_num);
	}
	
	return Status;
}

void ec20PowerInit(void);

static void Start_EC20_Application_Thread(void const *argument)
{
	USBH_HandleTypeDef 			*phost 		= NULL;
	ec20_app 					*app_data	= NULL;
	osEvent 					event;
	//EC20_AttachStateTypeDef 	attach_state;
	//EC20_HandleTypeDef 			*EC20_Handle;
	//unsigned int 				count = 0;
	char 						send_buf[64];
	USBH_StatusTypeDef 			Status = USBH_BUSY;

	while(NULL == argument)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "argument is null!\r\n");
	}

	phost = (USBH_HandleTypeDef *)argument;
	app_data = (ec20_app *)phost->app_data;
	while(NULL == app_data)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "app_data is null!\r\n");
	}

	while(0 == app_data->g_stop_flag || EC20_APPLICATION_DISCONNECT != app_data->Appli_state)
	{
		event = osMessageGet(app_data->AppliEvent, MAX_TIME_OUT);//osWaitForever);
		//__PRINT_LOG__(__CRITICAL_LEVEL__, "event!\r\n");

		if(event.status == osEventMessage)
		{
			//app_data->timeout_times = 0;
			switch(event.value.v)
			{				
			case EC20_APPLICATION_DISCONNECT:
				//app_data->g_stop_flag = 0;
				app_data->Appli_state = EC20_APPLICATION_DISCONNECT;
				Status = USBH_OK;
				break;

			case EC20_APPLICATION_READY:
				app_data->Appli_state = EC20_APPLICATION_READY;
				++app_data->timeout_times;
				EC20_send_data(phost, (unsigned char *)send_buf, 64, _CMD_TESTING_);
				Status = USBH_OK;
				break;

			case EC20_APPLICATION_SMS_DONE:
				__PRINT_LOG__(__CRITICAL_LEVEL__, "Waiting SMS_DONE!\r\n");
				app_data->Appli_state = EC20_APPLICATION_SMS_DONE;
				++app_data->timeout_times;
				Status = USBH_OK;
				break;

			/*case EC20_APPLICATION_PB_DONE:
				__PRINT_LOG__(__CRITICAL_LEVEL__, "Waiting PB_DONE!\r\n");
				app_data->Appli_state = EC20_APPLICATION_PB_DONE;
				++app_data->timeout_times;
				Status = USBH_OK;
				break;*/

			case EC20_APPLICATION_QUERY_CARD:
				app_data->Appli_state = EC20_APPLICATION_QUERY_CARD;
				++app_data->timeout_times;
				Status = EC20_send_data(phost, (unsigned char *)send_buf, 64, _CMD_QUERY_CARD_);
				break;

			case EC20_APPLICATION_QUERY_CS:
				app_data->Appli_state = EC20_APPLICATION_QUERY_CS;
				++app_data->timeout_times;
				Status = EC20_send_data(phost, (unsigned char *)send_buf, 64, _CMD_QUERY_CS_);
				break;

			case EC20_APPLICATION_QUERY_PS:
				app_data->Appli_state = EC20_APPLICATION_QUERY_PS;
				++app_data->timeout_times;
				Status = EC20_send_data(phost, (unsigned char *)send_buf, 64, _CMD_QUERY_PS_);
				break;

			case EC20_APPLICATION_CONFIG_PDP:
				app_data->Appli_state = EC20_APPLICATION_CONFIG_PDP;
				++app_data->timeout_times;
				Status = EC20_send_data(phost, (unsigned char *)send_buf, 64, _CMD_CONFIG_PDP_);
				break;

			case EC20_APPLICATION_ACTIVATE_PDP:
				app_data->Appli_state = EC20_APPLICATION_ACTIVATE_PDP;
				++app_data->timeout_times;
				osDelay(1000);
				Status = EC20_send_data(phost, (unsigned char *)send_buf, 64, _CMD_ACTIVATE_PDP_);
				break;

			case EC20_APPLICATION_RUNNING:
				app_data->Appli_state = EC20_APPLICATION_RUNNING;
				++app_data->timeout_times;
				//osDelay(100);
				Status = EC20_send_data(phost, (unsigned char *)send_buf, 64, _CMD_RUNNING_);
				break;

			default:
				Status = USBH_OK;
				break;
			}
		}
		else if(app_data->timeout_times <= MAX_TIME_OUT_NUM)
		{
				__PRINT_LOG__(__ERR_LEVEL__, "Waiting time out!!! Repeat state: %d!\r\n", app_data->Appli_state);
				++app_data->timeout_times;
				osMessagePut(app_data->AppliEvent, app_data->Appli_state, 0);// repeat this state
		}

		if(app_data->timeout_times > MAX_TIME_OUT_NUM)
		{
			__PRINT_LOG__(__ERR_LEVEL__, "Reach MAX TIME OUT NUM!!! Goto reset EC20!\r\n");
			app_data->timeout_times = 0;
			ec20PowerInit();//reset ec20 will lead to usb disconnect and connect again
			__PRINT_LOG__(__ERR_LEVEL__, "Reset EC20 completed!\r\n");
			break;
		}

		if(USBH_OK != Status)
		{
			__PRINT_LOG__(__ERR_LEVEL__, "USB send failed!!! Repeat state: %d!\r\n", app_data->Appli_state);
			osMessagePut(app_data->AppliEvent, app_data->Appli_state, 0);// repeat this state
		}
		else
		{ 
			//osDelay(100);
		}
	}

	app_data->g_stop_flag = 0;
	__PRINT_LOG__(__CRITICAL_LEVEL__, "app stop!\r\n");

	//delete_EC20_Application(phost);
	
	osThreadTerminate(osThreadGetId());
}

USBH_StatusTypeDef start_EC20_Application(USBH_HandleTypeDef *phost)
{
	ec20_app					*app_data	= NULL;
	
	while(NULL == phost)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "phost is null!\r\n");
	}

	app_data = (ec20_app *)phost->app_data;

	osThreadDef(EC20_Send_Thread, Start_EC20_Application_Thread, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
    app_data->EC20_Send_Thread_id = osThreadCreate(osThread(EC20_Send_Thread), phost);
	if(NULL == app_data->EC20_Send_Thread_id)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "create EC20_Send_Thread failed!\r\n");
		return USBH_FAIL;
	}

	while(USBH_OK != USBH_EC20_Receive(phost, (unsigned char *)(app_data->recv_buf), RECV_BUFF_SIZE))
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "waiting EC20 driver init!\r\n");
		HAL_Delay(100);
	}

	osMessagePut(app_data->AppliEvent, EC20_APPLICATION_READY, 0);

	return USBH_OK;
}

USBH_StatusTypeDef stop_EC20_Application(USBH_HandleTypeDef *phost)
{
	ec20_app 					*app_data	= NULL;
	unsigned char				i = 0;
	
	if(NULL == phost)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "phost is null!\r\n");
		return USBH_OK;
	}
	
	app_data = (ec20_app *)phost->app_data;
	if(NULL == app_data)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "app_data is null!\r\n");
		return USBH_OK;
	}

	app_data->g_stop_flag = 1;
	osMessagePut(app_data->AppliEvent, EC20_APPLICATION_DISCONNECT, 0);

	// if we receive disconnect interrupt, indicate that EC20 is closeing
	// fix me: if we can not wait EC20 status pin, we just stop it.
	while(1 == app_data->g_stop_flag && i < 10)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "Waiting Application Stop!\r\n");
		++i;
		HAL_Delay(500);
	}

	//if thread Terminate itself,the resource of this thread will be release
	//only when the system in idle. If the system no idle status, the resource
	//will not be free. So, we delete it by ourself again.
	osThreadTerminate(app_data->EC20_Send_Thread_id);

	__PRINT_LOG__(__CRITICAL_LEVEL__, "stop_EC20!\r\n");

	return USBH_OK;
}

USBH_StatusTypeDef delete_EC20_Application(USBH_HandleTypeDef *phost)
{
	
	ec20_app					*app_data	= NULL;

	__PRINT_LOG__(__CRITICAL_LEVEL__, "delete_EC20!\r\n");
		
	while(NULL == phost)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "phost is null!\r\n");
	}
	
	app_data = (ec20_app *)phost->app_data;
	while(NULL == app_data)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "app_data is null!\r\n");
	}

	app_data->ec20_recvdata = NULL;

	if(NULL != app_data->result_buff)
	{
		vPortFree(app_data->result_buff);
		app_data->result_buff = NULL;
	}

	osMessageDelete(app_data->AppliEvent);
	vPortFree(app_data);
	app_data = NULL;

	phost->app_data = NULL;

	return USBH_OK;
}

int	ec20_recv_ati(USBH_HandleTypeDef *phost)
{
	ec20_app 				*app_data		= NULL;

	if(NULL == phost->app_data)
		return -1;

	app_data = (ec20_app *)phost->app_data;

#ifdef __EC20_DEBUG__
	for(int i = 0; i < USBH_EC20_GetLastReceivedDataSize(phost); ++i)
	{
		putchar(app_data->recv_buf[i]);
	}
#endif
	memset(app_data->recv_buf, 0, RECV_BUFF_SIZE + 1);

	if(RECV_BUFF_SIZE == USBH_EC20_GetLastReceivedDataSize(phost))
	{
		app_data->ec20_recvdata = ec20_recv_ati;
	}
	else
	{
		app_data->ec20_recvdata = ec20_recv_cmd_select;
		app_data->timeout_times = 0;
		osMessagePut(app_data->AppliEvent, app_data->Appli_state + 1, 0);
	}

	return 0;
}

int	ec20_recv_init_done(USBH_HandleTypeDef *phost)
{
	ec20_app 				*app_data		= NULL;

	if(NULL == phost->app_data)
		return -1;

	app_data = (ec20_app *)phost->app_data;
	
#ifdef __EC20_DEBUG__
	for(int i = 0; i < USBH_EC20_GetLastReceivedDataSize(phost); ++i)
	{
		putchar(app_data->recv_buf[i]);
	}
#endif
	memset(app_data->recv_buf, 0, RECV_BUFF_SIZE + 1);

	app_data->ec20_recvdata = ec20_recv_cmd_select;
	app_data->timeout_times = 0;
	osMessagePut(app_data->AppliEvent, app_data->Appli_state + 1, 0);//jump to next state

	return 0;
}

int	ec20_recv_general(USBH_HandleTypeDef *phost)
{
	ec20_app 				*app_data		= NULL;

	if(NULL == phost->app_data)
		return -1;

	app_data = (ec20_app *)phost->app_data;

	if(RECV_BUFF_SIZE == USBH_EC20_GetLastReceivedDataSize(phost))// data recv no complete
	{
		if(NULL == app_data->result_buff)//first segment
		{
			if(NULL != strstr(app_data->recv_buf, cmd[app_data->cmd_index].result))
			{
				app_data->result_flag = 1;
			}
			else
			{
				app_data->result_buff = (char *)pvPortMalloc(strlen(cmd[app_data->cmd_index].result) + RECV_BUFF_SIZE + 1);//add 1 for \0
				if(NULL == app_data->result_buff)
				{
					__PRINT_LOG__(__ERR_LEVEL__, "malloc failed!\r\n");
				}
				else
				{
					memset(app_data->result_buff, 0, strlen(cmd[app_data->cmd_index].result) + RECV_BUFF_SIZE + 1);
					memcpy(app_data->result_buff,
							app_data->recv_buf + (RECV_BUFF_SIZE - strlen(cmd[app_data->cmd_index].result)),
							strlen(cmd[app_data->cmd_index].result));
				}
			}
		}
		else if(0 == app_data->result_flag)//other segment
		{
			memcpy(app_data->result_buff + strlen(cmd[app_data->cmd_index].result),	app_data->recv_buf, RECV_BUFF_SIZE);
			if(NULL != strstr(app_data->result_buff, cmd[app_data->cmd_index].result))
			{
				app_data->result_flag = 1;
			}
			else
			{
				memset(app_data->result_buff, 0, strlen(cmd[app_data->cmd_index].result) + RECV_BUFF_SIZE + 1);
				memcpy(app_data->result_buff,
						app_data->recv_buf + (RECV_BUFF_SIZE - strlen(cmd[app_data->cmd_index].result)),
						strlen(cmd[app_data->cmd_index].result));
			}
		}
		app_data->ec20_recvdata = ec20_recv_general;
	}
	else//last segment or frist segment
	{
		if(0 == app_data->result_flag && NULL == app_data->result_buff)//frist segment
		{
			if(NULL != strstr(app_data->recv_buf, cmd[app_data->cmd_index].result))
			{
				app_data->result_flag = 1;
			}
		}
		else if(0 == app_data->result_flag && NULL != app_data->result_buff)//last segment
		{
			memcpy(app_data->result_buff + strlen(cmd[app_data->cmd_index].result),	
				app_data->recv_buf, 
				USBH_EC20_GetLastReceivedDataSize(phost));
			if(NULL != strstr(app_data->result_buff, cmd[app_data->cmd_index].result))
			{
				app_data->result_flag = 1;
			}
		}
		//else is 1 == app_data->result_flag

		if(NULL != app_data->result_buff)
		{
			vPortFree(app_data->result_buff);
			app_data->result_buff = NULL;
		}

		if(1 == app_data->result_flag)
		{			
			app_data->result_flag = 0;
			
#ifdef __EC20_DEBUG__
			printf("%s(len:%d)\r\n", cmd[app_data->cmd_index].name, USBH_EC20_GetLastReceivedDataSize(phost));
			for(int i = 0; i < USBH_EC20_GetLastReceivedDataSize(phost); ++i)
			{
				putchar(app_data->recv_buf[i]);
			}
#endif
			memset(app_data->recv_buf, 0, RECV_BUFF_SIZE + 1);
			
			app_data->ec20_recvdata = ec20_recv_cmd_select;
			app_data->timeout_times = 0;
			osMessagePut(app_data->AppliEvent, app_data->Appli_state + 1, 0);//jump to next state
		}
		else
		{
			app_data->result_flag = 0;
			__PRINT_LOG__(__ERR_LEVEL__, "cmd: %s return failed!(len:%d timeout:%d)\r\n", 
											cmd[app_data->cmd_index].name,
											USBH_EC20_GetLastReceivedDataSize(phost),
											app_data->timeout_times);
			
			for(int i = 0; i < USBH_EC20_GetLastReceivedDataSize(phost); ++i)
			{
				putchar(app_data->recv_buf[i]);
			}
			memset(app_data->recv_buf, 0, RECV_BUFF_SIZE + 1);

			app_data->ec20_recvdata = ec20_recv_cmd_select;
			osMessagePut(app_data->AppliEvent, app_data->Appli_state, 0);// repeat this state
		}
	}

	return 0;
}

int	ec20_recv_running(USBH_HandleTypeDef *phost)
{
	ec20_app 				*app_data		= NULL;

	if(NULL == phost->app_data)
		return -1;

	app_data = (ec20_app *)phost->app_data;
#ifdef __EC20_DEBUG__
	printf("%s\r\n", cmd[app_data->cmd_index].name);
	for(int i = 0; i < USBH_EC20_GetLastReceivedDataSize(phost); ++i)
	{
		putchar(app_data->recv_buf[i]);
	}
#endif	
	memset(app_data->recv_buf, 0, RECV_BUFF_SIZE + 1);

	/*if(RECV_BUFF_SIZE == USBH_EC20_GetLastReceivedDataSize(phost))
	{
		app_data->ec20_recvdata = ec20_recv_running;
	}
	else*/
	{
		app_data->ec20_recvdata = ec20_recv_cmd_select;
		app_data->timeout_times = 0;
		osMessagePut(app_data->AppliEvent, EC20_APPLICATION_RUNNING, 0);
	}

	return 0;
}

ec20_cmd cmd[] = 
{
	{_CMD_TESTING_, 		"OK", 		ec20_recv_ati},	
	{_CMD_SMS_DONE_,		NULL,		ec20_recv_init_done},
	//{_CMD_PB_DONE_,			NULL,		ec20_recv_init_done},
	{_CMD_QUERY_CARD_, 		"READY", 	ec20_recv_general},
	{_CMD_QUERY_CS_, 		"OK", 		ec20_recv_general},
	{_CMD_QUERY_PS_, 		"OK", 		ec20_recv_general},
	{_CMD_CONFIG_PDP_, 		"OK", 		ec20_recv_general},
	{_CMD_ACTIVATE_PDP_, 	"OK", 		ec20_recv_general},
	{_CMD_RUNNING_, 		"OK", 		ec20_recv_running},
};

int ec20_find_cmd(USBH_HandleTypeDef * phost, char * buff)
{
	ec20_app 				*app_data		= NULL;
	uint16_t 				i;

	if(NULL == phost || NULL == phost->app_data)	
		return -2;

	app_data = (ec20_app *)phost->app_data;
	
	for(i = 0; i < NUM_OF_ARRAY(cmd); ++i)
	{
		if(NULL != strstr(buff, cmd[i].name))
		{
			//printf("match index: %d name: %s\r\n", i, cmd[i].name);
			app_data->ec20_recvdata = cmd[i].ec20_recvdata;
			app_data->cmd_index		= i;
			break;
		}
	}

	if(NUM_OF_ARRAY(cmd) == i)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "undef cmd(len:%d)\r\n", USBH_EC20_GetLastReceivedDataSize(phost));
		return -1;
	}

	return 0;
}

int	ec20_recv_cmd_select(USBH_HandleTypeDef *phost)
{
	ec20_app 				*app_data		= NULL;
	int						ret = -1;
	//uint16_t 				i;

	if(NULL == phost || NULL == phost->app_data)	
		return -2;

	app_data = (ec20_app *)phost->app_data;

	//we assert there is at most one cmd in one receive
	if(RECV_BUFF_SIZE == USBH_EC20_GetLastReceivedDataSize(phost))// data recv no complete
	{
		if(NULL == app_data->result_buff)//first segment
		{
			ret = ec20_find_cmd(phost, app_data->recv_buf);
			if(0 != ret)
			{
				app_data->result_buff = (char *)pvPortMalloc(strlen(_CMD_RUNNING_) + RECV_BUFF_SIZE + 1);//add 1 for \0
				if(NULL == app_data->result_buff)
				{
					__PRINT_LOG__(__ERR_LEVEL__, "malloc failed!\r\n");
				}
				else
				{
					memset(app_data->result_buff, 0, strlen(_CMD_RUNNING_) + RECV_BUFF_SIZE + 1);
					memcpy(app_data->result_buff,
							app_data->recv_buf + (RECV_BUFF_SIZE - strlen(_CMD_RUNNING_)),
							strlen(_CMD_RUNNING_));
				}
			}
		}
		else//other segment
		{
			memcpy(app_data->result_buff + strlen(_CMD_RUNNING_), app_data->recv_buf, RECV_BUFF_SIZE);
			ret = ec20_find_cmd(phost, app_data->result_buff);
			if(0 != ret)
			{
				memset(app_data->result_buff, 0, strlen(_CMD_RUNNING_) + RECV_BUFF_SIZE + 1);
				memcpy(app_data->result_buff,
						app_data->recv_buf + (RECV_BUFF_SIZE - strlen(_CMD_RUNNING_)),
						strlen(_CMD_RUNNING_));
			}
		}
	}
	else//last segment or frist segment
	{
		if(NULL == app_data->result_buff)//frist segment
		{
			ret = ec20_find_cmd(phost, app_data->recv_buf);
		}
		else
		{
			memcpy(app_data->result_buff + strlen(_CMD_RUNNING_), 
					app_data->recv_buf, 
					USBH_EC20_GetLastReceivedDataSize(phost));
			
			ret = ec20_find_cmd(phost, app_data->result_buff);
		}
	}

	//find it or short packet
	if(0 == ret || USBH_EC20_GetLastReceivedDataSize(phost) < RECV_BUFF_SIZE)
	{
		if(NULL != app_data->result_buff)
		{
			vPortFree(app_data->result_buff);
			app_data->result_buff = NULL;
		}
	}

	return ret;

	
	/*for(i = 0; i < NUM_OF_ARRAY(cmd); ++i)
	{
		if(NULL != strstr(app_data->recv_buf, cmd[i].name))
		//if(0 == strncmp(app_data->recv_buf, cmd[i].name, strlen(cmd[i].name)))
		{
			//printf("match index: %d name: %s\r\n", i, cmd[i].name);
			app_data->ec20_recvdata = cmd[i].ec20_recvdata;
			app_data->cmd_index		= i;
			break;
		}
	}

	if(NUM_OF_ARRAY(cmd) == i)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "undef cmd(len:%d)\r\n", USBH_EC20_GetLastReceivedDataSize(phost));
		return -1;
	}

	return 0;*/
}

Usb_Application_Class app_ec20 =
{
	USB_EC20_CLASS,
	new_EC20_Application,
	start_EC20_Application,
	stop_EC20_Application,
	delete_EC20_Application
};


