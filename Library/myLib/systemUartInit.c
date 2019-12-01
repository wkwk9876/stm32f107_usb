#include <stdio.h>

#include "systemMyLib.h"
#include "systemUartInit.h"

#include "FreeRTOS.h"					//os ??	  

#if 1
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

int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);
	USART1->DR=(uint8_t)ch;      
	return ch;
}
#endif

#if 1  

uint8_t USART_RX_BUF[USART_REC_LEN];    

uint16_t USART_RX_STA=0;       

uint8_t aRxBuffer[RXBUFFERSIZE];
UART_HandleTypeDef UART2_Handler; 

void uart_init(uint32_t bound)
{    
    UART2_Handler.Instance=USART2;                        
    UART2_Handler.Init.BaudRate=bound;                    
    UART2_Handler.Init.WordLength=UART_WORDLENGTH_8B;   
    UART2_Handler.Init.StopBits=UART_STOPBITS_1;        
    UART2_Handler.Init.Parity=UART_PARITY_NONE;            
    UART2_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   
    UART2_Handler.Init.Mode=UART_MODE_TX_RX;            
    HAL_UART_Init(&UART2_Handler);                        
    
    HAL_UART_Receive_IT(&UART2_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE);  
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO????
    GPIO_InitTypeDef GPIO_Initure;
    
    if(huart->Instance==USART2)//?????1,????1 MSP???
    {
        __HAL_RCC_USART2_CLK_ENABLE();
  
        __HAL_RCC_GPIOD_CLK_ENABLE();
    
        GPIO_Initure.Pin=GPIO_PIN_5;            //PA9
        GPIO_Initure.Mode=GPIO_MODE_AF_PP;        //??????
        GPIO_Initure.Pull=GPIO_PULLUP;            //??
        GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;        //??
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);           //???PA9

        GPIO_Initure.Pin = GPIO_PIN_6;
        GPIO_Initure.Mode = GPIO_MODE_INPUT;
        GPIO_Initure.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOD, &GPIO_Initure);
        
#if EN_USART1_RX
        HAL_NVIC_EnableIRQ(USART2_IRQn);                //??USART1????
        HAL_NVIC_SetPriority(USART2_IRQn,3,3);            //?????3,????3
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
    
    HAL_UART_IRQHandler(&UART2_Handler);    //??HAL?????????
    
    timeout=0;
    while (HAL_UART_GetState(&UART2_Handler)!=HAL_UART_STATE_READY)//????
    {
        timeout++;////????
        if(timeout>maxDelay) break;        
    }
     
    timeout=0;
    while(HAL_UART_Receive_IT(&UART2_Handler,(uint8_t *)aRxBuffer, RXBUFFERSIZE)!=HAL_OK)//????????,?????????RxXferCount?1
    {
        timeout++; //????
        if(timeout>maxDelay) break;    
    }
} 
#endif    

/*??????????????????????????*/
/*

//??1??????
void USART1_IRQHandler(void)                    
{ 
    uint8_t Res;
#if SYSTEM_SUPPORT_OS         //??OS
    OSIntEnter();    
#endif
    if((__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_RXNE)!=RESET))  //????(?????????0x0d 0x0a??)
    {
        HAL_UART_Receive(&UART1_Handler,&Res,1,1000); 
        if((USART_RX_STA&0x8000)==0)//?????
        {
            if(USART_RX_STA&0x4000)//????0x0d
            {
                if(Res!=0x0a)USART_RX_STA=0;//????,????
                else USART_RX_STA|=0x8000;    //????? 
            }
            else //????0X0D
            {    
                if(Res==0x0d)USART_RX_STA|=0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
                    USART_RX_STA++;
                    if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//??????,??????      
                }         
            }
        }            
    }
    HAL_UART_IRQHandler(&UART1_Handler);    
#if SYSTEM_SUPPORT_OS         //??OS
    OSIntExit();                                               
#endif
} 
#endif    
*/
