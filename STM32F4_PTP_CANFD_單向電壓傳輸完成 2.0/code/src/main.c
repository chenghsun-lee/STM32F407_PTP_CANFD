/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-October-2011
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
  * <h2><center>&copy; Portions COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */
/**
  ******************************************************************************
  * <h2><center>&copy; Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.</center></h2>
  * @file    main.c
  * @author  CMP Team
  * @version V1.0.0
  * @date    28-December-2012
  * @brief   Main program body      
  *          Modified to support the STM32F4DISCOVERY, STM32F4DIS-BB and
  *          STM32F4DIS-LCD modules. 
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, Embest SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
  * OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
  * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
  * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  ******************************************************************************
  */  
/* Includes ------------------------------------------------------------------*/
//For PTP 
#include "main.h"
#include "cmsis_os.h"
#include "stm32f4x7_eth.h"
#include "ethernetif.h"
#include "telnet.h"
#include "ptpd.h"
#include "pbuf.h"
#include <stdio.h>
#include "sys.h"
#include "pbuf.h"
#include "memp.h"
#include "time_msg.h"
#include "netconf.h"
#include "time.h"
#include "stm32f4x7_eth_bsp.h" 
#include "Lab612_TIM.h"
#include "Lab612_ADC.h"

//20230814 CANFD
#include <asf.h>
#include "app.h"

#define  USE_STDPERIPH_DRIVER 1

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define systic (*((volatile unsigned int *)0xE000E018))

/*--------------- Tasks Priority -------------*/
#define DHCP_TASK_PRIO   ( osPriorityIdle + 2 )      
#define LED_TASK_PRIO    ( osPriorityIdle + 1 )
#define netifMTU												(1500)
#define netifINTERFACE_TASK_STACK_SIZE	(350)
#define netifINTERFACE_TASK_PRIORITY		(osPriorityRealtime)
#define netifGUARD_BLOCK_TIME						(250)
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern unsigned int os_time;
MACFrameArray MACFrame_Array;
extern sys_mutex_t MAC_frame_mutex;
//確認同步狀態
int32_t OffsetSecond = 0 , OffsetNanosecond = 0;
//確認鎖定狀態
extern int ADPLL_Lock;


/* Private functions ---------------------------------------------------------*/	

/* Private function prototypes -----------------------------------------------*/
extern unsigned int os_tick_val(void);
extern uint32_t svcKernelSysTick (void);
void UART_Init(void);
void Delay(uint32_t nCount);
void TestLEDInit(void);
/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
 
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured to 
       168 MHz, this is done through SystemInit() function which is called from
       startup file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
  */  	


	UART_Init();  
	TestLEDInit();
	//20230814 CANFD
	APP_Initialize();	
	
	ETH_BSP_Config();
	Timer_Config();
	MAC_Init();	
	ptpd_init();	
	
	
#ifdef ADCOn
	ADC_Config();
#endif



	while(1){		
	
		if(ADPLL_Lock){
			GPIO_SetBits(GPIOD, GPIO_Pin_12);	
		}
	}
	
}






/**
  * @brief  Delay Function
  * @param  Delay tickets
  * @retval None
  */
void Delay(uint32_t nCount){
	uint32_t Count = 1;
	for(Count = 1; Count<= nCount; Count++){};
}



/**
  * @brief  Initializes the UART resources.
  * @param  None
  * @retval None
  */

void UART_Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	
	//USARTx configured as follows: 
	//115200 baud, 8 Bits, 1 Stop, No Parity, 
	//No Flow Control, Receive and Transmit Enabled.
	USART_StructInit(&USART_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 921600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	//COM1 = USART6
	STM_EVAL_COMInit(COM1, &USART_InitStructure);
	// Output a message using printf function.	
  USART_Cmd(USART6, ENABLE);
	//printf("\nUSART Initialized\n");
	

  /* Enable USART */
	printf("\nUSART Initialized\r\n");
}


//debug
void TestLEDInit(void){
		GPIO_InitTypeDef  GPIO_InitStructure;	
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_14;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
		GPIO_Init(GPIOD, &GPIO_InitStructure);	
}


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
int fputc(int ch, FILE *f)
{
	/* Send a CRLF for each LF. */
	if (ch == '\n') fputc('\r', f);

	/* Send the character. */
  USART_SendData(USART6, (uint8_t) ch);

  /* Loop until the end of transmission. */
  while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);

  return ch;
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif



/*********** Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.*****END OF FILE****/

