#include "usbh_ce20.h"
#include "systemlog.h"

#define RX_BUF_SIZE     64

//char tx_buf[] = {"ati;+csub\r\n"};
//char rx_buf[RX_BUF_SIZE];

/**
* @brief  The function informs user that data have been received
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_CE20_TransmitCallback(USBH_HandleTypeDef *phost)
{
  
}
  
/**
* @brief  The function informs user that data have been sent
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_CE20_ReceiveCallback(USBH_HandleTypeDef *phost)
{
	/*uint16_t i;
	//printf("Recv count %d\r\n", USBH_CDC_GetLastReceivedDataSize(phost));
	for(i = 0; i < USBH_CDC_GetLastReceivedDataSize(phost); ++i)
	{
		putchar(rx_buf[i]);
	}
	//printf("\r\n");
	USBH_CE20_Receive(phost, rx_buf, RX_BUF_SIZE);*/
}

void print_ep_desc(USBH_EpDescTypeDef ep)
{
	printf("=============ep %d=============\r\n", ep.bEndpointAddress & 0x0f);
	printf("bLength           : %d\r\n", ep.bLength);
	printf("DescriptorType    : 0x%02x\r\n", ep.bDescriptorType);
	printf("EndpointAddress   : 0x%02x\r\n", ep.bEndpointAddress);
	printf("Attributes        : 0x%02x\r\n", ep.bmAttributes);
	printf("MaxPacketSize     : %d\r\n", ep.wMaxPacketSize);
	printf("Interval          : %d\r\n", ep.bInterval);
}

static USBH_StatusTypeDef USBH_CE20_InterfaceInit  (USBH_HandleTypeDef *phost)
{
	USBH_StatusTypeDef status = USBH_FAIL;
	uint8_t interface;
	CE20_HandleTypeDef * CE20_Handle = NULL;
	
	interface = 2;// Only usb interface 2 (For AT command communication)
	
	USBH_SelectInterface (phost, interface);
	phost->pClassData[0] = (CE20_HandleTypeDef *)USBH_malloc (sizeof(CE20_HandleTypeDef));
	CE20_Handle =	(CE20_HandleTypeDef*) phost->pClassData[0]; 

	/*Collect the notification endpoint address and length*/
	if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80)
	{
		CE20_Handle->CommItf.NotifEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
		CE20_Handle->CommItf.NotifEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
		print_ep_desc(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0]);
	}

	/*Collect the class specific endpoint address and length*/
	if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress & 0x80)
	{    
		CE20_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
		CE20_Handle->DataItf.InEpSize	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
		print_ep_desc(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1]);
	}
	else
	{
		CE20_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
		CE20_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
		print_ep_desc(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1]);
	}

	if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress & 0x80)
	{    
		CE20_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress;
		CE20_Handle->DataItf.InEpSize	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].wMaxPacketSize;
		print_ep_desc(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2]);
	}
	else
	{
		CE20_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress;
		CE20_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].wMaxPacketSize;
		print_ep_desc(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2]);
	}  

	/*Allocate the length for host channel number out*/
	CE20_Handle->DataItf.OutPipe = USBH_AllocPipe(phost, CE20_Handle->DataItf.OutEp);

	/*Allocate the length for host channel number in*/
	CE20_Handle->DataItf.InPipe = USBH_AllocPipe(phost, CE20_Handle->DataItf.InEp);  

	/* Open channel for OUT endpoint */
	USBH_OpenPipe  (phost,
				CE20_Handle->DataItf.OutPipe,
				CE20_Handle->DataItf.OutEp,
				phost->device.address,
				phost->device.speed,
				USB_EP_TYPE_BULK,
				CE20_Handle->DataItf.OutEpSize); 

	/* Open channel for IN endpoint */
	USBH_OpenPipe  (phost,
				CE20_Handle->DataItf.InPipe,
				CE20_Handle->DataItf.InEp,
				phost->device.address,
				phost->device.speed,
				USB_EP_TYPE_BULK,
				CE20_Handle->DataItf.InEpSize);

	CE20_Handle->state = CE20_IDLE_STATE;
	//CE20_Handle->attach_state = CH340_READ_VENDOR_VERSION_STATE;

	USBH_LL_SetToggle  (phost, CE20_Handle->DataItf.OutPipe,0);
	USBH_LL_SetToggle  (phost, CE20_Handle->DataItf.InPipe,0);   
	status = USBH_OK; 

	//USBH_CE20_Transmit(phost, tx_buf, strlen(tx_buf));
	//USBH_CE20_Receive(phost, rx_buf, RX_BUF_SIZE);
	
	return status;
}

static USBH_StatusTypeDef USBH_CE20_InterfaceDeInit  (USBH_HandleTypeDef *phost)
{
	return USBH_OK;
}

/**
  * @brief  This function return last received data size
  * @param  None
  * @retval None
  */
uint16_t USBH_CE20_GetLastReceivedDataSize(USBH_HandleTypeDef *phost)
{
	CE20_HandleTypeDef *CE20_Handle =	(CE20_HandleTypeDef*) phost->pClassData[0]; 

	if(phost->gState == HOST_CLASS)
	{
		return USBH_LL_GetLastXferSize(phost, CE20_Handle->DataItf.InPipe);
	}
	else
	{
		return 0;
	}
}

/**
  * @brief  This function prepares the state before issuing the class specific commands
  * @param  None
  * @retval None
  */
USBH_StatusTypeDef  USBH_CE20_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
	USBH_StatusTypeDef Status = USBH_BUSY;
	CE20_HandleTypeDef *CE20_Handle =	(CE20_HandleTypeDef*) phost->pClassData[0]; 

	if((CE20_Handle->state == CE20_IDLE_STATE) || (CE20_Handle->state == CE20_TRANSFER_DATA))
	{
		CE20_Handle->pTxData = pbuff;
		CE20_Handle->TxDataLength = length;  
		if(0 == (length % CE20_Handle->DataItf.OutEpSize))
		{
			CE20_Handle->tx_zero_packet_flag = 1;
		}
		
		CE20_Handle->state = CE20_TRANSFER_DATA;
		CE20_Handle->data_tx_state = CE20_SEND_DATA; 
		
		Status = USBH_OK;
#if (USBH_USE_OS == 1)
		osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif      
	}
	return Status;    
}
  
/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef  USBH_CE20_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
	USBH_StatusTypeDef Status = USBH_BUSY;
	CE20_HandleTypeDef *CE20_Handle =	(CE20_HandleTypeDef*) phost->pClassData[0]; 

	if((CE20_Handle->state == CE20_IDLE_STATE) || (CE20_Handle->state == CE20_TRANSFER_DATA))
	{
		CE20_Handle->pRxData = pbuff;
		CE20_Handle->RxDataLength = length;  
		CE20_Handle->state = CE20_TRANSFER_DATA;
		CE20_Handle->data_rx_state = CE20_RECEIVE_DATA;     
		Status = USBH_OK;
#if (USBH_USE_OS == 1)
		osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif        
	}
	return Status;    
} 

static void CE20_ProcessTransmission(USBH_HandleTypeDef *phost)
{
	CE20_HandleTypeDef *CE20_Handle =	(CE20_HandleTypeDef*) phost->pClassData[0]; 
	USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;

	switch(CE20_Handle->data_tx_state)
	{

		case CE20_SEND_DATA:
			if(CE20_Handle->TxDataLength > CE20_Handle->DataItf.OutEpSize)
			{
				USBH_BulkSendData (phost,
				                 CE20_Handle->pTxData, 
				                 CE20_Handle->DataItf.OutEpSize, 
				                 CE20_Handle->DataItf.OutPipe,
				                 1);
			}
			else
			{
				USBH_BulkSendData (phost,
				                 CE20_Handle->pTxData, 
				                 CE20_Handle->TxDataLength, 
				                 CE20_Handle->DataItf.OutPipe,
				                 1);
			}

			CE20_Handle->data_tx_state = CE20_SEND_DATA_WAIT;
			break;

		case CE20_SEND_DATA_WAIT:

			URB_Status = USBH_LL_GetURBState(phost, CE20_Handle->DataItf.OutPipe); 

			/*Check the status done for transmission*/
			if(URB_Status == USBH_URB_DONE )
			{         
				if(CE20_Handle->TxDataLength > CE20_Handle->DataItf.OutEpSize)
				{
					CE20_Handle->TxDataLength -= CE20_Handle->DataItf.OutEpSize ;
					CE20_Handle->pTxData += CE20_Handle->DataItf.OutEpSize;
				}
				else
				{
					CE20_Handle->TxDataLength = 0;
				}

				if( CE20_Handle->TxDataLength > 0)
				{
					CE20_Handle->data_tx_state = CE20_SEND_DATA; 
				}
				else if(1 == CE20_Handle->tx_zero_packet_flag)// send zero packet
				{
					CE20_Handle->tx_zero_packet_flag = 0;
					CE20_Handle->data_tx_state = CE20_SEND_DATA; 
				}
				else
				{					
					CE20_Handle->data_tx_state = CE20_IDLE;					
					USBH_CE20_TransmitCallback(phost);
				}
#if (USBH_USE_OS == 1)
				osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif    
			}
			else if( URB_Status == USBH_URB_NOTREADY )
			{
				CE20_Handle->data_tx_state = CE20_SEND_DATA; 
#if (USBH_USE_OS == 1)
				osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif          
			}
			break;
		default:
			break;
	}
}
/**
* @brief  This function responsible for reception of data from the device
*  @param  pdev: Selected device
* @retval None
*/

static void CE20_ProcessReception(USBH_HandleTypeDef *phost)
{
	CE20_HandleTypeDef *CE20_Handle =	(CE20_HandleTypeDef*) phost->pClassData[0]; 
	USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
	uint16_t length;

	switch(CE20_Handle->data_rx_state)
	{

		case CE20_RECEIVE_DATA:

			USBH_BulkReceiveData (phost,
			                  CE20_Handle->pRxData, 
			                  CE20_Handle->DataItf.InEpSize, 
			                  CE20_Handle->DataItf.InPipe);
			CE20_Handle->data_rx_state = CE20_RECEIVE_DATA_WAIT;
			break;

		case CE20_RECEIVE_DATA_WAIT:

			URB_Status = USBH_LL_GetURBState(phost, CE20_Handle->DataItf.InPipe); 

			/*Check the status done for reception*/
			if(URB_Status == USBH_URB_DONE )
			{  
				length = USBH_LL_GetLastXferSize(phost, CE20_Handle->DataItf.InPipe);

				if(((CE20_Handle->RxDataLength - length) > 0) && (length > CE20_Handle->DataItf.InEpSize))
				{
					CE20_Handle->RxDataLength -= length ;
					CE20_Handle->pRxData += length;
					CE20_Handle->data_rx_state = CE20_RECEIVE_DATA; 
				}
				else
				{
					CE20_Handle->data_rx_state = CE20_IDLE;
					USBH_CE20_ReceiveCallback(phost);
				}
#if (USBH_USE_OS == 1)
				osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif          
			}
			else if(URB_Status == USBH_URB_NOTREADY)
			{
				//__PRINT_LOG__(__CRITICAL_LEVEL__, "URB_Status: %d\r\n", URB_Status);
#if (USBH_USE_OS == 1)
				osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif 
			}
			break;
		default:
			break;
	}
}

static USBH_StatusTypeDef USBH_CE20_Process(USBH_HandleTypeDef *phost)
{
	USBH_StatusTypeDef status = USBH_BUSY;
	USBH_StatusTypeDef req_status = USBH_OK;
	CE20_HandleTypeDef *CE20_Handle =	(CE20_HandleTypeDef*) phost->pClassData[0]; 

	switch(CE20_Handle->state)
	{

		case CE20_IDLE_STATE:
			status = USBH_OK;
			break;

		case CE20_TRANSFER_DATA:			 
			CE20_ProcessTransmission(phost);
			CE20_ProcessReception(phost);
			break;   

		case CE20_ERROR_STATE:
			req_status = USBH_ClrFeature(phost, 0x00); 
			if(req_status == USBH_OK )
			{        
				/*Change the state to waiting*/
				CE20_Handle->state = CE20_IDLE_STATE ;
			}    
			break;

		default:
			break;
	}

	return status;
}

/*static int usb_wwan_send_setup(USBH_HandleTypeDef *phost)
{
	phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_CLASS | \
	                          USB_REQ_RECIPIENT_INTERFACE;

	phost->Control.setup.b.bRequest = CDC_SET_CONTROL_LINE_STATE;
	phost->Control.setup.b.wValue.w = 0x00;//set null
	phost->Control.setup.b.wIndex.w = 0x02;// interface 2
	phost->Control.setup.b.wLength.w = 0x00;     
	
	return USBH_CtlReq(phost, NULL, 0);
}*/

static USBH_StatusTypeDef USBH_CE20_ClassRequest (USBH_HandleTypeDef *phost)
{
	USBH_StatusTypeDef status = USBH_FAIL ;  
	CE20_HandleTypeDef *CE20_Handle =	(CE20_HandleTypeDef*) phost->pClassData[0]; 

	/*Issue the get line coding request*/
	CE20_Handle->LineCoding.b.dwDTERate = 115200;
	CE20_Handle->LineCoding.b.bCharFormat = 0;
	CE20_Handle->LineCoding.b.bParityType = 0;
	CE20_Handle->LineCoding.b.bDataBits = 0x08;

	status = USBH_OK;
	if(status == USBH_OK && NULL != phost->pUser)
	{
		printf("bitrate: %d\r\n", CE20_Handle->LineCoding.b.dwDTERate);
		phost->pUser(phost, HOST_USER_CLASS_ACTIVE); 
	}

	return status;
}

static USBH_StatusTypeDef USBH_CE20_SOFProcess (USBH_HandleTypeDef *phost)
{
	return USBH_OK;
}

USBH_ClassTypeDef  CE20_Class = 
{
  "CE20",
  USB_CE20_CLASS,
  USBH_CE20_InterfaceInit,
  USBH_CE20_InterfaceDeInit,
  USBH_CE20_ClassRequest,
  USBH_CE20_Process,
  USBH_CE20_SOFProcess,
  NULL,
};



