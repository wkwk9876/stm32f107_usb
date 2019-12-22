/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "usbh_def.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_ch340.h"
#include "usbh_hub.h"

#include "main.h"

#include "main.h"
#include "systemclockset.h"
#include "systemUartInit.h"
#include "systemDelay.h"
#include "systemlog.h"

#include "lwip/ethernetif.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"


#define USE_DHCP

/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   10
   
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1 


USBH_HandleTypeDef hUSBHost;
struct netif gnetif; /* network interface structure */

/** @addtogroup STM32F1xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
//void SystemClock_Config(void);

/* Private functions ---------------------------------------------------------*/

static void USBH_UserProcess(USBH_HandleTypeDef * phost, uint8_t id)
{
	//CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
	/*switch (id)
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
		//__PRINT_LOG__(__CRITICAL_LEVEL__, "DEBUG : get defualt bandrate: %d!\r\n", CDC_Handle->LineCoding);
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
		break;*/
  }


int init_usb_host(USBH_HandleTypeDef * phost)
{
	USBH_StatusTypeDef ret;
	/* Start Host Library */
	ret = USBH_Init(phost, USBH_UserProcess, 0);
	__PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_Init complete\r\n");

	/* Add Supported Class */
	ret = USBH_RegisterClass(phost, USBH_MSC_CLASS);
	ret = USBH_RegisterClass(phost, USBH_CH340_CLASS);
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


static void TEST_Task(void const *argument)
{
	uint32_t i = 0;
	init_usb_host(&hUSBHost);

	while(1)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "I am live!(%d)\r\n", i++);
		HAL_Delay(1000);
	}
	
}

#if 1
static void Netif_Config(void)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
 
#ifdef USE_DHCP
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);
#else
  IP_ADDR4(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  IP_ADDR4(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
  IP_ADDR4(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* USE_DHCP */
  
  /* add the network interface */    
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
  
  /*  Registers the default network interface. */
  netif_set_default(&gnetif);
  
  //if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called.*/
    netif_set_up(&gnetif);
  }
  //else
  {
    /* When the netif link is down this function must be called */
    //netif_set_down(&gnetif);
  }
}



void my_lwip_init_done(void * arg)
{
	__PRINT_LOG__(__CRITICAL_LEVEL__, "lwip init done!\r\n");
}


#ifdef USE_DHCP

/* DHCP process states */
#define DHCP_OFF                   (uint8_t) 0
#define DHCP_START                 (uint8_t) 1
#define DHCP_WAIT_ADDRESS          (uint8_t) 2
#define DHCP_ADDRESS_ASSIGNED      (uint8_t) 3
#define DHCP_TIMEOUT               (uint8_t) 4
#define DHCP_LINK_DOWN             (uint8_t) 5


#define MAX_DHCP_TRIES  16
__IO uint8_t DHCP_state = DHCP_OFF;

/**
* @brief  DHCP Process
* @param  argument: network interface
* @retval None
*/
void DHCP_thread(void const * argument)
{
  struct netif *netif = (struct netif *) argument;
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  struct dhcp *dhcp;

  __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_thread(%d)!\r\n", DHCP_state);
  
  for (;;)
  {
    switch (DHCP_state)
    {
    case DHCP_START:
      {
        ip_addr_set_zero_ip4(&netif->ip_addr);
        ip_addr_set_zero_ip4(&netif->netmask);
        ip_addr_set_zero_ip4(&netif->gw);     
        dhcp_start(netif);
        DHCP_state = DHCP_WAIT_ADDRESS;

		__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_START!\r\n");
      }
      break;
      
    case DHCP_WAIT_ADDRESS:
      {                
        if (dhcp_supplied_address(netif)) 
        {
          DHCP_state = DHCP_ADDRESS_ASSIGNED;	
          
          __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: gw:%s!\r\n", ipaddr_ntoa(&netif->gw));

		  __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: ip:%s!\r\n", ipaddr_ntoa(&netif->ip_addr));

		  __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: mask:%s!\r\n", ipaddr_ntoa(&netif->netmask));
        }
        else
        {
          dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
    
          /* DHCP timeout */
          if (dhcp->tries > MAX_DHCP_TRIES)
          {
            DHCP_state = DHCP_TIMEOUT;
            
            /* Stop DHCP */
            dhcp_stop(netif);
            
            /* Static address used */
            IP_ADDR4(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
            IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
            IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
            netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));

            __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_TIMEOUT!\r\n");
            
          }
          else
          {
            __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_GETTING(%d %d)!\r\n", dhcp->state, dhcp->tries);
          }
        }
      }
      break;
    case DHCP_LINK_DOWN:
    {
      /* Stop DHCP */
      dhcp_stop(netif);
      DHCP_state = DHCP_OFF; 
	  __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_LINK_DOWN!\r\n");
    }
    break;
    default: break;
    }
    
    /* wait 250 ms */
    HAL_Delay(250);
  }
}
#endif  /* USE_DHCP */



static void start_lwip_thread(void const * argument)
{
	/* Create tcp_ip stack thread */
	tcpip_init(my_lwip_init_done, NULL);

	/* Initialize the LwIP stack */
	Netif_Config();


#ifdef USE_DHCP
	/* Start DHCPClient */
	osThreadDef(DHCP, DHCP_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 4);
	osThreadCreate (osThread(DHCP), &gnetif);
#endif

	while(0 == netif_is_up(&gnetif))
	{
        delay_ms(250);
	}
#ifdef USE_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_START;
#endif 
	__PRINT_LOG__(__CRITICAL_LEVEL__, "netif_is_up!\r\n");
	
	for( ;; )
	{
	/* Delete the Init Thread */ 
	osThreadTerminate(NULL);
	}
}
#endif

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	HAL_Init();

	/* Configure the system clock to 72 MHz */
	SystemClock_Config(RCC_PLL_MUL9);
	delay_init(72); 

	/* Add your application code here */
	uart_init(115200);

	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin =      GPIO_PIN_9;
	GPIO_InitStruct.Mode =     GPIO_MODE_OUTPUT_PP;    
	GPIO_InitStruct.Pull =     GPIO_PULLUP;      
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	HAL_Delay(1);

	//ec20PowerInit();

	__PRINT_LOG__(__CRITICAL_LEVEL__, "freertos start!\r\n");

	osThreadDef(TEST_Task, TEST_Task, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(TEST_Task), NULL);

	//osThreadDef(start_lwip_thread, start_lwip_thread, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
    //osThreadCreate(osThread(start_lwip_thread), NULL);

	/* Start scheduler */
	osKernelStart();
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 25000000
  *            HSE PREDIV1                    = 5
  *            HSE PREDIV2                    = 5
  *            PLL2MUL                        = 8
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
