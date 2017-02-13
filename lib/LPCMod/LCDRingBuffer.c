/*
 * LCDRingBuffer.c
 *
 *  Created on: Feb 8, 2017
 *      Author: cromwelldev
 */

#include "LCDRingBuffer.h"
#include "string.h"
#include "lib/LPCMod/xblastDebug.h"
#include <stdbool.h>

typedef enum
{
    LCDOperation_Idle,
    LCDOperation_Delay,
    LCDOperation_Data
}LCDOperation_t;

typedef struct
{
    LCDOperation_t lcdOperation;
    unsigned short time_uS;
}DelayOperation_t;

typedef struct
{
    LCDOperation_t lcdOperation;
    unsigned char targetAddr;
    unsigned char dataToWrite;
}DataOperation_t;

typedef enum
{
    InternalOp_ReadyForNewOp,
}InternalOp;

typedef union
{
    DelayOperation_t delayOp;
    DataOperation_t dataOp;
}CurrentOp;

#define _ringBufSize 512
static const unsigned short ringBufSize = _ringBufSize;
static unsigned char ringBuf[_ringBufSize];
static unsigned short inPos;
static unsigned short outPos;
static bool rollOver;
static InternalOp internalOp;
static CurrentOp currentOp;

static void putInBuf(unsigned char* data, unsigned char size);


void putDelay(unsigned short microseconds)
{
    DelayOperation_t operation;
    operation.lcdOperation = LCDOperation_Delay;
    operation.time_uS = microseconds;

    putInBuf((unsigned char *)&operation, sizeof(operation));
}

void putByte(unsigned char addr, unsigned char data)
{
    DataOperation_t operation;
    operation.lcdOperation = LCDOperation_Data;
    operation.targetAddr = addr;
    operation.dataToWrite = data;

    putInBuf((unsigned char *)&operation, sizeof(operation));
}

void LCDRingBufferInit(void)
{
    memset(ringBuf, 0x00, ringBufSize);
    inPos = 0;
    outPos = 0;
    rollOver = false;
    internalOp = InternalOp_ReadyForNewOp;
}

void updateLCDRingBuffer(void)
{
    switch(internalOp)
    {
    case InternalOp_ReadyForNewOp:
        if(outPos != inPos)
        {
            // need to process
        }
    break;
    }
}

static void putInBuf(unsigned char* data, unsigned char size)
{

    if((rollOver || (outPos == 0 && (inPos + size > ringBufSize))) && (((inPos + size) % ringBufSize) > outPos))
    {
        debugSPIPrint("LCD RingBuf overflow. inPos=%u  outPos=%u   size=%u   rollOver=%u\n", inPos, outPos, size, rollOver);
        return;
    }

    for(unsigned char i = 0; i < size; i++)
    {
        ringBuf[inPos] = data[i];
        inPos++;
        if(inPos >= ringBufSize)
        {
            inPos = 0;
            rollOver = true;
        }
    }
}
