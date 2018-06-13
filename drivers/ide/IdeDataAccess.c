/*
 * IdeDataAccess.c
 *
 *  Created on: Apr 7, 2018
 *      Author: cromwelldev
 */

#include "IdeDriver.h"
#include "IdeHelpers.h"
#include "IdeDriverInternal.h"
#include "lib/cromwell/cromString.h"
#include "lib/time/timeManagement.h"
#include "lib/LPCMod/xblastDebug.h"
#include "string.h"
#include "boot.h"

typedef struct {                 //PRD table entry. 8 bytes in length
    unsigned int address;
    unsigned short byteCount;
    unsigned short reserved : 15;
    unsigned short endoftable : 1;
}__attribute__((packed))PRD;    //No filling. SHouldn't matter since its elements makes it already aligned.

typedef struct PRDT{
    PRD __attribute__((aligned(4)))PRD1;      //All aligned on a Dword boundary
    PRD __attribute__((aligned(4)))PRD2;
    PRD __attribute__((aligned(4)))PRD3;
    PRD __attribute__((aligned(4)))PRD4;
}__attribute__((packed, aligned(64 * 1024))) PRDTable;  //Aligned on a 64K boundary.
                                                        //So no crossing unless struct is > 64K in size.
static int BootIdePIORead(int nDriveIndex, unsigned char* outBuffer, unsigned long long startSector, int n_sectors)
{
    unsigned char ideReadCommand = IDE_CMD_READ_MULTI_RETRY; /* 28-bit LBA */
    const int logicalSectorSize = tsaHarddiskInfo[nDriveIndex].m_logicalSectorSize;
    int maxSectorReadCountPerCommand = 256;
    int sectorProcessedCount = 0;

    if((startSector + n_sectors) >= 0x10000000 )
    {
        /* Requires LBA48 */
        maxSectorReadCountPerCommand = 65536;
        ideReadCommand = IDE_CMD_READ_EXT;
    }

    while(sectorProcessedCount < n_sectors)
    {
        int tempSectorCount = (n_sectors - sectorProcessedCount > maxSectorReadCountPerCommand ? maxSectorReadCountPerCommand : n_sectors - sectorProcessedCount);
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Read %u sectors", tempSectorCount);
        if(sendATACommandAndReceiveData(nDriveIndex, ideReadCommand, startSector, outBuffer + (logicalSectorSize * sectorProcessedCount), tempSectorCount % maxSectorReadCountPerCommand))
        {
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_ERROR, "error drive:%u  sec:%Lu", nDriveIndex, startSector);
            return 1;
        }
        sectorProcessedCount += tempSectorCount;
    }

    return 0;
}


static int BootIdePIOWrite(int nDriveIndex, const unsigned char* inBuffer, unsigned long long startSector, int n_sectors)
{
    unsigned char ideWriteCommand = IDE_CMD_WRITE_MULTI_RETRY; /* 28-bit LBA */
    const int logicalSectorSize = tsaHarddiskInfo[nDriveIndex].m_logicalSectorSize;
    int maxSectorWriteCountPerCommand = 256;
    int sectorProcessedCount = 0;

    if((startSector + n_sectors) >= 0x10000000 )
    {
        /* Requires LBA48 */
        maxSectorWriteCountPerCommand = 65536;
        ideWriteCommand = IDE_CMD_WRITE_EXT;
    }
    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Total:%u sec. max:%u", n_sectors, maxSectorWriteCountPerCommand);

    while(sectorProcessedCount < n_sectors)
    {
        int tempSectorCount = (n_sectors - sectorProcessedCount > maxSectorWriteCountPerCommand ? maxSectorWriteCountPerCommand : n_sectors - sectorProcessedCount);
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Write %u sec. Proc:%u", tempSectorCount, sectorProcessedCount);
        if(sendATACommandAndSendData(nDriveIndex, ideWriteCommand, startSector, inBuffer + (logicalSectorSize * sectorProcessedCount), tempSectorCount % maxSectorWriteCountPerCommand))
        {
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "error drive:%u  sec:%Lu", nDriveIndex, startSector);
            return 1;
        }
        sectorProcessedCount += tempSectorCount;
    }

    return 0;
}

static int BootIdeDMARead(int nDriveIndex, unsigned char* outBuffer, unsigned long long startSector, int n_sectors)
{
    unsigned char ideReadCommand = IDE_CMD_READ_DMA; /* 28-bit LBA */
    const int logicalSectorSize = tsaHarddiskInfo[nDriveIndex].m_logicalSectorSize;
    int maxSectorReadCountPerCommand = 256;
    int sectorProcessedCount = 0;

    if((startSector + n_sectors) >= 0x10000000 )
    {
        /* Requires LBA48 */
        maxSectorReadCountPerCommand = 65536;
        ideReadCommand = IDE_CMD_READ_DMA_EXT;
    }

    while(sectorProcessedCount < n_sectors)
    {
        //TODO: Populate PRD and configure IDE BusMaster
        int tempSectorCount = (n_sectors - sectorProcessedCount > maxSectorReadCountPerCommand ? maxSectorReadCountPerCommand : n_sectors - sectorProcessedCount);
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Read %u sectors", tempSectorCount);
        if(sendControlATACommand(nDriveIndex, ideReadCommand, startSector, NoFeatureField, tempSectorCount % maxSectorReadCountPerCommand))
        {
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_ERROR, "error drive:%u  sec:%Lu", nDriveIndex, startSector);
            return 1;
        }
        sectorProcessedCount += tempSectorCount;
    }

    return 0;
}


static int BootIdeDMAWrite(int nDriveIndex, const unsigned char* inBuffer, unsigned long long startSector, int n_sectors)
{
    unsigned char ideWriteCommand = IDE_CMD_WRITE_DMA; /* 28-bit LBA */
    const int logicalSectorSize = tsaHarddiskInfo[nDriveIndex].m_logicalSectorSize;
    int maxSectorWriteCountPerCommand = 256;
    int sectorProcessedCount = 0;

    if((startSector + n_sectors) >= 0x10000000 )
    {
        /* Requires LBA48 */
        maxSectorWriteCountPerCommand = 65536;
        ideWriteCommand = IDE_CMD_WRITE_DMA_EXT;
    }
    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Total:%u sec. max:%u", n_sectors, maxSectorWriteCountPerCommand);

    while(sectorProcessedCount < n_sectors)
    {
        //TODO: Populate PRD and configure IDE BusMaster
        int tempSectorCount = (n_sectors - sectorProcessedCount > maxSectorWriteCountPerCommand ? maxSectorWriteCountPerCommand : n_sectors - sectorProcessedCount);
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Write %u sec. Proc:%u", tempSectorCount, sectorProcessedCount);
        if(sendControlATACommand(nDriveIndex, ideWriteCommand, startSector, NoFeatureField, tempSectorCount % maxSectorWriteCountPerCommand))
        {
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "error drive:%u  sec:%Lu", nDriveIndex, startSector);
            return 1;
        }
        sectorProcessedCount += tempSectorCount;
    }

    return 0;
}


/* -------------------------------------------------------------------------------- */

int IdeDriver_Read(int nDriveIndex, void * pbBuffer, unsigned int startSector, int sectorCount)
{
    /* Only PIO for now */
    if(IdeDriver_DeviceIsATAPI(nDriveIndex))
    {
        return Internal_ATAPIDataRead(nDriveIndex, pbBuffer, startSector, sectorCount);
    }

    if(tsaHarddiskInfo[nDriveIndex].m_fUseDMA)
    {
        return BootIdeDMARead(nDriveIndex, pbBuffer, startSector, sectorCount);
    }

    return BootIdePIORead(nDriveIndex, pbBuffer, startSector, sectorCount);
}

int IdeDriver_Write(int nDriveIndex, const void * pbBuffer, unsigned int startSector, int sectorCount)
{
    /* Only PIO for now */
    int writeResult;
    if(IdeDriver_DeviceIsATAPI(nDriveIndex))
    {
        return 1;
    }

    if(tsaHarddiskInfo[nDriveIndex].m_fUseDMA)
    {
        writeResult = BootIdeDMAWrite(nDriveIndex, pbBuffer, startSector, sectorCount);
    }
    else
    {
        writeResult = BootIdePIOWrite(nDriveIndex, pbBuffer, startSector, sectorCount);
    }
    if(0 == writeResult)
    {
        return IdeDriver_FlushCache(nDriveIndex);
    }
    return 1;
}

int IdeDriver_FlushCache(int nDriveIndex)
{
    ide_command_t command;

    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "drive:%u   ext:%u   normal:%u", nDriveIndex, tsaHarddiskInfo[nDriveIndex].m_fFlushCacheExtSupported, tsaHarddiskInfo[nDriveIndex].m_fFlushCacheSupported);

    if(tsaHarddiskInfo[nDriveIndex].m_fFlushCacheExtSupported)        //LBA48 drive
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_TRACE, "Flush cache Ext");
        command = IDE_CMD_CACHE_FLUSH_EXT;
    }
    else if(tsaHarddiskInfo[nDriveIndex].m_fFlushCacheSupported)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_TRACE, "Flush cache Ext");
        command =  IDE_CMD_CACHE_FLUSH;
    }

    if(sendControlATACommand(nDriveIndex, command, NoStartLBA, NoFeatureField, NoSectorCount))
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_ERROR, "error");
        return 1;
    }

    return 0;
}




