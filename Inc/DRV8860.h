#ifndef _DRV8860_H_
#define _DRV8860_H_

#include <stdint.h>
#include "main.h"

//	The following macros are defined in such a way
//	to allow the debugging pins to mirror the output
//	being sent to the DRV8860.

#define RELAY_DEBUG_LATCH_Pin SPI3_CS_Pin
#define RELAY_DEBUG_LATCH_GPIO_Port SPI3_CS_GPIO_Port
#define RELAY_DEBUG_CLK_Pin LED_GREEN_Pin
#define RELAY_DEBUG_CLK_GPIO_Port LED_GREEN_GPIO_Port
#define RELAY_DEBUG_DOUT_Pin LED_RED_Pin
#define RELAY_DEBUG_DOUT_GPIO_Port LED_RED_GPIO_Port

#define RELAY_DEBUG_DIN_PASSTHROUGH_Pin	LED_AMBER_Pin
#define RELAY_DEBUG_DIN_PASSTHROUGH_GPIO_Port	LED_AMBER_GPIO_Port


//	Macros to set pins simultaneously
#define DRV8860_PIN_CLK(B) \
	HAL_GPIO_WritePin(RELAY_DEBUG_CLK_GPIO_Port, RELAY_DEBUG_CLK_Pin, B); \
	HAL_GPIO_WritePin(R_CLK_GPIO_Port, R_CLK_Pin, B)
#define DRV8860_PIN_LAT(B) \
	HAL_GPIO_WritePin(RELAY_DEBUG_LATCH_GPIO_Port, RELAY_DEBUG_LATCH_Pin, B); \
	HAL_GPIO_WritePin(R_LAT_GPIO_Port, R_LAT_Pin, B)
#define DRV8860_PIN_DOUT(B) \
	HAL_GPIO_WritePin(RELAY_DEBUG_DOUT_GPIO_Port, RELAY_DEBUG_DOUT_Pin, B); \
	HAL_GPIO_WritePin(R_DOUT_GPIO_Port, R_DOUT_Pin, B)
#define DRV8860_PIN_DIN() \
	HAL_GPIO_ReadPin(R_DIN_GPIO_Port, R_DIN_Pin); \
	HAL_GPIO_WritePin(RELAY_DEBUG_DIN_PASSTHROUGH_GPIO_Port, RELAY_DEBUG_DIN_PASSTHROUGH_Pin, (HAL_GPIO_ReadPin(R_DIN_GPIO_Port, R_DIN_Pin)))







uint32_t DRV8860_FaultRead();
void DRV8860_Update_Driver_Output(uint16_t pattern);

#endif// ifndef _DRV8860_H_
