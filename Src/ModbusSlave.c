/*
 * ModbusSlave.c
 *
 *  Created on: Aug 7, 2020
 *      Author: Constantino Flouras
 *
 * Description:
 * 		Implemetation of a Modbus slave for the Bacharach Heceta project.
 * 		It is assumed that the communication for this device will always be in the
 * 		format of request/response, e.g. we cannot receive additional requests while
 * 		we're responding.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "string.h"
#include "stm32l4xx_hal.h"
#include "main.h"
#include "ByteFIFO.h"
#include "ModbusSlave.h"
#include "ModbusDataModel.h"
#include "CRC.h"
#include "Configuration.h"
#include "LED.h"

//	Modbus will use its own FIFO structure.
//	This is necessary to store whether or not it meets the appropriate
//	timing criteria necessary to be considered contiguous.


//	Configuration of the ModbusSlave timer.
#define MODBUS_SLAVE_MSG_STREAM_MAXIMUM_GAP 	(1.5)
#define MODBUS_SLAVE_MSG_STREAM_TIMEOUT 		(3.5)

//	Determining how fast our timer is actually running.
//		MODBUS_SLAVE_TIMER_CLOCK is effectively ticks per seconds.
#define MODBUS_SLAVE_TIMER_CLOCK			(16000000)
#define MODBUS_SLAVE_TIMER_PRESCALER		(16)
//		Then, we divide this number by 100000 to get ticks per microsecond.
#define MODBUS_SLAVE_TIMER_TICKS_PER_US		((MODBUS_SLAVE_TIMER_CLOCK / MODBUS_SLAVE_TIMER_PRESCALER) / 1000000)
//		For any given MS value, convert it to ticks.
#define MODBUS_SLAVE_TIMER_MS_TO_TICKS(T)	(T * 1000 * MODBUS_SLAVE_TIMER_TICKS_PER_US)

//	Delays required for timer
static uint32_t m_n15CharTicks;
static uint32_t m_n35CharTicks;

//	ByteFIFO for incoming Modbus bytes.
//	When we're ready to take in this information for the Modbus Slave,
//	we'll use special access handlers that place this data into logical
//	input/output buffers.
DEFINE_STATIC_FIFO(m_sModbusSlaveBufferFIFO, ModbusByte_T, 128);

#define MODBUS_SLAVE_INPUT_BUFFER_SIZE 128
#define MODBUS_SLAVE_OUTPUT_BUFFER_SIZE 256

//	Input buffer
static uint8_t m_aModbusSlaveInputBuffer[MODBUS_SLAVE_INPUT_BUFFER_SIZE] = {0};
static uint32_t m_nModbusSlaveInputBufferPos = 0;

//	Output buffer
static uint8_t m_aModbusSlaveOutputBuffer[MODBUS_SLAVE_OUTPUT_BUFFER_SIZE] = {0};
static uint32_t m_nModbusSlaveOutputBufferPos = 0;

static bool m_bReadyToAcceptData = false;
static bool m_bSendingData = false;

/*
	Function:	ModbusSlave_GrabFIFO()
	Description:
		Returns a pointer to the FIFO structure.
*/
const FIFOControl_T * ModbusSlave_GetFIFO(void)
{
	return &m_sModbusSlaveBufferFIFO;
}

/*
	Function:	ModbusSlave_Init
	Description:
		Initialization for the ModbusSlave.
		The primary motivation for this initialization function is to calculate the
		necessary character timeouts in respect to the ModbusSlave timer's configuration.
*/
void ModbusSlave_Init(void)
{
	//	Determine the system clock rate.
	uint32_t nSystemClockRate = HAL_RCC_GetSysClockFreq();

	//	Determine the prescaler that our timer is set to.
	TIM_HandleTypeDef * phtim = Main_Get_Modbus_Slave_Timer_Handle();
	uint32_t nPrescaler = phtim->Instance->PSC;

	//	How fast is our clock running (e.g. how fast is our timer ticking?)
	//	Note that this is approximate, since I'm not using any form of
	//	floating point number calculation.
	uint32_t nTimerTicksPerSec = nSystemClockRate / (nPrescaler + 1);

	//	How long is each tick (in terms of nanoseconds)
	uint32_t nNanosecondsPerTimerTick = (1000000000 / nTimerTicksPerSec);

	//	Optional addition of one second to make this stricter
	//	nNanosecondsPerTimerTick += 1;

	//	Based on our baud rate, which is a variable, determine how fast a
	//	character time is.
	//	TODO:	This will eventually be a dynamic value, but for now, it is #define'd
	uint32_t nBaudRate = Configuration_GetBaudRate();

	//	Nanoseconds per char
	uint32_t nNanosecondsPerChar = (1000000000 / (nBaudRate / Configuration_GetMessageLength()));

	//	Update the number of ticks required for 1.5 and 3.5
	m_n15CharTicks = (nNanosecondsPerChar * 1.5) / nNanosecondsPerTimerTick;
	m_n35CharTicks = (nNanosecondsPerChar * 3.5) / nNanosecondsPerTimerTick;

}

/*
	Function:	ModbusSlave_Debug_StartTimer
	Description:
		Starts the ModbusSlave timer, used for determining if the incoming
		characters meet the proper criteria for incoming character times.
*/
void ModbusSlave_Debug_StartTimer()
{
	TIM_HandleTypeDef * phtim = Main_Get_Modbus_Slave_Timer_Handle();

	//	Disable the timer.
	HAL_TIM_Base_Stop(phtim);

	//	Clear the flags.
	phtim->Instance->SR &= ~(TIM_SR_CC1IF | TIM_SR_UIF);

	//	Reset the counter
	phtim->Instance->CNT = 0;

	//	Setup the timer values.
	ModbusSlave_SetupTimerValues(phtim);

	//	Simply starts the timer, for debugging purposes.
	HAL_TIM_Base_Start(phtim);
}

/*
	Function:	ModbusByte_IncomingMsgTimeoutFlag()
	Description:
		Spits out a boolean indicating whether or not the IncomingMsgTimeout flag was hit.
*/
bool ModbusByte_IncomingMsgTimeoutFlag()
{
	TIM_HandleTypeDef * phtim = Main_Get_Modbus_Slave_Timer_Handle();

	//	TODO:	Code to read from timer.
	bool bIncomingMsgTimeout = (phtim->Instance->SR & TIM_SR_UIF) != 0;
	return bIncomingMsgTimeout;
}

/*
	Function:	ModbusByte_IncomingMsgTimeoutFlag()
	Description:
		Spits out a boolean indicating whether or not the ContiguousDataTimeout flag was hit.
		The ContiguousDataTimeout
*/
bool ModbusByte_ContiguousDataTimeoutFlag()
{
	TIM_HandleTypeDef * phtim = Main_Get_Modbus_Slave_Timer_Handle();

	//	TODO:	Code to read from timer.
	bool bContiguousDataTimeout = (phtim->Instance->SR & TIM_SR_CC1IF) != 0;
	return bContiguousDataTimeout;
}

/*
	Function:	ModbusSlave_ConvertToModbusByte()
	Description:
		A ModbusByte_T structure pointer and a uint8_t is passed to this function.
		It will initialize the structure, assign the uint8_t to the nByte section of the
		structure, and determine based on the TIM whether or not this incoming character
		exceeded the CHAR_TIMEOUT or COMMAND_TIMEOUT
*/
void ModbusSlave_ConvertToModbusByte(ModbusByte_T * pModbusByte, uint8_t nByte, uint32_t nSR)
{
	//	Ensure that pModbusByte is not null.
	if (pModbusByte != NULL)
	{
		//	Begin to write out all zeroes.
		memset(pModbusByte, 0, sizeof(ModbusByte_T));

		//	Assign the parameters.
		pModbusByte->nByte = nByte;
		pModbusByte->bContiguousDataTimeout = (nSR & TIM_SR_CC1IF) != 0;
		pModbusByte->bIncomingMsgTimeout = (nSR & TIM_SR_UIF) != 0;

		//	Done.
	}
}

/*
	Function:	Modbus_UART_RxISR_8BIT
	Description:
		Modified version of the UART_RxISR_8BIT function that
		more appropriately handles the Modbus slave input.
*/
void ModbusSlave_UART_RxISR_8BIT(UART_HandleTypeDef *huart)
{
	uint16_t uhMask = huart->Mask;
	uint16_t  uhdata;

	//	Immediately grab the status register of the TIM2.
	uint32_t nTIM2_SR;
	nTIM2_SR = Main_Get_Modbus_Slave_Timer_Handle()->Instance->SR;

	/* Check that a Rx process is ongoing */
	if (huart->RxState == HAL_UART_STATE_BUSY_RX)
	{
		//	Reads the data from the RDR (Read Data Register)
		//	Note that by reading from RDR, we clear the RXNE flag in hardware.
		//	If any other data happens to come in after we're done here, we'll
		//	get interrupted again.
		uhdata = (uint16_t) READ_REG(huart->Instance->RDR);
		uint8_t res = (uint8_t)(uhdata & (uint8_t)uhMask);

		//	In tandem with the character result, determine whether or not we've
		//	met the appropriate timer requirements for this character.
		//	These booleans represent whether a "character time" or "gap time" has
		//	passed since the last queued message.
		ModbusByte_T sModbusByte = {0};
		ModbusSlave_ConvertToModbusByte(&sModbusByte, res, nTIM2_SR);

		//	Enqueues the result into the FIFO
		bool bInserted = FIFO_Enqueue(&m_sModbusSlaveBufferFIFO, &sModbusByte);

		//	Determine if we've overrun our FIFO. If so, we should disable
		//	the USART RX line for the time being, since we can't really
		//	take in any more data.
		if (!bInserted)
		{
			//	Some error handling that we'll worry about later.
		}

		//	Restart the incoming timer, clearing all flags.
		ModbusSlave_Debug_StartTimer();
	}
	else
	{
		/* Clear RXNE interrupt flag */
		__HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
	}
}


/*
	Function:	ModbusSlave_SetupTimerValues
	Description:
		Assigns the appropriate values to the period (ARR), CCR1 registers
		for the purposes of determining when timeouts have occurred.
*/
void ModbusSlave_SetupTimerValues(TIM_HandleTypeDef * htim)
{
	//	Setup the timer values.
	htim->Instance->CCR1 = m_n15CharTicks;
	htim->Instance->ARR =  m_n35CharTicks;

	//	Setup the timer interrupts.
	htim->Instance->DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);
}

/*
	Function:	ModbusSlave_UART_Receive_IT
	Description:
		A modified version of the HAL_UART_Receive_IT function.
		This version of the request function sets up the interrupt handler to accept
		a continuous stream of incoming bytes into a software-based FIFO.
*/
HAL_StatusTypeDef ModbusSlave_UART_Receive_IT(UART_HandleTypeDef *huart)
{
	//	Check that a Rx process is not already ongoing
	if (huart->RxState == HAL_UART_STATE_READY)
	{
		//	Lock the huart, since we're about to modify it.
		__HAL_LOCK(huart);

		//	Computation of UART mask to apply to RDR register
		UART_MASK_COMPUTATION(huart);

		huart->ErrorCode = HAL_UART_ERROR_NONE;
		huart->RxState = HAL_UART_STATE_BUSY_RX;

		//	Disable the overrun error detection
		SET_BIT(huart->Instance->CR3, USART_CR3_OVRDIS);

		//	Enable the UART Error Interrupt: (Frame error, noise error, overrun error)
		SET_BIT(huart->Instance->CR3, USART_CR3_EIE);

		//	Set the Rx ISR function pointer according to the data word length
		huart->RxISR = ModbusSlave_UART_RxISR_8BIT;

		__HAL_UNLOCK(huart);

		//	Enable the UART Parity Error interrupt and Data Register Not Empty interrupt
		SET_BIT(huart->Instance->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE);

		return HAL_OK;
	}
	else
	{
		return HAL_BUSY;
	}
}

/*
	Function:	ModbusSlave_PrepareForInput()
	Description:
		Sets up the ModbusSlave.c module to accept input
		from the USART1 serial.
*/
bool ModbusSlave_PrepareForInput()
{
	//	Grab the USART1 handle.
	UART_HandleTypeDef * pUSART = Main_Get_Modbus_UART_Handle();

	//	Result of our request.
	HAL_StatusTypeDef eResult;

	//	Do the request, and store the result.
	eResult = ModbusSlave_UART_Receive_IT(pUSART);

	//	Based on the result, do something about it.
	switch(eResult)
	{
		case HAL_OK:
			//	We've set up a request for an incoming character (just one)
			//	This is an OK state.
			return true;
			break;
		case HAL_ERROR:
			//	An error occurred.
			//	TODO:	Determine the best way to handle this.
			break;
		case HAL_BUSY:
			//	The USART is busy.
			//	TODO:	Determine the best way to handle this.
			break;
		default:
			break;
	}

	return false;
}



/*
	Function:	ModbusSlave_PrepareForOutput()
	Description:
		Sets up the ModbusSlave.c module to send output
		from the USART1 serial.
*/

bool ModbusSlave_PrepareForOutput(uint8_t * pBuffer, uint32_t nBufferLen)
{
	//	Return value
	bool bReturn = false;

	//	Grab the USART1 handle.
	UART_HandleTypeDef * pUSART = Main_Get_Modbus_UART_Handle();

	//	Result of our request.
	HAL_StatusTypeDef eResult;

	//	Are we currently sending data?
	if (!m_bSendingData)
	{
		//	We are now.
		m_bSendingData = true;

		//	Do the request, and store the result.
		eResult = HAL_UART_Transmit_IT(pUSART, pBuffer, nBufferLen);

		//	Based on the result, do something about it.
		switch(eResult)
		{
			case HAL_OK:
				//	We've set up a request for an incoming character (just one)
				//	This is an OK state.
				bReturn = true;
				break;
			case HAL_ERROR:
			case HAL_BUSY:
				//	Something went wrong.
				//	Guess we aren't actually sending, whoops.
				m_bSendingData = false;
				break;
			default:
				break;
		}
	}

	return bReturn;
}

typedef enum
{
	MODBUS_SLAVE_INIT,
	MODBUS_SLAVE_RECEIVE,
	MODBUS_SLAVE_SEND_WAIT,
}	Modbus_Slave_State_T;

static Modbus_Slave_State_T m_eModbusSlaveState = MODBUS_SLAVE_INIT;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == Main_Get_Modbus_UART_Handle())
	{
		m_bSendingData = false;
	}
}

//	MODBUS FRAMING
//		The following functions are responsible for ensuring that
//		an incoming Modbus command has proper framing / CRC


/*
	Function:	ModbusSlave_CheckCRC
	Description:
		Modified from Chris Prozzo's code from MGS-402.
		Takes in a pBuffer of nLength, and calculates the CRC.
		Assumes the CRC to verify against is stored in the last two bytes.
*/
bool ModbusSlave_CheckCRC(const uint8_t * pBuffer, uint32_t nBufferLen)
{
	//	Placeholder for our calculated CRCs
	uint16_t nCRCSent;
	uint16_t nCRCRecv;

	//	Result variable.
	bool bResult = false;

	//	Ensure that nBufferLen is at least 2.
	//	Guard to ensure we didn't pass a NULL pointer.
	if (pBuffer != NULL && nBufferLen >= 2)
	{
		//	Compute the CRC of nBufferLen bytes from pBuffer
		nCRCRecv  = CRC16(pBuffer, nBufferLen - 2);

		//	Build the CRC stored two bytes after nBufferLen.
		nCRCSent = *(pBuffer + nBufferLen - 1) * 256 + (*(pBuffer + nBufferLen - 2));

		//	Check if the CRC matches.
		if (nCRCSent == nCRCRecv)
		{
			bResult = true;
		}
	}

	return bResult;
}

/*
    Function: ModbusSlave_CollectInput
    Description:
        Accumulates a Modbus command from the RS485 USART.

        The buffer is assumed to be nBufferLen bytes long.  This function maintains
    	its current position within the buffer at *pBufferPos.

        This function will return true when it detects a complete Modbus command, with
        valid CRC. This function may also wipe the buffer if an invalid CRC is detected,
        and/or if a new round of data comes in.

        If this function returns true, the buffer contains a valid Modbus command.
*/
bool ModbusSlave_CollectInput(uint8_t * pBuff, uint32_t nBufferLen, uint32_t * pBufferPos)
{
	//	Grab this timer right away.
	//	We don't want to allow the timer to go off in the middle of
	//	grabbing necessary data, so we'll go ahead and grab this
	//	while it's safe to.
	bool bIncomingMsgTimeoutFlagCurrent = ModbusByte_IncomingMsgTimeoutFlag();

	//	A boolean to store whether or not we've gathered
	//	a complete Modbus command.
	bool bModbusCommandFound = false;

	//	A place to store the most recent input character
	//	grabbed from our FIFO.
	ModbusByte_T sInputChar = {0};

	//	Iterator for where we're pointing in the buffer.
	//	Note that this location will change.
	uint8_t * pBuffIter;

	//	The following while-loop collects input from the
	//	USART Modbus Slave buffer, and constantly loops until
	//	one of the following occurs:
	//		-	If a ModbusByte_T with the "M" bit is set, this indicates
	//			that the timeout for a complete Modbus message has elapsed.
	//			-->	Attempt to parse what is currently in the buffer, not including
	//				this byte with the "M" bit set.
	//				-	If the Modbus message in the buffer is valid, consider it to be complete
	//					and return true.
	//				-	If the Modbus message in the buffer is invalid, clear the buffer in its
	//					entirety and return false. Continue with processing the next bytes
	//					on the next call of this function.
	//		-	If there are no more bytes available, read TIM2 to determine if the timeout
	//			for a complete Modbus message has elapsed.
	//			--> Attempt to parse what is currently in the buffer.

	while (FIFO_Peek(&m_sModbusSlaveBufferFIFO, &sInputChar) && (*pBufferPos < nBufferLen))
	{
		//	Update pBuffIter based on the current value of *pBufferPos
		pBuffIter = &(pBuff[(*pBufferPos)]);

		//	Is the "M" bit set on this byte?
		//	If so, only allow this character to be enqueued if there is nothing
		//	else already in the buffer.
		if (sInputChar.bIncomingMsgTimeout && (*pBufferPos != 0))
		{
			//	It is, and there's something else in the buffer.
			//	Break out of the loop.
			break;
		}

		//	Otherwise, go ahead and enqueue this byte.
		if (FIFO_Dequeue(&m_sModbusSlaveBufferFIFO, &sInputChar))
		{
			//	This is a valid byte.
			//	Go ahead and append it, if there's room.
			//	Insert character
			*pBuffIter = (char) sInputChar.nByte;

			//	Increment the index.
			(*pBufferPos) += 1;
		}
	}

	//	Determine why we've escaped the while loop.
	//	-	Did the peek character have the "M" bit set?
	//	-	...or was there nothing else in the buffer, and so did the timer run out?
	if ( (sInputChar.bIncomingMsgTimeout || bIncomingMsgTimeoutFlagCurrent)
			&& *pBufferPos != 0)
	{
		//	This should be a complete Modbus command.
		//	CRC check
		bool bCRCPass = false;

		//	Do the CRC check and determine if that is the case.
		bCRCPass = ModbusSlave_CheckCRC( (const uint8_t *) pBuff, (*pBufferPos));

		//	Did the CRC check pass?
		if (bCRCPass)
		{
			//	This is a valid Modbus command.
			bModbusCommandFound = true;
		}
		else
		{
			//	This is an invalid Modbus command.
			//	Drop/wipe whatever is in the buffer.
			memset(pBuff, 0, nBufferLen);
			(*pBufferPos) = 0;
		}
	}

    return bModbusCommandFound;
}

/*
	Function:	ModbusFunction_Exception()
	Description:
		Given the MODBUS Request PDU, generate an exception response.
		Should always return MODBUS_EXCEPTION_OK, but if it doesn't,
		something has gone terribly wrong.
*/
ModbusException_T ModbusFunction_Exception(	uint8_t * pMbReqPDU, uint32_t nMbReqPDULen,
		   	   	   	   	   	   	   	   		uint8_t * pMbExcepRspPDU, uint32_t nMbExcepRspPDULen,
											uint32_t * pMbExcepRspPDUUsed, ModbusException_T eException)
{
	//	Memset the entirety of the exception response.
	memset(pMbExcepRspPDU, 0, nMbExcepRspPDULen);

	//	Write the values out to the pMbExcepRspPDU
	pMbExcepRspPDU[0] = pMbReqPDU[0] | 0x80;
	pMbExcepRspPDU[1] = eException;
	(*pMbExcepRspPDUUsed) = 2;

	return MODBUS_EXCEPTION_OK;

}

/*
	Function:	ModbusFunction_WriteRegisters()
	Description:
		Given the MODBUS Request PDU, generate a MODBUS Response PDU.
		Returns zero (no exception) upon success. Returns a
		non-zero value representing the error code upon failure.
*/
ModbusException_T ModbusFunction_WriteRegisters(		uint8_t * pMbReqPDU, uint32_t nMbReqPDULen,
		   	   	   	   	   	   	   	   					uint8_t * pMbRspPDU, uint32_t nMbRspPDULen,
														uint32_t * pMbRspPDUUsed,
														ModbusException_T (*pRegisterWrite)(uint16_t nAddress, uint16_t * nValue))
{
	//	Check #1:	0x0001 <= Quantity of Outputs <= 0x07D0
	//								AND
	//	Check #2:	Byte Count == Quantity of Registers * 2
	//	We need to ensure we didn't call too many or too few outputs.
	//	Quantity of outputs defined in byte 3 (upper) and 4 (lower)
	//	of mb_req_pdu.
	//
	//	Additionally, we need to ensure we've received
	//	enough bytes from the beginning to do this write.
	uint32_t nNumberOfRegisters = (pMbReqPDU[3] << 8) | (pMbReqPDU[4]);
	uint32_t nNumberOfBytes = (pMbReqPDU[5]);
	if (nNumberOfRegisters < 1 || nNumberOfRegisters > 0x007B
			|| (nNumberOfRegisters * 2) != nNumberOfBytes)
	{
		return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
	}

	//	Check #3:	Starting Address == OK
	//						AND
	//	Check #4:	Starting Address + Quantity of Registers == OK
	//	Check to ensure that the registers requested are valid registers
	//	that can be written to.
	//	NOTE:	I have adjusted this function slightly-- not only does it
	//			check the starting and ending register, it also checks every
	//			single register in between.

	uint32_t nStartAddress = (pMbReqPDU[1] << 8) | (pMbReqPDU[2]);
	uint16_t nRelativeRegisterCounter = 0;
	while ( nRelativeRegisterCounter < nNumberOfRegisters)
	{
		//	Current address
		uint32_t nCurrentAddress = (nStartAddress + nRelativeRegisterCounter);

		//	Attempt to write a "NULL" value.
		//	This effectively does nothing,
		ModbusException_T eException;

		eException = pRegisterWrite(nCurrentAddress, NULL);
		if (eException == MODBUS_EXCEPTION_OK)
		{
			nRelativeRegisterCounter++;
		}
		else
		{
			//	We can't write to this register.
			//	Throw the exception that was raised by the pRegisterWrite function.
			return eException;
		}
	}

	//	Restart the relative register counter at 0
	nRelativeRegisterCounter = 0;

	//	Now--we're actually going to do the write.
	while ( nRelativeRegisterCounter < nNumberOfRegisters)
	{
		//	Current address
		uint32_t nCurrentAddress = (nStartAddress + nRelativeRegisterCounter);

		//	Grab the appropriate bytes from the MbReqPDU-- the value we want to write.
		uint16_t nValue = (	(pMbReqPDU[6 + (nRelativeRegisterCounter*2)		] << 8) |
							(pMbReqPDU[6 + (nRelativeRegisterCounter*2) + 1 ]));
		uint16_t nValueResult = nValue;

		//	Attempt to write the value.
		ModbusException_T eException;
		eException = pRegisterWrite(nCurrentAddress, &nValueResult);

		if (eException != MODBUS_EXCEPTION_OK)
		{
			return eException;
		}

		//	If the nRegisterResult equal to the nRegisterValue?
		if (nValueResult != nValue)
		{
			//	Something went wrong-- we should have mirrored the response.
			return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
		}
		else
		{
			nRelativeRegisterCounter++;
		}
	}

	//	Our writes are complete.
	//	Now--build the response, if we've reached this point.

	//	Clear response
	memset(pMbRspPDU, 0, nMbRspPDULen);
	(*pMbRspPDUUsed) = 0;

	//	Build
	//	Function code
	pMbRspPDU[0] = pMbReqPDU[0];

	//	Starting address
	pMbRspPDU[1] = (nStartAddress >> 8) & 0xFF;
	pMbRspPDU[2] = (nStartAddress) 		& 0xFF;

	//	Number of registers
	pMbRspPDU[3] = (nRelativeRegisterCounter >> 8)  & 0xFF;
	pMbRspPDU[4] = (nRelativeRegisterCounter) 		& 0xFF;

	(*pMbRspPDUUsed) = 5;

	//	All good.
	return MODBUS_EXCEPTION_OK;
}

/*
	Function:	ModbusFunction_WriteRegister()
	Description:
		Given the MODBUS Request PDU, generate a MODBUS Response PDU.
		Returns zero (no exception) upon success. Returns a
		non-zero value representing the error code upon failure.
*/
ModbusException_T ModbusFunction_WriteRegister(		uint8_t * pMbReqPDU, uint32_t nMbReqPDULen,
		   	   	   	   	   	   	   	   					uint8_t * pMbRspPDU, uint32_t nMbRspPDULen,
														uint32_t * pMbRspPDUUsed,
														ModbusException_T (*pRegisterWrite)(uint16_t nAddress, uint16_t * nValue))
{
	//	Check #1:	0x0000 <= Register Value <= 0xFFFF
	//	Ensure that the register value is acceptable.
	uint16_t nRegisterValue = (pMbReqPDU[3] << 8) | (pMbReqPDU[4]);
	if (nRegisterValue < 0x0000 || nRegisterValue > 0xFFFF)
	{
		//	Throw an exception.
		//	This is merely to conform to the MODBUS specification, despite the fact
		//	this exception will never be reached due to 16-bit number limitations
		return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
	}

	//	Check #2:	Register Address == OK
	//	Check #3:	WriteSingleRegister == OK
	uint16_t nRegisterAddress = (pMbReqPDU[1] << 8) | (pMbReqPDU[2]);

	//	This variable should be initialized with the value we want to write.
	//	After running the pRegisterWrite function, it will be replaced with the
	//	actual value stored within the register. This value will then be used
	//	to generate our response.
	uint16_t nRegisterResult = nRegisterValue;
	ModbusException_T eResult = pRegisterWrite(nRegisterAddress, &nRegisterValue);

	if (eResult != MODBUS_EXCEPTION_OK)
	{
		return eResult;
	}

	//	If the nRegisterResult equal to the nRegisterValue?
	if (nRegisterResult != nRegisterValue)
	{
		//	Something went wrong-- we should have mirrored the response.
		return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
	}

	//	Build the response.
	//	Clear pMbRspPDU
	memset(pMbRspPDU, 0, nMbRspPDULen);

	//	The normal response is an echo of the request, returned after
	//	the register contents have been written.
	pMbRspPDU[0] = pMbReqPDU[0];

	pMbRspPDU[1] = (nRegisterAddress >> 8) & 0xFF;
	pMbRspPDU[2] = (nRegisterAddress) & 0xFF;

	pMbRspPDU[3] = (nRegisterResult >> 8) & 0xFF;
	pMbRspPDU[4] = (nRegisterResult) & 0xFF;
	(*pMbRspPDUUsed) = 5;

	//	All good.
	return MODBUS_EXCEPTION_OK;
}

/*
	Function:	ModbusFunction_ReadRegisters()
	Description:
		Given the MODBUS Request PDU, generate a MODBUS Response PDU.
		Returns zero (no exception) upon success. Returns a
		non-zero value representing the error code upon failure.
*/
ModbusException_T ModbusFunction_ReadRegisters(	uint8_t * pMbReqPDU, uint32_t nMbReqPDULen,
		   	   	   	   	   	   	   	   					uint8_t * pMbRspPDU, uint32_t nMbRspPDULen,
														uint32_t * pMbRspPDUUsed,
														ModbusException_T (*pRegisterRead)(uint16_t nAddress, uint16_t * nReturn))
{
	//	Check #1:	0x0001 <= Quantity of Outputs <= 0x07D0
	//	We need to ensure we didn't call too many or too few outputs.
	//	Quantity of outputs defined in byte 3 (upper) and 4 (lower)
	//	of mb_req_pdu.
	uint32_t nNumberOfRegisters = (pMbReqPDU[3] << 8) | (pMbReqPDU[4]);
	if (nNumberOfRegisters < 1 || nNumberOfRegisters > 0x007D)
	{
		return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
	}

	//	Check #2:	Starting address == OK
	//				Starting Address + Quantity of Outputs == OK
	//	As defined by the MODBUS Application Protocol Specification V1.1b,
	//	a failure here should throw an exception code 02, illegal data address.
	//	I would argue that any failure here (e.g. a request for a register that
	//	isn't defined in our data model) should also return the 02 exception code.

	//	Check #4:	ReadDiscreteOutputs == OK
	//	When we called all of our READ functions for the coils requested, did they run correctly?

	//	Clear pMbRspPDU
	memset(pMbRspPDU, 0, nMbRspPDULen);

	//	Function Field
	pMbRspPDU[0] = pMbReqPDU[0];

	//	Byte Count
	pMbRspPDU[1] = (2 * nNumberOfRegisters);

	//	Start inserting the requested registers into the output buffer.
	uint32_t nStartAddress = (pMbReqPDU[1] << 8) | (pMbReqPDU[2]);
	uint16_t nRelativeRegisterCounter = 0;

	while ( nRelativeRegisterCounter < nNumberOfRegisters)
	{
		//	Current address
		uint32_t nCurrentAddress = (nStartAddress + nRelativeRegisterCounter);

		//	Attempt to read the value
		uint16_t nValue;
		ModbusException_T eException;

		eException = pRegisterRead(nCurrentAddress, &nValue);

		if (eException == MODBUS_EXCEPTION_OK)
		{
			//	This coil was able to be read.
			//	Insert this into the output.
			pMbRspPDU[2 + (nRelativeRegisterCounter * 2)] = 		(nValue >> 8) & 0xFF;
			pMbRspPDU[2 + (nRelativeRegisterCounter * 2) + 1] = 	(nValue) & 0xFF;
		}
		else
		{
			//	This coil wasn't able to be read properly--an error occurred.
			//	Propagate the error up to the caller.
			return eException;
		}
		nRelativeRegisterCounter++;
	}

	//	If we've reached this point, we've completed filling the pMbRspPDU.
	(*pMbRspPDUUsed) = 1 + 1 + (pMbRspPDU[1]);

	return MODBUS_EXCEPTION_OK;
}

/*
	Function:	ModbusFunction_AppendObject()
	Description:
		Given a buffer, its length, and a nObjectID, attempts to
		add an object to the buffer in the format of
			[nObjectID, nSize, aData].
		If the object cannot fit or some other error occurs, 0 is returned.
		Otherwise, the number of bytes written is returned.

		Failure conditions:
		-	Couldn't read the ObjectID	-->	doesn't exist
		-	Couldn't read the ObjectID  --> doesn't fit

		How do we differentiate between the two?
*/
unsigned int ModbusFunction_AppendObject(	uint8_t * pBuffer,
											int nBytesLeft,
											uint8_t nObjectID,
											bool * bDoesntFit)
{
	//	Number of bytes that have been written out.
	uint8_t nBytesWritten = 0;
	uint8_t nBytesToBeUsed = 0;

	//	Can this object be read?
	if (MODBUS_EXCEPTION_OK == ModbusDataModel_ReadObjectID(nObjectID, NULL, 0, &nBytesToBeUsed))
	{
		//	Cool. Determine whether or not we can actually put this into the buffer.
		if ( ((nBytesToBeUsed + 2) < nBytesLeft))
		{
			//	Is the buffer defined?
			if ((pBuffer != NULL))
			{
				//	We can. Go ahead and do it.
				pBuffer[0] = nObjectID;
				pBuffer[1] = nBytesToBeUsed;
				ModbusDataModel_ReadObjectID(nObjectID, &(pBuffer[2]), (nBytesLeft-2), &nBytesWritten);

				//	Account for the two bytes written at the beginning.
				nBytesWritten += 2;
			}
		}
		else
		{
			//	Uh-oh-- it doesn't fit.
			if (bDoesntFit != NULL)
			{
				(*bDoesntFit) = true;
			}
		}
	}

	//	Finally, return the bytes that were truly written out.
	return nBytesWritten;
}











/*
	Function:	ModbusFunction_ReadDeviceIdentification
	Description:
		Device identification functions and subfunctions.
		Note that this function will also require the MEI Type to be set to 14 (0x0E),
		otherwise, it will return an illegal function exception.
*/
ModbusException_T ModbusFunction_ReadDeviceIdentification(	uint8_t * pMbReqPDU, uint32_t nMbReqPDULen,
	   	   													uint8_t * pMbRspPDU, uint32_t nMbRspPDULen,
															uint32_t * pMbRspPDUUsed)
{
	//	Storage for an exception response.
	ModbusException_T eException;

	//	Check #1:	MEI Type == 0x0E
	//	If this isn't the correct MEI Type, this is effectively an
	//	invalid function request, and as such, we'll report so.
	uint8_t nMEIType = (pMbReqPDU[1]);
	if (nMEIType != 0x0E)
	{
		eException = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
		return eException;
	}

	//	Check #2:	Object ID OK
	//	Determine whether or not this Object ID exists
	//	Note that this isn't the place where we'll do the actual
	//	read-- there are more checks that need to be done.
	uint8_t nObjectID = (pMbReqPDU[3]);


	eException = ModbusDataModel_ReadObjectID(nObjectID, NULL, 0, NULL);
	if (eException != MODBUS_EXCEPTION_OK)
	{
		//	This wasn't a valid Object ID.
		//	Throw the exception that was raised by the function.
		return eException;
	}

	//	We're going to begin to build the response.
	//	Note that it's certainly possible for us to fail later-- that's okay,
	//	as it'll be indicated by the eException value returned.
	memset(pMbRspPDU, 0, nMbRspPDULen);
	(*pMbRspPDUUsed) = 0;

	//	Basically the "header" of all responses.
	//	Some of these values will get changed later.
	pMbRspPDU[0] = 0x2B;			//	Function Code
	pMbRspPDU[1] = 0x0E;			//	MEI Type
	pMbRspPDU[2] = pMbReqPDU[2];	//	Read Dev ID code
	pMbRspPDU[3] = pMbRspPDU[2];	//	Conformity level	(TODO: echos for now)
	pMbRspPDU[4] = 0;				//	More follows flag (TODO: implement)
	pMbRspPDU[5] = nObjectID;		//	Next Object ID
	pMbRspPDU[6] = 0;				//	Number of objects
	(*pMbRspPDUUsed) += 7;

	//	Helper variables, for our loop.
	int nObjectIDMax = 0;
	int nObjectIDStart = nObjectID;
	int nNumObjectsAdded = 0;

	int nObjectIDRelativeCnt = 0;
	
	//	The list of Objects will start at [7]
	uint8_t * pNextObject = &(pMbRspPDU[7]);
	int nNumberOfBytesLeft = (int) (pMbRspPDU + nMbRspPDULen - &(pMbRspPDU[7]));

	//	Check #3:	Read DeviceID Code OK
	//	This determines the type of read of which we'd like to complete.
	uint8_t nReadDevIDCode = (pMbReqPDU[2]);
	switch (nReadDevIDCode)
	{
		//	Category:	Basic
		case 0x01:

			//		ObjID	ObjName					ObjType
			//		-----	-------					-------
			//		0x00	VendorName				ASCII_String
			//		0x01	ProductCode				ASCII_String
			//		0x02	MajorMinorRevision		ASCII_String

			//	We're required to return a set number of objects in this scenario.
			//	This will *always* be the range of 0x00-0x02.
			nObjectIDMax = 0x03;
			break;
		case 0x02:
			//	Regular identification
			//	(stream access only)
			nObjectIDMax = 0x80;
			break;
		case 0x03:
			//	Extended identification
			//	(stream access only)
			nObjectIDMax = 0xFF;
			break;
		case 0x04:
			//	One specific identification object
			//	(individual access only)
			nObjectIDMax = nObjectIDStart+1;
			break;
		default:
			//	If the ReadDevID code is illegal, send back
			//	an exception 0x03 response.
			eException = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
			break;
	}

	//	Primary loop, of which objects are inserted into the buffer
	while (	(nObjectIDStart + nObjectIDRelativeCnt) < nObjectIDMax)
	{
		bool bDoesntFit = false;
		int nBytesAdded = ModbusFunction_AppendObject(pNextObject, nNumberOfBytesLeft, (nObjectIDStart + nObjectIDRelativeCnt), &bDoesntFit);

		//	Determine if we've reached a failure scenario.
		//	And if so-- why?
		if (nBytesAdded == 0)
		{
			//	Depends on the device ID that we've requested--
			if ((nObjectIDStart + nObjectIDRelativeCnt) < 0x03)
			{
				eException = MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
				break;
			}
			else
			{
				//	Does this not fit?
				if (bDoesntFit)
				{
					//	Oh yeah we're out of room, go ahead and terminate.
					//	But this will be the next object.
					pMbRspPDU[4] = 0xFF;						//	More follows flag (TODO: implement)
					pMbRspPDU[5] = (nObjectIDStart + 			//	Next Object ID, pick up where we left off.
									nObjectIDRelativeCnt);

					//	Graceful break.
					break;
				}

				//	It's certainly possible that this isn't implemented.
				//	We can continue to move on.
			}
		}
		else
		{
			nNumObjectsAdded += 1;
			pNextObject += nBytesAdded;
			(*pMbRspPDUUsed) += nBytesAdded;
		}
		nObjectIDRelativeCnt += 1;


	}

	//	After we're done, write number of objects written
	pMbRspPDU[6] = nNumObjectsAdded;

	return eException;
}

/*
	Function:	ModbusSlave_BuildFrame
	Description:
		Build a complete, valid Modbus frame that can be sent out.
		Returns the number of bytes used in the final buffer.
*/
uint32_t ModbusSlave_BuildFrame(uint8_t * pPDU, uint32_t nPDUSize,
								uint8_t * pOutputBuffer, uint32_t nOutputBufferSize)
{
	uint32_t nOutputBufferBytesUsed = 0;

	//	Ensure that pOutputBuffer is not NULL,
	//	pPDU is not NULL, nPDUSize is greater than 0,
	//	and nOutputBufferSize is large enough to contain
	//	all of these components together.
	if (	(pOutputBuffer != NULL) &&
			(pPDU != NULL) &&
			(nPDUSize > 0) &&
			((1 + nPDUSize + 2) <= nOutputBufferSize))
	{
		//	Memset the entirety of the output buffer to zero.
		memset(pOutputBuffer, 0, nOutputBufferSize);

		//	Grab the slave address, and store it in the output buffer.
		uint8_t * pOutputBufferSlaveAddr = &pOutputBuffer[0];
		(*pOutputBufferSlaveAddr) = Configuration_GetModbusAddress();
		nOutputBufferBytesUsed += 1;

		//	Grab the pPDU, and store it in the output buffer.
		uint8_t * pOutputBufferPDU = &pOutputBuffer[1];
		memcpy(pOutputBufferPDU, pPDU, nPDUSize);
		nOutputBufferBytesUsed += nPDUSize;

		//	Calculate the CRC on the remaining bytes, and store it
		//	in the outgoing buffer.
		//	Note that the number of bytes to process is
		//	1 (slave address) + the nPDU size.
		uint8_t * pOutputBufferCRC = &pOutputBuffer[1 + nPDUSize];
		uint16_t nCRCRecv  = CRC16(pOutputBuffer, (uint32_t) (pOutputBufferCRC - pOutputBuffer));
		pOutputBufferCRC[0] = ((nCRCRecv) & 0xFF);
		pOutputBufferCRC[1] = ((nCRCRecv >> 8) & 0xFF);
		nOutputBufferBytesUsed += 2;
	}

	//	Return the number of bytes used in the output buffer.
	return nOutputBufferBytesUsed;
}














/*
	Function:	ModbusSlave_BuildRespond()
	Description:
		Given an incoming Modbus command within a buffer, generate
		a response and place it in an outgoing buffer.

		Boolean value
		Note that Modbus commands are variable length.
*/
void ModbusSlave_BuildResponse(uint8_t * pInputBuffer, uint32_t nInputBufferLen,
							   uint8_t * pOutputBuffer, uint32_t nOutputBufferLen,
							   uint32_t * pOutputBufferLenUsed)
{
#define MODBUS_PDU_LEN 256
	//	Recall that Modbus commands are variable length.
	//	To determine what we've got to work with, we need to determine
	//	the type of command that's being requested of us.

	//	Ignore the first byte-- that's the address.
	//	If we've reached this point, we're already assuming that this command
	//	is addressed to us and we're the ones that are supposed to respond.

	//	Parse the second byte-- that's the function requested.
	//	What did the master request that we do?
	uint8_t nFunctionCode = pInputBuffer[1];

	//	Grab the pMbReqPDU and pMbRespPDU
	uint8_t * pMbReqPDU = &pInputBuffer[1];
	uint32_t nMbReqPDULen = nInputBufferLen-1;

	uint8_t aMbRspPDU[MODBUS_PDU_LEN];
	uint8_t * pMbRspPDU = aMbRspPDU;
	uint32_t nMbRspPDULen = MODBUS_PDU_LEN;
	uint32_t nMbRspPDUUsed = 0;

	//	Build the exception handler
	//	Upon success, this value should remain zero.
	ModbusException_T eMbException = 0;

	switch(nFunctionCode)
	{
		case 0x01:
			//	Read Coil Status
			//
			//		QUERY:
			//		0	NODE
			//		1	FUNCTION
			//		2	START ADDRESS (HI)
			//		3	START ADDRESS (LO)
			//		4	#	POINTS HI
			//		5	#	POINTS LO



			break;
		case 0x03:
			eMbException = ModbusFunction_ReadRegisters(		pMbReqPDU, nMbReqPDULen,
												   	   	   	   	pMbRspPDU, nMbRspPDULen,
																&nMbRspPDUUsed,
																ModbusDataModel_ReadHoldingRegister);
			break;
		case 0x04:
			eMbException = ModbusFunction_ReadRegisters(		pMbReqPDU, nMbReqPDULen,
												   	   	   	   	pMbRspPDU, nMbRspPDULen,
																&nMbRspPDUUsed,
																ModbusDataModel_ReadInputRegister);
			break;
		case 0x06:
			eMbException = ModbusFunction_WriteRegister(		pMbReqPDU,	nMbReqPDULen,
																pMbRspPDU,	nMbRspPDULen,
																&nMbRspPDUUsed,
																ModbusDataModel_WriteHoldingRegister);
			break;
		case 0x10:
			eMbException = ModbusFunction_WriteRegisters(		pMbReqPDU,	nMbReqPDULen,
																pMbRspPDU,	nMbRspPDULen,
																&nMbRspPDUUsed,
																ModbusDataModel_WriteHoldingRegister);
			break;
		case 0x2B:
			eMbException = ModbusFunction_ReadDeviceIdentification(	pMbReqPDU,	nMbReqPDULen,
																	pMbRspPDU,	nMbRspPDULen,
																	&nMbRspPDUUsed);
			break;
		default:
			eMbException = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
			break;
	}

	if (eMbException != MODBUS_EXCEPTION_OK)
	{
		ModbusFunction_Exception(	pMbReqPDU, nMbReqPDULen,
				   	   	   	   	    pMbRspPDU, nMbRspPDULen,
								    &nMbRspPDUUsed, eMbException);
	}

	//	Append the CRC to the end of the response.
	uint32_t nTotalBytes =
			ModbusSlave_BuildFrame(	pMbRspPDU, nMbRspPDUUsed,
									pOutputBuffer, nOutputBufferLen);

	(*pOutputBufferLenUsed) = nTotalBytes;
}






















/*
	Function:	ModbusSlave_Process()
	Description:
		Main process function for the ModbusSlave.
*/
void ModbusSlave_Process(void)
{
	//	Storage for whether or not a Modbus command is ready.
	bool bValidModbusCommand = false;
	bool bSent = false;

	//	If incoming data hasn't been initialized yet, go ahead and do that.
	//	Note that this flag could become "unset" if for whatever reason, initializing
	//	does not pass.
	if (!m_bReadyToAcceptData)
	{
		if (FIFO_GetFree(&m_sModbusSlaveBufferFIFO))
		{
			m_bReadyToAcceptData = ModbusSlave_PrepareForInput();
		}
	}

	switch(m_eModbusSlaveState)
	{
		case MODBUS_SLAVE_INIT:
			m_eModbusSlaveState = MODBUS_SLAVE_RECEIVE;
			break;

		case MODBUS_SLAVE_RECEIVE:
			//	Request data from the FIFO.
			//	Note that this function will only return true if we've seen
			//	an entire Modbus command.
			bValidModbusCommand =
					ModbusSlave_CollectInput(
							m_aModbusSlaveInputBuffer,
							MODBUS_SLAVE_INPUT_BUFFER_SIZE,
							&m_nModbusSlaveInputBufferPos);

			if (bValidModbusCommand)
			{
				//	This is a valid Modbus command.
				//	Next, determine if this Modbus command is addressed to us.
				if (m_aModbusSlaveInputBuffer[0] == Configuration_GetModbusAddress())
				{
					//	Yep, it wants us to respond.

					//	Go ahead and indicate to the LED module that we're communicating.
					LED_CommunicationUpdate();

					//	Flip this to set so that we can transmit data.
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

					//	Build up the response.
					ModbusSlave_BuildResponse(m_aModbusSlaveInputBuffer, MODBUS_SLAVE_INPUT_BUFFER_SIZE,
											  m_aModbusSlaveOutputBuffer, MODBUS_SLAVE_OUTPUT_BUFFER_SIZE,
											  &m_nModbusSlaveOutputBufferPos);

					//	Attempt to send.
					bSent = ModbusSlave_PrepareForOutput(m_aModbusSlaveOutputBuffer, m_nModbusSlaveOutputBufferPos);
					m_eModbusSlaveState = MODBUS_SLAVE_SEND_WAIT;
				}
				else
				{
					//	While this is indeed a valid Modbus command, it isn't
					//	addressed to us specifically.

					//	This wasn't anything useful to us, go ahead and wipe the buffer.
					memset(m_aModbusSlaveInputBuffer, 0, MODBUS_SLAVE_INPUT_BUFFER_SIZE);
					m_nModbusSlaveInputBufferPos = 0;
				}
			}
			break;
		case MODBUS_SLAVE_SEND_WAIT:
			//	Interrupt will transition us out of this state.
			if (!m_bSendingData)
			{
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
				memset(m_aModbusSlaveInputBuffer, 0, MODBUS_SLAVE_INPUT_BUFFER_SIZE);
				m_nModbusSlaveInputBufferPos = 0;
				memset(m_aModbusSlaveOutputBuffer, 0, MODBUS_SLAVE_OUTPUT_BUFFER_SIZE);
				m_nModbusSlaveOutputBufferPos = 0;
				m_eModbusSlaveState = MODBUS_SLAVE_RECEIVE;
			}
			break;

	}
}












/*
	Function:	HAL_TIM_OC_DelayElapsedCallback
	Description:
		Callback function whenever the Capture Control Register is hit.
		This is used to wiggle a GPIO pin for the time being.
*/
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
}

