/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LAB612_FILTER_H
#define __LAB612_FILTER_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/ 

#define wn 820//815//810//791.12//751.55
#define dfilter 0.61957//0.623//0.6272//0.6422//0.676

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
float MagComp(float f);
float AngComp(float f);

#endif 