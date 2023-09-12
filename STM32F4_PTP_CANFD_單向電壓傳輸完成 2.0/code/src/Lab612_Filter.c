/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <main.h>
#include "Lab612_Filter.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


float MagComp(float f){
	float r = 0;
	float w = 2 * M_PI * f;	
	float u = w / wn;	
	r = ((float)1) / sqrtf(((float)1 - u * u) + ((float)2 * dfilter * u) *  ((float)2 * dfilter * u));
	return r;
}

float AngComp(float f){
	float r = 0;
	float w = 2 * M_PI * f;	
	float u = w / wn;	
	r = -1 * atan( ((float)2 * dfilter * u) / ((float)1 - u * u));	
	r *= 180.0 / M_PI;
	return r;
}

