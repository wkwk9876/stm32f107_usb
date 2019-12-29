#ifndef __SYS_UART_INIT__
#define __SYS_UART_INIT__

#include <stdio.h>

#include "systemMyLib.h"

#define PUT_CHAR_TO_NETWORK

#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USARTx_REMAP_ENABLE()	 		 __HAL_AFIO_REMAP_USART2_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_5
#define USARTx_TX_GPIO_PORT              GPIOD
#define USARTx_RX_PIN                    GPIO_PIN_6
#define USARTx_RX_GPIO_PORT              GPIOD

//#define USARTx_RX_ENABLE
#define USARTx_IRQ						 USART2_IRQn


#define USART_REC_LEN  			200  	
#define EN_USART1_RX 			1	
#define RXBUFFERSIZE   			1 
	  	
extern uint8_t                  USART_RX_BUF[USART_REC_LEN]; 
extern uint16_t                 USART_RX_STA;         		
extern UART_HandleTypeDef       UART1_Handler;
extern uint8_t                  aRxBuffer[RXBUFFERSIZE];

void uart_init(uint32_t bound);

#endif
