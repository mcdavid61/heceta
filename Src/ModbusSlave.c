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

//	Modbus will use its own FIFO structure.
//	This is necessary to store whether or not it meets the appropriate
//	timing criteria necessary to be considered contiguous.


//	Configuration of the ModbusSlave timer.
#define MODBUS_SLAVE_BAUD_RATE 			(19200)
#define MODBUS_SLAVE_CHARACTER_LEN		(11)
#define MODBUS_SLAVE_CHARACTER_TIME_US 	((unsigned int) (((MODBUS_SLAVE_CHARACTER_LEN) * 1000000) / MODBUS_SLAVE_BAUD_RATE))
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
	uint32_t nNanosecondsPerTimerTick = (1000000000 / nTimerTicksPerSec) + 1;

	//	Based on our baud rate, which is a variable, determine how fast a
	//	character time is.
	//	TODO:	This will eventually be a dynamic value, but for now, it is #define'd
	uint32_t nBaudRate = MODBUS_SLAVE_BAUD_RATE;

	//	Nanoseconds per char
	uint32_t nNanosecondsPerChar = (1000000000 / (nBaudRate / MODBUS_SLAVE_CHARACTER_LEN));

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
		Sets up the Command.c module to accept input
		from the USART3 serial.
*/
bool ModbusSlave_PrepareForInput()
{
	//	Grab the USART3 handle.
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
		Sets up the Command.c module to accept input
		from the USART3 serial.
*/

bool ModbusSlave_PrepareForOutput(uint8_t * pBuffer, uint32_t nBufferLen)
{
	//	Return value
	bool bReturn = false;

	//	Grab the USART3 handle.
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
	Function:	ModbusFunction_ReadCoilStatus()
	Description:
		Read coil status function, implemented as specified in
		Modbus Specification Protocol V1.1B
*/
void ModbusFunction_ReadCoilStatus(uint8_t * pInputBuffer, uint32_t nInputBufferLen,
		   	   	   	   	   	   	   uint8_t * pOutputBuffer, uint32_t nOutputBufferLen,
								   uint32_t * pOutputBufferLenUsed)
{
	//	Note that this is the raw pInputBuffer, meaning that
	//	the slave address is stored in byte 0.
	//	The beginning of the Modbus request PDU, or mb_req_pdu,
	//	starts at byte 1.
	const uint8_t * pMbReqPDU = &pInputBuffer[1];

	//	Check #1:	Function code supported.
	//	Since this ModbusFunction() was called, we're assuming that this passed.

	//	Check #2:	0x0001 <= Quantity of Outputs <= 0x07D0
	//	We need to ensure we didn't call too many or too few outputs.
	//	Quantity of outputs defined in byte 3 (upper) and 4 (lower)
	//	of mb_req_pdu.
	uint32_t nNumberOfCoils = (pMbReqPDU[3] << 8) | (pMbReqPDU[4]);

	if (nNumberOfCoils < 1 && nNumberOfCoils > 0x07D0)
	{
		//	We've exceed the bounds of the permitted quantity of outputs.
		//	Generate an exception response.

		//ModbusFunction_GenerateExceptionResponse(pOutputBuffer, pOutputBufferLen, nFunctionCode, nExceptionCode);
	}

	//	Check #3:	Starting address == OK
	//				Starting Address + Quantity of Outputs == OK
	//	As defined by the MODBUS Application Protocol Specification V1.1b,
	//	a failure here should throw an exception code 02, illegal data address.
	//	I would argue that any failure here (e.g. a request for a register that
	//	isn't defined in our data model) should also return the 02 exception code.

	//	Check #4:	ReadDiscreteOutputs == OK
	//	When we called all of our READ functions for the coils requested, did they run correctly?

	//	Time to build our response.

	//	Ensure that the entirety of the output buffer is memset() to 0
	memset(pOutputBuffer, 0, nOutputBufferLen);

	//	Address field (which Modbus slave is responding)
	pOutputBuffer[0] = MODBUS_SLAVE_ADDRESS;

	//	Response PDU
	uint8_t * pMbRespPDU = &pOutputBuffer[1];

	//	Function field
	pMbRespPDU[0] = 0x01;

	//	Byte Count
	//	To be filled in later.
	pMbRespPDU[1] = (nNumberOfCoils / 8) + !!(nNumberOfCoils % 8 != 0);

	//	Start inserting the requested registers into the output buffer.
	uint32_t nStartAddress = (pMbReqPDU[1] << 8) | (pMbReqPDU[2]);

	//	A relative register counter-- how many registers
	//	have been read in to return their status?
	uint16_t nRelativeCoilCounter = 0;

	//	Exception response?
	bool bException = false;

	while ( nRelativeCoilCounter < nNumberOfCoils)
	{
		//	Current address
		uint32_t nCurrentAddress = (nStartAddress + nRelativeCoilCounter);

		//	Attempt to read the value
		bool bValue;
		bool bSuccess;

		bSuccess = ModbusDataModel_ReadCoil(nCurrentAddress, &bValue);

		if (bSuccess)
		{
			//	This coil was able to be read.
			//	Insert this into the output.
			pMbRespPDU[2 + (nRelativeCoilCounter/8)] |= bValue << (nRelativeCoilCounter % 8);
		}
		else
		{
			//	This coil wasn't able to be read.
			//	Generate the exception response, and break out of the loop.
			//	TODO:	Figure out the best way to transition to this.
			break;
		}

		nRelativeCoilCounter++;
	}

	if (bException)
	{
		//	Handle code
		//ModbusFunction_GenerateExceptionResponse(pOutputBuffer, pOutputBufferLen, nFunctionCode, nExceptionCode);
	}
	else
	{
		//	We read all coils, good to return.
		//	Where do we need to put the CRC?
		//						SLAVE ADDR 	+ FUNCTION CODE	+ #BYTES 	+ VARIABLE BYTES
		uint32_t nCRCLocation = 1 			+ 1 			+ 1 		+ pMbRespPDU[1];


	}





	//	Ultimately, I'm going to tie the request processing of the following checks together:
	//	-	Starting Address == OK
	//	-	Starting Address + Quantity of Outputs == OK
	//		-->	Effectively translates to every single requested register is a readable register
	//		-->	If we encounter a register that isn't readable (e.g. NULL read function),
	//			then 0x02 (ILLEGAL DATA ADDRESS) is thrown.
	//		-->	If something goes wrong in the processing of a read function,
	//			then 0x04 (SLAVE DEVICE FAILURE) is thrown.


}


/*
	Function:	ModbusFunction_ReadHoldingRegisters()
	Description:
		Given the MODBUS Request PDU, generate a MODBUS Response PDU.
		Returns zero (no exception) upon success. Returns a
		non-zero value representing the error code upon failure.
*/
ModbusException_T ModbusFunction_ReadHoldingRegisters(	uint8_t * pMbReqPDU, uint32_t nMbReqPDULen,
		   	   	   	   	   	   	   	   			uint8_t * pMbRespPDU, uint32_t nMbRespPDULen,
												uint32_t * pMbRespPDUUsed)
{
	//	Check #1:	0x0001 <= Quantity of Outputs <= 0x07D0
	//	We need to ensure we didn't call too many or too few outputs.
	//	Quantity of outputs defined in byte 3 (upper) and 4 (lower)
	//	of mb_req_pdu.
	uint32_t nNumberOfRegisters = (pMbReqPDU[3] << 8) | (pMbReqPDU[4]);
	if (nNumberOfRegisters < 1 && nNumberOfRegisters > 0x007D)
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

	//	Clear pMbRespPDU
	memset(pMbRespPDU, 0, nMbRespPDULen);

	//	Function Field
	pMbRespPDU[0] = 0x03;

	//	Byte Count
	pMbRespPDU[1] = (2 * nNumberOfRegisters);

	//	Start inserting the requested registers into the output buffer.
	uint32_t nStartAddress = (pMbReqPDU[1] << 8) | (pMbReqPDU[2]);
	uint16_t nRelativeRegisterCounter = 0;

	//	Exception response?
	bool bException = false;

	while ( nRelativeRegisterCounter < nNumberOfRegisters)
	{
		//	Current address
		uint32_t nCurrentAddress = (nStartAddress + nRelativeRegisterCounter);

		//	Attempt to read the value
		uint16_t nValue;
		bool bSuccess;

		bSuccess = ModbusDataModel_ReadHoldingRegister(nCurrentAddress, &nValue);

		if (bSuccess)
		{
			//	This coil was able to be read.
			//	Insert this into the output.
			pMbRespPDU[2 + (nRelativeRegisterCounter * 2)] = 		(nValue >> 8) & 0xFF;
			pMbRespPDU[2 + (nRelativeRegisterCounter * 2) + 1] = 	(nValue) & 0xFF;
		}
		else
		{
			//	This coil wasn't able to be read.
			//	Generate the exception response, and break out of the loop.
			return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
		}

		nRelativeRegisterCounter++;
	}

	//	If we've reached this point, we've completed filling the pMbRespPDU.
	(*pMbRespPDUUsed) = 1 + 1 + (pMbRespPDU[1]);
	return MODBUS_EXCEPTION_OK;
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
		(*pOutputBufferSlaveAddr) = Configuration_GetSlaveAddress();
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

	uint8_t aMbRespPDU[MODBUS_PDU_LEN];
	uint8_t * pMbRespPDU = aMbRespPDU;
	uint32_t nMbRespPDULen = MODBUS_PDU_LEN;
	uint32_t nMbRespPDUUsed = 0;

	//	Build the exception handler
	//	Upon success, this value should remain zero.
	uint8_t nMbException = 0;

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
			ModbusFunction_ReadHoldingRegisters(   pMbReqPDU, nMbReqPDULen,
												   pMbRespPDU, nMbRespPDULen,
												   &nMbRespPDUUsed);
			break;
		default:
			nMbException = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
			break;
	}

	if (nMbException)
	{
		//	TODO:	There was an exception, we'll handle it later.
	}

	//	Append the CRC to the end of the response.
	uint32_t nTotalBytes =
			ModbusSlave_BuildFrame(	pMbRespPDU, nMbRespPDUUsed,
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
				if (m_aModbusSlaveInputBuffer[0] == 0x01)
				{
					//	Yep, it wants us to respond.

					//	Flip this to set so that we can transmit data.
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

					//	Build up the response.
					ModbusSlave_BuildResponse(m_aModbusSlaveInputBuffer, MODBUS_SLAVE_INPUT_BUFFER_SIZE,
											  m_aModbusSlaveOutputBuffer, MODBUS_SLAVE_OUTPUT_BUFFER_SIZE,
											  &m_nModbusSlaveOutputBufferPos);

					//	Attempt to send.
					bSent = ModbusSlave_PrepareForOutput(m_aModbusSlaveOutputBuffer, m_nModbusSlaveOutputBufferPos);
					if (bSent)
					{
						printf("response sent");
					}

					m_eModbusSlaveState = MODBUS_SLAVE_SEND_WAIT;
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

