/*
 * delay.h
 *
 *  Created on: Feb 7, 2020
 *      Author: dmcmasters
 */

#ifndef DELAY_H_
#define DELAY_H_

#include <stdint.h>

void delay_us(uint32_t delay);
void delay_ms(uint32_t delay);
void delay_sec(uint32_t delay);

#endif /* DELAY_H_ */
