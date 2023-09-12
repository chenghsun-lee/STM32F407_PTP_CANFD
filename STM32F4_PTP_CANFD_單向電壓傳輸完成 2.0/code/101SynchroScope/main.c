/**
  ******************************************************************************
  * @file    TIM_TimeBase/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "stm32f4_discovery.h"
#include <misc.h>			 // I recommend you have a look at these in the ST firmware folder
#include <stm32f4xx_usart.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src
#include "arm_math.h" 
#include "scopeConfig.h"
 
 
/**********************************************************************
 NOTE:
        Use system_stm32f4xx.c and stm32f4xx.h in 
				  .\STM32F4-Discovery_FW_V1.1.0\Libraries\CMSIS\ST\STM32F4xx\

        Change HSE_VALUE value to 8000000 at line 92 in stm32f4xx.h in directory
          .\STM32F4-Discovery_FW_V1.1.0\Libraries\CMSIS\ST\STM32F4xx\Include
					
				Change PLL_M to 8 at line 149 in system_stm32f4xx.c in directory 
				  .\STM32F4-Discovery_FW_V1.1.0\Libraries\CMSIS\ST\STM32F4xx\Source\Templates
					
 **********************************************************************/
/* ------------------------------------------------------------------- 
* External Input and Output buffer Declarations for FFT Bin Example 
* ------------------------------------------------------------------- */ 

static float32_t testOutput[SAMPLES_PER_SECOND/2]; 
 
/* ------------------------------------------------------------------ 
* Global variables for FFT Bin Example 
* ------------------------------------------------------------------- */ 
uint32_t fftSize = 1024; 
uint32_t ifftFlag = 0; 
uint32_t doBitReverse = 1; 
 
/* Reference index at which max energy of bin ocuurs */ 
uint32_t refIndex = 213, testIndex = 0; 


void fft(float*) ;

void init_TIM3(void) ;
void ADC1_ChVbat_DMA_Config(void);
void ADC1_2_Simu_DMA_Config(void);
void init_USART1(uint32_t);
extern __IO uint16_t ADCDualConvertedValue[2];
__IO uint32_t Voltage11 = 0;
__IO uint32_t Voltage12 = 0;
__IO uint32_t counter=0;
__IO uint16_t RAWBuffer[SAMPLES_PER_SECOND][2][2];
__IO uint8_t  RAWbank=0;
__IO uint16_t RAWIndex=0;
__IO uint16_t RAWFlag=0;
__IO uint16_t ADCIndex=0;
__IO uint16_t ADCFlag=0;

float32_t src1[SAMPLES_PER_SECOND/2];
float32_t src2[SAMPLES_PER_SECOND/2];
	
void Delay(__IO uint32_t nCount) {
  while(nCount--) {
  }
}



int main(void) {
  unsigned long v1,v2;
  int bank;
	int i, iFFT;
	uint16_t capture1, capture2, ticks;
	int avg1, avg2;


  init_USART1(115200); // initialize USART1 @ 9600 baud


  init_TIM3() ;


	/* ADC1 configuration */

	ADC1_2_Simu_DMA_Config();

  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConv(ADC1);

  while (1){

		if (ADCFlag == 1) {
			ADCFlag = 0;
			v1=Voltage11;
			v2=Voltage12;
			printf("DC  : %6ld uV \t%6lduV\n\r", v1*3300000/SAMPLES_PER_SECOND/4096, v2*3300000/SAMPLES_PER_SECOND/4096);

			avg1 = v1;
			avg2 = v2;

//		if (RAWFlag == 1) {
			bank = (RAWbank-1)&1;
			capture1 = TIM_GetCapture4(TIM3);
			v1=0;
			v2=0;
			iFFT=0;
      for(i=0; i<SAMPLES_PER_SECOND; i=i++) {
				v1+=RAWBuffer[i][0][bank];
				v2+=RAWBuffer[i][1][bank];
				if ((i%4) == 0) {
					src1[iFFT] = RAWBuffer[i][0][bank] - avg1;
					src2[iFFT] = RAWBuffer[i][1][bank] - avg2;
					src1[iFFT+1] = 0;
					src2[iFFT+1] = 0;
					iFFT = iFFT+2;
				}
			}
			capture2 = TIM_GetCapture4(TIM3);
			if (capture2 > capture1) {
				ticks = capture2-capture1;
			} else {
				ticks = capture2 - capture1+65536;
			}
			printf("DC1 : %6ld uV \t%6lduV\t%d uS\n\r", v1*3300000/SAMPLES_PER_SECOND/4096, v2*3300000/SAMPLES_PER_SECOND/4096, ticks*10000/SAMPLES_PER_SECOND);
  		capture1 = TIM_GetCapture4(TIM3);
    	fft(src1) ;
			fft(src2) ;
			capture2 = TIM_GetCapture4(TIM3);
						if (capture2 > capture1) {
				ticks = capture2-capture1;
			} else {
				ticks = capture2 - capture1+65536;
			}
			printf("%d uS\n\r", ticks*10000/SAMPLES_PER_SECOND);
			printf("\n\r");
			////			printf("%d %d\n\r", capture2, capture1);		
		}

	

    /*
     * You can do whatever you want in here
     */
  }
}

void fft(float *src) {
	arm_status status; 
	arm_cfft_radix4_instance_f32 S_CFFT; 
	float32_t maxValue; 
	 
	status = ARM_MATH_SUCCESS; 
	 
	/* Initialize the CFFT/CIFFT module */  
	status = arm_cfft_radix4_init_f32(&S_CFFT, fftSize,  
	  								ifftFlag, doBitReverse); 
////	status = arm_rfft_init_f32(&S, &S_CFFT, 2048, ifftFlag, doBitReverse);
	 
	/* Process the data through the CFFT/CIFFT module */ 
	///arm_cfft_radix4_f32(&S_CFFT, testInput_f32_10khz); 
	arm_cfft_radix4_f32(&S_CFFT, src); 
////	 arm_rfft_f32(&S, src, dst);
	 
	/* Process the data through the Complex Magnitude Module for  
	calculating the magnitude at each bin */ 
	///arm_cmplx_mag_f32(testInput_f32_10khz, testOutput,  fftSize);  
	arm_cmplx_mag_f32(src, testOutput,  fftSize);  
  testOutput[0] = 0;	 
	/* Calculates maxValue and returns corresponding BIN value */ 
	arm_max_f32(testOutput, fftSize, &maxValue, &testIndex); 
   if (status == ARM_MATH_SUCCESS) {
		printf("MAX: %f, index=%d\n\r", maxValue, testIndex);	 
	 	printf("%f %f %f %f\n\r", testOutput[0], testOutput[1], testOutput[2], testOutput[3]); 
	 }
	 else {
		 printf("FFT Failed\n\r");
	 }

}



/**
  * @brief  Display ADC converted value on LCD
  * @param  None
  * @retval None
  */
void Display(void)
{

  // The reference is connected to 3.3v source
  Voltage11 = (ADCDualConvertedValue[0])*3300/0xFFF;
	Voltage12 = (ADCDualConvertedValue[1])*3300/0xFFF;
  
  printf("%d %d\n\r", Voltage11, Voltage12);
	

//  delay(100);
}
