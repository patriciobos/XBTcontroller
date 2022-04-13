/*
 * API_delay.h
 *
 *  Created on: 25 mar. 2022
 *      Author: patricio
 */

#ifndef API_INC_API_DELAY_H_
#define API_INC_API_DELAY_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo_144.h"

typedef uint32_t tick_t; // Qué biblioteca se debe incluir para que esto compile?
typedef bool bool_t;      // Qué biblioteca se debe incluir para que esto compile?

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

typedef struct{
   tick_t startTime;
   tick_t duration;
   bool_t running;
} delay_t;

/*=====[Prototypes (declarations) of public functions]=======================*/

void delayInit( delay_t * delay, tick_t duration );

bool_t delayRead( delay_t * delay );

void delayWrite( delay_t * delay, tick_t duration );

/*=====[Prototypes (declarations) of public interrupt functions]=============*/



#endif /* API_INC_API_DELAY_H_ */



