/*
 * spoof_FlashLowLevel.c
 *
 *  Created on: Dec 7, 2016
 *      Author: cromwelldev
 */

#include "FlashLowLevel.h"
#include <string.h>
#include <stdio.h>

bool forceDeviceNotBusy;
unsigned int busyCheckCount;
unsigned int howManyTimeBusyShouldReportTrue;
bool firstBusyRead;
unsigned char lastStatusRegisterState;

#define FlashSize 1024 * 1024
unsigned char memoryBuffer[FlashSize];

bool blockEraseSupported;
bool sectorEraseSupported;
bool chipEraseSupported;

void FlashLowLevel_Init(void)
{
    busyCheckCount = 0;
    howManyTimeBusyShouldReportTrue = 3;
    firstBusyRead = true;
    lastStatusRegisterState = 0;
    forceDeviceNotBusy = true;

    memset(memoryBuffer, 0x22, FlashSize);

    blockEraseSupported = false;
    sectorEraseSupported = true;
    chipEraseSupported = true;
}

bool FlashLowLevel_ReadDevice(void)
{
    flashDevice.flashType.m_bDeviceId = 0x05;
    flashDevice.flashType.m_bManufacturerId = 0xAA;
    flashDevice.flashType.m_dwLengthInBytes = FlashSize;
    sprintf(flashDevice.flashType.m_szFlashDescription, "Spoof flash device");

    flashDevice.m_fIsBelievedCapableOfWriteAndErase = true;
    flashDevice.m_pbMemoryMappedStartAddress = 0;
    flashDevice.m_szAdditionalErrorInfo[0] = '\0';

    return true;
}

bool FlashLowLevel_DeviceIsBusy(void)
{
    if(forceDeviceNotBusy)
    {
        return false;
    }

    if(busyCheckCount++ >= howManyTimeBusyShouldReportTrue)
    {
        forceDeviceNotBusy = true;
        busyCheckCount = 0;
        return false;
    }

    return true;
}

void FlashLowLevel_InititiateBlockErase(unsigned int addr)
{
    if(blockEraseSupported)
    {
        forceDeviceNotBusy = false;
        busyCheckCount = 0;
        howManyTimeBusyShouldReportTrue = 4;
        memset(memoryBuffer + addr, 0xff, 4 * 1024);
    }
}

void FlashLowLevel_InititiateSectorErase(unsigned int addr)
{
    if(sectorEraseSupported)
    {
        forceDeviceNotBusy = false;
        busyCheckCount = 0;
        howManyTimeBusyShouldReportTrue = 10;
        memset(memoryBuffer + addr, 0xff, 64 * 1024);
    }
}

void FlashLowLevel_InititiateChipErase(void)
{
    if(chipEraseSupported)
    {
        forceDeviceNotBusy = false;
        busyCheckCount = 0;
        howManyTimeBusyShouldReportTrue = 20;
        memset(memoryBuffer, 0xff, FlashSize);
    }
}

void FlashLowLevel_WriteByte(unsigned char byte, unsigned int addr)
{
    if(forceDeviceNotBusy)
    {
        forceDeviceNotBusy = false;
        busyCheckCount = 0;
        howManyTimeBusyShouldReportTrue = 4;
        memoryBuffer[addr] = byte;
    }
}

unsigned char FlashLowLevel_ReadByte(unsigned int addr)
{
    return memoryBuffer[addr];
}
