/*
 * IdeHelpers.c
 *
 *  Created on: Mar 26, 2018
 *      Author: cromwelldev
 */

#include "IdeHelpers.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/time/timeManagement.h"
#include "lib/LPCMod/xblastDebug.h"
#include "boot.h"
#include <stdbool.h>

tsHarddiskInfo tsaHarddiskInfo[2];  // static struct stores data about attached drives

static int BootIdeWaitDataReady(unsigned uIoBase);
static int BootIdeIssueAtaCommand(unsigned uIoBase, ide_command_t command, tsIdeCommandParams * params, bool skipFirstWait);
static int BootIdeWriteData(unsigned uIoBase, const void * buf, unsigned int size);

static void populateLBAField(tsIdeCommandParams* inout, int nDriveIndex, unsigned long long lba, bool isLBA48)
{
    if(isLBA48)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Using LBA48 command structure");
        /* 48-bit LBA access required for this block */
        inout->m_bCountSectorExt = 0;

         /* This routine can have a max LBA of 32 bits (due to unsigned int data type used for block parameter) */
        inout->m_wCylinderExt = (lba >> 32) & 0xffff;; /* 47:32 */
        inout->m_bSectorExt = (lba >> 24) & 0xff; /* 31:24 */
        inout->m_wCylinder = (lba >> 8) & 0xffff; /* 23:8 */
        inout->m_bSector = lba & 0xff; /* 7:0 */
        inout->m_bDrivehead = IDE_DH_DRIVE(nDriveIndex) | IDE_DH_LBA;
    } else {
            // Looks Like we do not have LBA 48 need
        inout->m_bSector = lba & 0xff; /* lower byte of block (lba) */
        inout->m_wCylinder = (lba >> 8) & 0xffff; /* middle 2 bytes of block (lba) */
        inout->m_bDrivehead = IDE_DH_DEFAULT | /* set bits that must be on */
            ((lba >> 24) & 0x0f) | /* lower nibble of byte 3 of block */
            IDE_DH_DRIVE(nDriveIndex) | IDE_DH_LBA;
    }
}

static int commandIsEXT(ide_command_t command)
{
    switch(command)
    {
    case IDE_CMD_READ_EXT:
    case IDE_CMD_READ_DMA_EXT:
    case IDE_CMD_READ_MULTIPLE_EXT:
    case IDE_CMD_WRITE_EXT:
    case IDE_CMD_WRITE_DMA_EXT:
    case IDE_CMD_WRITE_MULTIPLE_EXT:
    case IDE_CMD_CACHE_FLUSH_EXT:
        return 1;
    default:
        break;
    }

    return 0;
}

static int copy_swap_trim(unsigned char *dst, unsigned char *src, int len)
{
    unsigned char tmp;
    int i;
        for (i=0; i < len; i+=2) {
        tmp = src[i];     //allow swap in place
        dst[i] = src[i+1];
        dst[i+1] = tmp;
    }

    --dst;
    for (i=len; i>0; --i) {
        if (dst[i] != ' ') {
            dst[i+1] = 0;
            break;
        }
    }
    return i;
}

int BootIdeSendIdentifyDevice(int nDriveIndex)
{
    int returnValue;
    unsigned char cm, ch;
    unsigned short identifyDataBuf[IDE_SECTOR_SIZE / sizeof(unsigned short)];


    if(1 < nDriveIndex) return 1;


    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Send IDE_CMD_IDENTIFY on drive %u.", nDriveIndex);
    returnValue = sendControlATACommand(nDriveIndex, IDE_CMD_IDENTIFY, NoStartLBA, NoFeatureField, NoSectorCount);

    if(returnValue)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Error on drive %u. Checking for ATAPI device.", nDriveIndex);
        cm = IoInputByte(IDE_REG_LBA_MID(tsaHarddiskInfo[nDriveIndex].m_fwPortBase));
        ch = IoInputByte(IDE_REG_LBA_HIGH(tsaHarddiskInfo[nDriveIndex].m_fwPortBase));
        /* cm == 0x3c && ch == 0xc3 is for SATA ATA drive */
        if((0x14 == cm && 0xEB == ch) || (0x69 == cm && 0x96 == ch))
        {
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Drive is ATAPI, surely DVD drive.");
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Issuing ATAPI IDENTIFY command.");
            if(sendControlATACommand(nDriveIndex, IDE_CMD_PACKET_IDENTIFY, NoStartLBA, NoFeatureField, NoSectorCount))
            {
                XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "ATAPI IDENTIFY command returned error. Drive not detected. Halting!");
                return -1;
            }
            tsaHarddiskInfo[nDriveIndex].m_fAtapi = true;
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Successfully identified ATAPI device.");

        }
        else
        {
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Magic ATAPI values identifier not valid.");
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Unknown device at %s position. Halting init.", nDriveIndex ? "slave" : "master");
            return -1;
        }
    }
    else if(-1 == returnValue)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "No device at %s position. Halting init.", nDriveIndex? "slave" : "master");
        return -1;
    }
    else
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "IDE_CMD_IDENTIFY successful.");
    }

    if(getATAData(nDriveIndex, identifyDataBuf, IDE_SECTOR_SIZE))
    {
        return 1;
    }

    tsaHarddiskInfo[nDriveIndex].m_fDriveExists = 1;

    /* Common return data for "IDE_CMD_IDENTIFY" and "IDE_CMD_PACKET_IDENTIFY" */
    tsaHarddiskInfo[nDriveIndex].m_wAtaRevisionSupported = identifyDataBuf[88];

    /* Legacy drive structure info, still used for drive password generation */
    tsaHarddiskInfo[nDriveIndex].m_wCountCylinders = identifyDataBuf[1];
    tsaHarddiskInfo[nDriveIndex].m_wCountHeads = identifyDataBuf[3];
    tsaHarddiskInfo[nDriveIndex].m_wCountSectorsPerTrack = identifyDataBuf[6];


    /* Drive size and LBA48 capabilities */
    tsaHarddiskInfo[nDriveIndex].m_bLbaMode = identifyDataBuf[49] & (1 << 9) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bExtendedSectorCountSupport = identifyDataBuf[69] & (1 << 3) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_b48bitsAddrSupport = identifyDataBuf[83] & (1 << 10) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal = *((unsigned int *)&identifyDataBuf[60]);
    if(tsaHarddiskInfo[nDriveIndex].m_b48bitsAddrSupport)
    {
        tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal = *((unsigned long long *)&identifyDataBuf[100]);
    }
#if 0
    /* Only supported in ATA Specs ACS-4 and upward */
    if(tsaHarddiskInfo[nDriveIndex].m_bExtendedSectorCountSupport)
    {
        tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal = *((unsigned long long *)&identifyDataBuf[230]);
    }
#endif

    /* Drive sector size */
    tsaHarddiskInfo[nDriveIndex].m_logicalSectorSize = IDE_SECTOR_SIZE;
    if(identifyDataBuf[106] & (1 << 12))
    {
        tsaHarddiskInfo[nDriveIndex].m_logicalSectorSize = *((unsigned int *)&identifyDataBuf[117]) * sizeof(unsigned short);
    }
    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Drive%u sector size: %u", nDriveIndex, tsaHarddiskInfo[nDriveIndex].m_logicalSectorSize);

    {
        unsigned short* pw = (unsigned short *)&(identifyDataBuf[10]);
        tsaHarddiskInfo[nDriveIndex].s_length = copy_swap_trim(tsaHarddiskInfo[nDriveIndex].m_szSerial, (unsigned char*)pw, 10 * sizeof(unsigned short));
        tsaHarddiskInfo[nDriveIndex].m_szSerial[20] = '\0';

        pw = (unsigned short*)&(identifyDataBuf[27]);
        tsaHarddiskInfo[nDriveIndex].m_length = copy_swap_trim(tsaHarddiskInfo[nDriveIndex].m_szIdentityModelNumber, (unsigned char*)pw, 20 * sizeof(unsigned short));
        tsaHarddiskInfo[nDriveIndex].m_szIdentityModelNumber[40] = '\0';

        copy_swap_trim(tsaHarddiskInfo[nDriveIndex].m_szFirmware, (unsigned char *)&(identifyDataBuf[23]), 4 * sizeof(unsigned short));
        tsaHarddiskInfo[nDriveIndex].m_szFirmware[8] = '\0';
    }

    tsaHarddiskInfo[nDriveIndex].m_maxBlockPerDRQ = (unsigned char)identifyDataBuf[47];
    tsaHarddiskInfo[nDriveIndex].m_bDMASupported = identifyDataBuf[49] & (1 << 8) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bIORDY = identifyDataBuf[49] & (1 << 11) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bTimingFieldsValid = identifyDataBuf[53] & (1 << 1) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bDMAModeFieldValid = identifyDataBuf[53] & (1 << 2) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bCurrentBlockPerDRQValid = identifyDataBuf[59] & (1 << 8) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_currentBlockPerDRQ = (unsigned char)identifyDataBuf[59];
    tsaHarddiskInfo[nDriveIndex].m_DMAMultiwordSelected = (identifyDataBuf[63] >> 8) & 0b111;
    tsaHarddiskInfo[nDriveIndex].m_DMAMultiwordSupported = identifyDataBuf[63] & 0b111;
    if(tsaHarddiskInfo[nDriveIndex].m_bTimingFieldsValid)
    {
        /* PIO Mode 2 is at least supported */
        tsaHarddiskInfo[nDriveIndex].m_pioModeSupported = 2;
        /* PIO Mode 3 bit set ? */
        if((unsigned char)identifyDataBuf[64] & 0x01)
        {
            tsaHarddiskInfo[nDriveIndex].m_pioModeSupported = 3;
        }
        /* PIO Mode 4 bit set ? */
        if((unsigned char)identifyDataBuf[64] & 0x02)
        {
            tsaHarddiskInfo[nDriveIndex].m_pioModeSupported = 4;
        }
        /* Using Manufacturer’s recommended Multiword DMA transfer cycle time for now */
        tsaHarddiskInfo[nDriveIndex].m_minDMAMultiwordCycle_ns = identifyDataBuf[66];
        tsaHarddiskInfo[nDriveIndex].m_minPIOcycle_ns = identifyDataBuf[67];
        if(tsaHarddiskInfo[nDriveIndex].m_bIORDY)
        {
            tsaHarddiskInfo[nDriveIndex].m_minPIOcycle_ns = identifyDataBuf[68];
        }
    }
    tsaHarddiskInfo[nDriveIndex].m_fHasSMARTcapabilities = identifyDataBuf[82] & (1 << 0) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bSecurityFeaturesSupport = identifyDataBuf[82] & (1 << 1) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bDeviceResetSupport = identifyDataBuf[82] & (1 << 9) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bSetMaxSecuritySupport = identifyDataBuf[83] & (1 << 8) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_fFlushCacheExtSupported = identifyDataBuf[83] & (1 << 12) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_fFlushCacheSupported = identifyDataBuf[83] & (1 << 13) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_SMARTErrorLoggingSupported = identifyDataBuf[84] & (1 << 0) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_SMARTSelfTestSupported = identifyDataBuf[84] & (1 << 1) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_fSMARTEnabled = identifyDataBuf[85] & (1 << 0) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bSecurityFeaturesEnabled = identifyDataBuf[85] & (1 << 1) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_UltraDMAmodeSelected = (identifyDataBuf[88] >> 8) & 0b1111111;
    tsaHarddiskInfo[nDriveIndex].m_UltraDMAmodeSupported = identifyDataBuf[88] & 0b1111111;

    if(0 == nDriveIndex)
    {
        /* Cable conductor count can only be read on MASTER drive */
        //TODO: Validate this works with SATA drives through SATA-IDE converters
        if((identifyDataBuf[93] >> 13) & 0b111 == 0b010)
        {
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "Detected 80-conductors IDE cable");
            tsaHarddiskInfo[nDriveIndex].m_bCableConductors = 80;
        }
    }
    else
    {
        tsaHarddiskInfo[nDriveIndex].m_bCableConductors = tsaHarddiskInfo[0].m_bCableConductors;
    }

    tsaHarddiskInfo[nDriveIndex].m_masterPassSupport = identifyDataBuf[88];
    tsaHarddiskInfo[nDriveIndex].m_bSecuritySupported = identifyDataBuf[128] & (1 << 0) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bSecurityEnabled = identifyDataBuf[128] & (1 << 1) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bSecurityLocked = identifyDataBuf[128] & (1 << 2) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_bSecurityCountExpired = identifyDataBuf[128] & (1 << 4) ? 1 : 0;
    tsaHarddiskInfo[nDriveIndex].m_currentSecurityLevel = identifyDataBuf[128] & (1 << 8) ? 1 : 0;

    return 0;
}

int BootIdeSendSoftReset(void)
{
    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Reset entire ATA bus");
    return 0;
}

int sendControlATACommand(int nDriveIndex, ide_command_t command, unsigned long long startLBA, unsigned char FeatureField, unsigned short sectorCount)
{
#define DoNotSkipFirstWait false
    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "drive:%u  command:0x%02X  start:%Lu, Feature:0x%02x, count:%u", nDriveIndex, command, startLBA, FeatureField, sectorCount);

    const unsigned int uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_LBA | IDE_DH_DRIVE(nDriveIndex);
    tsicp.m_bPrecomp = FeatureField;

    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);     //Select device first
    populateLBAField(&tsicp, nDriveIndex, startLBA, commandIsEXT(command));
    tsicp.m_bCountSector = sectorCount;
    if(commandIsEXT(command))
    {
        tsicp.m_bCountSectorExt = (sectorCount >> 8);
    }


    if(BootIdeWaitNotBusy(uIoBase))
    {
        //printk("  %d:  Not Ready\n", driveId);
        return 1;
    }

    return BootIdeIssueAtaCommand(uIoBase, command, &tsicp, DoNotSkipFirstWait);
}

int sendATACommandAndSendData(int nDriveIndex, ide_command_t command, unsigned long long startLBA, const unsigned char* dataBuffer, unsigned int sizeInSectors)
{
    const unsigned int uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    if(sendControlATACommand(nDriveIndex, command, startLBA, NoFeatureField, sizeInSectors))
    {
        return 1;
    }


    BootIdeWriteData(uIoBase, dataBuffer, sizeInSectors * tsaHarddiskInfo[nDriveIndex].m_logicalSectorSize);

    if (BootIdeWaitNotBusy(uIoBase))
    {
        return 1;
    }

    return 0;
}

int sendATACommandAndReceiveData(int nDriveIndex, ide_command_t command, unsigned long long startLBA, unsigned char* dataBuffer, unsigned int sizeInSectors)
{
    const unsigned int uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    if(sendControlATACommand(nDriveIndex, command, startLBA, NoFeatureField, sizeInSectors))
    {
        return 1;
    }


    return getATAData(nDriveIndex, dataBuffer, sizeInSectors * tsaHarddiskInfo[nDriveIndex].m_logicalSectorSize);
}

int getATAData(int nDriveIndex, void* output, unsigned int length)
{
    const unsigned int uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Reading %u bytes", length);

    BootIdeWaitDataReady(uIoBase);
    if(BootIdeReadData(uIoBase, output, length))
    {
        return 1;
    }

    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Helper routines
//


int BootIdeWaitNotBusy(unsigned uIoBase)
{
    unsigned char b = ATA_SR_BSY;                                //Start being busy

    IoInputByte(IDE_REG_ALTSTATUS(uIoBase)); /* waste 100ns */
    IoInputByte(IDE_REG_ALTSTATUS(uIoBase)); /* waste 100ns */
    IoInputByte(IDE_REG_ALTSTATUS(uIoBase)); /* waste 100ns */
    IoInputByte(IDE_REG_ALTSTATUS(uIoBase)); /* waste 100ns */

    while((b & ATA_SR_BSY) && !(b & ATA_SR_DRQ))          //Device is not ready until bit7(BSY) is cleared and bit3(DRQ) is set.
    {
        b = IoInputByte(IDE_REG_STATUS(uIoBase));

        if(b == 0 || (b & ATA_SR_DF))  //No device
        {
            return -1;
        }
    }

    return b&0x01;         //bit0 == ERR
}


static int BootIdeWaitDataReady(unsigned uIoBase)
{
    int i = 0x800000;
//    wait_us(1);

    //Assuming that the while assertion, i decrementing and if condition assertion all
    //take only a single cycle per operation(very unlikely), it would take around 34ms
    //for i to reach 0 with a CPU running at 733MHz. So no need for extra smalldelay here.
    //Since our program is single-threaded anyway, there's not much harm in polling the
    //HDD's STATUS register until the necessary state is reached.
    do
    {
        if (((IoInputByte(IDE_REG_STATUS(uIoBase)) & 0x88) == 0x08))       //DRQ bit raised and BSY bit cleared.
        {
            if(IoInputByte(IDE_REG_STATUS(uIoBase)) & 0x01)
            {
                XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "Error bit raised.");
                return 2;        //ERR bit raised, return 2.
            }
            return 0;                                                           //Everything good, move on.
        }
        i--;
    } while (i != 0);

    if(IoInputByte(IDE_REG_STATUS(uIoBase)) & 0x01)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "Error bit raised after timed out.");
        return 2;                //ERR bit raised.
    }
    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_WARN, "Timeout error.");
    return 1;                                                                   //Timeout error.
}

static int BootIdeIssueAtaCommand(
    unsigned uIoBase,
    ide_command_t command,
    tsIdeCommandParams * params, bool skipFirstWait)
{
    int n;

    //IoInputByte(IDE_REG_STATUS(uIoBase));     //No need since we'll be checking ALT_STATUS register in BootIdeWaitNotBusy function.
    if(!skipFirstWait)
    {
        n=BootIdeWaitNotBusy(uIoBase);
        if(n == -1)    // as our command may be being used to clear the error, not a good policy to check too closely!
        {
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "Error on wait 1: ret=%d, error %02X", n, IoInputByte(IDE_REG_ERROR(uIoBase)));
            return n;
        }
    }

    IoOutputByte(IDE_REG_FEATURE(uIoBase), params->m_bPrecomp);

    /* 48-bit LBA */
    /* this won't hurt for non 48-bit LBA commands since we re-write these registers below */
    IoOutputByte(IDE_REG_SECTOR_COUNT(uIoBase), params->m_bCountSectorExt);
    IoOutputByte(IDE_REG_LBA_LOW(uIoBase), params->m_bSectorExt);
    IoOutputByte(IDE_REG_LBA_MID(uIoBase), params->m_wCylinderExt & 0xFF);
    IoOutputByte(IDE_REG_LBA_HIGH(uIoBase), (params->m_wCylinderExt >> 8) );
    /* End 48-bit LBA */

    IoOutputByte(IDE_REG_SECTOR_COUNT(uIoBase), params->m_bCountSector);    //Sector count reg
    IoOutputByte(IDE_REG_LBA_LOW(uIoBase), params->m_bSector);        //LBA LOW
    IoOutputByte(IDE_REG_LBA_MID(uIoBase), params->m_wCylinder & 0xFF);    //LBA MID
    IoOutputByte(IDE_REG_LBA_HIGH(uIoBase), (params->m_wCylinder >> 8));    //LBA HIGH
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), params->m_bDrivehead);     //DEVICE REG

    IoOutputByte(IDE_REG_COMMAND(uIoBase), command);                //COMMAND REG
//    wait_us(1);

    n=BootIdeWaitNotBusy(uIoBase);
    if(n)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "Error on wait 2: ret=%d, error 0x%02X", n, IoInputByte(IDE_REG_ERROR(uIoBase)));
        return n;
    }

    return 0;
}

int BootIdeReadData(unsigned uIoBase, void * buf, unsigned int size)
{
    unsigned short * ptr = (unsigned short *) buf;
    if (BootIdeWaitDataReady(uIoBase))
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "Data not ready...");
        return 1;
    }

    while (size > 1)
    {
        *ptr++ = IoInputWord(IDE_REG_DATA(uIoBase));
        size -= 2;
    }


    if(IoInputByte(IDE_REG_STATUS(uIoBase)) & 0x01)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "Ended with an error.");
        return 2;
    }
    return 0;
}


// issues a block of data ATA-style
static int BootIdeWriteData(unsigned uIoBase, const void * buf, unsigned int size)
{
    register unsigned short * ptr = (unsigned short *) buf;
    int n;

    n=BootIdeWaitDataReady(uIoBase);

    while (size > 1)
    {

        IoOutputWord(IDE_REG_DATA(uIoBase), *ptr);
        size -= 2;
        ptr++;
    }


    n=BootIdeWaitNotBusy(uIoBase);
    if(n)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "Waiting for good status reg returned error : %d", n);
        return n;
    }

    if(IoInputByte(IDE_REG_STATUS(uIoBase)) & 0x01) return 2;     //ERR flag raised.

    return 0;
}



int BootIdeWriteAtapiData(unsigned uIoBase, void * buf, unsigned int size)
{
    unsigned short * ptr = (unsigned short *) buf;
    unsigned short w;
    int n;

    n=BootIdeWaitDataReady(uIoBase);

    wait_us_blocking(1);

    w=IoInputByte(IDE_REG_LBA_MID(uIoBase));
    w|=(IoInputByte(IDE_REG_LBA_HIGH(uIoBase)))<<8;

    n=IoInputByte(IDE_REG_STATUS(uIoBase));
    if(n&1)  // error
    {
        return 1;
    }

    while (size > 1)
    {

        IoOutputWord(IDE_REG_DATA(uIoBase), *ptr);
        size -= 2;
        ptr++;
    }
    n=IoInputByte(IDE_REG_STATUS(uIoBase));
    if(n&1)  // error
    {
        return 1;
    }
    wait_us_blocking(1);
    n=BootIdeWaitNotBusy(uIoBase);
    if(n)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "Waiting for good status reg returned error : %d", n);
        return n;
    }
    wait_us_blocking(1);

   if(IoInputByte(IDE_REG_STATUS(uIoBase)) & 0x01) return 2;

    return 0;
}

int BootIdeIssueAtapiPacketCommandAndPacket(int nDriveIndex, unsigned char *pAtapiCommandPacket12Bytes)
{
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    unsigned     uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_LBA | IDE_DH_DRIVE(nDriveIndex);
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);

    tsicp.m_wCylinder=2048;
    BootIdeWaitNotBusy(uIoBase);
    if(BootIdeIssueAtaCommand(uIoBase, IDE_CMD_ATAPI_PACKET, &tsicp, false))
    {
            return 1;
    }

    if(BootIdeWaitNotBusy(uIoBase))
    {
        return 1;
    }

    if(BootIdeWriteAtapiData(uIoBase, pAtapiCommandPacket12Bytes, 12))
    {
        return 1;
    }

    if(pAtapiCommandPacket12Bytes[0]!=0x1e)
    {
        if(BootIdeWaitDataReady(uIoBase))
        {
            return 1;
        }
    }
    return 0;
}
