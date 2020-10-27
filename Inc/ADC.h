/*
 * ADC.h
 *
 *  Created on: Apr 13, 2020
 *      Author: dmcmasters
 */

#ifndef ADC_H_
#define ADC_H_

#define ADC_NUM_CHANNELS                  4
#define ADC_VREF_CAL_VOLT                 3000
#define ADC_MAX_COUNTS                    4095
#define ADC_VREF_VOLT                     1200

#define ADC_3V3_TOLERANCE_LOW 	3135
#define ADC_3V3_TOLERANCE_HIGH 	3465
#define ADC_VIN_TOLERANCE_LOW 	21600
#define ADC_VIN_TOLERANCE_HIGH 	26400

// Temperature tolerances are defined in celsius (C).
#define ADC_TEMPERATURE_TOLERANCE_LOW     (-25)
#define ADC_TEMPERATURE_TOLERANCE_HIGH    (60)

// Number of iterations required before we report back ADC results
#define ADC_ITERATIONS                    (1)

#define TS30                              ((uint16_t*)((uint32_t)0x1FFF75A8))
#define TS110                             ((uint16_t*)((uint32_t)0x1FFF75CA))

#define VREFINT_CALDATA                   ((uint16_t*)((uint32_t)0x1FFF75AA))

#include <stdint.h>
#include <stdbool.h>

void     ADC_Process(void);
uint16_t ADC_Get_Supply_Voltage(void);
uint16_t ADC_Get_3V3_Voltage(void);
uint16_t ADC_Get_VrefInt_Voltage(void);
uint16_t ADC_Get_Temperature(void);
bool     ADC_StartupTasksComplete(void);

#endif/* ADC_H_ */
