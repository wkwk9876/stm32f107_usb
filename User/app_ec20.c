#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "usbh_ec20.h"

#include "app_ec20.h"

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

	app_data->Appli_state = EC20_APPLICATION_IDLE;

	phost->app_data = app_data;

	return USBH_OK;
}


void USBH_EC20_ReceiveCallback(USBH_HandleTypeDef *phost)
{
	ec20_app 				*app_data		= NULL;
	uint16_t 				i;

	if(NULL == phost->app_data)
		return;

	app_data = (ec20_app *)phost->app_data;

	for(i = 0; i < USBH_EC20_GetLastReceivedDataSize(phost); ++i)
	{
		putchar(app_data->recv_buf[i]);
	}
	
    //memset(app_data->recv_buf, 0, RECV_BUFF_SIZE);
	USBH_EC20_Receive(phost, (unsigned char *)app_data->recv_buf, RECV_BUFF_SIZE);
}

static void Start_EC20_Application_Thread(void const *argument)
{
	USBH_HandleTypeDef 			*phost 		= NULL;
	ec20_app 					*app_data	= NULL;
	osEvent 					event;
	//EC20_AttachStateTypeDef 	attach_state;
	EC20_HandleTypeDef 			*EC20_Handle;
	//unsigned int 				count = 0;
	char 						send_buf[64];

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
		event = osMessageGet(app_data->AppliEvent, osWaitForever);
		//__PRINT_LOG__(__CRITICAL_LEVEL__, "event!\r\n");

		if(event.status == osEventMessage)
		{
			switch(event.value.v)
			{
			case EC20_APPLICATION_DISCONNECT:
				//app_data->g_stop_flag = 0;
				app_data->Appli_state = EC20_APPLICATION_DISCONNECT;
				break;

			case EC20_APPLICATION_READY:
				app_data->Appli_state = EC20_APPLICATION_READY;
				//if(USBH_OK == )//check ec20 driver init complete
				{
					osMessagePut(app_data->AppliEvent, EC20_APPLICATION_RUNNING, 0);
				}
				break;

			case EC20_APPLICATION_RUNNING:
				EC20_Handle =  (EC20_HandleTypeDef*) phost->pClassData[0];
				app_data->Appli_state = EC20_APPLICATION_RUNNING;
				if(EC20_IDLE_STATE == EC20_Handle->state)
				{  			
				    memset(app_data->recv_buf, 0, RECV_BUFF_SIZE);
					USBH_EC20_Receive(phost, (unsigned char *)(app_data->recv_buf), RECV_BUFF_SIZE);           
				    while(0 == app_data->g_stop_flag)
				    {
						unsigned int len = snprintf(send_buf, 64, "ati;+csub\r\n");//ati;+csub//AT+CREG?
						USBH_EC20_Transmit(phost, (unsigned char *)send_buf, strlen(send_buf));

						app_data->tx_total_num += len;
						printf("Send(%d)\r\n", app_data->tx_total_num);
						//__PRINT_LOG__(__CRITICAL_LEVEL__, "Send(%d): %s", tx_total_num, send_buf);

						osDelay(1000);			  
				    }  
				}
				else
				{
					osMessagePut(app_data->AppliEvent, EC20_APPLICATION_RUNNING, 0);
				}
				break;

			default:
				break;
			}
		}
	}

	__PRINT_LOG__(__CRITICAL_LEVEL__, "app stop!\r\n");

	delete_EC20_Application(phost);
	
	osThreadTerminate(osThreadGetId());
}

USBH_StatusTypeDef start_EC20_Application(USBH_HandleTypeDef *phost)
{
	ec20_app					*app_data	= NULL;
	
	while(NULL == phost)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "phost is null!\r\n");
	}

	app_data = (ec20_app *)phost->app_data;

	osThreadDef(EC20_Send_Thread, Start_EC20_Application_Thread, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
    app_data->EC20_Send_Thread_id = osThreadCreate(osThread(EC20_Send_Thread), phost);

	osMessagePut(app_data->AppliEvent, EC20_APPLICATION_READY, 0);

	return USBH_OK;
}

USBH_StatusTypeDef stop_EC20_Application(USBH_HandleTypeDef *phost)
{
	ec20_app 					*app_data	= NULL;
	
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

	osMessageDelete(app_data->AppliEvent);
	vPortFree(app_data);
	app_data = NULL;

	phost->app_data = NULL;

	return USBH_OK;
}

Usb_Application_Class app_ec20 =
{
	USB_EC20_CLASS,
	new_EC20_Application,
	start_EC20_Application,
	stop_EC20_Application,
	delete_EC20_Application
};


