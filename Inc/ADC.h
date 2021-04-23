/*
 * ADC.h
 *
 *  Created on: Apr 13, 2020
 *      Author: dmcmasters
 */

#ifndef ADC_H_
#define ADC_H_

#define ADC_NUM_CHANNELS                   (4)
#define ADC_VREF_CAL_VOLT                  (3000)
#define ADC_MAX_COUNTS                     (4095)
#define ADC_VREF_VOLT                      (1200)

#define ADC_3V3_NOMINAL                    (3300)
#define ADC_24V_NOMINAL                    (24000)

#define ADC_3V3_TOLERANCE_LOW              (3135) // 3.3 - 5%
#define ADC_3V3_TOLERANCE_HIGH             (3465) // 3.3 + 5%
#define ADC_3V3_HYSTERISIS_LOW             (3200)
#define ADC_3V3_HYSTERISIS_HIGH            (3400)

#define ADC_24V_TOLERANCE_LOW              (21600) // 24 - 10%
#define ADC_24V_TOLERANCE_HIGH             (26400) // 24 + 10%
#define ADC_24V_HYSTERISIS_LOW             (22100)
#define ADC_24V_HYSTERISIS_HIGH            (25900)

// Temperature tolerances are defined in celsius (C).
#define ADC_TEMPERATURE_TOLERANCE_LOW      (-25)
#define ADC_TEMPERATURE_TOLERANCE_HIGH     (60)
#define ADC_TEMPERATURE_HYSTERISIS_LOW     (-22)
#define ADC_TEMPERATURE_HYSTERISIS_HIGH    (55)

// Number of iterations required before we report back ADC results
#define ADC_ITERATIONS                     (1)

#define ADC_TEMP1                          (30.0)
#define ADC_TEMP2                          (110.0)

#define ADC_TEMP1_COUNTS                   ((uint16_t*)((uint32_t)0x1FFF75A8))
#define ADC_TEMP2_COUNTS                   ((uint16_t*)((uint32_t)0x1FFF75CA))

#define VREFINT_CALDATA                    ((uint16_t*)((uint32_t)0x1FFF75AA))

#include <stdint.h>
#include <stdbool.h>

void     ADC_Process(void);
uint16_t ADC_Get_Supply_Voltage(void);
uint16_t ADC_Get_3V3_Voltage(void);
uint16_t ADC_Get_VrefInt_Voltage(void);
uint16_t ADC_Get_Temperature(void);
bool     ADC_StartupTasksComplete(void);

#endif/* ADC_H_ */
