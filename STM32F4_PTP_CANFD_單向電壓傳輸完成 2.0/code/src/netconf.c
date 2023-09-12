///**
//  ******************************************************************************
//  * @file    netconf.c
//  * @author  MCD Application Team
//  * @version V1.0.0
//  * @date    31-October-2011
//  * @brief   Network connection configuration
//  ******************************************************************************
//  * @attention
//  *
//  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
//  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
//  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
//  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
//  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
//  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
//  *
//  * <h2><center>&copy; Portions COPYRIGHT 2011 STMicroelectronics</center></h2>
//  ******************************************************************************
//  */
///**
//  ******************************************************************************
//  * <h2><center>&copy; Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.</center></h2>
//  * @file    netconf.c
//  * @author  CMP Team
//  * @version V1.0.0
//  * @date    28-December-2012
//  * @brief   Network connection configuration     
//  *          Modified to support the STM32F4DISCOVERY, STM32F4DIS-BB and
//  *          STM32F4DIS-LCD modules. 
//  ******************************************************************************
//  * @attention
//  *
//  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
//  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
//  * TIME. AS A RESULT, Embest SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
//  * OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
//  * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
//  * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
//  ******************************************************************************
//  */
///* Includes ------------------------------------------------------------------*/
#include "ethernetif.h"
#include "netconf.h"
#include "stm32f4x7_eth.h"
#include "lwip/sys.h"
#include <string.h>

sys_mutex_t MAC_frame_mutex;
struct netMACif xnetMACif; /* network interface structure */
extern MACFrameArray MACFrame_Array;
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void MAC_Init(void)
{
	memset(MAC_frame_mutex.data, 0, sizeof(int32_t)*3);
	MAC_frame_mutex.def.mutex = MAC_frame_mutex.data;
	MAC_frame_mutex.id = osMutexCreate(&MAC_frame_mutex.def);
	if (MAC_frame_mutex.id == NULL)
	  while(1);
	MACFrame_Array.head = 0;
	MACFrame_Array.tail = 0;
  low_level_init_test(&xnetMACif);		
  netMACif_set_default(&xnetMACif);
}
