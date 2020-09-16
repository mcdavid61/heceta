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
#include <string.h>
#include "ModbusDataModel.h"
#include "ModbusSlave.h"
#include "Configuration.h"
#include "Relay.h"
#include "ADC.h"

/*
	Function:	ModbusDataModel_ReturnResetState()
				ModbusDataModel_ReturnSetState()
	Description:
		Explicitly returns reset (0) or set (1)
		at all times.
*/
uint16_t m_nTestCounter = 0;
void ModbusDataModel_WritePollingValue(uint16_t nPollingValue)
{
	m_nTestCounter = nPollingValue;
}
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
	void * pWriteFunction = NULL;
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
	void * pWriteFunction = NULL;

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
	Function:	ModbusDataModel_WriteHoldingRegister()
	Description:
		Attempts to write a specific register, as defined by the
		ModbusDataModel.h file. If the specific register does not exist,
		this function will return an exception.

		If the uint16_t * nValue pointer is NULL, this function will simply
		check to ensure that the register exists and is writeable, returning
		MODBUS_EXCEPTION_OK if that is the case, and MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS
		if not.
*/
ModbusException_T ModbusDataModel_WriteHoldingRegister(uint16_t nAddress, uint16_t * nValue)
{
	//	The ModbusException_T to return.
	//	For now, return the maximum value possible.
	ModbusException_T eReturn = MODBUS_EXCEPTION_UNKNOWN;

	//	Holding values, to store the read/write functions.
	//	These define the format of the functions.
	uint16_t (*pReadFunction)(void) = NULL;
	bool (*pWriteFunction)(uint16_t nValue) = NULL;

	(void) pReadFunction;

	//	Using the header file, determine where we can write the register
	//	requested. If it's not defined, the function will remain NULL.
	switch(nAddress)
	{
		FOREACH_HOLDING_REGISTER(HOLDING_REGISTER);
		default:
			break;
	}

	//	Determine if there's a valid response for this particular address.
	if (pWriteFunction != NULL)
	{
		//	There is.
		//	If there's a value that was requested to be written, go ahead
		//	and attempt to write it.
		//	If the value was invalid, the write function will return false
		//	and thus, we'll throw an illegal data value error.
		bool bValueWriteOK;
		if (nValue != NULL)
		{
			bValueWriteOK = pWriteFunction((*nValue));
		}

		//	This was successful, as far as this function is concerned.
		//	Go ahead and set this function's return value to OK.
		eReturn = bValueWriteOK ? MODBUS_EXCEPTION_OK : MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
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
	void * pWriteFunction = NULL;

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


/*
	Function:	ModbusDataModel_ReadObjectIDHelper_Str
	Description:
		Allows the caller to request data about a string-based object ID.
		The boolean-based return value will return whether or not the data was written out
		to the specified buffer.
		The pBuffer and nBufferLen variables represent the destination buffer.
		The *nBufferUsed variable represents a location where the number of bytes written out is saved.
		Even if the other variables are NULL and/or if the function returns false, the number of bytes
		that would have been written are still returned.
*/
bool ModbusDataModel_ReadObjectIDHelper_Str(uint8_t * pBuffer, int nBufferLen,	uint8_t * nBufferUsed, uint8_t * pStr)
{
	//	Return variable, to determine whether or not we could pull the string.
	//	This variable will only be true if the string was returned.
	bool bOK = false;

	//	Determine how long the string is that we need to copy.
	//	Whether or not the string is copied, as long as the nStr and nBufferUsed variables
	//	are correctly passed, this string length will be returned regardless.
	uint16_t nStrLen;
	if (pStr != NULL)
	{
		nStrLen = strlen(pStr);
	}

	//	If the buffer is not NULL and the buffer length is large enough, go ahead
	//	and copy the string into the buffer.
	if (pBuffer != NULL && nBufferLen > 0 && (nBufferLen - nStrLen) > 0 && nStrLen > 0)
	{
		memcpy(pBuffer, pStr, nStrLen);
		bOK = true;
	}

	//	If the buffer length used is requested to be returned, go ahead and return that.
	//	Note that as long as this variable is set to something, the bytes that would have been
	//	used is still returned.
	if (nBufferUsed != NULL)
	{
		(*nBufferUsed) = nStrLen;
	}

	//	Finally, return whether or not the bytes were actually copied.
	//	Yes, I'm aware this is kind of a weird function.
	return bOK;
}

/*
	Function:	ModbusDataModel_ReadObjectID
	Description:
		Attempts to read the Object ID as specified. If the pBuffer is null, it will simply
		return the OK status if there is a valid way to acquire this Object ID.
*/
ModbusException_T ModbusDataModel_ReadObjectID(	uint16_t nObjectID,	uint8_t * pBuffer,
																	int nBufferLen,
																	uint8_t * nBufferUsed)
{
	//	The ModbusException_T to return.
	//	For now, return the maximum value possible.
	ModbusException_T eReturn = MODBUS_EXCEPTION_UNKNOWN;

	//	Holding values, to store the read/write functions.
	//	These define the format of the functions.
	uint8_t * pASCIIStr = NULL;

	//	Using the header file, determine where we can read the ObjectID
	//	requested. If we can, add it to the buffer.
	switch(nObjectID)
	{
		FOREACH_OBJECT_ID(OBJECT_ID);
		default:
			break;
	}

	//	Determine if there's a valid response for this particular Object ID.
	if (pASCIIStr != NULL)
	{
		//	There is.
		//	Figure out what the appropriate response is.
		ModbusDataModel_ReadObjectIDHelper_Str(pBuffer, nBufferLen, nBufferUsed, pASCIIStr);

		eReturn = MODBUS_EXCEPTION_OK;
	}
	else
	{
		eReturn = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
	}

	return eReturn;
}


