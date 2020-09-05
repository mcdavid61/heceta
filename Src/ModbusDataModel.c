/*
 * ModbusDataModel.c
 *
 *  Created on: Aug 28, 2020
 *      Author: Constantino Flouras
 *
 *  Description:
 *  	The following module implements all of the MODBUS functions
 *  	supported by the Heceta Relay Module.
 *
 *  	Functions are implemented as such: each function will return a
 *  	ModbusException_T type. A successful action will return EXCEPTION_OK
 *  	(or value 0), and any parameters of which are pointers will contain
 *  	valid values. Any failure action will return the appropriate
 *  	ModbusException_T type. Returning the ModbusException_T type allows
 *  	the exception to propagate up the stack to the caller.
 *
 *  	It is safe to assume that a non-zero value
 *  	indicates some sort of failure.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ModbusDataModel.h"
#include "ModbusSlave.h"
/*
	Function:	ModbusDataModel_ReturnResetState()
				ModbusDataModel_ReturnSetState()
	Description:
		Explicitly returns reset (0) or set (1)
		at all times.
*/
uint16_t m_nTestCounter = 0;
uint16_t ModbusDataModel_ReturnResetState_uint16_t()
{
	return m_nTestCounter++;
}
bool ModbusDataModel_ReturnResetState()
{
	return false;
}
bool ModbusDataModel_ReturnSetState()
{
	return true;
}

/*
	Function:	ModbusDataModel_ReadCoil()
	Description:
		Attempts to read a specific coil, as defined by the
		ModbusDataModel.h file. If the specific coil does not exist,
		this function will return false.
*/
bool ModbusDataModel_ReadCoil(uint16_t nAddress, bool * bReturn)
{
	bool (*pReadFunction)(void) = NULL;
	void (*pWriteFunction)(void) = NULL;
	(void) pWriteFunction;

	bool bSuccess = false;

	//	Using the header file, determine where we can read the coil
	//	requested. If it's not defined, the function will remain NULL.
	switch(nAddress)
	{
		FOREACH_COIL(COIL);
		default:
			break;
	}

	//	Determine if there's a valid response for this particular address.
	if (pReadFunction != NULL)
	{
		//	There is.
		//	Figure out what the appropriate response is.
		if (bReturn != NULL)
		{
			(*bReturn) = pReadFunction();
		}

		//	Set the success variable to true.
		bSuccess = true;
	}
	else
	{
		//	There's no valid response for this particular address.
		//	Do not touch the value of (*bReturn).
		//	Set the success value to false.
		bSuccess = false;
	}

	return bSuccess;
}

/*
	Function:	ModbusDataModel_ReadHoldingRegister()
	Description:
		Attempts to read a specific register, as defined by the
		ModbusDataModel.h file. If the specific register does not exist,
		this function will return an exception.
*/
ModbusException_T ModbusDataModel_ReadHoldingRegister(uint16_t nAddress, uint16_t * nReturn)
{
	//	The ModbusException_T to return.
	//	For now, return the maximum value possible.
	ModbusException_T eReturn = MODBUS_EXCEPTION_UNKNOWN;

	//	Holding values, to store the read/write functions.
	//	These define the format of the functions.
	uint16_t (*pReadFunction)(void) = NULL;
	void (*pWriteFunction)(void) = NULL;

	(void) pWriteFunction;

	//	Using the header file, determine where we can read the coil
	//	requested. If it's not defined, the function will remain NULL.
	switch(nAddress)
	{
		FOREACH_HOLDING_REGISTER(HOLDING_REGISTER);
		default:
			break;
	}

	//	Determine if there's a valid response for this particular address.
	if (pReadFunction != NULL)
	{
		//	There is.
		//	Figure out what the appropriate response is.
		if (nReturn != NULL)
		{
			(*nReturn) = pReadFunction();
		}

		//	This was successful, as far as this function is concerned.
		//	Go ahead and set this function's return value to OK.
		eReturn = MODBUS_EXCEPTION_OK;
	}
	else
	{
		//	There's no valid response for this particular address.
		//	This should return an MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS
		//	since there's no way to process this address.
		eReturn = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
	}

	return eReturn;
}

/*
	Function:	ModbusDataModel_ReadInputRegister()
	Description:
		Attempts to read a specific register, as defined by the
		ModbusDataModel.h file. If the specific register does not exist,
		this function will return an exception.
*/
ModbusException_T ModbusDataModel_ReadInputRegister(uint16_t nAddress, uint16_t * nReturn)
{
	//	The ModbusException_T to return.
	//	For now, return the maximum value possible.
	ModbusException_T eReturn = MODBUS_EXCEPTION_UNKNOWN;

	//	Holding values, to store the read/write functions.
	//	These define the format of the functions.
	uint16_t (*pReadFunction)(void) = NULL;
	void (*pWriteFunction)(void) = NULL;

	(void) pWriteFunction;

	//	Using the header file, determine where we can read the coil
	//	requested. If it's not defined, the function will remain NULL.
	switch(nAddress)
	{
		FOREACH_INPUT_REGISTER(INPUT_REGISTER);
		default:
			break;
	}

	//	Determine if there's a valid response for this particular address.
	if (pReadFunction != NULL)
	{
		//	There is.
		//	Figure out what the appropriate response is.
		if (nReturn != NULL)
		{
			(*nReturn) = pReadFunction();
		}

		//	This was successful, as far as this function is concerned.
		//	Go ahead and set this function's return value to OK.
		eReturn = MODBUS_EXCEPTION_OK;
	}
	else
	{
		//	There's no valid response for this particular address.
		//	This should return an MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS
		//	since there's no way to process this address.
		eReturn = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
	}

	return eReturn;
}

