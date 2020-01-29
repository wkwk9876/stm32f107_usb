#include "stm32f1xx_hal.h"
#include "usbh_def.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_ec20.h"
#include "usbh_hub.h"

#include "main.h"
#include "test_usbh.h"

USBH_HandleTypeDef hUSBHost;

extern Usb_Application_Class app_ec20;

Usb_Application_Class * AppClass[] = 
{
	&app_ec20,
};

static int get_activeclass_app(USBH_HandleTypeDef * phost)
{
	int i;

	for(i = 0; i < sizeof(AppClass) / sizeof(AppClass[0]); ++i)
	{
		if(AppClass[i]->ClassCode == phost->pActiveClass->ClassCode)
		{
			phost->app_class = AppClass[i];
			return 0;
		}
	}
	return -1;
}

static void USBH_UserProcess(USBH_HandleTypeDef * phost, uint8_t id)
{
	switch (id)
	{
	case HOST_USER_SELECT_CONFIGURATION:
		break;

	case HOST_USER_CONNECTION:		
		break;

	case HOST_USER_CLASS_SELECTED:
		if(0 == get_activeclass_app(phost))
		{
			Usb_Application_Class * app_class = (Usb_Application_Class *)phost->app_class;
			app_class->new_app(phost);
		}
		break;

	case HOST_USER_CLASS_ACTIVE:
		if(NULL != phost && NULL != phost->app_class)
		{
			Usb_Application_Class * app_class = (Usb_Application_Class *)phost->app_class;
			app_class->start_app(phost);
		}
		break;

	case HOST_USER_DISCONNECTION:
		__PRINT_LOG__(__CRITICAL_LEVEL__, "DEBUG : disconnect!\r\n");
		if(NULL != phost && NULL != phost->app_class)
		{
			Usb_Application_Class * app_class = (Usb_Application_Class *)(phost->app_class);
			app_class->stop_app(phost);
			//app_class->delete_app(phost);
		}
		break;

	default:
		break;
	}
}

int init_usb_host(USBH_HandleTypeDef * phost)
{
	USBH_StatusTypeDef ret;
	/* Start Host Library */
	ret = USBH_Init(phost, USBH_UserProcess, 0);
	while(USBH_FAIL == ret)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "USBH_Init failed!\r\n");
		HAL_Delay(1000);
	}
	__PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_Init complete\r\n");

	/* Add Supported Class */
	ret = USBH_RegisterClass(phost, USBH_MSC_CLASS);
	ret = USBH_RegisterClass(phost, USBH_EC20_CLASS);
	ret = USBH_RegisterClass(phost, USBH_HUB_CLASS);
	__PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_RegisterClass complete\r\n");

	/* Start Host Process */
	ret = USBH_Start(phost);
	__PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_Start started!\r\n");

	return ret;
}

void ec20PowerInit()
{
    /* 1、初始化各个管脚。 */
    GPIO_InitTypeDef  GPIO_InitStructure;

	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	__PRINT_LOG__(__CRITICAL_LEVEL__, "ec20PowerInit started!\r\n");

	GPIO_InitStructure.Pin =      GPIO_PIN_9;
	GPIO_InitStructure.Mode =     GPIO_MODE_OUTPUT_PP;    
	GPIO_InitStructure.Speed =    GPIO_SPEED_FREQ_LOW;        
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);


	GPIO_InitStructure.Pin =      __MOD_EC20_STATUS_PIN__;
	GPIO_InitStructure.Mode =     GPIO_MODE_INPUT;    
	GPIO_InitStructure.Pull =     GPIO_PULLUP;      
	HAL_GPIO_Init(__MOD_EC20_STATUS_GROUP__, &GPIO_InitStructure);

	GPIO_InitStructure.Pin =      __MOD_EC20_RST_PIN__;
	GPIO_InitStructure.Mode =     GPIO_MODE_OUTPUT_PP;        
	HAL_GPIO_Init(__MOD_EC20_RST_GROUP__, &GPIO_InitStructure);

	GPIO_InitStructure.Pin =      __MOD_EC20_PWRKEY_PIN__;
	GPIO_InitStructure.Mode =     GPIO_MODE_OUTPUT_PP;    
	GPIO_InitStructure.Speed = 	  GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(__MOD_EC20_PWRKEY_GROUP__, &GPIO_InitStructure);


	HAL_GPIO_WritePin(__MOD_EC20_RST_GROUP__, __MOD_EC20_RST_PIN__, GPIO_PIN_RESET);

	if (0 == HAL_GPIO_ReadPin(__MOD_EC20_STATUS_GROUP__, __MOD_EC20_STATUS_PIN__))
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "opened!\r\n");
	
		HAL_GPIO_WritePin(__MOD_EC20_PWRKEY_GROUP__, __MOD_EC20_PWRKEY_PIN__, GPIO_PIN_RESET);

		HAL_Delay(40);

		HAL_GPIO_WritePin(__MOD_EC20_PWRKEY_GROUP__, __MOD_EC20_PWRKEY_PIN__, GPIO_PIN_SET);

		HAL_Delay(700);

		HAL_GPIO_WritePin(__MOD_EC20_PWRKEY_GROUP__, __MOD_EC20_PWRKEY_PIN__, GPIO_PIN_RESET);

		for (;;)
        {
            if (1 == HAL_GPIO_ReadPin(__MOD_EC20_STATUS_GROUP__, __MOD_EC20_STATUS_PIN__))
            {
                /* STATUS管脚变成高组态，管脚处于默认的拉高状态，说明模组已经关机。 */
                break;
            }
            else
            {
                /* 模组没有完成初始化的情况下，先进行适当延时 */
                HAL_Delay(100);
            }
        } 

		HAL_GPIO_WritePin(__MOD_EC20_PWRKEY_GROUP__, __MOD_EC20_PWRKEY_PIN__, GPIO_PIN_RESET);

		HAL_Delay(40);

		HAL_GPIO_WritePin(__MOD_EC20_PWRKEY_GROUP__, __MOD_EC20_PWRKEY_PIN__, GPIO_PIN_SET);

		HAL_Delay(700);

		HAL_GPIO_WritePin(__MOD_EC20_PWRKEY_GROUP__, __MOD_EC20_PWRKEY_PIN__, GPIO_PIN_RESET);

		
		for (;;)
		{
			if (0 == HAL_GPIO_ReadPin(__MOD_EC20_STATUS_GROUP__, __MOD_EC20_STATUS_PIN__))
			{
				/* STATUS管脚变成高组态，管脚处于默认的拉高状态，说明模组已经关机。 */
				break;
			}
			else
			{
				/* 模组没有完成初始化的情况下，先进行适当延时 */
				HAL_Delay(100);
			}
		} 
	}
	else
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "closed!\r\n");
	
		HAL_GPIO_WritePin(__MOD_EC20_PWRKEY_GROUP__, __MOD_EC20_PWRKEY_PIN__, GPIO_PIN_RESET);

		HAL_Delay(40);

		HAL_GPIO_WritePin(__MOD_EC20_PWRKEY_GROUP__, __MOD_EC20_PWRKEY_PIN__, GPIO_PIN_SET);

		HAL_Delay(700);

		HAL_GPIO_WritePin(__MOD_EC20_PWRKEY_GROUP__, __MOD_EC20_PWRKEY_PIN__, GPIO_PIN_RESET);

		__PRINT_LOG__(__CRITICAL_LEVEL__, "power on!\r\n");
		
		for (;;)
		{
			if (0 == HAL_GPIO_ReadPin(__MOD_EC20_STATUS_GROUP__, __MOD_EC20_STATUS_PIN__))
			{
				break;
			}
			else
			{
				HAL_Delay(100);
			}
		} 
	}

	__PRINT_LOG__(__CRITICAL_LEVEL__, "EC20 power init completed\r\n");
}

void start_usbh_thread(void const * argument)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOC_CLK_ENABLE();

	//init put hub in reset status
	GPIO_InitStruct.Pin =      GPIO_PIN_9;
	GPIO_InitStruct.Mode =     GPIO_MODE_OUTPUT_PP;    
	GPIO_InitStruct.Pull =     GPIO_PULLUP;      
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	HAL_Delay(1);

	ec20PowerInit();

	init_usb_host(&hUSBHost);
	
	for( ;; )
	{
		/* Delete the Init Thread */ 
		osThreadTerminate(NULL);
	}
}

