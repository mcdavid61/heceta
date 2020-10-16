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

#define ADC_3V3_TOLERANCE_LOW 	3230
#define ADC_3V3_TOLERANCE_HIGH 	3370
#define ADC_VIN_TOLERANCE_LOW 	22800
#define ADC_VIN_TOLERANCE_HIGH 	25200

//	Temperature tolerances are defined in celsius (C).
#define ADC_TEMPERATURE_TOLERANCE_LOW 	(-25)
#define ADC_TEMPERATURE_TOLERANCE_HIGH 	(60)

//	Number of iterations required before we report back ADC results
#define ADC_ITERATIONS (1)

#define TS30	((uint16_t*)((uint32_t)0x1FFF75A8))
#define TS110	((uint16_t*)((uint32_t)0x1FFF75CA))

#include <stdint.h>

void ADC_Process(void);
uint16_t ADC_Get_Supply_Voltage(void);
uint16_t ADC_Get_3V3_Voltage(void);
uint16_t ADC_Get_VrefInt_Voltage(void);
uint16_t ADC_Get_Temperature(void);


#endif /* ADC_H_ */
