/*
 * ADC.h
 *
 *  Created on: Apr 13, 2020
 *      Author: dmcmasters
 */

#ifndef ADC_H_
#define ADC_H_

#define ADC_NUM_CHANNELS	4
#define	ADC_REF_VOLT			3300
#define	ADC_MAX_COUNTS		4095

#define TS30	((uint16_t*)((uint32_t)0x1FFF75A8))
#define TS110	((uint16_t*)((uint32_t)0x1FFF75CA))

void ADC_Process(void);


#endif /* ADC_H_ */
