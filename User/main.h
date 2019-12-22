/**
  ******************************************************************************
  * @file    Templates/Inc/main.h 
  * @author  MCD Application Team
  * @brief   Header for main.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
//#include "stm3210c_eval.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


/* 模组 PWRKEY 对应的 GPIO 端口 */       
#define __MOD_EC20_PWRKEY_GROUP__          GPIOD
#define __MOD_EC20_PWRKEY_PIN__            GPIO_PIN_0
                                         
/* 模组RST管脚对应的GPIO端口 */          
#define __MOD_EC20_RST_GROUP__             GPIOD
#define __MOD_EC20_RST_PIN__               GPIO_PIN_1

/* 模组STATUS管脚对应的GPIO端口。当模块正常开机时，STATUS 会输出低电平。否则，STATUS 变为高阻抗状态。该端口被配置为内部上拉。 */          
#define __MOD_EC20_STATUS_GROUP__          GPIOD
#define __MOD_EC20_STATUS_PIN__            GPIO_PIN_3


#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
