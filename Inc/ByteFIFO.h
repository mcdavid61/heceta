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
    uint8_t * pBuffer;       //Pointer to start of buffer
    uint32_t nBufferLength;  //Number of bytes pointed to by pBuffer
    volatile uint32_t nHead; //Index/offset of the location to insert the next byte
    volatile uint32_t nTail; //Index/offset of the location to remove the next byte
} ByteFIFOControl_T;

//
// Macros for easy definition
//
//-A potentially global fifo
#define DEFINE_BYTE_FIFO(name, length) \
    static uint8_t name##sBuffer[length]; \
    ByteFIFOControl_T name = \
    { \
        name##sBuffer, \
        length, \
        0, \
        0 \
    };
//-A local fifo
#define DEFINE_STATIC_BYTE_FIFO(name, length) \
    static uint8_t name##sBuffer[length]; \
    static ByteFIFOControl_T name = \
    { \
        name##sBuffer, \
        length, \
        0, \
        0 \
    };

//
// Access functions
//
uint32_t ByteFIFO_GetBytesQueued(const ByteFIFOControl_T * pFifo);
uint32_t ByteFIFO_GetBytesFree(const ByteFIFOControl_T * pFifo);
bool ByteFIFO_GetEmptyState(const ByteFIFOControl_T * pFifo);
bool ByteFIFO_Enqueue(ByteFIFOControl_T * pFifo, uint8_t nChar);
int32_t ByteFIFO_Dequeue(ByteFIFOControl_T * pFifo);

#endif
