/*
 * RAMIntegrity.c
 *
 *  Created on: Oct 15, 2020
 *      Author: BFS
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "RAMIntegrity.h"
#include "cmsis_gcc.h"
#include "Fault.h"

typedef enum
{
	RAMINTEGRITY_STARTUP,
	RAMINTEGRITY_IDLE,
}	RAMIntegrity_State_T;

static RAMIntegrity_State_T m_eRAMIntegrityState = RAMINTEGRITY_STARTUP;

//	Determine which chunk of RAM we're currently working with.
//	Aka, split up the RAM into nicely sized paged chunks.
static uint32_t m_nRAMChunk = 0;

//	Start of the RAM region.
extern uint32_t _sdata;

//	Represents the end of the RAM region.
extern uint32_t _estack;

//	Size of RAM
#define RAM_SIZE_BYTES ((((uint32_t)&_estack) - ((uint32_t)&_sdata)))
#define RAM_PAGE_SIZE_BYTES (32)
#define RAM_PAGE_SIZE_WORDS (RAM_PAGE_SIZE_BYTES/4)
#define RAM_PAGE_COUNT (RAM_SIZE_BYTES/RAM_PAGE_SIZE_BYTES)

/*
	Function:	RAMIntegrity_Run()
	Description:
		Runs a RAM integrity check on a given set of RAM boundaries.
*/
bool RAMIntegrity_Run(uint32_t * pStart, uint32_t * pEnd)
{
	/*
		WARNING:	This function requires careful attention as to how
					it reads and alters the state of system RAM. If specified,
					variables designated with "register" MUST be stored within
					generic registers at runtime [r0-r12], or this function may
					cause undesired behavior.

					It is CRITICAL that this is checked using a disassembly
					tool to ensure that this function completes as intended.
	*/

	//	pAddress is responsible for storing the present address of which
	//	will be checked for integrity. It serves two purposes:
	//		-	will store the address currently being checked.
	//		-	upon function completion, will equal the pEnd value
	//			to indicate SUCCESS, or NULL to indicate FAILURE.
	//		-	this behavior allows me to eliminate a boolean register.

	//	These are cast as uint32_t. This is because these are purely
	//	addresses in memory, and we want to make it explicitly clear
	//	when we're trying to access where these things point to in memory.
	register uint32_t pAddress = (uint32_t) pStart;
	register uint32_t pAddressEnd = (uint32_t) pEnd;

	register uint32_t nTemporaryWord;
	register uint32_t nBASEPRI = __get_PRIMASK();



	//	Disable interrupts, using the PRIMASK.
	__set_PRIMASK(1);

	//	Iterate through each byte:
	while (pAddress < pAddressEnd)
	{
		//	Within this loop--	we can ONLY modify REGISTERS.

		//	Save the contents of each byte into a temporary value on the stack.
		nTemporaryWord = VOLATILE_DEREF(uint32_t, pAddress);

		//	Write the value 0x55, or 0b01010101 to the address under test.
		VOLATILE_DEREF(uint32_t, pAddress) = 0x55555555;

		//	Read the address under test, and verify that its value is 0x55.
		//		If this does not match, the integrity test fails.
		if (VOLATILE_DEREF(uint32_t, pAddress) != 0x55555555)
		{
			VOLATILE_DEREF(uint32_t, pAddress) = nTemporaryWord;
			pAddress = (uint32_t) NULL;
			break;
		}

		//	Write the value 0xAA, or 0b01010101 to the address under test.
		VOLATILE_DEREF(uint32_t, pAddress) = 0xAAAAAAAA;

		//	Read the address under test, and verify that its value is 0xAA.
		//		If this does not match, the integrity test fails.
		if (VOLATILE_DEREF(uint32_t, pAddress) != 0xAAAAAAAA)
		{
			VOLATILE_DEREF(uint32_t, pAddress) = nTemporaryWord;
			pAddress = (uint32_t) NULL;
			break;
		}

		//	Write the temporary value back to the address under test.
		VOLATILE_DEREF(uint32_t, pAddress) = nTemporaryWord;

		//	Read the address under test, and verify that its value matches the original value.
		//		If this does not match, the integrity test fails.
		if (VOLATILE_DEREF(uint32_t, pAddress) != nTemporaryWord)
		{
			VOLATILE_DEREF(uint32_t, pAddress) = nTemporaryWord;
			pAddress = (uint32_t) NULL;
			break;
		}

		//	Increase the counter.
		pAddress += sizeof(void *);
	}

	//	Enable interrupts, using the PRIMASK.
	__set_PRIMASK(nBASEPRI);

	return (pAddress == pAddressEnd);
}


/*
	Function:	RAMIntegrity_Process()
	Description:
		Primary processing function for the RAMIntegrity module.
*/
void RAMIntegrity_Process(void)
{
	//	I've decided to intentionally use these pointers as uint8_t *,
	//	so that pointer arithmetic doesn't attempt to outsmart me.
	uint8_t * pStart;
	uint8_t * pEnd;
	bool bSuccess = false;

	switch(m_eRAMIntegrityState)
	{
		case RAMINTEGRITY_STARTUP:
			//	Upon startup, the entirety of the RAM must be checked.
			pStart = (uint8_t *) &_sdata;
			pEnd = (uint8_t *) &_estack;
			bSuccess = RAMIntegrity_Run( (uint32_t *) pStart, (uint32_t *) pEnd);
			m_eRAMIntegrityState = RAMINTEGRITY_IDLE;
			break;
		case RAMINTEGRITY_IDLE:
			//	In the IDLE state, whenever we're called, we'll check one page of
			//	RAM at a time.
			//	This is less time consuming than checking everything.
			pStart = ((uint8_t *) &_sdata) + (m_nRAMChunk * RAM_PAGE_SIZE_BYTES);
			pEnd = pStart + RAM_PAGE_SIZE_BYTES;
			bSuccess = RAMIntegrity_Run( (uint32_t *) pStart, (uint32_t *) pEnd);

			//	Increment the m_nRAMChunk or overflow back to zero if we've exceeded the limits.
			m_nRAMChunk = ((m_nRAMChunk == RAM_PAGE_COUNT-1) ? 0 : m_nRAMChunk+1);
			break;
	}

	if (!bSuccess)
	{
		Fault_Activate(FAULT_RAM_INTEGRITY);
	}
}

/*
	Function:	RAMIntegrity_StartupTasksComplete()
	Description:
		Returns whether or not startpu tasks as required by the RAMIntegrity
		module have been completed.
*/
bool RAMIntegrity_StartupTasksComplete(void)
{
	//	As long as we're not in the STARTUP state, we've completed our tasks.
	return (m_eRAMIntegrityState != RAMINTEGRITY_STARTUP);
}


