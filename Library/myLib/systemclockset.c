#ifndef __SYS_CLOCK_SET_H__
#define __SYS_CLOCK_SET_H__

#include <stdio.h>
#include "systemMyLib.h"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_rcc.h"
#include "systemclockset.h"
#include "systemlog.h"


//SystemClock_Config(25, 432, 2, 9);
void SystemClock_Config(uint32_t pll)
{
    RCC_OscInitTypeDef      RCC_OscInitStructure;// Oscillator
    RCC_ClkInitTypeDef      RCC_ClkInitStructure;
    
    HAL_StatusTypeDef       ret = HAL_OK;
    
    RCC_OscInitStructure.OscillatorType=RCC_OSCILLATORTYPE_HSE;    	//????HSE
    RCC_OscInitStructure.HSEState=RCC_HSE_ON;                      	//??HSE
	RCC_OscInitStructure.HSEPredivValue=RCC_HSE_PREDIV_DIV1;		//HSE???
    RCC_OscInitStructure.PLL.PLLState=RCC_PLL_ON;					//??PLL
    RCC_OscInitStructure.PLL.PLLSource=RCC_PLLSOURCE_HSE;			//PLL?????HSE
    RCC_OscInitStructure.PLL.PLLMUL=pll; 							//?PLL????
    ret=HAL_RCC_OscConfig(&RCC_OscInitStructure);//???
    
    if(ret!=HAL_OK) while(1);
    
    //??PLL???????????HCLK,PCLK1?PCLK2
    RCC_ClkInitStructure.ClockType=(RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStructure.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK;		//??????????PLL
    RCC_ClkInitStructure.AHBCLKDivider=RCC_SYSCLK_DIV1;				//AHB?????1
    RCC_ClkInitStructure.APB1CLKDivider=RCC_HCLK_DIV2; 				//APB1?????2
    RCC_ClkInitStructure.APB2CLKDivider=RCC_HCLK_DIV1; 				//APB2?????1
    ret=HAL_RCC_ClockConfig(&RCC_ClkInitStructure,FLASH_LATENCY_2);	//????FLASH?????2WS,???3?CPU???
		
    if(ret!=HAL_OK) while(1);
}

#endif
