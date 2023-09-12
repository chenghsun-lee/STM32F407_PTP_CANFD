/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LAB612_FREQUENCY_H
#define __LAB612_FREQUENCY_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "main.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/ 

#define ADC_CCR_ADDRESS    ((uint32_t)0x40012308)

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ADCConvert(void);
void UART_Output(int FreqCNT, float OutF, float Amp, float OutAngle);
 
#if OutputFormat == 1
#define PMUUARTLen 26
#define INTDigit 1000
#elif OutputFormat == 2
#define PMUUARTLen 33
#elif OutputFormat == 3
#define PMUUARTLen 29
#define INTDigit 1000
#endif
	
 
#ifdef __cplusplus
}
#endif

#endif /* __STM32F4x7_ETH_BSP_H */