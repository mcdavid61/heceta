/*-----------------------------------------------------------------------------
 *  @file		Command.c
 *
 *  @copyright	2020, Bacharach Inc. as an unpublished work
 *              All Rights Reserved.
 *
 * 	The information contained herein is confidential property of Bacharach Inc.
 *  The use, copying, transfer or disclosure of such information is prohibited
 *  except by express written agreement with Bacharach Inc.
 *
 *  @date		Feb 13, 2020
 *  @author 	dmcmasters
 *
 *  @brief
 *-------------------------------------------------------------------------------*/

#include "Command.h"
#include "main.h"
#include "Relay.h"
#include "Switches.h"
#include "ByteFIFO.h"
#include "string.h"

#define	COMM_TIMEOUT_LIMIT	5000

bool	CommTimeout = FALSE;
uint32_t commTimeoutCounter = 0;
uint32_t commTimeoutTick = 0;


//	Incoming byte variable
//	This is the location of which incoming bytes received from
//	the USART3 interrupt are stored. Upon completion of an incoming byte,
//	this will eventually be transferred into a FIFO, which will then be read
//	by the command processor.
static uint8_t m_nIncomingByte = '\0';

//	ByteFIFO for incoming serial bytes.
//	When we're ready to take in this information for the command processor,
//	we'll use special access handlers that place this data into logical
//	input/output buffers.
DEFINE_STATIC_BYTE_FIFO(m_sCommandBufferFIFO, 128);

//	Input buffer
static char m_aSerialInputBuffer[SERIAL_INPUT_BUFFER_SIZE] = {0};
static uint32_t m_nSerialInputBufferPos = 0;

//	Output buffer
static char m_aSerialOutputBuffer[SERIAL_OUTPUT_BUFFER_SIZE] = {0};


extern uint16_t ADC_24V_Mon;
extern uint16_t ADC_3V3_Mon;
extern uint16_t ADC_Temperature;
extern uint16_t ADC_VrefInt;

bool Command_Has_Comm_Timed_Out(void)
{
	return CommTimeout;
}

typedef enum
{
	COMMAND_INIT,
	COMMAND_IDLE,
}	Command_State_T;

Command_State_T m_eCommandState = COMMAND_INIT;
bool m_bCharDataArrived = false;

/*
	Function:	Command_PrepareForInput()
	Description:
		Sets up the Command.c module to accept input
		from the USART3 serial.
*/
bool Command_PrepareForInput()
{
	//	Grab the USART3 handle.
	UART_HandleTypeDef * pUSART = Main_Get_UART_Handle();

	//	Result of our request.
	HAL_StatusTypeDef eResult;

	//	Do the request, and store the result.
	eResult = HAL_UART_Receive_IT(pUSART, &m_nIncomingByte, 1);

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
	Function:	Command_Process()
	Description:
		Main process function for the serial menu.
		This has been adjusted to allow for usage of interrupts.
*/
void Command_Process(void)
{
	switch(m_eCommandState)
	{
		case COMMAND_INIT:
			//	We haven't yet asked for a command at this point in time.
			//	Go ahead and do so.
			if (Command_PrepareForInput())
			{
				m_eCommandState = COMMAND_IDLE;
			}
			break;

		case COMMAND_IDLE:
			//	We're waiting to receive input from the user.
			//	We won't do anything ourselves to get this input, but rather,
			//	whenever it comes in, we'll rely on the interrupt to give us that data.
			Command_CollectRS232Input(m_aSerialInputBuffer, SERIAL_INPUT_BUFFER_SIZE, &m_nSerialInputBufferPos);

			//	If something has arrived... go ahead and accept it.
			if (m_nSerialInputBufferPos > 0)
			{
				char nIncomingChar = m_aSerialInputBuffer[0];
				Command_ClearInputBuffer();

				switch(nIncomingChar)
				{
				case 'r':
				case 'R':
					Relay_Run_Demo();
					break;

				case 'v':
				case 'V':
					printf("\n\r");
					printf("╔══════════╤══════════╤══════════╤══════════╗\n\r");
					printf("║    24V   │   3.3V   │ VrefInt  │  Temp.   ║\n\r");
					printf("╟──────────┼──────────┼──────────┼──────────╢\n\r");
					printf("║ %5d mV │ %5d mV │ %5d mV │  %3d °C  ║\n\r", ADC_24V_Mon, ADC_3V3_Mon, ADC_VrefInt, ADC_Temperature);
					printf("╚══════════╧══════════╧══════════╧══════════╝\n\r");
					break;

				case 's':
				case 'S':
					printf("\n\r");
					printf("╔═══╤═══╤═══╤═══╤═══╤═══╤═══╤═══╗\n\r");
					printf("║ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 ║\n\r");
					printf("╟───┼───┼───┼───┼───┼───┼───┼───╢\n\r");
					printf("║ %d │ %d │ %d │ %d │ %d │ %d │ %d │ %d ║\n\r", SW1, SW2, SW3, SW4, SW5, SW6, SW7, SW8);
					printf("╚═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╝\n\r");
					break;

				case '[':
					Relay_Set_Relay(0);
					break;

				case ']':
					Relay_Set_Relay(0xFFFF);
					break;

				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					Relay_Set_Relay(1 << (nIncomingChar - '1'));
					break;

				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
				case 'g':
					Relay_Set_Relay(1 << (nIncomingChar - 'a' + 9));
					break;
				case '?':
				default:
					printf("\n\rR - Run demo\n\r");
					printf("S - Read switches\n\r");
					printf("V - Read voltages and temperature\n\r");
					printf("[ - All off.\n\r");
					printf("] - All on.\n\r");
					printf("1-9, a-g - Individual Relay\n\r");
					printf("? - Help\n\r");

					break;
				}

				printf("\n\r> ");
			}
	}
}


/*	HAL UART callback functions	*/

/*
	Function:	HAL_UART_RxCpltCallback()
	Description:
		Whenever data is received on the serial USART, go ahead and
		attempt to insert it into the appropriate FIFO.

*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//	Our input buffer, m_nIncomingByte, now contains
	//	the next byte sent across the USART.

	//	Go ahead and attempt to insert it into the FIFO.
	bool bInserted = ByteFIFO_Enqueue(&m_sCommandBufferFIFO, m_nIncomingByte);

	//	TODO:	There should be some handling here... for now,
	//			it doesn't actually do anything.

	/*
	 * This was the previous handling that I was using for debugging.
		I've left it as a comment for now, but it's safe to remove.

	if (bInserted)
	{
		//	We were able to insert this value into the FIFO.
		//	Go ahead and spit it back out on the serial console.
		printf("%c", m_nIncomingByte);
	}
	else
	{
		//	We couldn't insert this for whatever reason-- perhaps a full FIFO.
		//	Don't do anything at the moment-- this is lost data.
	}

	*/

	//	Initialize the next request for incoming data.
	Command_PrepareForInput();
}



/*
	Function:	HAL_UART_ErrorCallback()
	Description:
		Overwrites a weak function.
		Allows us to have some sense of control over failures
*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{

}

/*
    Function: Command_CollectRS232Input
    Description:
        Accumulates a line of text from the front panel RS-232 serial port.  This
    function will accumulate the entered text at the buffer pointed to by pBuff.

        The buffer is assumed to be nBufferLen bytes long.  This function maintains
    its current position within the buffer at *pPosition.

        When the function has received a '\r', it returns true.
*/
bool Command_CollectRS232Input(char * const pBuff, uint32_t nBufferLen, uint32_t * pBufferPos)
{
	//	A place to store the most recent input character
	//	grabbed from our FIFO.
	int32_t nInputChar = -1;

	//	A boolean to store whether or not we've seen a complete line
	//	of data.
	bool bLineOfDataFound = false;

	//	Iterator for where we're pointing in the buffer.
	//	Note that this location will change.
	char * pBuffIter;

	//	The following do-while loop collects input from the
	//	USART buffer, until one of the following occurs:
	//	-	an end of line '\r' or '\n' is seen.
	//	-	an escape character '\e' is seen.
	//	-	there is no more characters left in the buffer
	do
	{
		//	Update pBuffIter based on the current value of *pBufferPos
		pBuffIter = &(pBuff[(*pBufferPos)]);

		//	Dequeues a single character from the FIFO
		nInputChar = ByteFIFO_Dequeue(&m_sCommandBufferFIFO);

		//	If there's no character for us to dequeue, then exit the while loop.
		if (nInputChar == -1)
		{
			break;
		}

	    //	If it is a '\r' or '\n', append a null terminator, and return true.
	    if (nInputChar == '\r' || nInputChar == '\n')
	    {
	        //	Affix the null terminator to the last buffer position.
	        *pBuffIter = '\0';

	        //	Indicate that we've seen a line of data.
	        bLineOfDataFound = true;

	        //	Break out of the do-while loop.
	        break;
	    }

	    //	If it is an escape character, replace the input line with a solitary '\e'
		if (nInputChar == '\e')
		{
			//	Clear the entirety of the input buffer.
			memset(pBuff, 0, nBufferLen);

			//	Set the first and second characters to escape and null terminator respectively.
			pBuff[0] = '\e';
			pBuff[1] = '\0';

			//	Reset the position appropriately.
			(*pBufferPos) = 1;

			//	Indicate that we've seen a "line" of data.
			bLineOfDataFound = true;

			//	Break out of the do-while loop.
			break;
		}

		//	If it is a backspace character, remove a character from the buffer if one exists.
		if (nInputChar == 0x08 || nInputChar == 0x7F)

		{
			//	Is there anything for us to delete?
			if (*pBufferPos > 0)
			{
				//	There is, remove it.
				(*pBufferPos) -= 1;
				pBuff[(*pBufferPos)] = '\0';

				//	Echo out the erase.
				printf("\b \b");
			}

			continue;
		}

		//	Otherwise, this is a valid character.
		//	Go ahead and echo this out to the console, and append it,
		//	if there's room.
		//	Note that we're using nBufferLen - 1 to leave room for the
		//	null terminator, '\0'
		else if (*pBufferPos < nBufferLen - 1)
		{
			//	Insert character
			*pBuffIter = (char) nInputChar;

			//	Increment the index.
			(*pBufferPos) += 1;

			//	Echo out the character back.
			printf("%c", (char) (nInputChar & 0xFF) );
		}


	}
	while (nInputChar >= 0);

    return bLineOfDataFound;
}

/*
	Function:	Command_ClearInputBuffer
	Description:
		Clears the input buffer.
*/
void Command_ClearInputBuffer(void)
{
	memset(m_aSerialInputBuffer, 0, SERIAL_INPUT_BUFFER_SIZE);
	m_nSerialInputBufferPos = 0;
}

/*************************** END OF FILE **************************************/
