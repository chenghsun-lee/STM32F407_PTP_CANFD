/* Includes ------------------------------------------------------------------*/
#include "Lab612_TIM.h"
#include "main.h"
//For DMA
#include "scopeConfig.h" 
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

//For PTP
uint32_t Tim4Period;
uint32_t Tim2Period;
uint32_t Tim2Prescaler;

/* Private functions ---------------------------------------------------------*/	
/* Private function prototypes -----------------------------------------------*/
void Timer_NVIC_Config();
void Timer_Config(void);
void TIM2_Configuration(void);

/* Private functions ---------------------------------------------------------*/
void Timer_Config(void){
	/* Config NVIC*/
	Timer_NVIC_Config();
	
	Tim2Prescaler = 1;
	Tim2Period = 84000000/SAMPLE_RATE;
	TIM2_Configuration(); 

}


void Timer_NVIC_Config(void){	
	NVIC_InitTypeDef   NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);   //f4 
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Init(&NVIC_InitStructure);	


#ifdef ADCOn

	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

}



/**
  * @brief  TIM2,TIM4,TIM5,TIM7 for PTP. Tim3 for ADC.
  * @param  None
  * @retval None
  */

 /* PB5 Trigger PA15 ,Output PA3 */
void TIM2_Configuration(void)
{
	//RCC_ClocksTypeDef RCC_Clocks;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure_TIM2;
	TIM_OCInitTypeDef TIM_OCInitStructure_TIM2;

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* length of the pulse in us (APB1 Clock : 84MHz) */
	TIM_TimeBaseStructure_TIM2.TIM_Period = Tim2Period - 1; //Set ARR Value
	TIM_TimeBaseStructure_TIM2.TIM_Prescaler = Tim2Prescaler - 1;
	TIM_TimeBaseStructure_TIM2.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure_TIM2.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure_TIM2);

	/* Configure TIM2 channel 4 as output : PA3 */
	TIM_OCInitStructure_TIM2.TIM_OCMode = TIM_OCMode_PWM1; //CNT < CCR ωȃڱǬ
	TIM_OCInitStructure_TIM2.TIM_OutputState = TIM_OutputState_Enable; //׽ҒűٻƜР
	TIM_OCInitStructure_TIM2.TIM_Pulse = Tim2Period/4; //set CCR value
	TIM_OC2Init(TIM2, &TIM_OCInitStructure_TIM2);	
	
	TIM_OC4Init(TIM2, &TIM_OCInitStructure_TIM2);	


#ifdef PWMTimestamp
	/* OC2 fast enable */
	TIM_OC2FastConfig(TIM2, TIM_OCFast_Enable);
#endif

	/* OC4 fast enable */
	TIM_OC4FastConfig(TIM2, TIM_OCFast_Enable);

	/* ƾӡĲ֯PA15 */
	TIM_SelectInputTrigger(TIM2, TIM_TS_ETRF);
	TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Trigger); //20220623 ADPLL 
	
	
	/* Set TIM2_CC2 : output PB3 */
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_OC2Ref);
	/* Set TIM2_CC4 : output */
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_OC4Ref);
	TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);

	
	/* Enables or Disable the TIM2 Preload on ARR Register */
	TIM_ARRPreloadConfig(TIM2, ENABLE); //ARPE bit in the TIMx_CR1 register ENABLE
	/* Enable TIM2_IT_Trigger */	
	TIM_ITConfig(TIM2, TIM_IT_CC1|TIM_IT_Trigger, ENABLE);	
	
	TIM_SetCounter(TIM2, 0);
	/* CEN Enable */
	TIM_Cmd(TIM2, ENABLE);

}





