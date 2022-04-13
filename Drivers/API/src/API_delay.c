/*
 * API_delay.c
 *
 *  Created on: 25 mar. 2022
 *      Author: patricio
 */


#include "API_delay.h"


void delayInit( delay_t * delay, tick_t duration ){

	delay->duration = duration;
	delay->running = false;
}

bool_t delayRead( delay_t * delay ){

	bool_t retValue = false;

	if(!(delay->running)) {
		delay->startTime = HAL_GetTick();
		delay->running = true;
		retValue = false;
	}
	else
		if (HAL_GetTick()- delay->startTime >= delay->duration) {
			delay->running = false;
			retValue = true;
		}
		else
			retValue = false;

	return retValue;
}

void delayWrite( delay_t * delay, tick_t duration ){
	delay->duration = duration;
}
