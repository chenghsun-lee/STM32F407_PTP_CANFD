#include "time_msg.h"
#include "stm32f4x7_eth.h"
#include "limits.h"
//#include "ethernetif.h"
#include "datatypes.h"

//Timestamp originTimestamp;

void getTime(TimeInternal *time);
void fromInternalTime(const TimeInternal *internal, Timestamp *external);


//void issuetime(void)
//{
//	
//	TimeInternal internalTime;
//	
//	getTime(&internalTime);
//	fromInternalTime(&internalTime, &originTimestamp);
//	
////	printf("%x\n", originTimestamp.secondsField.lsb);
////	printf("%x\n", originTimestamp.nanosecondsField);
////	printf("%x\n", originTimestamp.secondsField.msb);
//	
//}
