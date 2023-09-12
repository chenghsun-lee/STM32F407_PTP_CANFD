/* Includes ------------------------------------------------------------------*/
#include "Lab612_ADC_109.h"
#include "main.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BUFFERSIZE  (1 *6 ) //  40KHz x6 x2 HT/TC at 1KHz  ** (1200 *6 )
#define ADC123 0

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint16_t ADC1ConvertedValue[2];
volatile uint16_t ADC2ConvertedValue[2];
volatile uint16_t ADC3ConvertedValue[2];
__IO uint16_t ADC1ConvertedValue[2] = { 0,0 };
__IO uint16_t ADC2ConvertedValue[2] = { 0,0 };
__IO uint16_t ADC3ConvertedValue[2] = { 0,0 };
__IO uint32_t ADCTripleConvertedValues[BUFFERSIZE];

/* Private functions ---------------------------------------------------------*/	
/* Private function prototypes -----------------------------------------------*/
void ADC_Config(void);
void ADC_NVIC_Config(void);
void ADC_Configuration_ADC1(void); 
void ADC_Configuration_ADC2(void);
void ADC_Configuration_ADC3(void);
void ADC_Configuration_ADC_Three_Phase(void);

/* Private functions ---------------------------------------------------------*/
void ADC_Config(void){
	/* Config NVIC*/
	ADC_NVIC_Config();
	ADC_Configuration_ADC_Three_Phase();
}


void ADC_NVIC_Config(void){	
	NVIC_InitTypeDef   NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);   //f4 	

	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
#if ADC123
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream4_IRQn; //ADC1
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn; //ADC2 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn; //ADC3
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#else
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn; //ADC1
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif 
}



void ADC_Configuration_ADC1(void)
{
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef       DMA_InitStructure;
	GPIO_InitTypeDef      GPIO_InitStructure;

	/* Enable ADCx, DMA and GPIO clocks ****************************************/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);


	DMA_DeInit(DMA2_Stream4);
	/* DMA2 Stream4 channel0 configuration **************************************/
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;//ADC1_DR_Address; //(uint32_t)ADCx_DR_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC1ConvertedValue; //(uint32_t)&uhADCxConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 2; //1;//2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream4, &DMA_InitStructure);

	DMA_ITConfig(DMA2_Stream4, DMA_IT_TC, ENABLE);

	DMA_Cmd(DMA2_Stream4, ENABLE);

	/* Configure ADC1 Channel8 pin (PB0) as analog input ******************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* Configure ADC1 Channel10 pin (PC0) as analog input ******************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	/* ADC Common Init **********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_TripleMode_RegSimult; //ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_2; //ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC1 Init ****************************************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE; //ENABLE; //DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;// DISABLE; //ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_CC1;//ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 2;//1; //2
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC3 regular channel7 configuration **************************************/
	//ADC_RegularChannelConfig(ADC3, ADC_Channel_7, 1, ADC_SampleTime_3Cycles);
	  /* ADC1 regular PB0 (ADC1 Channel8) configuration 								--ADCConvertedValue[0]*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_56Cycles);
	/* ADC1 regular PC0 (ADC1 Channel10) configuration 								--ADCConvertedValue[1]*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 2, ADC_SampleTime_56Cycles);

	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);


	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); //ENABLE);  //original f107

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);
	ADC_SoftwareStartConv(ADC1); // Start ADC1 conversion

}

void ADC_Configuration_ADC2(void)
{
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef       DMA_InitStructure;
	GPIO_InitTypeDef      GPIO_InitStructure;

	/* Enable ADCx, DMA and GPIO clocks ****************************************/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);


	DMA_DeInit(DMA2_Stream2);
	/* DMA2 Stream2 channel1 configuration **************************************/
	DMA_InitStructure.DMA_Channel = DMA_Channel_1;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC2->DR;//ADC1_DR_Address; //(uint32_t)ADCx_DR_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC2ConvertedValue; //(uint32_t)&uhADCxConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 2; //1;//2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);

	DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);

	DMA_Cmd(DMA2_Stream2, ENABLE);

	/* Configure ADC2 Channel4 pin (PA4) as analog input ******************************/
	/*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);*/
	/* Configure ADC2 Channel6 pin (PA6) as analog input ******************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	/* ADC Common Init **********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_TripleMode_RegSimult; //ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_2; //ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC2 Init ****************************************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE; //ENABLE; //DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;// DISABLE; //ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_CC1;//ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 2;//1; //2
	ADC_Init(ADC2, &ADC_InitStructure);

	/* ADC2 regular PA4 (ADC2 Channel4) configuration 								--ADCConvertedValue[0]*/
	ADC_RegularChannelConfig(ADC2, ADC_Channel_4, 1, ADC_SampleTime_56Cycles);
	/* ADC2 regular PA6 (ADC2 Channel6) configuration 								--ADCConvertedValue[1]*/
	ADC_RegularChannelConfig(ADC2, ADC_Channel_6, 2, ADC_SampleTime_56Cycles);

	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);

	/* Enable ADC2 DMA */
	ADC_DMACmd(ADC2, ENABLE);


	ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE); //ENABLE);  //original f107

	/* Enable ADC2 */
	ADC_Cmd(ADC2, ENABLE);
	ADC_SoftwareStartConv(ADC2); // Start ADC1 conversion

}

void ADC_Configuration_ADC3(void)
{
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef       DMA_InitStructure;
	GPIO_InitTypeDef      GPIO_InitStructure;

	/* Enable ADCx, DMA and GPIO clocks ****************************************/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);


	DMA_DeInit(DMA2_Stream0);
	/* DMA2 Stream0 channel2 configuration **************************************/
	DMA_InitStructure.DMA_Channel = DMA_Channel_2;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC3->DR;//ADC1_DR_Address; //(uint32_t)ADCx_DR_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC3ConvertedValue; //(uint32_t)&uhADCxConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 2; //1;//2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);

	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);

	DMA_Cmd(DMA2_Stream0, ENABLE);

	/* Configure ADC3 Channel12 pin (PC2) as analog input ******************************/
	/*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);*/
	/* Configure ADC3 Channel13 pin (PC3) as analog input ******************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	/* ADC Common Init **********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_TripleMode_RegSimult; //ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_2; //ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC3 Init ****************************************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE; //ENABLE; //DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;// DISABLE; //ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_CC1;//ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 2;//1; //2
	ADC_Init(ADC3, &ADC_InitStructure);

	/* ADC3 regular PC2 (ADC3 Channel2) configuration 								--ADCConvertedValue[0]*/
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_56Cycles);
	/* ADC3 regular PC3 (ADC3 Channel3) configuration 								--ADCConvertedValue[1]*/
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 2, ADC_SampleTime_56Cycles);

	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

	/* Enable ADC3 DMA */
	ADC_DMACmd(ADC3, ENABLE);


	ADC_ITConfig(ADC3, ADC_IT_EOC, ENABLE); //ENABLE);  //original f107

	/* Enable ADC3 */
	ADC_Cmd(ADC3, ENABLE);
	ADC_SoftwareStartConv(ADC3); // Start ADC1 conversion

}





void ADC_Configuration_ADC_Three_Phase(void)
{
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_InitTypeDef       ADC_InitStructure_slave;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef       DMA_InitStructure;
	GPIO_InitTypeDef      GPIO_InitStructure;

	/* Enable ADCx, DMA and GPIO clocks ****************************************/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	/* Enable ADCx, DMA and GPIO clocks ****************************************/
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

	/* Enable ADCx, DMA and GPIO clocks ****************************************/
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2  , ENABLE);
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

	DMA_DeInit(DMA2_Stream0);
	/* DMA2 Stream0 channel0 configuration ***ADC1*********************************/
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)0x40012308;//CDR_ADDRESS;//&ADC1->DR ;//ADC1_DR_Address; //(uint32_t)ADCx_DR_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADCTripleConvertedValues[0]; //(uint32_t)&uhADCxConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BUFFERSIZE;//3; //BUFFERSIZE; //1;//2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; //DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; //DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable; //DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	/******************/
	/*open dualbuffer mode*/
	DMA_DoubleBufferModeConfig(DMA2_Stream0, (uint32_t)ADCTripleConvertedValues, DMA_Memory_0);
	DMA_DoubleBufferModeCmd(DMA2_Stream0, ENABLE);
	/******************/
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
	//DMA_ITConfig(DMA2_Stream4, DMA_IT_TC | DMA_IT_HT, ENABLE);
	DMA_Cmd(DMA2_Stream0, ENABLE);

	/* Configure ADC1 Channel8 pin (PB0) as analog input ******************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* Configure ADC1 Channel10 pin (PC0) as analog input ******************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	/* Configure ADC2 Channel4 pin (PA4) as analog input ******************************/
  /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);*/
  /* Configure ADC2 Channel6 pin (PA6) as analog input ******************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure ADC3 Channel12 pin (PC2) as analog input ******************************/
  /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);*/
  /* Configure ADC3 Channel13 pin (PC3) as analog input ******************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* ADC Common Init **********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_TripleMode_RegSimult; //ADC_DualMode_RegSimult; //ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6; //ADC_Prescaler_Div6;     //ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1; //ADC_DMAAccessMode_1 ; //ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC1 Init ****************************************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE; //ENABLE; //DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;// DISABLE; //ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //ADC_ExternalTrigConvEdge_None;ADC_ExternalTrigConvEdge_Rising
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;//20181021 change;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 2;//1; //2
	ADC_Init(ADC1, &ADC_InitStructure);


	/*ADC_InitStructure_slave.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure_slave.ADC_ScanConvMode = DISABLE; //ENABLE; //DISABLE;
  ADC_InitStructure_slave.ADC_ContinuousConvMode = DISABLE;// DISABLE; //ENABLE;
  ADC_InitStructure_slave.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //ADC_ExternalTrigConvEdge_Rising; //ADC_ExternalTrigConvEdge_None;
  //ADC_InitStructure_slave.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T4_CC4;//ADC_ExternalTrigConv_T1_CC1;
  ADC_InitStructure_slave.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure_slave.ADC_NbrOfConversion = 1;//1; //2
	ADC_Init(ADC2, &ADC_InitStructure_slave); */
	ADC_Init(ADC2, &ADC_InitStructure);

	ADC_Init(ADC3, &ADC_InitStructure);

	/* ADC3 regular channel7 configuration **************************************/
	//ADC_RegularChannelConfig(ADC3, ADC_Channel_7, 1, ADC_SampleTime_3Cycles);
	  /* ADC1 regular PB0 (ADC1 Channel8) configuration 								--ADCConvertedValue[0]*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_3Cycles);
	/////ADC_InjectedChannelConfig(ADC1, ADC_Channel_8,  1, ADC_SampleTime_3Cycles);  
	/////ADC_RegularChannelConfig(ADC1, ADC_Channel_12,  1, ADC_SampleTime_3Cycles);  
	/* ADC1 regular PC0 (ADC1 Channel10) configuration 								--ADCConvertedValue[1]*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 2, ADC_SampleTime_3Cycles);

	/* ADC2 regular PA4 (ADC2 Channel4) configuration 								--ADCConvertedValue[0]*/
	ADC_RegularChannelConfig(ADC2, ADC_Channel_4, 1, ADC_SampleTime_3Cycles);
	/////ADC_InjectedChannelConfig(ADC2, ADC_Channel_4,  1, ADC_SampleTime_3Cycles);  
	/////ADC_RegularChannelConfig(ADC2, ADC_Channel_12,  1, ADC_SampleTime_3Cycles);  
	/* ADC2 regular PA6 (ADC2 Channel6) configuration 								--ADCConvertedValue[1]*/
	ADC_RegularChannelConfig(ADC2, ADC_Channel_6, 2, ADC_SampleTime_3Cycles);

	/* ADC3 regular PC2 (ADC3 Channel2) configuration 								--ADCConvertedValue[0]*/
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);
	/////ADC_RegularChannelConfig(ADC3, ADC_Channel_12,  1, ADC_SampleTime_3Cycles);  
	/* ADC3 regular PC3 (ADC3 Channel3) configuration 								--ADCConvertedValue[1]*/
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 2, ADC_SampleTime_3Cycles);



	/* Enable DMA request after last transfer (Multi-ADC mode)  */
	ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);

	/* Enable DMA request after last transfer (Single-ADC mode) */
  //ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

  /* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);
	/* Enable ADC2 DMA */
  //ADC_DMACmd(ADC2, ENABLE);
	/* Enable ADC3 DMA */
  //ADC_DMACmd(ADC3, ENABLE);


  //ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); //ENABLE);  //original f107
	//ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE);
	//ADC_ITConfig(ADC3, ADC_IT_EOC, ENABLE);

  /* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);
	ADC_Cmd(ADC2, ENABLE);
	ADC_Cmd(ADC3, ENABLE);
	//ADC_SoftwareStartConv(ADC1); // Start ADC1 conversion
	//ADC_SoftwareStartConv(ADC2); // Start ADC2 conversion
	//ADC_SoftwareStartConv(ADC3); // Start ADC3 conversion

}



