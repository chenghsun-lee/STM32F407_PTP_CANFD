/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Lab612_TIM_IT.h"
#include <time.h>
#include <math.h>
#include "ethernetif.h"
#include "ptpd.h"
/**********LAB612 ADC************/
#include "Lab612_ADC.h"

//20230814 CANFD
#include <asf.h>
#include "app.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/




/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//TIM2
extern uint32_t Tim2Period;

uint16_t TIM2WaveCNT = 0;



/* Private functions ---------------------------------------------------------*/

/* 負責調變PA3的長度---------------------------------------------*/

//For Check
int InTheState = 0;
int ADPLL_Lock = 0;
uint32_t LockTime = 0;

extern int32_t OffsetSecond, OffsetNanosecond;

void TIM2_IT_PA3_ADPLL(void){	
	/* Reset the Counter Register value */
	TIM2->CNT = 0; //20220623 ADPLL

	//如果Timer被中斷
	if (TIM_GetITStatus(TIM2, TIM_IT_Trigger) != RESET){
		//printf("T2 %d\n",TIM2WaveCNT);
		if(!ADPLL_Lock){	
			
			if((abs(OffsetSecond) == 0) && (abs(OffsetNanosecond) <= PTPOffsetWorkRange) && abs(OffsetNanosecond) ){
				if(TIM2WaveCNT == SAMPLE_RATE){
					InTheState = 1;
					Tim2Period--;				
				}
				else if(TIM2WaveCNT > SAMPLE_RATE){
#ifdef PSADPLL
					Tim2Period += (TIM2WaveCNT - SAMPLE_RATE);
#else
					Tim2Period++; //20221030 P Control					
#endif						
			
					if(InTheState){								
						ADPLL_Lock = 1;										
					}
				}
				else{
#ifdef PSADPLL
					Tim2Period += (TIM2WaveCNT - SAMPLE_RATE);
#else
					Tim2Period--; //20221030 P Control
#endif			
		
				}
				TIM_SetAutoreload(TIM2, Tim2Period);		
				LockTime++;			
			}
		}
		else{	
			GPIO_SetBits(GPIOD, GPIO_Pin_12);	
		}
		
		//為了測試ADPLL平均補償
		TIM2WaveCNT = 0;
	}	
	
	//如果Timer正常數完ARR
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET){	

		TIM2WaveCNT++;
		TIM_SetAutoreload(TIM2, Tim2Period);
		
	}
	//重置	
	TIM_ClearITPendingBit(TIM2, TIM_IT_Trigger|TIM_IT_CC1);
}



/* Handle ADC---------------------------------------------------*/

#ifdef ADCOn
//DMA
uint16_t V1[2] = {0};
uint16_t SmpCnt1 = 0;
extern __IO uint16_t ADCDualConvertedValue[2];
int DualState = 0;


void DMA2_Stream0_IT(void){

	if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0)){

		if(ADPLL_Lock){
			
			V1[DualState] = ADCDualConvertedValue[0];
			
			if(TIM2WaveCNT <= SAMPLE_RATE / DEBUG_LED_FLASH_Frequecy / 2){
				GPIO_SetBits(GPIOD, GPIO_Pin_14);	
			}
			else{
				GPIO_ResetBits(GPIOD, GPIO_Pin_14);	
			}
			
			if(DualState){				
				SmpCnt1 = TIM2WaveCNT;
				APP_TransmitSwitchState();			
			}
			
			DualState = (DualState) ? 0 : 1;			

		}
		
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
	}

}
#endif