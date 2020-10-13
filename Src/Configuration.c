/*
 * Configuration.c
 *
 *  Created on: Sep 3, 2020
 *      Author: Constantino Flouras
 */
#include "main.h"
#include "Switches.h"
#include "Configuration.h"
#include "Version.h"
#include "ModbusSlave.h"

//	The active Modbus configuration in use.
//	This is what all accessors and modules will reference in order
//	to determine output parameters, timing, etc.
ModbusConfiguration_T m_sModbusConfiguration = {0};
ManualOutputConfiguration_T m_sManualOutputConfiguration = {0};

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
uint16_t Configuration_GetModbusAddress(void)
{
	return m_sModbusConfiguration.nModbusAddress;
}

/*
	Function:	Configuration_GetBaudRate()
				Configuration_IsBaudRate19200()
	Description:
		Returns the baud rate.
		The more specific function asking if the baud rate is
		19200 is used for the Modbus command address
*/
uint16_t Configuration_GetBaudRate(void)
{
	return m_sModbusConfiguration.nBaudRate;
}
uint16_t Configuration_IsBaudRate19200()
{
	return (m_sModbusConfiguration.nBaudRate == 19200);
}

/*
	Function:	Configuration_GetParity()
	Description:
		Returns the parity.
*/
uint16_t Configuration_GetParity(void)
{
	return m_sModbusConfiguration.nParity;
}

/*
	Function:	Configuration_GetStopBits()
	Description:
		Returns the stop bits.
*/
uint16_t Configuration_GetStopBits(void)
{
	return m_sModbusConfiguration.nStopBits;
}

/*
	Function:	Configuration_GetMessageLength()
	Description:
		Returns the message length in bits, based on the current configuration.
*/
uint16_t Configuration_GetMessageLength(void)
{
	return START_BITS + CHARACTER_BITS + !!(Configuration_GetParity()) + m_sModbusConfiguration.nStopBits;
}

/*
	Function:	Configuration_GetFaultRelayMap()
				Configuration_SetFaultRelayMap()
	Description:
		Returns the relay map.
*/
uint16_t Configuration_GetFaultRelayMap()
{
	return EEPROM_GetFaultRegisterMap();
}
ModbusException_T Configuration_SetFaultRelayMap(uint16_t nFaultRelayMap)
{
	//	By default, we only allow setting this parameter if
	//	the correct parameter unlock lock has been specified.
	ModbusException_T eReturn = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

	if (m_sModbusConfiguration.nParameterUnlockCode == CONFIGURATION_PARAMETER_UNLOCK_CODE)
	{
		EEPROM_SetFaultRegisterMap(nFaultRelayMap);
		eReturn = MODBUS_EXCEPTION_OK;
	}

	return eReturn;
}

/*
	Function:	Configuration_GetParameterUnlockCode()
				Configuration_SetParameterUnlockCode()
	Description:
		Grabs the parameter unlock code that is written to the configuration.
*/
uint16_t Configuration_GetParameterUnlockCode(void)
{
	return m_sModbusConfiguration.nParameterUnlockCode;
}
ModbusException_T Configuration_SetParameterUnlockCode(uint16_t nParameterUnlockCode)
{
	m_sModbusConfiguration.nParameterUnlockCode = nParameterUnlockCode;
	return MODBUS_EXCEPTION_OK;
}


//	Manual Output Configuration Parameters
//	The following are helper functions to set the Manual Output Configuration Parameters

/*
	Function:	Configuration_SetManualOverrideEnabled
	Description:
		Allows the MODBUS master to enable the manual override enable flag.
		Returns true or false depending on whether the value was allowed to be written.
*/
ModbusException_T Configuration_SetManualOverrideEnabled(uint16_t nValue)
{
	//	Return value.
	ModbusException_T eReturn = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;

	//	Is nValue equal to zero or one?
	if (nValue <= 1)
	{
		m_sManualOutputConfiguration.bEnabled = (bool) nValue;
		eReturn = MODBUS_EXCEPTION_OK;
	}

	return eReturn;
}
uint16_t Configuration_GetManualOverrideEnabled(void)
{
	return m_sManualOutputConfiguration.bEnabled;
}

ModbusException_T Configuration_SetGreenLED(uint16_t nValue)
{
	//	Return value.
	ModbusException_T eReturn = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;

	if (nValue <= 1)
	{
		m_sManualOutputConfiguration.bGreenLED = (bool) nValue;
		eReturn = MODBUS_EXCEPTION_OK;
	}

	return eReturn;
}
ModbusException_T Configuration_SetRedLED(uint16_t nValue)
{
	//	Return value.
	ModbusException_T eReturn = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;

	if (nValue <= 1)
	{
		m_sManualOutputConfiguration.bRedLED = (bool) nValue;
		eReturn = MODBUS_EXCEPTION_OK;
	}

	return eReturn;
}
ModbusException_T Configuration_SetAmberLED(uint16_t nValue)
{
	//	Return value.
	ModbusException_T eReturn = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;

	if (nValue <= 1)
	{
		m_sManualOutputConfiguration.bAmberLED = (bool) nValue;
		eReturn = MODBUS_EXCEPTION_OK;
	}

	return eReturn;
}
uint16_t Configuration_GetGreenLED(void)
{
	return m_sManualOutputConfiguration.bGreenLED;
}
uint16_t Configuration_GetRedLED(void)
{
	return m_sManualOutputConfiguration.bRedLED;
}
uint16_t Configuration_GetAmberLED(void)
{
	return m_sManualOutputConfiguration.bAmberLED;
}

//	Version information return is also handled by the configuration
//	Note that the version info is located in

/*
	Function:	Configuration_GetMajorVersion()
				Configuration_GetMinorVersion()
				Configuration_GetBuildVersion()
	Description:
		Returns the specified parameter.
*/
uint16_t Configuration_GetMajorVersion(void)
{
	return SOFTWARE_VERSION_MAJOR;
}
uint16_t Configuration_GetMinorVersion(void)
{
	return SOFTWARE_VERSION_MINOR;
}
uint16_t Configuration_GetBuildVersion(void)
{
	return SOFTWARE_VERSION_BUILD;
}

//	Module Controller UID information
//	There isn't a very clear definition for how this is generated yet
//	So uh... it returns fun hex values for now!

/*
	Function:	Configuration_GetControllerUID_1_2()
				Configuration_GetControllerUID_3_4()
				Configuration_GetControllerUID_5_6()
				Configuration_GetControllerUID_7_8()
	Description:
		Returns the specified parameter.
		UID 1_2: 12xxxxxx
		UID 3_4: xx34xxxx
		UID 5_6: xxxx56xx
		UID 7_8: xxxxxx78
*/
uint16_t Configuration_GetControllerUID_1_2(void)
{
	return 0x0123;
}
uint16_t Configuration_GetControllerUID_3_4(void)
{
	return 0x4567;
}
uint16_t Configuration_GetControllerUID_5_6(void)
{
	return 0x89AB;
}
uint16_t Configuration_GetControllerUID_7_8(void)
{
	return 0xCDEF;
}
