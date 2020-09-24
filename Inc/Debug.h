/*
 * Debug.h
 *
 *  Created on: Jun 29, 2020
 *      Author: dmcmasters
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

//	Debug macros
//	#define DEBUG_USE_J19_HEADER_AS_RELAY_OUTPUT

//	Other debug macro enables, if required by certain macros
#ifdef DEBUG_USE_J19_HEADER_AS_RELAY_OUTPUT
#define DEBUG_DISABLE_LEDS
#endif




uint16_t Debug_Write(char *ptr, uint16_t len);
void DEBUG_GPIO_INIT(void);


#endif /* _DEBUG_H_ */
