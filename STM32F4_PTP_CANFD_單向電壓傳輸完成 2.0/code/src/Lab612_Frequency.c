/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Lab612_Frequency.h"
#include "Lab612_ADC.h"
//#include "scopeConfig.h"
#include "arm_math.h" 
#include <math.h>
#include <stdio.h>
#include "stm32f4xx.h"
#if UsingDFT			
#include "Lab612_DFT.h"
#endif
#if  UsingZC			
#include "Lab612_ZC.h"
#endif
#include "ptpd.h"
#include "ethernetif.h"
#include <time.h>
#include "Lab612_Filter.h"


/** @addtogroup ADC_DualADC_RegulSimu_DMAmode1
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/



extern __IO uint16_t ADCDualConvertedValue[2];
__IO uint32_t VSum1 = 0;
__IO uint32_t Voltage12 = 0;
__IO uint32_t counter=0;
__IO uint8_t  RAWbank=0;
__IO uint16_t RAWFlag=0;
__IO uint16_t ADCIndex=0;

__IO uint16_t RAWBuffer[WindowShift][2][2] = {0};
#if  UsingFFT
float FFTInput[2*SAMPLE_RATE] = {0};
#endif
/* Private function prototypes -----------------------------------------------*/
//FFT
void FFT612(float *Input);
void FFTInit(void);


/**
  * @brief Work in while(1)
  * @param  None
  * @retval None
  */
/* For ADCConvert--------------------------*/
uint64_t LocalVSum1;
#define IntialDelayTime 0
uint32_t AVG;
int freq_i = 0;
int bank;
int32_t StartTIMCNT, EndTIMCNT, TimeLength;
int DelayCNT = 0;
void ADCConvert(void) {
	if(DelayCNT < IntialDelayTime){
		DelayCNT++;
	}
	else{
		LocalVSum1 = VSum1;
		AVG = LocalVSum1/ WindowShift;
		bank = (RAWbank - 1) & 1;
		
#if  UsingFFT
		FFTInit();			
#elif  UsingZC			
		ZeroCrossing();
#elif UsingDFT			
		DFT();
#endif			
	}
}


#define local_UTC_offset 8  
#define local_UTC_offset_sec local_UTC_offset*3600
extern int16_t UTCoffset;
struct ptptime_t TIMptptimeUART;
time_t TIMsecondsUART;
char TIMSec[32];



//20221030 UART
extern uint8_t volatile ubTxIndex;
extern char aTxBuffer[UART_BUFFERSIZE];
extern uint16_t volatile UARTLen;
extern __IO uint16_t RAWBuffer[WindowShift][2][2];
extern int bank;
extern uint64_t LocalVSum1;
extern uint8_t PrintFreqCNT;;

float LastF = 0;
void UART_Output(int FreqCNT, float OutF, float Amp, float OutAngle) {

	Amp /= MagComp(OutF);	
	OutAngle -= AngComp(OutF);	
	if (OutAngle < -180) {
		OutAngle += 360;
	}

	else if (OutAngle > 180) {
		OutAngle = 360 - OutAngle;
	}

	if (FreqCNT == 0) {
		ETH_PTPTime_GetTime(&TIMptptimeUART);
		TIMsecondsUART = (time_t)(TIMptptimeUART.tv_sec + local_UTC_offset_sec - UTCoffset);
		strftime(TIMSec, sizeof(TIMSec), "%Y%m%d %H:%M:%S\r\n", localtime(&TIMsecondsUART));
		printf("%s", TIMSec);
	}
	
	#if OutputFormat == 1
	sprintf(aTxBuffer, "%d,%d,%d,%d\r\n", FreqCNT, (int)(OutF * INTDigit), (int)(PT* Amp * INTDigit), (int)(OutAngle * INTDigit));
	UARTLen = PMUUARTLen;
	#elif OutputFormat == 2
	sprintf(aTxBuffer, "%d,%.3f,%.3f,%.3f,%.3f\r\n", FreqCNT, OutF, Amp, OutAngle, (OutF - LastF)*(float)ReportRate);
	UARTLen = PMUUARTLen;
	#elif OutputFormat == 3
	sprintf(aTxBuffer, "%d,%d,%d,%d,%d\r\n", FreqCNT, (int)(OutF * INTDigit), (int)(Amp * INTDigit), (int)(OutAngle * INTDigit) , (int)((OutF - LastF)* ReportRate * INTDigit));
	UARTLen = PMUUARTLen;
	#endif
	
	ubTxIndex = 0;
	USART_ITConfig(USART6, USART_IT_TXE, ENABLE);

	LastF = OutF;
}





/**
  * @brief 準備FFT
  * @param  None
  * @retval None
  */
/* For ADCConvert--------------------------*/
#if  UsingFFT
int iFFT;
void FFTInit(void){
#if ADCPrint 
	//20220502
	//4096是代表2^12，取決於12BIT的ADC解析度
	//前面直接算平均
	//printf("DC : %6lldmV\n\r", LocalVSum1 * 3300 / SAMPLE_RATE / 4096); //以前學長的
	#if 0
	printf("DC : %6lldmV\n\r", LocalVSum1 * VResolution / SAMPLE_RATE/1000);
	#endif
#endif

	LocalVSum1 = 0;
	iFFT = 0;
	//printf("\n");
				
				
#if TimingMeasurement
		StartTIMCNT = TIM_GetCounter(TIM5);
#endif

	for (freq_i = 0; freq_i < SAMPLE_RATE / 5; freq_i++) {		

		//在這裡算平均頗沒有效率的，不如之前加總平均or之後FFTOutput[0]				
		LocalVSum1 += RAWBuffer[freq_i][0][bank];
		//printf("%d\n",RAWBuffer[freq_i][0][bank]); //顯示原始資料
		if(freq_i < 20){
			//printf("%d\n", RAWBuffer[freq_i][0][bank]);  //顯示電壓
		}
		FFTInput[iFFT] = RAWBuffer[freq_i][0][bank];
		iFFT++;
		FFTInput[iFFT] = 0; 
		iFFT++;
		
		if(iFFT == 64*2){

			FFT612(FFTInput);		


			iFFT = 0;
		}
		
	}
		
#if TimingMeasurement
	EndTIMCNT = TIM_GetCounter(TIM5);
	//因為TIM5是下數的
	TimeLength = 99999*( EndTIMCNT > StartTIMCNT) + StartTIMCNT - EndTIMCNT;
	printf("T %.1f\n",(float)TimeLength/10);
	//printf("%d %d\n", EndTIMCNT, StartTIMCNT);			
#endif	
				
		


#if ADCPrint 
		//printf("%d uS\r", TimeLength * VRef / SAMPLE_RATE / 5);
#endif

}


#endif






/**
  * @brief FFT本身
  * @param  None
  * @retval None
  */
/* For ADCConvert--------------------------*/
/* FFT---------------------------------------*/
#if  UsingFFT
uint32_t FFTSize = 64;
uint8_t ifftFlag = 0; 
uint8_t doBitReverse = 1;  
/* Reference index at which max energy of bin ocuurs */ 
uint32_t refIndex = 213, MaxIndex = 0; 
float FFT0Temp = 0;
double FFTMainAngle = 0;
static float FFTOutput[64] = { 0 }; 

void FFT612(float *Input) {
	arm_status status; 
	arm_cfft_radix4_instance_f32 S_CFFT; 
	float32_t maxValue; 
	status = ARM_MATH_SUCCESS; 
	 
	/* Initialize the CFFT/CIFFT module */  
	status = arm_cfft_radix4_init_f32(&S_CFFT, FFTSize, ifftFlag, doBitReverse); 

	if (status == ARM_MATH_ARGUMENT_ERROR) {		
		printf("ARM_MATH_ARGUMENT_ERROR\n");
	}

	/* Process the data through the CFFT/CIFFT module */ 
	arm_cfft_radix4_f32(&S_CFFT, Input); 
	/* Process the data through the Complex Magnitude Module for  
	calculating the magnitude at each bin */ 
	arm_cmplx_mag_f32(Input, FFTOutput,  FFTSize);  
	
	
#if PrintFFT
	#if 0
	printf("DC mVFFT : %lld \n", (uint64_t)FFTOutput[0] * VResolution/ FFTSize /1000);
	#endif
#endif
	
	FFT0Temp = FFTOutput[0];
	FFTOutput[0] = 0;	 
	
	/* Calculates maxValue and returns corresponding BIN value */ 
	arm_max_f32(FFTOutput, FFTSize/2, &maxValue, &MaxIndex); 
	FFTMainAngle = (double)180 * atan2((double)FFTOutput[2*(MaxIndex) + 1] , (double)FFTOutput[2*(MaxIndex)]) / (double)PI;
	
	FFTOutput[0] = FFT0Temp;		
	if (status == ARM_MATH_SUCCESS) {
		
#if PrintFFT
		#if 1
		//printf("FFT: MAX_Value: %f, index = %d\n", maxValue, MaxIndex);	 
		//printf("Fundamental Freq Mag %lld\n", (uint64_t)maxValue *2* VResolution/ FFTSize / 1000 );	
		//printf("tan-1 = %lf\n", atan2((double)FFTOutput[2*(MaxIndex - 1)] , (double)FFTOutput[2*(MaxIndex - 1) - 1]) );	
		//printf("%lf %lf\n", (double)FFTOutput[2*(MaxIndex - 1) + 1] , (double)FFTOutput[2*(MaxIndex - 1)]);			
		//printf("Fundamental Freq Angle = %lf\n", FFTMainAngle);		
		
		printf("%d ", MaxIndex*60);	 
		//printf("%d",(uint16_t)(uint64_t)maxValue * 1000 * 2 * VResolution/ FFTSize /1000);
		printf(" %d\n",(uint16_t)(FFTMainAngle*1000));
		#endif 
		//printf(",%lf\n", FFTMainAngle);
#endif
	}
	else {
		printf("FFT Failed\n");
	}
}

#endif






