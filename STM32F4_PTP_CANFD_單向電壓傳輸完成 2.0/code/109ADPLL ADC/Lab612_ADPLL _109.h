/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LAB612_ADPLL_H
#define __LAB612_ADPLL_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/ 
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ADPLL_TIM2(void);
void ADPLL_TIM3(void);
void ADPLL_TIM4(void);
void ADPLL_EXTI0(void);
void ADPLL_ADC_IT(void);
void ADPLL_DMA2_Stream0(void);
void ADPLL_DMA2_Stream2(void);
void ADPLL_DMA2_Stream0_2(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4x7_ETH_BSP_H */