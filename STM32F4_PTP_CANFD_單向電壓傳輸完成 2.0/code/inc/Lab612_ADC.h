/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LAB612_ADC_H
#define __LAB612_ADC_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/	

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ADC_Config(void);
void ADC1_2_Simu_DMA_Config(void); 
#ifdef __cplusplus
}
#endif

#endif /* __STM32F4x7_ETH_BSP_H */