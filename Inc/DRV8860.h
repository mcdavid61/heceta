#ifndef _DRV8860_H_
#define _DRV8860_H_

#include <stdint.h>
#include "main.h"

//	The following macros are defined in such a way
//	to allow the debugging pins to mirror the output
//	being sent to the DRV8860.
#ifdef DEBUG_USE_J19_HEADER_AS_RELAY_OUTPUT

#define RELAY_DEBUG_LATCH_Pin SPI3_CS_Pin
#define RELAY_DEBUG_LATCH_GPIO_Port SPI3_CS_GPIO_Port
#define RELAY_DEBUG_CLK_Pin LED_GREEN_Pin
#define RELAY_DEBUG_CLK_GPIO_Port LED_GREEN_GPIO_Port
#define RELAY_DEBUG_DOUT_Pin LED_RED_Pin
#define RELAY_DEBUG_DOUT_GPIO_Port LED_RED_GPIO_Port

#define RELAY_DEBUG_DIN_PASSTHROUGH_Pin	LED_AMBER_Pin
#define RELAY_DEBUG_DIN_PASSTHROUGH_GPIO_Port	LED_AMBER_GPIO_Port

#define DRV8860_PIN_PASSTHROUGH() \
	HAL_GPIO_WritePin(RELAY_DEBUG_DIN_PASSTHROUGH_GPIO_Port, RELAY_DEBUG_DIN_PASSTHROUGH_Pin, (HAL_GPIO_ReadPin(R_DIN_GPIO_Port, R_DIN_Pin)))
#define DRV8860_PIN_CLK_DBG(B) \
	HAL_GPIO_WritePin(RELAY_DEBUG_CLK_GPIO_Port, RELAY_DEBUG_CLK_Pin, B); \
	DRV8860_PIN_PASSTHROUGH();
#define DRV8860_PIN_LAT_DBG(B) \
	HAL_GPIO_WritePin(RELAY_DEBUG_LATCH_GPIO_Port, RELAY_DEBUG_LATCH_Pin, B); \
	DRV8860_PIN_PASSTHROUGH();
#define DRV8860_PIN_DOUT_DBG(B) \
	HAL_GPIO_WritePin(RELAY_DEBUG_DOUT_GPIO_Port, RELAY_DEBUG_DOUT_Pin, B); \
	DRV8860_PIN_PASSTHROUGH();
#define DRV8860_PIN_DIN_DBG() \
	DRV8860_PIN_PASSTHROUGH();
#define DRV8860_PIN_FLT_DBG() \
	DRV8860_PIN_PASSTHROUGH();
#else

//	If these macros are in use, debugging is entirely disabled.
#define DRV8860_PIN_CLK_DBG(B)
#define DRV8860_PIN_LAT_DBG(B)
#define DRV8860_PIN_DOUT_DBG(B)
#define DRV8860_PIN_DIN_DBG()
#define DRV8860_PIN_FLT_DBG()

#endif


//	Macros to set pins simultaneously
#define DRV8860_PIN_CLK(B) \
	HAL_GPIO_WritePin(R_CLK_GPIO_Port, R_CLK_Pin, B); \
	DRV8860_PIN_CLK_DBG(B)
#define DRV8860_PIN_LAT(B) \
	HAL_GPIO_WritePin(R_LAT_GPIO_Port, R_LAT_Pin, B); \
	DRV8860_PIN_LAT_DBG(B)
#define DRV8860_PIN_DOUT(B) \
	HAL_GPIO_WritePin(R_DOUT_GPIO_Port, R_DOUT_Pin, B); \
	DRV8860_PIN_DOUT_DBG(B)
#define DRV8860_PIN_DIN() \
	HAL_GPIO_ReadPin(R_DIN_GPIO_Port, R_DIN_Pin); \
	DRV8860_PIN_DIN_DBG()
#define DRV8860_PIN_FLT() \
	HAL_GPIO_ReadPin(R_FLT_GPIO_Port, R_FLT_Pin); \
	DRV8860_PIN_FLT_DBG()


#define DRV8860_CR_OAEN  	(1 << 7)
#define DRV8860_CR_PWNC2  	(1 << 6)
#define DRV8860_CR_PWNC1  	(1 << 5)
#define DRV8860_CR_PWNC0  	(1 << 4)
#define DRV8860_CR_ET3  	(1 << 3)
#define DRV8860_CR_ET2  	(1 << 2)
#define DRV8860_CR_ET1  	(1 << 1)
#define DRV8860_CR_ET0  	(1 << 0)

#define DRV8860_FR_OUT8_OCP  	(1 << 15)
#define DRV8860_FR_OUT7_OCP  	(1 << 14)
#define DRV8860_FR_OUT6_OCP  	(1 << 13)
#define DRV8860_FR_OUT5_OCP  	(1 << 12)
#define DRV8860_FR_OUT4_OCP  	(1 << 11)
#define DRV8860_FR_OUT3_OCP  	(1 << 10)
#define DRV8860_FR_OUT2_OCP  	(1 << 9)
#define DRV8860_FR_OUT1_OCP  	(1 << 8)
#define DRV8860_FR_OUT8_OL  	(1 << 7)
#define DRV8860_FR_OUT7_OL  	(1 << 6)
#define DRV8860_FR_OUT6_OL  	(1 << 5)
#define DRV8860_FR_OUT5_OL  	(1 << 4)
#define DRV8860_FR_OUT4_OL  	(1 << 3)
#define DRV8860_FR_OUT3_OL  	(1 << 2)
#define DRV8860_FR_OUT2_OL  	(1 << 1)
#define DRV8860_FR_OUT1_OL  	(1 << 0)

#define DRV8860_A (0)
#define DRV8860_B (1)

typedef uint16_t DRV8860_FaultRegister_T;
typedef uint8_t DRV8860_DataRegister_T;
typedef uint8_t DRV8860_ControlRegister_T;

void DRV8860_ControlRegisterWrite(DRV8860_ControlRegister_T * aWrite, uint8_t nDevCount);
void DRV8860_DataRegisterWrite(DRV8860_DataRegister_T * aWrite, uint8_t nDevCount);
void DRV8860_ControlRegisterRead(DRV8860_ControlRegister_T * aRead, uint8_t nDevCount);
void DRV8860_DataRegisterRead(DRV8860_DataRegister_T * aRead, uint8_t nDevCount);

void DRV8860_Update_Driver_Output(uint16_t nPattern);

#endif// ifndef _DRV8860_H_
