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
#include <string.h>
#include "cmsis_os.h"

#include "main.h"

#include "lwip/lwipopts.h"
#include "test_lwip.h"
#include "test_lwip_seq_api.h"
#include "test_lwip_tcp_udp_echo_server.h"

#include "test_usbh.h"


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
static void TEST_Task(void const *argument)
{
	uint32_t i = 0;

	while(1)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "I am live!(%d)\r\n", i++);		
		HAL_Delay(1000);
	}	
}

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
	uart_init(921600);

	__HAL_RCC_GPIOC_CLK_ENABLE();

	//init put hub in reset status
	GPIO_InitStruct.Pin =      GPIO_PIN_9;
	GPIO_InitStruct.Mode =     GPIO_MODE_OUTPUT_PP;    
	GPIO_InitStruct.Pull =     GPIO_PULLUP;      
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	HAL_Delay(1);
	
	__PRINT_LOG__(__CRITICAL_LEVEL__, "freertos start!\r\n");

	osThreadDef(TEST_Task, TEST_Task, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(TEST_Task), NULL);

	//osThreadDef(start_usbh_thread, start_usbh_thread, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
    //osThreadCreate(osThread(start_usbh_thread), NULL);
    
#if LWIP_SOCKET
	__PRINT_LOG__(__CRITICAL_LEVEL__, "lwip socket start!\r\n");
	osThreadDef(start_lwip_thread, start_lwip_thread, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(start_lwip_thread), NULL);
#elif LWIP_NETCONN
	osThreadDef(start_lwip_thread_seq, start_lwip_thread_seq, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(start_lwip_thread_seq), NULL);
#endif

	osThreadDef(start_lwip_echo_thread, start_lwip_echo_thread, osPriorityNormal, 0, 2 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(start_lwip_echo_thread), NULL);

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
