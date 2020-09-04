/*
 * Configuration.h
 *
 *  Created on: Sep 3, 2020
 *      Author: Constantino Flouras
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define PARITY_NONE (0x00)
#define PARITY_ODD 	(0x01)
#define PARITY_EVEN (0x02)

#define START_BITS  (1)
#define CHARACTER_BITS  (8)

/*
	Structure:	ModbusConfiguration_T
	Description:
		Configures the RS485 line to accept MODBUS messages.
*/
typedef struct
{
	//	Stores the current address of this Heceta relay module.
	uint8_t nModbusAddress;

	//	Word Format
	uint16_t nBaudRate;
	uint8_t nParity;
	uint8_t nStopBits;

}	ModbusConfiguration_T;

void Configuration_Init(void);
uint8_t Configuration_GetModbusAddress(void);
uint16_t Configuration_GetBaudRate(void);
uint8_t Configuration_GetParity(void);
uint8_t Configuration_GetStopBits(void);
uint8_t Configuration_GetMessageLength(void);

#endif /* CONFIGURATION_H_ */
