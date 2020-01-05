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
  RCC_ClkInitTypeDef clkinitstruct = { 0 };
  RCC_OscInitTypeDef oscinitstruct = { 0 };
  RCC_PeriphCLKInitTypeDef rccperiphclkinit = { 0 };

  /* Configure PLLs ------------------------------------------------------ */
  /* PLL2 configuration: PLL2CLK = (HSE / HSEPrediv2Value) * PLL2MUL = (25 / 5) 
   * 8 = 40 MHz */
  /* PREDIV1 configuration: PREDIV1CLK = PLL2CLK / HSEPredivValue = 40 / 5 = 8
   * MHz */
  /* PLL configuration: PLLCLK = PREDIV1CLK * PLLMUL = 8 * 9 = 72 MHz */
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  oscinitstruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState = RCC_HSE_ON;
  oscinitstruct.HSEPredivValue = RCC_HSE_PREDIV_DIV3;//RCC_HSE_PREDIV_DIV5;
  oscinitstruct.PLL.PLLMUL = RCC_PLL_MUL9;
  oscinitstruct.Prediv1Source = RCC_PREDIV1_SOURCE_PLL2;

  oscinitstruct.PLL.PLLState = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL2.PLL2State = RCC_PLL2_ON;
  oscinitstruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
  oscinitstruct.PLL2.PLL2MUL = RCC_PLL2_MUL8;

  if (HAL_RCC_OscConfig(&oscinitstruct) != HAL_OK)
  {
    /* Start Conversation Error */
    //Error_Handler();
  }

  /* USB clock selection */
  rccperiphclkinit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  rccperiphclkinit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV3;
  HAL_RCCEx_PeriphCLKConfig(&rccperiphclkinit);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
   * clocks dividers */
  clkinitstruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                            RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;

  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2) != HAL_OK)
  {
    /* Start Conversation Error */
    //Error_Handler();
  }
}

#endif
