/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Lab612_Frequency.h"
#include "Lab612_DFTTable.h"
#include "Lab612_DFT.h"
#include "Lab612_ADC.h"
#include <math.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "ethernetif.h"
#include <time.h>
#include "ptpd.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FreqStart 55//50
#define FreqEnd 65//70
#define DFT_StartF 11//10
#define DFT_EndF 13//14


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


//20221030 UART
extern uint8_t volatile ubTxIndex;
extern char aTxBuffer[UART_BUFFERSIZE];
extern uint16_t volatile UARTLen;
extern __IO uint16_t RAWBuffer[WindowShift][2][2];
extern int bank;
extern uint64_t LocalVSum1;
extern uint8_t PrintFreqCNT;;


float IpDFT(int mid, float* Amp, float* phase);
void DFT_Calculate(int StartPos);
float DFTAngleCalibrate(float input);
void UART_Output(int FreqCNT, float OutF, float Amp, float OutAngle);


/**
  * @brief DFT
  * @param  None
  * @retval None
  */

uint16_t DFTPos; //描述現在儲存到哪裡
uint16_t DFTWindow[Window_Size] = { 0 };
uint64_t DFTVSum = 0;
uint8_t DFTCNT = 0;

int j;

uint64_t DFTwVSum[WindowShiftTime] = { 0 };
int DFTStart = 0;

#if UsingDFT	




void DFT(void) {
	uint64_t avg = 0;

#if TimingMeasurement
	extern int32_t StartTIMCNT, EndTIMCNT, TimeLength;
	StartTIMCNT = TIM_GetCounter(TIM5);
#endif

	DFTVSum -= DFTwVSum[DFTCNT];
	DFTwVSum[DFTCNT] = LocalVSum1;
	DFTVSum += DFTwVSum[DFTCNT];
	avg = (uint32_t)(DFTVSum / (DFTStart ? Window_Size : WindowShift * (DFTCNT + 1)));



	DFTPos = WindowShift * (DFTCNT);
	DFTPos %= Window_Size;

	for (j = 0; j < WindowShift; j++) {
		//Store in the window
		DFTWindow[WindowShift * DFTCNT + j] = RAWBuffer[j][0][bank];;
	}
	if (DFTStart) {
		DFT_Calculate(DFTPos);
	}
	DFTCNT++;
	if (DFTCNT == WindowShiftTime) {
		DFTStart = 1;
		DFTCNT = 0;
	}


#if TimingMeasurement
	EndTIMCNT = TIM_GetCounter(TIM5);
	//因為TIM5是下數的
	TimeLength = 99999 * (EndTIMCNT > StartTIMCNT) + StartTIMCNT - EndTIMCNT;
#if 0
	sprintf(aTxBuffer, "T %d\r\n", (int)TimeLength);
	UARTLen = sizeof("T 100\r\n");
	ubTxIndex = 0;
	USART_ITConfig(USART6, USART_IT_TXE, ENABLE);
#else
	printf("T %d\r\n", (int)TimeLength);
#endif
#endif

}






typedef struct				//定義複數結構,下面通過尤拉公式運算
{
	float real, imag;
}complex;

complex Result_Point[Window_Size] = { 0 };



void DFT_Calculate_Point(int k, int StartPos) {
	int n = 0;
	Result_Point[k].real = 0;
	Result_Point[k].imag = 0;

	for (n = 0; n < Window_Size; n++)
	{

#ifdef UseHann
#if SAMPLE_RATE == 3840
		Result_Point[k].real += DFT_3840_cos_Hann[k - 10][n] * (float)DFTWindow[(n + StartPos) % Window_Size];  //複數的實部
		Result_Point[k].imag -= DFT_3840_sin_Hann[k - 10][n] * (float)DFTWindow[(n + StartPos) % Window_Size];  //複數的實部
#elif SAMPLE_RATE == 1920
		Result_Point[k].real += DFT_1920_cos_Hann[k - 10][n] * (float)DFTWindow[(n + StartPos) % Window_Size];  //複數的實部
		Result_Point[k].imag -= DFT_1920_sin_Hann[k - 10][n] * (float)DFTWindow[(n + StartPos) % Window_Size]; //複數的實部
#endif
#else	
#if SAMPLE_RATE == 3840
		Result_Point[k].real += DFT_3840_cos[k - 10][n] * (float)DFTWindow[(n + StartPos) % Window_Size];  //複數的實部
		Result_Point[k].imag -= DFT_3840_sin[k - 10][n] * (float)DFTWindow[(n + StartPos) % Window_Size];  //複數的實部
#elif SAMPLE_RATE == 1920
		Result_Point[k].real += DFT_1920_cos[k - 10][n] * (float)DFTWindow[(n + StartPos) % Window_Size];  //複數的實部
		Result_Point[k].imag -= DFT_1920_sin[k - 10][n] * (float)DFTWindow[(n + StartPos) % Window_Size]; //複數的實部
#endif
#endif
	}	
}

float Ampl[Window_Size];   	//存儲幅值計算結果
float VDC = 0;



void DFT_Calculate(int StartPos)
{
	int Max = 1;
	float OAmp, OAngle, OF;

	for (j = DFT_StartF; j <= DFT_EndF; j++)
	{
		DFT_Calculate_Point(j, StartPos);
		Ampl[j] = sqrt(Result_Point[j].real * Result_Point[j].real + Result_Point[j].imag * Result_Point[j].imag);  //計算幅值
		Ampl[j] /=  (float)(SAMPLE_RATE/10) - 0.5; //這是因為Hann window的補償，要除以B
		if (j != 0 && Ampl[Max] < Ampl[j]) {
			Max = j;
		}
	}


#if 0
	//找到左鄰居
	printf("Left index = %d, %f\n", Max - 1, Ampl[Max - 1]);
	printf("Max index = %d, %f\n", Max, Ampl[Max]);
	//找到右	鄰居
	printf("Right index = %d, %f\n", Max + 1, Ampl[Max + 1]);
	printf("\n");
#endif
	//開始頻率插值	
	OF = IpDFT(Max, &OAmp, &OAngle);
#if  !UsingSTM32

	printf("F = %f, ", IpDFT(Max, &OAmp, &OAngle));
	//printf("Before Amp = %f, Angle = %f\n", Ampl[Max] * VResolution / (Window_Size), atan(Result_Point[Max].imag / Result_Point[Max].real) * (float)180 / (float)M_PI);
	printf("Amp = %f, Angle = %f\n", OAmp * VResolution / (Window_Size), OAngle * (float)180 / (float)M_PI);

#else
	OAmp *= VResolution;
	OAngle += (float)M_PI / (float)2;
	OAngle *= (float)180 / (float)M_PI;
	OAngle = DFTAngleCalibrate(OAngle);
	UART_Output(PrintFreqCNT, OF, OAmp, OAngle);
	PrintFreqCNT++;
#endif

}


float IpDFT(int mid, float* Amp, float* phase) {
	float R;
	int bigIndex = 0;
	float addSign = 1;
	float delta;
	float alpha;
	if (Ampl[mid - 1] > Ampl[mid + 1]) {
		bigIndex = mid - 1;
		addSign = -1;
	}
	else {
		bigIndex = mid + 1;
		addSign = 1;
	}
#ifdef UseHann
	alpha = fabs(Ampl[mid] / Ampl[bigIndex]);
	delta = addSign * (2 - alpha) / (1 + alpha);
#else
	delta = addSign * fabs(Ampl[bigIndex] / (Ampl[bigIndex] + Ampl[mid]));
#endif
	//printf("delta = %f\n", delta);
	R = ((float)mid + delta) * DelatF;
	* Amp = 2 * Ampl[mid] * (M_PI * delta * (1 - delta * delta)) / sin(M_PI * delta);
	*phase = atan2(Result_Point[mid].imag , Result_Point[mid].real) - M_PI * delta;

	return R;
}


float DFTAngleCalibrate(float input) {

	while (input > 360) {
		input -= 360;
	}
	while (input < -360) {
		input += 360;
	}
	if (input < -180) {
		input += 360;
	}

	else if (input > 180) {
		input = 360 - input;
	}

	//用中斷的方式輸出負的相位角會有bug
	/*
	if (input < 0) 
		input += 360;
	
*/
	return input;
}




#endif


