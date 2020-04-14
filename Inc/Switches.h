/*
 * Switches.h
 *
 *  Created on: Apr 13, 2020
 *      Author: dmcmasters
 */

#ifndef SWITCHES_H_
#define SWITCHES_H_

#define SW1	 HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)
#define SW2	 HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)
#define SW3	 HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin)
#define SW4	 HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin)
#define SW5	 HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin)
#define SW6	 HAL_GPIO_ReadPin(SW6_GPIO_Port, SW6_Pin)
#define SW7	 HAL_GPIO_ReadPin(SW7_GPIO_Port, SW7_Pin)
#define SW8	 HAL_GPIO_ReadPin(SW8_GPIO_Port, SW8_Pin)

uint8_t	Switches_Read(void);

#endif /* SWITCHES_H_ */
