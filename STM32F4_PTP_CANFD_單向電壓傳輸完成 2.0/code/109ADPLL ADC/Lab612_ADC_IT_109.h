/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LAB612_ADC_IT_109_H
#define __LAB612_ADC_IT_109_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/ 
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ADPLL_ADC_IT(void);
void DMA2_Stream0_IT(void);
void DMA2_Stream2_IT(void);
void DMA2_Stream0_2_IT(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4x7_ETH_BSP_H */