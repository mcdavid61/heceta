/*
 * SPIFlash.c
 *
 *  Created on: Sep 24, 2020
 *      Author: BFS
 *
 *  Description:
 *  	Handles all of the non-volatile data and the corresponding requests.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "main.h"
#include "SPIFlash.h"
#include "stm32l4xx_hal.h"


typedef enum
{
	NVVER_V0,
	NVVER_MAX = 0xFFFF,
}	NVConf_Version_T;

typedef struct
{
	//	Configuration structure version
	NVConf_Version_T nVersion;

	//	Parameters
	uint16_t nFaultMap;

	//	CRC
	uint16_t nCRC;
}	NonVolatileConfiguration_T;



static const uint8_t m_nWREN = Write_Enable_WREN;    //	A 1 byte command buffer
static const uint8_t m_nWRDI = Write_Disable_WRDI;    //	A 1 byte command buffer
static const uint8_t m_nRDSR = Read_Status_Register_RDSR;




//	In previous implementations of the SPIFlash.c driver, the steps for each operation
//	were predetermined in an array, and the array of steps referenced elements outside
//	of the array itself.
//	For this implementation, I've chosen instead to use a dynamic SPIStep buffer that can
//	be "set-up" and used as requested by the SPIFlash driver.
//	There are numerous "helper" generator steps that can be called by the main generators.
static SPIStep_T m_aSPIStep[SPIFLASH_MAX_STEPS] = {0};
static int m_nSPIStepIndex = 0;
static void (*m_pSPIPostOperationFunc)(void *) = NULL;
static void * m_pSPIPostOperationFuncArgs = NULL;

//	State of SPIFlash_T
static SPIFlash_State_T m_eSPIFlashState = SPIFLASH_STATE_IDLE;
static bool m_bSPIOperationInProgress = false;

//	Write status
static SPIWriteStatus_T m_sSPIWriteStatus = {0};

//	Command buffer, location of where the command bytes are stored.
//	Note that not all fields of this command buffer will be used for all commands.
//	In the format of:	[Byte 0 (OPCODE)] [Byte 1 (UpperAddr)] [Byte 2 (LowerAddr)]
static uint8_t m_aCommandBuffer[3] = {0};

//	Data buffer
//	TODO:	Determine if it's worth it for us to internally have a data buffer.
//			I personally don't believe that it's worth it for us to.
static uint8_t m_aSPIDataBuffer[SPIFLASH_PAGE_SIZE];  	//	Outgoing Data / Incoming Data


//	Debug code
#define DEBUG_SPIFLASH_CONSTANT_READS_AND_WRITES
#ifdef DEBUG_SPIFLASH_CONSTANT_READS_AND_WRITES
	static uint16_t nInternalCounter = 0;
	static uint8_t aExpectedContentsBuffer[32] = {0};
	static uint8_t aTestBuffer[32] = {0};
#endif

/*
	-----------------------------------------------------------------------
	Category:	SPIStep_T Helper Functions
	Description:
		The following are SPIStep_T helper functions. They are used for the
		purpose of generating specific steps for the SPIStep_T array used
		when an SPI request is made.
	-----------------------------------------------------------------------
*/

/*
	Function:	SPIStep_WriteEnable
	Description:
		Configures the next SPIStep_T for a write enable / disable.
*/
void SPIStep_WriteEnable(SPIStep_T * pStep, bool bEnable)
{
	pStep->pTransmitData = bEnable ? &m_nWREN : &m_nWRDI;
	pStep->pReceiveData = NULL;
	pStep->nByteCount = 1;
	pStep->bSetCSHigh = true;
}

/*
	Function:	SPIStep_Write
	Description:
		Configures the next SPIStep_T for the actual write command.
		This will write bytes out of a buffer, with a maximum count of 32 per transaction.
		This function will enforce page writes such that they cannot logically cross page boundaries.
*/
void SPIStep_Write(SPIStep_T * pStep, uint8_t * pBuffer, uint16_t nPage, uint16_t nPageOffset, uint16_t nBytesToWrite)
{
	//	The following code ensures that we cannot cross page boundaries.
	//	If the page boundary is exceeded, write however many bytes are left in the page.
	uint32_t nByteCount = (nPageOffset + nBytesToWrite <= SPIFLASH_PAGE_SIZE) ? nBytesToWrite : (SPIFLASH_PAGE_SIZE - nPageOffset);

	pStep->pTransmitData = pBuffer;
	pStep->pReceiveData = NULL;
	pStep->nByteCount = nByteCount;
	pStep->bSetCSHigh = false;
}

/*
	Function:	SPIStep_Read
	Description:
		Configures the next SPIStep_T for the actual read command.
		This will read bytes in to a buffer, with no maximum count enforced.
*/
void SPIStep_Read(SPIStep_T * pStep, uint8_t * pBuffer, uint16_t nPage, uint16_t nPageOffset, uint16_t nBytesToRead)
{
	//	The following code ensures that we cannot cross page boundaries.
	//	If the page boundary is exceeded, write however many bytes are left in the page.
	pStep->pTransmitData = NULL;
	pStep->pReceiveData = pBuffer;
	pStep->nByteCount = nBytesToRead;
	pStep->bSetCSHigh = false;
}

/*
	Function:	SPIStep_Command
				SPIStep_CommandAddress
	Description:
		Configures the specified SPIStep_T as a specific op command, as specified by nCommand.
		If using the CommandAddress version, appends an address to the end as well.
*/
void SPIStep_Command(SPIStep_T * pStep, uint8_t nCommand)
{
	//	Set up the m_aCommandBuffer.
	m_aCommandBuffer[0] = nCommand;

	pStep->pTransmitData = m_aCommandBuffer;
	pStep->pReceiveData = NULL;
	pStep->nByteCount = 1;
	pStep->bSetCSHigh = false;
}
void SPIStep_CommandAddress(SPIStep_T * pStep, uint8_t nCommand, uint16_t nPage, uint16_t nPageOffset)
{
	//	Same as a regular command
	SPIStep_Command(pStep, nCommand);

	//	But append the address
	SPIFLASH_CONSTRUCT_ADDRESS((&m_aCommandBuffer[1]), nPage, nPageOffset);
	pStep->nByteCount += 2;
}

/*
	-----------------------------------------------------------------------
	Category:	SPIFlash Operation Functions
	Description:
		The following are SPIFlash Operation functions. They use the
		SPIStep_T helper functions to generate a complete list of steps
		used for their corresponding functions.
	-----------------------------------------------------------------------
*/

/*
	Function:	SPIStep_IsFree()
	Description:
		Determines whether or not the SPIStep / SPI bus is free, based on
		a simple memory compare of the entire range to zero.
*/
bool SPIFlash_IsFree()
{
	//	Boolean value, assumes that we are okay until otherwise determined.
	bool bFree = true;

	//	Pointer to the SPIStep_T structure
	uint8_t * pBuffer = (uint8_t *) &m_aSPIStep[0];

	//	Counters
	uint32_t nTotal = sizeof(m_aSPIStep);
	uint32_t nCntr = 0;

	//	Check
	for (nCntr = 0; nCntr < nTotal; nCntr++)
	{
		if (pBuffer[nCntr] != 0x00)
		{
			bFree = false;
			break;
		}
	}

	return bFree;
}


/*
	Function:	SPIFlash_PageWrite()
	Description:
		Kickstarts the process of writing a 32 byte (or smaller) page to the flash.
*/
bool SPIFlash_PageWrite(uint8_t * pBuffer, uint16_t nPage, uint16_t nPageOffset, uint16_t nSize)
{
	//	Boolean value, returning the success or failure of this function
	//	Until deemed otherwise, we must assume that this function will fail.
	bool bReturn = false;

	//	Determine if the SPIStep_T is in use.
	bool bSPIStepsClear = SPIFlash_IsFree();

	//	Determine if the criteria for actually setting this up has passed
	if ( (pBuffer != NULL) &&
			(nPage < SPIFLASH_PAGE_COUNT) &&
				(  (nPageOffset + nSize) < SPIFLASH_PAGE_SIZE) &&
					bSPIStepsClear)
	{
		//	Good to go.
		//	Setup the steps.
		SPIStep_WriteEnable(&m_aSPIStep[0], true);
		SPIStep_CommandAddress(&m_aSPIStep[1], Page_Program_PP, nPage, nPageOffset);
		SPIStep_Write(&m_aSPIStep[2], pBuffer, nPage, nPageOffset, nSize);
		m_aSPIStep[2].bSetCSHigh = true;
	}

	return bReturn;
}

/*
	Function:	SPIFlash_WriteHelper
	Description:
		The responsibility of this function is to set up the SPIFlash write steps
		based on the parameters defined in the SPIWriteStatus_T structure.

		It will also set up the complete callback step.
*/
void SPIFlash_WriteHelper(void * pArgs)
{
	SPIWriteStatus_T * pSPIWriteStatus = (SPIWriteStatus_T *) pArgs;

	//	The number of bytes of which will actually get written during this SPI write command.
	uint32_t nByteCount = (pSPIWriteStatus->nPageOffset + pSPIWriteStatus->nBytesLeft <= SPIFLASH_PAGE_SIZE) ? pSPIWriteStatus->nBytesLeft : (SPIFLASH_PAGE_SIZE - pSPIWriteStatus->nPageOffset);

	SPIStep_WriteEnable(&m_aSPIStep[0], true);
	SPIStep_CommandAddress(&m_aSPIStep[1], Page_Program_PP, pSPIWriteStatus->nPage, pSPIWriteStatus->nPageOffset);
	SPIStep_Write(&m_aSPIStep[2], pSPIWriteStatus->pBuffer, pSPIWriteStatus->nPage, pSPIWriteStatus->nPageOffset, nByteCount);
	m_aSPIStep[2].bSetCSHigh = true;

	//	Adjust the write status for the next operation.
	pSPIWriteStatus->pBuffer = &pSPIWriteStatus->pBuffer[nByteCount];
	pSPIWriteStatus->nPage = pSPIWriteStatus->nPage + 1;
	pSPIWriteStatus->nPageOffset = (pSPIWriteStatus->nPageOffset + nByteCount) % SPIFLASH_PAGE_SIZE;
	pSPIWriteStatus->nBytesLeft -= nByteCount;

	m_pSPIPostOperationFunc = pSPIWriteStatus->nBytesLeft ? SPIFlash_WriteHelper : NULL;
	m_pSPIPostOperationFuncArgs = pSPIWriteStatus->nBytesLeft ? pSPIWriteStatus : NULL;
}


/*
	Function:	SPIFlash_Write()
	Description:
		Kickstarts the process of writing a buffer to the flash.
		Note that this supports writing across page boundaries.
*/
bool SPIFlash_Write(uint8_t * pBuffer, uint16_t nPage, uint16_t nPageOffset, uint16_t nSize)
{
	//	Boolean value, returning the success or failure of this function
	//	Until deemed otherwise, we must assume that this function will fail.
	bool bReturn = false;

	//	Determine if the SPIStep_T is in use.
	bool bSPIStepsClear = SPIFlash_IsFree();

	//	Determine if the criteria for actually setting this up has passed
	//	Note that the third check, regarding nPageOffset has changed from its PageWrite equivalent.
	if ( (pBuffer != NULL) &&
			(nPage < SPIFLASH_PAGE_COUNT) &&
				(  (nPageOffset) < SPIFLASH_PAGE_SIZE) &&
					bSPIStepsClear)
	{
		//	We're in good shape to kick off this write.
		//	Go ahead and pass this to our "write helper" function.
		m_sSPIWriteStatus.pBuffer = pBuffer;
		m_sSPIWriteStatus.nPage = nPage;
		m_sSPIWriteStatus.nBytesLeft = nSize;
		m_sSPIWriteStatus.nPageOffset = nPageOffset;
		SPIFlash_WriteHelper(&m_sSPIWriteStatus);

		bReturn = true;
	}

	return bReturn;
}

/*
	Function:	SPIFlash_Read()
	Description:
		Kickstarts the process of reading from the flash to the buffer.
*/
bool SPIFlash_Read(uint8_t * pBuffer, uint16_t nPage, uint16_t nPageOffset, uint16_t nSize)
{
	//	Boolean value, returning the success or failure of this function
	//	Until deemed otherwise, we must assume that this function will fail.
	bool bReturn = false;

	//	Determine if the SPIStep_T is in use.
	bool bSPIStepsClear = SPIFlash_IsFree();

	//	Determine if the criteria for actually setting this up has passed
	if ( (pBuffer != NULL) &&
			(nPage < SPIFLASH_PAGE_COUNT) &&
				(  ((nPage * SPIFLASH_PAGE_SIZE) + nPageOffset + nSize) < SPIFLASH_CHIP_SIZE) &&
					bSPIStepsClear)
	{
		//	Good to go.
		//	Setup the steps.
		SPIStep_CommandAddress(&m_aSPIStep[0], Read_Data_Bytes_READ, nPage, nPageOffset);
		SPIStep_Read(&m_aSPIStep[1], pBuffer, nPage, nPageOffset, nSize);
		m_aSPIStep[1].bSetCSHigh = true;
		bReturn = true;
	}
	return bReturn;
}

/*
	Function:	SPIFlash_OperationCompleteCallback()
	Description:
		The operation pending is complete.
		Run the appropriate helper function, if it exists, and clear it if not.
		Clear the appropriate flags, so another operation may take place.
*/
void SPIFlash_OperationCompleteCallback()
{
	//	Clear the entirety of the m_aSPIStep
	memset(m_aSPIStep, 0, sizeof(m_aSPIStep));

	//	Capture the current callback pointers
	void (*pSPIPostOperationFunc)(void *) = m_pSPIPostOperationFunc;
	void * pSPIPostOperationFuncArgs = m_pSPIPostOperationFuncArgs;

	//	Reset the complete callback to NULL
	m_pSPIPostOperationFunc = NULL;
	m_pSPIPostOperationFuncArgs = NULL;

	//	Reset the step index to zero
	m_nSPIStepIndex = 0;

	//	If there is a helper function to call, go ahead and run it.
	if (pSPIPostOperationFunc != NULL)
	{
		//	If there is a helper function-- we will assume that the
		//	SPIFlash still needs to process something, so it will
		//	remain in the SPIFLASH_STATE_PROCESS state unless this
		//	function explicitly changes it / completes it for us.
		pSPIPostOperationFunc(pSPIPostOperationFuncArgs);
	}
	else
	{
		//	If there isn't a post-operation step that needs to be completed,
		//	then go ahead and assume that the SPIFlash is IDLE.
		m_eSPIFlashState = SPIFLASH_STATE_IDLE;
	}

	//	Note that it is the responsibility of the function to ensure that there
	//	is some form of a transition from the SPI
}



static NonVolatileConfiguration_T m_nNVDataCurrent;
static NonVolatileConfiguration_T m_nNVDataRead;













void SPIFlash_WriteNonVolatileConfiguration()
{

}

/*

*/

/*
    Function:   SPIFlash_SetupNextOperation
    Description:
        Configures the SPI flash to run the next step as specified
        in the SPIFlash_T array.

    WARNING:
    	Called from two locations: main processing loop as well as interrupts.

*/
static bool SPIFlash_SetupNextOperation(SPIStep_T * aSteps, int nSteps, int nStepIndex)
{
	HAL_StatusTypeDef eOperationStatus = 0xFF;
	bool bReturn = false;

	//	Guard to ensure that the parameters passed are indeed valid.
	if (aSteps != NULL)
	{
		//	Boolean flag, to indicate that one of these operations is in progress.
		//	The only way this flag can be "unset" is if the completion callback is hit.
		m_bSPIOperationInProgress = true;

		//	Go ahead and, via software, pull the chip select line low.
		HAL_GPIO_WritePin(EE_CS_GPIO_Port, EE_CS_Pin, GPIO_PIN_RESET);

		//	Figure out what the requested next step is
		SPIStep_T * pNextStep = &aSteps[nStepIndex];

		//	What does this next step want us to do?
		if (pNextStep->pTransmitData != NULL && pNextStep->nByteCount > 0)
		{
			//	Data transmit.
			eOperationStatus = HAL_SPI_Transmit_IT(Main_Get_SPI_Handle(), pNextStep->pTransmitData, pNextStep->nByteCount);
		}
		else if (pNextStep->pReceiveData != NULL && pNextStep->nByteCount > 0)
		{
			//	Data receive.
			eOperationStatus = HAL_SPI_Receive_IT(Main_Get_SPI_Handle(), pNextStep->pReceiveData, pNextStep->nByteCount);
		}

		//	If this was successful, we've kicked off this operation, and so
		//	we can proceed to move forward with the nStepIndex.
		if (eOperationStatus == HAL_OK)
		{
			bReturn = true;
		}
		else
		{
			//	Ultimately, nothing was run.
			//	Go ahead and "unset" this flag before we escape.
			m_bSPIOperationInProgress = false;
		}
	}

	return bReturn;
}

/*
	Function:	HAL_SPI_TxCpltCallback()
				HAL_SPI_RxCpltCallback()
				HAL_SPI_TxRxCpltCallback()
	Description:
		Callback functions, as specified by the HAL.
		These are overridden from their weak definitions so that they call our callback
		that was specified.
*/
void SPIFlash_Callback(SPIStep_T * aSteps, int nSteps, int * nStepIndex)
{
	//	Callback function is shared amongst the three types of callbacks
	//	that could be possibly triggered.
	bool bHandled = false;

	//	No longer an operation/step in progress.
	m_bSPIOperationInProgress = false;

	//	Does this step request that the CS line be pulled back up?
	if (aSteps[*nStepIndex].bSetCSHigh)
	{
		//	Go ahead and, via software, pull the chip select line high.
		HAL_GPIO_WritePin(EE_CS_GPIO_Port, EE_CS_Pin, GPIO_PIN_SET);
	}

	//	Increment this number, and then proceed to set up the next step.
	*nStepIndex += 1;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	SPIFlash_Callback(m_aSPIStep, SPIFLASH_MAX_STEPS, &m_nSPIStepIndex);
}
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	SPIFlash_Callback(m_aSPIStep, SPIFLASH_MAX_STEPS, &m_nSPIStepIndex);
}
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	SPIFlash_Callback(m_aSPIStep, SPIFLASH_MAX_STEPS, &m_nSPIStepIndex);
}

/*
	Function:	SPIFlash_Process()
	Description:
		The process function continuously reads the SPIFlash, and if it
		doesn't match what we expect it to in memory (say because of a change),
		then we'll go ahead and rewrite it out.
*/
void SPIFlash_Process()
{

	//	The following is the actual SPIFlash_Process code that should remain
	//	in the final release of the product build.

	bool bSPIStepsClear = true;

	switch(m_eSPIFlashState)
	{
		case SPIFLASH_STATE_IDLE:
			//	IDLE state of the SPIFlash.
			//	Determine if there's any operations that are presently pending. If so,
			//	go ahead and run them
			bSPIStepsClear = SPIFlash_IsFree();
			if (!bSPIStepsClear)
			{
				//	The SPISteps_T buffer is not clear, and in a state of IDLEness.
				//	Something is waiting to run.
				//	Kick off that operation.
				if (SPIFlash_SetupNextOperation(m_aSPIStep, SPIFLASH_MAX_STEPS, m_nSPIStepIndex))
				{
					m_eSPIFlashState = SPIFLASH_STATE_PROCESS;
				}
				else
				{
					SPIFlash_OperationCompleteCallback();
				}

				/*
				 *	//	Determine where the SPIStep_T array is presently at.
					if (*nStepIndex < nSteps)
					{
						//	Attempt to complete the callback by
						//	setting up the next operation.
						bHandled = SPIFlash_SetupNextOperation(aSteps, nSteps, *nStepIndex);
					}

					if (!bHandled)
					{
						//	There weren't any more steps to complete.
						//	Complete the operation with the generic callback.
						SPIFlash_OperationCompleteCallback();
					}
				 */
			}
			break;
		case SPIFLASH_STATE_PROCESS:
			//	We ourselves cannot escape this state--
			//	An interrupt from the SPI1 bus will cause us to escape when all operations are complete.
			if (!m_bSPIOperationInProgress)
			{
				m_eSPIFlashState = SPIFLASH_STATE_IDLE;
			}
			break;
		default:
			break;
	}

	//	The following is the DEBUG code used to test the state machine as specified above.
#ifdef DEBUG_SPIFLASH_CONSTANT_READS_AND_WRITES
	switch(nInternalCounter)
	{
		case 0:
			SPIFlash_Read(aTestBuffer, 0, 0, 32);
			nInternalCounter++;
			break;
		case 1:
			if (SPIFlash_IsFree())
			{
				//	Generate a random buffer
				int nCntr = 0;
				for (nCntr = 0; nCntr < 32; nCntr++)
				{
					aExpectedContentsBuffer[nCntr] = rand() & 0xFF;
				}
				SPIFlash_Write(aExpectedContentsBuffer, 0, 0, 32);
				nInternalCounter++;
			}
			break;
		case 2:
			if (SPIFlash_IsFree())
			{
				SPIFlash_Read(aTestBuffer, 0, 0, 32);
				nInternalCounter++;
			}
			break;
		case 3:
			if (SPIFlash_IsFree())
			{
				volatile int memory_compare_stat = memcmp(aTestBuffer, aExpectedContentsBuffer, 32);
				nInternalCounter++;
			}
			break;
		case 4:
			break;
		default:
			nInternalCounter = 0;
	}

#endif

}
