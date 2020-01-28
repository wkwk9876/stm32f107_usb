#ifndef __TEST_USBH_H__
#define __TEST_USBH_H__

#include "usbh_def.h"

/* 模组 PWRKEY 对应的 GPIO 端口 */       
#define __MOD_EC20_PWRKEY_GROUP__          GPIOD
#define __MOD_EC20_PWRKEY_PIN__            GPIO_PIN_0
                                         
/* 模组RST管脚对应的GPIO端口 */          
#define __MOD_EC20_RST_GROUP__             GPIOD
#define __MOD_EC20_RST_PIN__               GPIO_PIN_1

/* 模组STATUS管脚对应的GPIO端口。当模块正常开机时，STATUS 会输出低电平。否则，STATUS 变为高阻抗状态。该端口被配置为内部上拉。 */          
#define __MOD_EC20_STATUS_GROUP__          GPIOD
#define __MOD_EC20_STATUS_PIN__            GPIO_PIN_3


typedef struct
{
	uint8_t              ClassCode; 
	USBH_StatusTypeDef  (*new_app)        (struct _USBH_HandleTypeDef *phost);
	USBH_StatusTypeDef  (*start_app)      (struct _USBH_HandleTypeDef *phost);
	USBH_StatusTypeDef  (*stop_app)       (struct _USBH_HandleTypeDef *phost);
	USBH_StatusTypeDef  (*delete_app)     (struct _USBH_HandleTypeDef *phost);
}Usb_Application_Class;


void start_usbh_thread(void const * argument);
#endif

