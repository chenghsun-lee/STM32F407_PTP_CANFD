/**
   @file fputc_debug.c
   @brief Trying to redirect printf() to debug port
   @date 2012/06/25
*/
 
#include <stdio.h>
#include <stm32f4xx.h>
 
int fputc(int c, FILE *stream)
{
	
	// wait until data register is empty
	while( !(USART1->SR & 0x00000040) );
	USART_SendData(USART1, c);

	return c;
//   return(ITM_SendChar(c);
}
