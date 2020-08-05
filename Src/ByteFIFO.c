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

/*
    Function: ByteFIFO_GetBytesQueued
    Description:
        Gets the number of bytes presently in the FIFO.
*/
uint32_t ByteFIFO_GetBytesQueued(const ByteFIFOControl_T * pFifo)
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
    Function: ByteFIFO_GetBytesFree
    Description:
        Gets the number of bytes that could be inserted into the FIFO
    in its present state.
*/
uint32_t ByteFIFO_GetBytesFree(const ByteFIFOControl_T * pFifo)
{
    //Length - amount in it - 1, because we can't both use the last byte
    //and tell full from empty.
    return pFifo->nBufferLength - ByteFIFO_GetBytesQueued(pFifo) - 1;
}

/*
    Function: ByteFIFO_GetEmptyState
    Description:
        Returns TRUE if Fifo is empty
*/
bool ByteFIFO_GetEmptyState(const ByteFIFOControl_T * pFifo)
{
    return pFifo->nHead == pFifo->nTail;
}

/*
    Function: ByteFIFO_Enqueue
    Description:
        Adds nChar to the end of the Fifo and returns TRUE if successful.
*/
bool ByteFIFO_Enqueue(ByteFIFOControl_T * pFifo, uint8_t nChar)
{
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
        pFifo->pBuffer[nHead] = nChar;

        //Update to the new Head
        pFifo->nHead = nNewHead;

        //Return success
        return true;
    }
    else
    {
        //This isn't going to fit, return failure and leave the
        //FIFO alone.
        return false;
    }
}

/*
    Function: ByteFIFO_Dequeue
    Description:
        If there is a character in the Fifo, removes it and returns it.
        If there is no character, returns -1.
*/
int32_t ByteFIFO_Dequeue(ByteFIFOControl_T * pFifo)
{
    uint32_t nTail = pFifo->nTail;

    //See if there's anything there
    if(pFifo->nHead != nTail)
    {
        int32_t nReturnValue = pFifo->pBuffer[nTail];

        //Find the new tail, accounting for wrapping, and put it back
        //into the control structure.
        nTail++;
        if(nTail >= pFifo->nBufferLength)
        {
            nTail = 0;
        }
        pFifo->nTail = nTail;

        //Send the new value back
        return nReturnValue;
    }
    else
    {
        //Head and Tail are equal.  We're empty.
        return -1;
    }
}
