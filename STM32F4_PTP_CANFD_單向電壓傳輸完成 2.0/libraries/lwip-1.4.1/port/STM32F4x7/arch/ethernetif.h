#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include <stdint.h>
#include "lwip/err.h"
#include "lwip/netif.h"
#include "stm32f4x7_eth.h"

struct ptptime_t {
  s32_t tv_sec;
  s32_t tv_nsec;
};

void netMACif_set_default(struct netMACif *xnetMACif);
void low_level_init_test(struct netMACif *netMACif);
err_t low_level_output(struct netif *netif,struct pbuf *p);

err_t new_low_level_output(struct pbuf *p, TimeInternal *time);
err_t new_send_something(struct pbuf *p, PtpClock *ptpClock, Timestamp *originTimestamp, TimeInternal *time);


err_t send_something(struct netif *netif, struct pbuf *p);
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(void * pvParameters);
void ethernetif_input_test(void * pvParameters);

void copy_buffer(u8 *buffer_des, u8 *buffer_src, u16_t *len_des, u16_t *len_src);
void copy_PTP_buffer(u8 *buffer_des, u8 *buffer_src, u16_t *len_des, u16_t *len_src, TimeInternal *time_des, TimeInternal *time_src);



#if LWIP_PTP
void ETH_PTPTime_SetTime(struct ptptime_t * timestamp);
void ETH_PTPTime_GetTime(struct ptptime_t * timestamp);
void ETH_PTPTime_UpdateOffset(struct ptptime_t * timeoffset);
void ETH_PTPTime_AdjFreq(int32_t Adj);

/* Examples of subsecond increment and addend values using SysClk = 144 MHz
 
 Addend * Increment = 2^63 / SysClk

 ptp_tick = Increment * 10^9 / 2^31

 +-----------+-----------+------------+
 | ptp tick  | Increment | Addend     |
 +-----------+-----------+------------+
 |  119 ns   |   255     | 0x0EF8B863 |
 |  100 ns   |   215     | 0x11C1C8D5 |
 |   50 ns   |   107     | 0x23AE0D90 |
 |   20 ns   |    43     | 0x58C8EC2B |
 |   14 ns   |    30     | 0x7F421F4F |
 +-----------+-----------+------------+
*/

/* Examples of subsecond increment and addend values using SysClk = 168 MHz
 
 Addend * Increment = 2^63 / SysClk

 ptp_tick = Increment * 10^9 / 2^31

 +-----------+-----------+------------+
 | ptp tick  | Increment | Addend     |
 +-----------+-----------+------------+
 |  119 ns   |   255     | 0x0CD53055 |
 |  100 ns   |   215     | 0x0F386300 |
 |   50 ns   |   107     | 0x1E953032 |
 |   20 ns   |    43     | 0x4C19EF00 |
 |   14 ns   |    30     | 0x6D141AD6 |
 +-----------+-----------+------------+
*/

/* Select SysClk = 168MHz */

#define ADJ_FREQ_BASE_ADDEND      0xDA2835AC
#define ADJ_FREQ_BASE_INCREMENT   15


/*
#define ADJ_FREQ_BASE_ADDEND      0xFBB83DEE
#define ADJ_FREQ_BASE_INCREMENT   13
*/


#endif

#endif 
