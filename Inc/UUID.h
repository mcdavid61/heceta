/*
 * UUID.h
 * A simple header for reading the STM32 device UUID
 * Tested with STM32L4
 *  *
 *  Created on: Apr 7, 2020
 *      Author: dmcmasters
 */

#ifndef UUID_H_
#define UUID_H_

#include <stdint.h>
/**
 * The STM32 factory-programmed UUID memory.
 * Three values of 32 bits each starting at this address
 * Use like this: STM32_UUID[0], STM32_UUID[1], STM32_UUID[2]
 */
#define STM32_UUID ((uint32_t *)0x1FFF7590)


//*****************************************************************************
// Function Prototypes
uint32_t UUID_Get_ID(void);

#endif /* UUID_H_ */
