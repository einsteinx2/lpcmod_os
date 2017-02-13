/*
 * LCDRingBuffer.c
 *
 *  Created on: Feb 8, 2017
 *      Author: cromwelldev
 */

#include "LCDRingBuffer.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/time/timeManagement.h"
#include "xblast/HardwareIdentifier.h"

#define DISPLAY_E         0x04

typedef struct
{
    unsigned char dataToWrite;
    unsigned char rsFlag;
    unsigned short delayInUs;
}DataOperation_t;

typedef enum
{
    InternalOp_ReadyForNewOp,
    InternalOp_WriteHighNibble,
    InternalOp_WriteLowNibble
}InternalOp;

#define _ringBufSize 512
static const unsigned short ringBufSize = _ringBufSize;
static unsigned char ringBuf[_ringBufSize];
static unsigned short inPos;
static unsigned short outPos;
static bool rollOver;
static InternalOp internalOp;
static DataOperation_t currentOp;
static unsigned int systickStartValue;

static void putInBuf(DataOperation_t* input);
static void getFromBuf(DataOperation_t* output);
static void xecuter3LCDWrite(unsigned char data, unsigned char RSFlag);
static void smartXXLCDwrite(unsigned char data);

void putInLCDRingBuffer(unsigned char data, unsigned char RS, unsigned short delay)
{
    DataOperation_t operation;
    operation.dataToWrite = data;
    operation.rsFlag = RS;
    operation.delayInUs = delay;

    putInBuf(&operation);
}

void LCDRingBufferInit(void)
{
    inPos = 0;
    outPos = 0;
    rollOver = false;
    internalOp = InternalOp_ReadyForNewOp;
    systickStartValue = 0;
    currentOp.delayInUs = 0;
}

void updateLCDRingBuffer(void)
{
    if(getElapseMicroSecondsSince(systickStartValue) >= currentOp.delayInUs)
    {
        switch(internalOp)
        {
        default:
        case InternalOp_ReadyForNewOp:
            if(outPos != inPos || rollOver)
            {
                getFromBuf(&currentOp);
                internalOp = InternalOp_WriteHighNibble;
            }
            // Do nothing
        break;
        case InternalOp_WriteHighNibble:
            if(isXecuter3())
            {
                xecuter3LCDWrite(currentOp.dataToWrite & 0xF0, currentOp.rsFlag);
            }
            else
            {
                //Maps     0,b6,b7,b4,b5,E,RS,0
                unsigned char highNibble = ((currentOp.dataToWrite >> 2) & 0x28) | currentOp.rsFlag;
                highNibble |= (currentOp.dataToWrite >> 0) & 0x50;
                smartXXLCDwrite(highNibble);
            }

            systickStartValue = getUS();
            internalOp = InternalOp_WriteLowNibble;
            break;
        case InternalOp_WriteLowNibble:
            if(isXecuter3())
            {
                xecuter3LCDWrite(currentOp.dataToWrite << 4, currentOp.rsFlag);
            }
            else
            {
                //Maps     0,b2,b3,b0,b1,E,RS,0
                unsigned char lowNibble = ((currentOp.dataToWrite << 2) & 0x28) | currentOp.rsFlag;
                lowNibble |= (currentOp.dataToWrite << 4) & 0x50;
                smartXXLCDwrite(lowNibble);
            }

            systickStartValue = getUS();
            internalOp = InternalOp_ReadyForNewOp;
            break;
        }
    }
}

static void putInBuf(DataOperation_t* input)
{
    unsigned char* data = (unsigned char*)input;

    const unsigned short projectedEnd = inPos + sizeof(DataOperation_t);

    //Drop if no space left.
    if((rollOver && (projectedEnd > outPos)) ||
    (rollOver == false && ((projectedEnd % ringBufSize) < projectedEnd && (projectedEnd % ringBufSize) > outPos)))
    {
        debugSPIPrint("LCD RingBuf overflow. inPos=%u  outPos=%u   size=%u   rollOver=%u\n", inPos, outPos, size, rollOver);
        return;
    }

    for(unsigned char i = 0; i < sizeof(DataOperation_t); i++)
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

static void getFromBuf(DataOperation_t* output)
{
    unsigned char* ptr = (unsigned char*)output;

    for(unsigned char i = 0; i < sizeof(DataOperation_t); i++)
    {
        ptr[i] = ringBuf[outPos];
        outPos++;
        if(outPos >= ringBufSize)
        {
            outPos = 0;
            rollOver = false;
        }
    }
}

static void xecuter3LCDWrite(unsigned char data, unsigned char RSFlag)
{
    WriteToIO(X3_DISP_O_DAT, data); //Place b6,b7,b4,b5,x,x,x,x
    WriteToIO(X3_DISP_O_CMD, RSFlag);
    //Raise E signal line
    //2
    WriteToIO(X3_DISP_O_CMD, RSFlag | DISPLAY_E);
    //Drop E signal line
    //3
    WriteToIO(X3_DISP_O_CMD, RSFlag);
}

static void smartXXLCDwrite(unsigned char data)
{
    WriteToIO(LCD_DATA, data); //Place bit7,bit6,bit5,bit4,E,RS,x
    data |= DISPLAY_E;
    //Raise E signal line
    WriteToIO(LCD_DATA, data); //Place bit7,bit6,bit5,bit4,E,RS,x
    data ^= DISPLAY_E;
    //Drop E signal line
    WriteToIO(LCD_DATA, data); //Place bit7,bit6,bit5,bit4,E,RS,x
}
