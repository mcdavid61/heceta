/*
 * Configuration.c
 *
 *  Created on: Sep 3, 2020
 *      Author: Constantino Flouras
 */
#include "main.h"
#include "Switches.h"
#include "Configuration.h"

//	The active Modbus configuration in use.
//	This is what all accessors and modules will reference in order
//	to determine output parameters, timing, etc.
ModbusConfiguration_T m_sModbusConfiguration = {0};

/*
	Function:	Configuration_Init()
	Description:
		Based off of the DIP switches located on the unit, builds the
		configuration for the Heceta Relay Module communications.

		Note that this function is called as a precursor to the USART
		init, since the parity and stop bit selection is based off of that.
*/
void Configuration_Init(void)
{
	//	Read in DIP switches.
	//	Note that in this function, I've bit-flipped the entire number
	//	so that "ON" is 1 and "OFF" is 0.
	uint8_t nSwitches = ~(Switches_Read());

	//	Modbus Address
	uint8_t nModbusAddress = 0;

	nModbusAddress += !!(nSwitches & SWITCH_BIT(1)) 	? (40) 			: (0);
	nModbusAddress += !!(nSwitches & SWITCH_BIT(2)) 	? (20) 			: (0);
	nModbusAddress += !!(nSwitches & SWITCH_BIT(3)) 	? (10) 			: (0);
	nModbusAddress =  (nModbusAddress) 			? (nModbusAddress) 		: (80);

	m_sModbusConfiguration.nModbusAddress = nModbusAddress;

	//	Baud Rate
	m_sModbusConfiguration.nBaudRate = !!(nSwitches & SWITCH_BIT(8)) ? 19200 : 9600;

	//	Word Format

	//	Stop bits
	m_sModbusConfiguration.nStopBits = 1;
	if (!(nSwitches & SWITCH_BIT(6)) && !!(nSwitches & SWITCH_BIT(7)))
	{
		m_sModbusConfiguration.nStopBits = 2;
	}

	//	Parity
	m_sModbusConfiguration.nParity = PARITY_NONE;

	if (!!(nSwitches & SWITCH_BIT(6)))
	{
		m_sModbusConfiguration.nParity = !!(nSwitches & SWITCH_BIT(7)) ? PARITY_EVEN : PARITY_ODD;
	}
}

/*
	Function:	Configuration_GetModbusAddress()
	Description:
		Returns the Modbus address.
*/
uint8_t Configuration_GetModbusAddress(void)
{
	return m_sModbusConfiguration.nModbusAddress;
}

/*
	Function:	Configuration_GetBaudRate()
	Description:
		Returns the baud rate.
*/
uint16_t Configuration_GetBaudRate(void)
{
	return m_sModbusConfiguration.nBaudRate;
}

/*
	Function:	Configuration_GetParity()
	Description:
		Returns the parity.
*/
uint8_t Configuration_GetParity(void)
{
	return m_sModbusConfiguration.nParity;
}

/*
	Function:	Configuration_GetStopBits()
	Description:
		Returns the stop bits.
*/
uint8_t Configuration_GetStopBits(void)
{
	return m_sModbusConfiguration.nStopBits;
}

/*
	Function:	Configuration_GetMessageLength()
	Description:
		Returns the message length in bits, based on the current configuration.
*/
uint8_t Configuration_GetMessageLength(void)
{
	return START_BITS + CHARACTER_BITS + !!(Configuration_GetParity()) + m_sModbusConfiguration.nStopBits;
}

