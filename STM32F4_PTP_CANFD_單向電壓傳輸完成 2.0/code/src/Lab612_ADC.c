/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Lab612_ADC.h"
//#include "scopeConfig.h"
#include "arm_math.h" 
#include <stdio.h>
#include "stm32f4xx.h"

/** @addtogroup ADC_DualADC_RegulSimu_DMAmode1
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC_CCR_ADDRESS    ((uint32_t)0x40012308)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint16_t ADCDualConvertedValue[2];

/* Private function prototypes -----------------------------------------------*/
void DMA_Config(void);
void GPIO_Config(void);
void ADC1_CH9_Config(void);
void ADC2_CH12_Config(void);
void ADC1_ChVbat_DMA_Config(void); //No define
void ADC1_2_Simu_DMA_Config(void);


/* Private functions ---------------------------------------------------------*/
void ADC_Config(void){	
	/* ADC1 configuration */
	ADC1_2_Simu_DMA_Config();	
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConv(ADC1);
	ADC_SoftwareStartConv(ADC2); 
}


/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
void ADC1_2_Simu_DMA_Config()
{
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  /*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */

  /* Enable peripheral clocks *************************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);

  /* DMA2 Stream0 channel0 configuration **************************************/
  DMA_Config();
  
  /* ADCs configuration ------------------------------------------------------*/
  /* Configure ADC Channel 9, 12 pin as analog input */
  GPIO_Config();

  /* ADC Common Init */
  ADC_CommonInitStructure.ADC_Mode = ADC_DualMode_RegSimult;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode =   ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);
  

  /* ADC1 regular channels 9 configuration */
  ADC1_CH9_Config();

  /* ADC2 regular channels 12 configuration */
  ADC2_CH12_Config(); 

  /* Enable DMA request after last transfer (Multi-ADC mode)  */
  ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE); 

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC2 */
  ADC_Cmd(ADC2, ENABLE);

}

/**
  * @brief  ADC1 regular channels 9 configuration
  * @param  None
  * @retval None
  */
void ADC1_CH9_Config(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; //ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //ENABLE;	
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC4; //ADC_ExternalTrigInjecConv_T2_CC1;//20220501 change;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 2;//1; 
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channels 10 configuration */ 
  //ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_84Cycles); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_3Cycles);
  //ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_3Cycles);
}

/**
  * @brief  ADC2 regular channels 12 configuration
  * @param  None
  * @retval None
  */
void ADC2_CH12_Config(void)
{
  ADC_InitTypeDef ADC_InitStructure;

  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; //ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC4; //ADC_ExternalTrigInjecConv_T2_CC1;//20220501 change;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 2;//1;
  ADC_Init(ADC2, &ADC_InitStructure);

  /* ADC2 regular channels 12 configuration */ 
  //ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1, ADC_SampleTime_84Cycles);
  ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);
  //ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);
}
/**
  * @brief  DMA Configuration
  * @param  None
  * @retval None
  */
void DMA_Config(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  DMA_InitStructure.DMA_Channel = DMA_Channel_0; 
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADCDualConvertedValue;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC_CCR_ADDRESS; 
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = DMA_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; //DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; //DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable; 		
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);

  DMA_DoubleBufferModeConfig(DMA2_Stream0, (uint32_t)ADCDualConvertedValue, DMA_Memory_0);  
  DMA_DoubleBufferModeCmd(DMA2_Stream0, ENABLE);
	
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
  
  /* DMA2_Stream0 enable */
  DMA_Cmd(DMA2_Stream0, ENABLE);
}

/**
  * @brief Configure ADC Channels 9, 12 pins as analog inputs
  * @param  None
  * @retval None
  */
void GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
	 /*
     ADC Channel 9 -> PB1
     ADC Channel 12 -> PC2
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}