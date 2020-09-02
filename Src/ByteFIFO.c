/*
 *	ByteFIFO.c
 *
 *  Created on: July 30, 2020
 *      Author: Constantino Flouras and A T Alexander
 *
 *  Description:
 *      A FIFO for bytes, useful for UARTS and safe for
 *   	interrupts.
*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ByteFIFO.h"
#include "string.h"

/*
    Function: FIFO_GetQueued
    Description:
        Gets the number of entries presently in the FIFO.
*/
uint32_t FIFO_GetQueued(const FIFOControl_T * pFifo)
{
    uint32_t nHead = pFifo->nHead;
    uint32_t nTail = pFifo->nTail;

    if(nHead >= nTail)
    {
        return nHead-nTail;
    }
    else
    {
        return pFifo->nBufferLength - (nTail - nHead);
    }
    return pFifo->nHead;
}

/*
    Function: FIFO_GetBytesFree
    Description:
        Gets the number of entries that could be inserted into the FIFO
    	in its present state.
*/
uint32_t FIFO_GetFree(const FIFOControl_T * pFifo)
{
    //Length - amount in it - 1, because we can't both use the last entry
    //and tell full from empty.
    return pFifo->nBufferLength - FIFO_GetQueued(pFifo) - 1;
}

/*
    Function: FIFO_GetEmptyState
    Description:
        Returns TRUE if Fifo is empty
*/
bool FIFO_GetEmptyState(const FIFOControl_T * pFifo)
{
    return pFifo->nHead == pFifo->nTail;
}

/*
    Function: FIFO_Enqueue
    Description:
        Adds nChar to the end of the Fifo and returns TRUE if successful.
*/
bool FIFO_Enqueue(FIFOControl_T * pFifo, void * pEnqueue)
{
	//	Return value
	bool bReturn = false;

    uint32_t nHead = pFifo->nHead;
    uint32_t nNewHead;

    //Find the new head index
    nNewHead = nHead + 1;

    //Account for wrapping the Head index
    if(nNewHead >= (uint16_t)(pFifo->nBufferLength))
    {
        //We wrapped
        nNewHead = 0;
    }

    //See if there's room for this byte
    if(nNewHead != pFifo->nTail)
    {
        //There's room.  Stick the character in at the existing Head.
    	if (pEnqueue != NULL)
    	{
    		memcpy( ((uint8_t *) pFifo->pBuffer)
    				+ (nHead * pFifo->nSize),
					((uint8_t *) pEnqueue),
					pFifo->nSize);
    	}
    	//pFifo->pBuffer[nHead] = nChar;

        //Update to the new Head
        pFifo->nHead = nNewHead;

        //Return success
        bReturn = true;
    }
    else
    {
        //This isn't going to fit, return failure and leave the
        //FIFO alone.
    	bReturn = false;
    }

    return bReturn;
}

/*
    Function: FIFO_Dequeue
    Description:
        If there is a character in the Fifo, removes it and returns it.
        If there is no character, returns -1.
*/
bool FIFO_Dequeue(FIFOControl_T * pFifo, void * pDequeue)
{
    uint32_t nTail = pFifo->nTail;

    //See if there's anything there
    if(pFifo->nHead != nTail)
    {
    	if (pDequeue != NULL)
    	{
    		memcpy(pDequeue, (uint8_t *) pFifo->pBuffer + (nTail * pFifo->nSize), pFifo->nSize);
    	}
        //int32_t nReturnValue = pFifo->pBuffer[nTail];

        //Find the new tail, accounting for wrapping, and put it back
        //into the control structure.
        nTail++;
        if(nTail >= pFifo->nBufferLength)
        {
            nTail = 0;
        }
        pFifo->nTail = nTail;

        //Send the new value back
        return true;
    }
    else
    {
        //Head and Tail are equal.  We're empty.
        return false;
    }
}

/*
    Function: FIFO_Dequeue
    Description:
        If there is a character in the Fifo, returns it, without removing it.
        If there is no character, returns -1 and clears the pDequeue flag.
*/
bool FIFO_Peek(FIFOControl_T * pFifo, void * pDequeue)
{
    uint32_t nTail = pFifo->nTail;

    //See if there's anything there
    if(pFifo->nHead != nTail)
    {
    	if (pDequeue != NULL)
    	{
    		memcpy(pDequeue, ((uint8_t *) pFifo->pBuffer) + (nTail * pFifo->nSize), pFifo->nSize);
    	}

        //Send the new value back
        return true;
    }
    else
    {
    	//	Wipe the dequeue value.
    	if (pDequeue != NULL)
		{
			memset(pDequeue, 0, pFifo->nSize);
		}
        //Head and Tail are equal.  We're empty.
        return false;
    }
}

/*
	Function: FIFO_GetIterator
	Description:
		Effectively speaking, this returns the index that refers to the
		beginning of the FIFO. Note that the beginning is the tail.
		Returns true if the pIterator is initialized, and false if it is not.
*/
bool FIFO_GetIterator(const FIFOControl_T * pFifo, uint32_t * pIterator)
{
	//	Return value
	bool bReturn = false;

	//	If either of the pointers are NULL, return false.
	if (pFifo == NULL || pIterator == NULL)
	{
		bReturn = false;
	}
	else
	{
		*pIterator = pFifo->nTail;
		bReturn = true;
	}

	return bReturn;
}

/*
	Function: FIFO_Iterate
	Description:
		Iterates through a FIFO, returning the next sequential entity.
		If there is no more in the FIFO, returns NULL.
*/
void * FIFO_Iterate(const FIFOControl_T * pFifo, uint32_t * pIterator)
{
	//	What we're going to return.
	void * pReturn;

	//	If either of the pointers are NULL, return NULL.
	if (pFifo == NULL || pIterator == NULL)
	{
		pReturn = NULL;
	}
	else
	{
		//	Grab the value of pIterator.
		//	Recall that pIterator is effectively speaking, the "current" value of
		//	which we'd like to return.
		uint32_t nCurr = *pIterator;

		if (nCurr == pFifo->nHead)
		{
			//	We've reached the end of the FIFO.
			pReturn = NULL;
		}
		else
		{
			//	Grab a pointer to the next entry.
			pReturn = pFifo->pBuffer + (pFifo->nSize * nCurr);

			//	Increase the value of pIterator, wrapping if we've reached the max.
			*pIterator = (*pIterator + 1) % pFifo->nBufferLength;
		}
	}

	return pReturn;


}
