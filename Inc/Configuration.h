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

#include "Version.h"
#include "ModbusSlave.h"

//	Static configuration variables
#define TERM(T) T"\0"
#define QUOTE(Q) #Q
#define QUOTE_EXP(Q) QUOTE(Q)

#define VENDOR_NAME		TERM("Bacharach")
#define PRODUCT_CODE 	TERM("HMZ-RM1")
#define MAJOR_MINOR_REV QUOTE_EXP(SOFTWARE_VERSION_MAJOR)"."\
                        QUOTE_EXP(SOFTWARE_VERSION_MINOR)"."\
                        QUOTE_EXP(SOFTWARE_VERSION_BUILD)"\0"

#define CONFIGURATION_PARAMETER_UNLOCK_CODE (1234)
#define CONFIGURATION_PARAMETER_UNLOCK_TIMEOUT_MS (1 * 60000)
#define CONFIGURATION_MANUAL_OUTPUT_TIMEOUT_MS (1 * 60000)
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

	//	Fault relay map
	//	Stored as a uint16_t in EEPROM module

	//	Parameter Unlock Code
	uint16_t nParameterUnlockCode;
	uint32_t nParameterUnlockTimeout;

}	ModbusConfiguration_T;

/*
	Structure:	ManualOutputConfiguration_T
	Description:
		Configures the manual outputs (LEDs, etc.) for manual
		overrides. This is typically for debugging purposes.
*/
typedef struct
{
	//	Whether or not these status overrides are enabled.
	uint32_t nTimeout;
	bool bEnabled;

	//	The override values.
	bool bGreenLED;
	bool bRedLED;
	bool bAmberLED;

}	ManualOutputConfiguration_T;

void Configuration_Init(void);
void Configuration_Process(void);
uint16_t Configuration_GetModbusAddress(void);
uint16_t Configuration_GetBaudRate(void);
uint16_t Configuration_IsBaudRate19200();
uint16_t Configuration_GetParity(void);
uint16_t Configuration_GetStopBits(void);
uint16_t Configuration_GetMessageLength(void);

ModbusException_T Configuration_SetFaultRelayMap(uint16_t nFaultRelayMap);
uint16_t Configuration_GetFaultRelayMap();


ModbusException_T Configuration_SetParameterUnlockCode(uint16_t nParameterUnlockCode);
uint16_t Configuration_GetParameterUnlockCode(void);

ModbusException_T Configuration_SetManualOverrideEnabled(uint16_t nValue);
uint16_t Configuration_GetManualOverrideEnabled(void);

ModbusException_T Configuration_SetGreenLED(uint16_t nValue);
ModbusException_T Configuration_SetRedLED(uint16_t nValue);
ModbusException_T Configuration_SetAmberLED(uint16_t nValue);
uint16_t Configuration_GetGreenLED(void);
uint16_t Configuration_GetRedLED(void);
uint16_t Configuration_GetAmberLED(void);

uint16_t Configuration_GetMajorVersion(void);
uint16_t Configuration_GetMinorVersion(void);
uint16_t Configuration_GetBuildVersion(void);

uint16_t Configuration_GetControllerUID_1_2(void);
uint16_t Configuration_GetControllerUID_3_4(void);
uint16_t Configuration_GetControllerUID_5_6(void);
uint16_t Configuration_GetControllerUID_7_8(void);

bool Configuration_MB_VendorName(uint8_t * pBuffer, uint8_t nBufferLen,	uint16_t * nBufferUsed);
bool Configuration_MB_ProductCode(uint8_t * pBuffer, uint8_t nBufferLen, uint16_t * nBufferUsed);
bool Configuration_MB_MajorMinorRevision(uint8_t * pBuffer, uint8_t nBufferLen,	uint16_t * nBufferUsed);

#endif /* CONFIGURATION_H_ */
