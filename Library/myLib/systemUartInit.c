#include <stdio.h>

#include "systemMyLib.h"
#include "systemUartInit.h"

#include "FreeRTOS.h"					//os ??	  
#include "task.h"

#pragma import(__use_no_semihosting)             
              
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       

void _sys_exit(int x) 
{ 
	x = x; 
} 

#ifdef PUT_CHAR_TO_NETWORK
#include "test_lwip.h"
#include "test_lwip_seq_api.h"
extern struct TX_buffer_manage * txbuf;
#endif

int fputc(int ch, FILE *f)
{ 	
	while((USARTx->SR&0X40)==0);
	USARTx->DR=(uint8_t)ch;   
#ifdef PUT_CHAR_TO_NETWORK
	uint32_t mask;
	
	mask = taskENTER_CRITICAL_FROM_ISR();

	if(NULL != txbuf)
	{
		txbuf->tx_buffer[txbuf->p_write & TX_BUFFER_MASK] = ch;
		++txbuf->p_write;
	}

	taskEXIT_CRITICAL_FROM_ISR(mask);
#endif
	return ch;
}


uint8_t		USART_RX_BUF[USART_REC_LEN];    
uint16_t	USART_RX_STA = 0;       
uint8_t		aRxBuffer[RXBUFFERSIZE];

UART_HandleTypeDef DEBUG_UART_Handler; 

void uart_init(uint32_t bound)
{    
    DEBUG_UART_Handler.Instance=USARTx;                        
    DEBUG_UART_Handler.Init.BaudRate=bound;                    
    DEBUG_UART_Handler.Init.WordLength=UART_WORDLENGTH_8B;   
    DEBUG_UART_Handler.Init.StopBits=UART_STOPBITS_1;        
    DEBUG_UART_Handler.Init.Parity=UART_PARITY_NONE;            
    DEBUG_UART_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   
    DEBUG_UART_Handler.Init.Mode=UART_MODE_TX_RX;            
    HAL_UART_Init(&DEBUG_UART_Handler);                        
    
    HAL_UART_Receive_IT(&DEBUG_UART_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE);  
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO????
    GPIO_InitTypeDef GPIO_InitStruct;
    
    if(huart->Instance==USARTx)
    {
		USARTx_TX_GPIO_CLK_ENABLE();
		USARTx_RX_GPIO_CLK_ENABLE();

		__HAL_RCC_AFIO_CLK_ENABLE();

		USARTx_REMAP_ENABLE();
		//AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;

		/* Enable USARTx clock */
		USARTx_CLK_ENABLE();

		/* UART TX GPIO pin configuration  */
		GPIO_InitStruct.Pin       = USARTx_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

		HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

		/* UART RX GPIO pin configuration  */
		GPIO_InitStruct.Pin = USARTx_RX_PIN;

		HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
        
#if USARTx_RX_ENABLE
        HAL_NVIC_EnableIRQ(USARTx_IRQ);               
        HAL_NVIC_SetPriority(USARTx_IRQ,3,3);         
#endif    
    }

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance==USART2)//?????1
    {
        if((USART_RX_STA&0x8000)==0)//?????
        {
            if(USART_RX_STA&0x4000)//????0x0d
            {
                if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//????,????
                else USART_RX_STA|=0x8000;    //????? 
            }
            else //????0X0D
            {    
                if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
                    USART_RX_STA++;
                    if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//??????,??????      
                }         
            }
        }

    }
}
 
//??1??????
void USART2_IRQHandler(void)                    
{ 
    uint32_t timeout=0;
    uint32_t maxDelay=0x1FFFF;
    
    HAL_UART_IRQHandler(&DEBUG_UART_Handler);   
    
    timeout=0;
    while (HAL_UART_GetState(&DEBUG_UART_Handler)!=HAL_UART_STATE_READY)
    {
        timeout++;
        if(timeout>maxDelay) break;        
    }
     
    timeout=0;
    while(HAL_UART_Receive_IT(&DEBUG_UART_Handler,(uint8_t *)aRxBuffer, RXBUFFERSIZE)!=HAL_OK)
    {
        timeout++; 
        if(timeout>maxDelay) break;    
    }
}  


