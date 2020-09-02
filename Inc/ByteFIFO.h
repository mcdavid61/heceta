/*
    File:   ByteFIFO.h
    Author: A T Alexander
    Description:
        A FIFO for bytes, useful for UARTS and safe for
    interrupts.
*/
#ifndef BYTE_FIFO_H_
#define BYTE_FIFO_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

//
// A FIFO Control Structure
//
typedef struct
{
    void * pBuffer;       	 //	Pointer to start of buffer
    uint32_t nBufferLength;  //	Number of bytes pointed to by pBuffer
    volatile uint32_t nHead; //	Index/offset of the location to insert the next byte
    volatile uint32_t nTail; //	Index/offset of the location to remove the next byte
    uint32_t nSize;			 //	Size of each element within the void pointer.
} FIFOControl_T;

//
// Macros for easy definition
//
//-A potentially global fifo
#define DEFINE_FIFO(name, type, length) \
    static type name##sBuffer[length]; \
    FIFOControl_T name = \
    { \
        name##sBuffer, \
        length, \
        0, \
		0, \
		sizeof(type), \
    };

//-A local fifo
#define DEFINE_STATIC_FIFO(name, type, length) \
    static type name##sBuffer[length]; \
    static FIFOControl_T name = \
    { \
        name##sBuffer, \
        length, \
        0, \
        0, \
		sizeof(type), \
    };

//
// Access functions
//
uint32_t FIFO_GetQueued(const FIFOControl_T * pFifo);
uint32_t FIFO_GetFree(const FIFOControl_T * pFifo);
bool FIFO_GetEmptyState(const FIFOControl_T * pFifo);
bool FIFO_Enqueue(FIFOControl_T * pFifo, void * pEnqueue);
bool FIFO_Dequeue(FIFOControl_T * pFifo, void * pDequeue);
bool FIFO_Peek(FIFOControl_T * pFifo, void * pDequeue);
bool FIFO_GetIterator(const FIFOControl_T * pFifo, uint32_t * pIterator);
void * FIFO_Iterate(const FIFOControl_T * pFifo, uint32_t * pIterator);


#endif
