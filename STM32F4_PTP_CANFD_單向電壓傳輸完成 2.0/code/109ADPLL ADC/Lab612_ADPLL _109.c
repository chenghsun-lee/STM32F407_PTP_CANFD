/* Includes ------------------------------------------------------------------*/
#include "Lab612_ADPLL.h"
#include "Lab612_ADPLL_SP.h"
#include "main.h"
#include <time.h>
#include "ethernetif.h"
/* Private typedef -----------------------------------------------------------*/
struct ptptime_t ptptime_UART;
struct ptptime_t ptptime_ADPLL_T3;
struct ptptime_t ptptime_ADPLL_T4;
/* Private define ------------------------------------------------------------*/
#define BUFFERSIZE  (1 * 6) //  40KHz x6 x2 HT/TC at 1KHz   ** (1200 * 6)
#define buffer_max  1 //1200 => 4800Hz //1920 => 3840Hz  //original 1200 justin
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint32_t ADCTripleConvertedValues[BUFFERSIZE];
extern __IO uint16_t ADC1ConvertedValue[2];
extern __IO uint16_t ADC2ConvertedValue[2];
extern __IO uint16_t ADC3ConvertedValue[2];
extern unsigned int os_time;

extern volatile uint16_t ADC1ConvertedValue[2];
extern volatile uint16_t ADC2ConvertedValue[2];
extern volatile uint16_t ADC3ConvertedValue[2];


int32_t time_PTP;
int32_t time_ADPLL;

__IO uint32_t cnt1 = 0;
__IO uint32_t cnt2 = 0;
__IO uint32_t cnt_adc1 = 0;
__IO uint32_t cnt_dma1 = 0;
__IO uint32_t cnt_adc2 = 0;
__IO uint32_t cnt_dma2 = 0;
__IO uint32_t cnt_adc3 = 0;
__IO uint32_t cnt_dma3 = 0;
__IO uint32_t cnt_adc123 = 0;
__IO uint16_t t2cnt = 0;
__IO uint16_t Tim4_Fix = 0;

__IO uint32_t Tim4cnt_n1 = 0;
__IO uint32_t Tim2cnt = 0;
__IO uint32_t Tim2cnt_time = 0;
__IO uint16_t Tim4cnt = 0;
__IO uint16_t Tim4cnt_n = 0;
__IO uint16_t Print_cnt_adc1 = 0;
__IO uint16_t Print_cnt_dma1 = 0;
__IO uint16_t Print_cnt_adc2 = 0;
__IO uint16_t Print_cnt_dma2 = 0;
__IO uint16_t Print_cnt_adc3 = 0;
__IO uint16_t Print_cnt_dma3 = 0;

#define DirectSetPA3Freq 4800


#if DirectSetPA3Freq == 1000
__IO uint32_t Tim2Prescaler = 5;
__IO uint32_t Tim2Period = 16800;
#elif DirectSetPA3Freq == 4800
__IO uint32_t Tim2Prescaler = 1*5;
__IO uint32_t Tim2Period = 17500/5;
#elif DirectSetPA3Freq == 9600
__IO uint32_t Tim2Prescaler = 1;
__IO uint32_t Tim2Period = 17500/2;
#endif
__IO uint32_t Tim2Frequency = 240000;

__IO uint32_t Tim4Period = 8400;  // 84M/1750=48000Hz
__IO uint32_t Tim4Frequency = 10000;
__IO uint32_t Tim2fix = 0;

/* 20210316 */
TimeInternal ADPLL_Timestamp;
uint8_t T2 = 0;
u16 ADC1_dma_value1[buffer_max];
u16 ADC1_dma_value2[buffer_max];
u16 ADC2_dma_value1[buffer_max];
u16 ADC2_dma_value2[buffer_max];
u16 ADC3_dma_value1[buffer_max];
u16 ADC3_dma_value2[buffer_max];
#define systic (*((volatile unsigned int *)0xE000E018))
	
/* Private functions ---------------------------------------------------------*/



/* Private function prototypes -----------------------------------------------*/
extern void xPortSysTickHandler(void);
extern uint32_t svcKernelSysTick(void);
extern unsigned int  os_tick_val(void);
extern u16 readADC1(u8 channel);
void ADPLL_TIM2(void);
void ADPLL_TIM3(void);
void ADPLL_TIM4(void);
void ADPLL_EXTI0(void);
void ADPLL_ADC_IT(void);
void ADPLL_DMA2_Stream0(void);
void ADPLL_DMA2_Stream2(void);
void ADPLL_DMA2_Stream0_2(void);

/* Private functions ---------------------------------------------------------*/

void ADPLL_TIM2(){
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET){
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
		cnt2++;
		if (cnt2 == Tim2fix){
			if (T2 == 1){
				TIM_SetAutoreload(TIM2, Tim2Period - 1);
				Tim2Period--;
			}
			else if (T2 == 2){
				TIM_SetAutoreload(TIM2, Tim2Period + 1);
				Tim2Period++;
			}
			else{			
				TIM_SetAutoreload(TIM2, Tim2Period);
			}
			
		}
	
	}
	
}


void ADPLL_TIM3(){
	if (TIM_GetITStatus(TIM3, TIM_IT_Trigger) != RESET){
		
		TIM_ClearITPendingBit(TIM3, TIM_IT_Trigger);
		
		
		/* It can help user views TIM2 Trigger ~ TIM2 IRQ Time */
		Tim4cnt_n = TIM_GetCounter(TIM4);
		ADPLL_Timestamp.nanoseconds = 0;
		ADPLL_Timestamp.seconds++;
		t2cnt++;
		/* Clear TIM2 Trigger interrupt */
		Tim4cnt = cnt1;
		Tim2cnt = cnt2;
		if (Tim4cnt != 0){
			/* If Phase Lead */
			if (Tim4cnt > Tim4Frequency){
				TIM_SetAutoreload(TIM4, Tim4Period + 1);
				Tim4Period++;
			}

			/* If Phase Delay */
			else if (Tim4cnt < Tim4Frequency){
				TIM_SetAutoreload(TIM4, Tim4Period - 1);
				Tim4Period--;
			}

			/* Phase State */
			else{
				TIM_SetAutoreload(TIM4, Tim4Period);
			}
		}
		if (Tim2cnt != 0){
			if (Tim2cnt > Tim2Frequency){
				TIM_SetAutoreload(TIM2, Tim2Period + 1);
				Tim2Period++;
				T2 = 0;
			}

			/* If Phase Delay */
			else if (Tim2cnt < Tim2Frequency){
				TIM_SetAutoreload(TIM2, Tim2Period - 1);
				Tim2Period--;
				T2 = 0;
			}

			/* Phase State */
			else{				
				//printf("%d\n",t2cnt);
				if (t2cnt > 10){
					if (Tim2cnt > Tim2Frequency){
						Tim2fix = (Tim2cnt - Tim2Frequency) * Tim2Period;
						TIM_SetAutoreload(TIM2, Tim2Period + 1);
						Tim2Period++;
					}
					else if (Tim2cnt < Tim2Frequency){
						Tim2fix = (Tim2Frequency - Tim2cnt) * Tim2Period;
						TIM_SetAutoreload(TIM2, Tim2Period - 1);
						Tim2Period--;
					}
					
					else{
						Tim2fix = 0;
						TIM_SetAutoreload(TIM2, Tim2Period + 1);
						Tim2Period++;
						T2 = 1;
					
					}						
				}
				else{
					TIM_SetAutoreload(TIM2, Tim2Period);
					Tim2fix = 0;
				}
				
			}
		}
		/* Reset TIM Counter */
		cnt1 = 0;
		cnt2 = 0;
		Print_cnt_adc1 = cnt_adc1;
		cnt_adc1 = 0;
		cnt_adc2 = 0;
		cnt_adc3 = 0;

		Print_cnt_dma1 = cnt_dma1;
		cnt_dma1 = 0;
		cnt_dma2 = 0;
		cnt_dma3 = 0;

	}	
}
	

void ADPLL_TIM4(){
#ifdef Early_Print_Time_Mechanism
	TIMDisplayTimeToUSART();
#endif	
	
	if (TIM_GetITStatus(TIM4, TIM_IT_Trigger) != RESET){
		TIM_ClearITPendingBit(TIM4, TIM_IT_Trigger);
		

		Tim4_Fix = TIM_GetCounter(TIM4);
		TIM_SetCounter(TIM4, Tim4_Fix + Tim4cnt_n);

	}
	if (TIM_GetITStatus(TIM4, TIM_IT_CC4) != RESET){
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);
		

		cnt1++;
		if (cnt1 == Tim4Frequency)		{
			ETH_PTPTime_GetTime(&ptptime_ADPLL_T4);
			Tim4cnt_n1 = ptptime_ADPLL_T4.tv_nsec;
		}

	}
	
}	


uint16_t button = 0;	
void ADPLL_EXTI0(void)
{
	if (EXTI_GetFlagStatus(EXTI_Line0) == SET)
	{
		//time_ADPLL = ADPLL_Timestamp.nanoseconds;
		//ETH_PTPTime_GetTime(&ptptime_UART);
		//time_PTP = ptptime_UART.tv_nsec;
		//button = 1;
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}





void ADPLL_ADC_IT(void){
	/* Get injected channel_8 converted value */
	//ADC1ConvertedValue[0] = ADC_GetConversionValue(ADC1);
	  //ADC1ConvertedValue[1] = ADC_GetConversionValue(ADC1);
	int ii;

	//	 printf("ADCIRQ\n");
		// ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

		///ADC1ConvValue[ii++] = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);
	  /* Clear ADC1 JEOC pending interrupt bit */
	  //ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
	/*	if(cnt_adc< 1024)
		{
		ADC1ConvValue[cnt_adc]=readADC1(ADC_Channel_8);   //PB0 (ADC Channel8)
		//ADC1ConvValue[cnt_adc]=readADC1(ADC_Channel_14);  //PC4 (ADC Channel14) potentiometer
		}
	*/
	////if(ADC_GetITStatus(ADC1,ADC_IT_EOC)){	
	cnt_adc1++; //adc counter 
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC); //clear
////}
	////if(ADC_GetITStatus(ADC2,ADC_IT_EOC)){	
	cnt_adc2++; //adc counter 
	ADC_ClearITPendingBit(ADC2, ADC_IT_EOC); //clear
////}
	////if(ADC_GetITStatus(ADC3,ADC_IT_EOC)){	
	cnt_adc3++; //adc counter 
	ADC_ClearITPendingBit(ADC3, ADC_IT_EOC); //clear
////}
	//printf("ADC1=%d ,%d \n",ADC1ConvertedValue[0],ADC1ConvertedValue[1]);	
	//printf("ADC1=%d ,%d \n",ADC1ConvertedValue[0],ADC1ConvertedValue[1]);
	//ADC1ConvValue[]=readADC1(ADC_Channel_8);

	//printf("ADC1ConvValue=%d \n",ADC1ConvValue);
}


void ADPLL_DMA2_Stream0(void)
{
	/*	int iii;
		//printf("AABBCC\n");
		 if(DMA_GetITStatus(DMA2_Stream4,DMA_IT_TCIF4))
	  {
			cnt_dma1++;

			if(cnt_dma1 < buffer_max)
				{
					ADC1_dma_value1[cnt_dma1]=ADC1ConvertedValue[0];
					ADC1_dma_value2[cnt_dma1]=ADC1ConvertedValue[1];
					//printf("ADCV = %d\n",ADCConvertedValue[0]);
				}

		DMA_ClearITPendingBit(DMA2_Stream4,DMA_IT_TCIF4);
		}
	*/
	/* Test on DMA Stream Half Transfer interrupt */
#if 1 
	if (DMA_GetITStatus(DMA2_Stream4, DMA_IT_HTIF4))
	{
		/* Clear DMA Stream Half Transfer interrupt pending bit */
		DMA_ClearITPendingBit(DMA2_Stream4, DMA_IT_HTIF4);
		//printf("\nA\n%d %d %d\n",ADCTripleConvertedValues[cnt_adc123+0],ADCTripleConvertedValues[cnt_adc123+1],ADCTripleConvertedValues[cnt_adc123+2]);
	/* Turn LED3 off: Half Transfer */
	//STM_EVAL_LEDOff(LED3);

	// Add code here to process first half of buffer (ping)
		ADC1_dma_value1[cnt_dma1] = ADCTripleConvertedValues[0];
		ADC2_dma_value1[cnt_dma2] = ADCTripleConvertedValues[1];
		ADC3_dma_value1[cnt_dma3] = ADCTripleConvertedValues[2];
		//cnt_adc123=cnt_adc123+3;
		/*cnt_dma1++;
		cnt_dma2++;
		cnt_dma3++;
		cnt_adc1++;
		cnt_adc2++;
		cnt_adc3++;*/
	}
#endif
	/* Test on DMA Stream Transfer Complete interrupt */
  //	int iii;
	if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))
	{

		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		//DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
			//printf("\nB\n%d %d %d\n",ADCTripleConvertedValues[cnt_adc123+0],ADCTripleConvertedValues[cnt_adc123+1],ADCTripleConvertedValues[cnt_adc123+2]);
		/* Turn LED3 on: End of Transfer */
		//STM_EVAL_LEDOn(LED3);

		// Add code here to process second half of buffer (pong)

			/////for(iii=0;iii<buffer_max*6;iii=iii+6){
		if (cnt_dma1 < buffer_max)
		{
			// for 2ADC 2CH =>  use for(iii=0;iii<buffer_max*2;iii=iii+2)
			  //ADC1_dma_value1[cnt_dma1]=ADCTripleConvertedValues[iii] & 0x0000FFFF; 
			  //ADC2_dma_value1[cnt_dma2]=ADCTripleConvertedValues[iii]>>16;
			//
			  //ADC1_dma_value2[cnt_dma1]=ADCTripleConvertedValues[iii+1] & 0x0000FFFF;
			  //ADC2_dma_value2[cnt_dma2]=ADCTripleConvertedValues[iii+1]>>16;


			  // for 3ADC 2CH =>  use for(iii=0;iii<buffer_max*3;iii=iii+3)
			  /*ADC1_dma_value1[cnt_dma1]=ADCTripleConvertedValues[iii] & 0x0000FFFF;
			  ADC2_dma_value1[cnt_dma2]=ADCTripleConvertedValues[iii]>>16;

			  ADC3_dma_value1[cnt_dma1]=ADCTripleConvertedValues[iii+1] & 0x0000FFFF;
			  ADC1_dma_value2[cnt_dma2]=ADCTripleConvertedValues[iii+1]>>16;

			  ADC2_dma_value2[cnt_dma1]=ADCTripleConvertedValues[iii+2] & 0x0000FFFF;
			  ADC3_dma_value2[cnt_dma2]=ADCTripleConvertedValues[iii+2]>>16;*/

			  /*ADC1_dma_value1[cnt_dma1]=ADCTripleConvertedValues[iii+0] & 0x0000FFFF;
			  ADC2_dma_value1[cnt_dma2]=ADCTripleConvertedValues[iii+1] & 0x0000FFFF;

			  ADC3_dma_value1[cnt_dma1]=ADCTripleConvertedValues[iii+2] & 0x0000FFFF;
			  ADC1_dma_value2[cnt_dma2]=ADCTripleConvertedValues[iii+3] & 0x0000FFFF;

			  ADC2_dma_value2[cnt_dma1]=ADCTripleConvertedValues[iii+4] & 0x0000FFFF;
			  ADC3_dma_value2[cnt_dma2]=ADCTripleConvertedValues[iii+5] & 0x0000FFFF; */
			ADC1_dma_value1[cnt_dma1] = ADCTripleConvertedValues[0] & 0x0000FFFF;
			ADC2_dma_value1[cnt_dma2] = ADCTripleConvertedValues[1] & 0x0000FFFF;

			ADC3_dma_value1[cnt_dma1] = ADCTripleConvertedValues[2] & 0x0000FFFF;
			ADC1_dma_value2[cnt_dma2] = ADCTripleConvertedValues[3] & 0x0000FFFF;

			ADC2_dma_value2[cnt_dma1] = ADCTripleConvertedValues[4] & 0x0000FFFF;
			ADC3_dma_value2[cnt_dma2] = ADCTripleConvertedValues[5] & 0x0000FFFF;

			//1st request: ADC_CDR[31:0] = ADC2_DR[15:0] | ADC1_DR[15:0]
			//2nd request: ADC_CDR[31:0] = ADC1_DR[15:0] | ADC3_DR[15:0]
			//3rd request: ADC_CDR[31:0] = ADC3_DR[15:0] | ADC2_DR[15:0]
			//4th request: ADC_CDR[31:0] = ADC2_DR[15:0] | ADC1_DR[15:0]


								//ADC3_dma_value1[cnt_dma3]=ADCTripleConvertedValues[iii+2];	

								//ADC1_dma_value2[cnt_dma1]=ADCTripleConvertedValues[2];
								//ADC2_dma_value2[cnt_dma2]=ADCTripleConvertedValues[3];
								//ADC3_dma_value2[cnt_dma3]=ADCTripleConvertedValues[iii+2];		
		}
		//cnt_adc123=cnt_adc123+3;
//				cnt_dma1++;        //original yes
//				cnt_dma2++;
//				cnt_dma3++;
//				cnt_adc1++;
//				cnt_adc2++;
//				cnt_adc3++;
/*	}
		//bufferindex = (bufferindex+1) & 0x01;

		//if(cnt_adc123 > 2400*6){cnt_adc123 =0;}
				cnt_dma1++;
				cnt_dma2++;
				cnt_dma3++;
				cnt_adc1++;
				cnt_adc2++;
				cnt_adc3++;*/
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);

	}

}




void ADPLL_DMA2_Stream2(void){
	int iii;
	//printf("AABBCC\n");
	if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF2))
	{
		cnt_dma2++;

		if (cnt_dma2 < buffer_max)
		{
			ADC2_dma_value1[cnt_dma2] = ADC2ConvertedValue[0];
			ADC2_dma_value2[cnt_dma2] = ADC2ConvertedValue[1];
			//printf("ADCV = %d\n",ADCConvertedValue[0]);
		}

		DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF2);
	}

}

void ADPLL_DMA2_Stream0_2(void){
	int iii;
	//printf("AABBCC\n");
	if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))
	{
		cnt_dma3++;

		if (cnt_dma3 < buffer_max)
		{
			ADC3_dma_value1[cnt_dma3] = ADC3ConvertedValue[0];
			ADC3_dma_value2[cnt_dma3] = ADC3ConvertedValue[1];
			//printf("ADCV = %d\n",ADCConvertedValue[0]);
		}

		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
	}

}
