/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Lab612_Frequency.h"
#include "Lab612_ZC.h"
#include "Lab612_ADC.h"
#include <math.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "ethernetif.h"
#include <time.h>
#include "ptpd.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint8_t ubTxIndex;
extern char aTxBuffer[UART_BUFFERSIZE];
extern uint16_t UARTLen;
extern __IO uint16_t RAWBuffer[SAMPLE_RATE][2][2];
extern int bank;
extern int AVG;
extern uint64_t LocalVSum1;
extern uint8_t PrintFreqCNT;;

/**
  * @brief 零交越
  * @param  None
  * @retval None
  */

float AngleCalibrate(float input);
void ZCUART_Output(int FreqCNT, float ZCF, float Vpp, float ZCAngle);


struct ZeroCrossingStruct {
	uint16_t M;
	float N;

	float F; // Frequency
	float V; // Voltage
	float A; // Angle
};

uint16_t ZCX[2][2];
uint16_t ZCY[2][2];




#if UsingZC	
int ZCStart = 0;
uint16_t ZCPos; //描述現在儲存到哪裡
uint16_t ZCWindow[Window_Size] = { 0 };
uint64_t VSum = 0;
uint64_t wVSum[WindowShiftTime] = { 0 };
uint64_t V2Sum = 0;
uint64_t wV2Sum[WindowShiftTime] = { 0 };
uint16_t LastY = 0;
uint8_t ZCCNT = 0;

void ZeroCrossing() {
	struct	ZeroCrossingStruct  ZC;
	uint16_t i = 0;
	uint16_t temp = 0;
	uint64_t avg = 0;
	uint16_t last = 0;
	uint16_t windowCrossCNT = 0;
	uint8_t head = 0;

#if TimingMeasurement
	extern int32_t StartTIMCNT, EndTIMCNT, TimeLength;
	StartTIMCNT = TIM_GetCounter(TIM5);
#endif

	VSum -= wVSum[ZCCNT];
	wVSum[ZCCNT] = LocalVSum1;
	VSum += wVSum[ZCCNT];
	avg = (uint32_t)(VSum / (ZCStart ? Window_Size : WindowShift * (ZCCNT + 1)));

	V2Sum -= wV2Sum[ZCCNT];
	wV2Sum[ZCCNT] = 0;


	ZCPos = WindowShift * (ZCCNT + 1);
	ZCPos %= Window_Size;
	if (ZCPos) {
		LastY = ZCWindow[ZCPos - 1];
	}
	else {
		LastY = ZCWindow[Window_Size - 1];
	}

	for (i = 0; i < WindowShift; i++) {
		temp = RAWBuffer[i][0][bank];

		//For RMS
		wV2Sum[ZCCNT] += (temp - avg) * (temp - avg);
		//Store in the window

		ZCWindow[WindowShift * ZCCNT + i] = temp;
	}
	V2Sum += wV2Sum[ZCCNT];

	//ZC Main procedure
	if (ZCStart) {

		last = LastY;
		for (i = ZCPos; i < Window_Size; i++) {
			temp = ZCWindow[i];
			if ((last < avg) && (temp >= avg)) {
				if (!head) {
					//Determine angle
					
					ZC.A = (-1)*((float)(i - ZCPos) - (float)(temp - avg) / (float)(temp - last)) * (float)360 / (float)(SAMPLE_RATE);
		

					
					ZCY[0][0] = last;
					ZCY[0][1] = temp;
					ZCX[0][0] = i - 1;
					ZCX[0][1] = i;
					if (i == 0) {
						ZCX[0][0] = Window_Size - 1;
					}
					head = 1;
				}

				ZCY[1][0] = last;
				ZCY[1][1] = temp;
				ZCX[1][0] = i - 1;
				ZCX[1][1] = i;

				windowCrossCNT++;
			}
			last = temp;
		}
		if (ZCPos) {
			for (i = 0; i < ZCPos; i++) {
				temp = ZCWindow[i];
				if ((last < avg) && (temp >= avg)) {
					ZCY[1][0] = last;
					ZCY[1][1] = temp;
					ZCX[1][0] = i - 1;
					ZCX[1][1] = i;
					windowCrossCNT++;
				}
				last = temp;
			}
		}

		ZC.M = (ZCX[1][0] < ZCX[0][1]) * (Window_Size) + ZCX[1][0] - ZCX[0][1];
		ZC.N = (float)ZC.M + (float)(ZCY[0][0]) / (float)(ZCY[0][0] + ZCY[0][1]) + (float)(ZCY[1][1]) / (float)(ZCY[1][0] + ZCY[1][1]);
		ZC.F = (float)((windowCrossCNT - 1) * SAMPLE_RATE) / ZC.N;
		ZC.V = 2.82842712474619 * (float)(sqrt((float)(V2Sum / (float)(Window_Size))) * VResolution)  / 2;

		ZC.A *= ZC.F;
		ZC.A = AngleCalibrate(ZC.A);
#if UsingSTM32 		
		UART_Output(PrintFreqCNT++, ZC.F, ZC.V , ZC.A);
#else
		printf("f = %.3f, a = %.3f, v = %.3f\n", ZC.F, ZC.A, ZC.V);
#endif
		
		
		PrintFreqCNT %= 60;

	}


	ZCCNT++;
	if (ZCCNT == WindowShiftTime) {
		ZCStart = 1;
		ZCCNT = 0;
	}


#if TimingMeasurement
	EndTIMCNT = TIM_GetCounter(TIM5);
	//因為TIM5是下數的
	TimeLength = 99999 * (EndTIMCNT > StartTIMCNT) + StartTIMCNT - EndTIMCNT;
	sprintf(aTxBuffer, "T %d\r\n", (int)TimeLength);
	UARTLen = sizeof("T 100\r\n");
	ubTxIndex = 0;
	USART_ITConfig(USART6, USART_IT_TXE, ENABLE);
#endif
}


float AngleCalibrate(float input) {

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
