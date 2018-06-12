/*
 * IdeHelpers.h
 *
 *  Created on: Mar 26, 2018
 *      Author: cromwelldev
 */

#ifndef DRIVERS_IDE_IDEHELPERS_H_
#define DRIVERS_IDE_IDEHELPERS_H_

#include "lib/LPCMod/xblastDebug.h"
#include <stdbool.h>

typedef enum {
    IDE_CMD_NOOP = 0,
    IDE_CMD_RECALIBRATE = 0x10,
    IDE_CMD_READ_MULTI_RETRY = 0x20,
    IDE_CMD_READ_MULTI = IDE_CMD_READ_MULTI_RETRY,
    IDE_CMD_READ_MULTI_NORETRY = 0x21,

    IDE_CMD_READ_EXT = 0x24, /* 48-bit LBA */
    IDE_CMD_READ_DMA_EXT = 0x25,
    IDE_CMD_READ_MULTIPLE_EXT = 0x29,
    IDE_CMD_WRITE_MULTI_RETRY = 0x30,
    IDE_CMD_WRITE_EXT = 0x34
    ,
    IDE_CMD_WRITE_DMA_EXT = 0x35,
    IDE_CMD_WRITE_MULTIPLE_EXT = 0x39,

    IDE_CMD_DRIVE_DIAG = 0x90,
    IDE_CMD_SET_PARAMS = 0x91,
    IDE_CMD_STANDBY_IMMEDIATE = 0x94, /* 2 byte command- also send
                                         IDE_CMD_STANDBY_IMMEDIATE2 */

    IDE_CMD_SMART = 0xB0,
    IDE_CMD_WRITE_MULTIPLE = 0xC5,
    IDE_CMD_READ_MULTIPLE = 0xC4,
    IDE_CMD_SET_MULTIPLE_MODE = 0xC6,
    IDE_CMD_READ_DMA = 0xC8,
    IDE_CMD_WRITE_DMA = 0xCA,
    IDE_CMD_STANDBY_IMMEDIATE2 = 0xE0,
    IDE_CMD_CACHE_FLUSH = 0xE7,
    IDE_CMD_CACHE_FLUSH_EXT = 0xEA,

    //Get info commands
    IDE_CMD_IDENTIFY = 0xEC,
    IDE_CMD_PACKET_IDENTIFY = 0xA1,

    ATAPI_SOFT_RESET = 0x08,

    IDE_CMD_SET_FEATURES = 0xEF,

    IDE_CMD_ATAPI_PACKET = 0xA0,

    //IDE security commands
    IDE_CMD_SECURITY_SET_PASSWORD = 0xF1,
    IDE_CMD_SECURITY_UNLOCK = 0xF2,
    IDE_CMD_SECURITY_DISABLE = 0xF6
} ide_command_t;


////////////////////////////////////
// IDE types and constants
#define IDE_SECTOR_SIZE         0x200
#define IDE_BASE1                     (0x1F0u) /* primary controller */

#define IDE_REG_EXTENDED_OFFSET       (0x204u) /* 0x3F4 */

/* Read/Write registers*/
#define IDE_REG_DATA(base)              ((base) + 0u) /* word register */
#define IDE_REG_SECTOR_COUNT(base)      ((base) + 2u)                          //Sector Count register (read-write)
#define IDE_REG_LBA_LOW(base)           ((base) + 3u)                          //(read-write)
#define IDE_REG_LBA_MID(base)           ((base) + 4u)                          //(read-write)
#define IDE_REG_LBA_HIGH(base)          ((base) + 5u)                          //(read-write)
#define IDE_REG_DRIVEHEAD(base)         ((base) + 6u)                          //Device control register (read-write)
#define IDE_REG_DEVICE(base)            ((base) + 6u)                          //Same as above but easier to remember


/* Read only registers*/
#define IDE_REG_ERROR(base)             ((base) + 1u)                          //Error register (read-only)
#define IDE_REG_STATUS(base)            ((base) + 7u)                          //Status register (read-only)
#define IDE_REG_ALTSTATUS(base)         ((base) + IDE_REG_EXTENDED_OFFSET + 2u) // Same as STATUS but does not affect IRQs

/* Write Only registers*/
#define IDE_REG_FEATURE(base)           ((base) + 1u)                          //Features register (write-only)
#define IDE_REG_COMMAND(base)           ((base) + 7u)                          //Command Register(write-only)
#define IDE_REG_CONTROL(base)           ((base) + IDE_REG_EXTENDED_OFFSET + 2u) // ATA bus control reg. b1:Enable/disable IRQ firing, b2:soft reset whole bus, b7:LBA48 hi-byte readback toggle

/*
Normal command input(sending to ATA device) must send the following registers:
      -Features
      -Sector Count
      -LBA Low
      -LBA Mid
      -LBA High
      -Device
      -Command

Command register must be the last one to be sent for the command to execute. It's then a good practice
to send all the necessary registers in the same order as presented above.


Normal command output(receiving from ATA device) will require a read on the following registers
    -Error
    -Sector Count
    -LBA Low
    -LBA Mid
    -LBA High
    -Device
    -Status
*/

/* Status Register bits */
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Inlex
#define ATA_SR_ERR     0x01    // Error


/* Error Register bits */
#define ATA_ER_BBK      0x80    // Bad sector
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // No media
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // No media
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

/* Control Register bits */
#define ATA_CTRL_nIEN   0x02    // 1: Disable device sending interrupt
#define ATA_CTRL_SRST   0x04    // 1: Software reset entire bus
#define ATA_CTRL_HOB    0x80    // 1: Read back the High Order Byte of the last LBA48 value sent to an IO port



typedef struct {
    unsigned char m_bPrecomp;
    unsigned char m_bCountSector;
    unsigned char m_bSector;
    unsigned short m_wCylinder;
    unsigned char m_bDrivehead;

       /* 48-bit LBA */
    unsigned char m_bCountSectorExt;
    unsigned char m_bSectorExt;
    unsigned short m_wCylinderExt;

#       define IDE_DH_DEFAULT (0xA0)
#       define IDE_DH_HEAD(x) ((x) & 0x0F)
#       define IDE_DH_MASTER  (0x00)
#       define IDE_DH_SLAVE   (0x10)
#       define IDE_DH_DRIVE(x) ((((x) & 1) != 0)?IDE_DH_SLAVE:IDE_DH_MASTER)
#       define IDE_DH_LBA     (0x40)
#       define IDE_DH_CHS     (0x00)

} tsIdeCommandParams;

#define IDE_DEFAULT_COMMAND { 0x00u, 0x01, 0x00, 0x0000, IDE_DH_DEFAULT | IDE_DH_SLAVE, 0x00, 0x00, 0x0000 }
#define NoStartLBA  0
#define NoFeatureField 0
#define NoSectorCount 0

typedef enum
{
    UltraDMAMode0 = 0x01,
    UltraDMAMode1 = 0x02,
    UltraDMAMode2 = 0x04,
    UltraDMAMode3 = 0x08,
    UltraDMAMode4 = 0x10,
    UltraDMAMode5 = 0x20,
    UltraDMAMode6 = 0x40
}UltraDMAMode;

typedef struct tsHarddiskInfo {  // this is the retained knowledge about an IDE device after init
    unsigned short m_fwPortBase;
    unsigned char m_fDriveExists;
    unsigned char m_fAtapi;  // true if a CDROM, etc

    unsigned short m_wCountCylinders;           /* Word1 */
    unsigned short m_wCountHeads;               /* Word3 */
    unsigned short m_wCountSectorsPerTrack;     /* Word6 */
    unsigned long long m_dwCountSectorsTotal;   /* If 48bitsAddrSupport=0, Word60-61. */
                                                /* Else */
                                                /* If 48bitsAddrSupport=1, Word100-103 */
                                                /* Else */
                                                /* ExtendedSectorCountSupport=1, Word230-233 */

    unsigned char m_bLbaMode;                   /* Word49, 9 */
    unsigned char m_b48bitsAddrSupport;         /* Word83, 10 */
    unsigned char m_bExtendedSectorCountSupport;/* Word69, 3 */
    unsigned short m_logicalSectorSize;         /* If Word106,12=1, sector size in Word117-118. If not sector size is 256Words(512bytes). */

    unsigned char m_szSerial[20 + sizeof('\0')];               /* Word10-19 */
    unsigned char s_length;                                    /* Serial string length */
    char m_szFirmware[8 + sizeof('\0')];                       /* Word23-26 */
    unsigned char m_szIdentityModelNumber[40 + sizeof('\0')];  /* Word27-46 */
    unsigned char m_length;                                    /* Model string length */

    unsigned short m_wAtaRevisionSupported;     /* Word88 */
    unsigned char m_fFlushCacheSupported;       /* Word83, 13 */
    unsigned char m_fFlushCacheExtSupported;    /* Word83, 12 */

    unsigned char m_bIORDY;                     /* Word49, 11 */
    unsigned char m_maxBlockPerDRQ;             /* Word47, 7:0 */   /* Obsolete ? */
    unsigned char m_bCurrentBlockPerDRQValid;   /* Word59, 8 */     /* Obsolete ? */
    unsigned char m_currentBlockPerDRQ;         /* Word59, 7:0 */   /* Obsolete ? */
    unsigned short m_minPIOcycle_ns;            /* Word67 or Word68 if IORDY supported */
    unsigned char m_pioModeSupported;           /* Word64 7:0 */
    unsigned char m_bCableConductors;           // Word93, bits 15-13 = 010 then, 80 conductors, else 40 */

    unsigned char m_bDMASupported;              /* Word49, 8 */
    unsigned char m_bDMAModeFieldValid;         /* Word53, 2 */
    unsigned char m_bTimingFieldsValid;         /* Word53, 1 */
    unsigned char m_DMAMultiwordSelected;       /* Word63, 10-8 */
    unsigned char m_DMAMultiwordSupported;      /* Word63, 2-0 */
    unsigned short m_minDMAMultiwordCycle_ns;   /* Word65 or Word66 */
    UltraDMAMode m_UltraDMAmodeSelected;        /* Word88, 14-8 */
    UltraDMAMode m_UltraDMAmodeSupported;       /* Word88, 6-0 */

    unsigned char m_bSecurityFeaturesSupport;   /* Word82, 1 */
    unsigned char m_bSecurityFeaturesEnabled;   /* Word85, 1 */
    unsigned char m_bSetMaxSecuritySupport;     /* Word83, 8 */
    unsigned short m_masterPassSupport;         /* Word92   Values of 0x0000 and 0xFFFF means feature is not supported */
    unsigned char m_currentSecurityLevel;       /* Word128, 8  0=high  1=maximum */
    unsigned char m_bSecurityCountExpired;      /* Word128, 4 */
    unsigned char m_bSecurityLocked;            /* Word128, 2 */
    unsigned char m_bSecurityEnabled;           /* Word128, 1 */
    unsigned char m_bSecuritySupported;         /* Word128, 0 */

    unsigned char m_fHasSMARTcapabilities;      /* Word82,bit0 */
    unsigned char m_fSMARTEnabled;              /* Word85,bit0 */
    unsigned char m_SMARTSelfTestSupported;     /* Word84,bit1 */
    unsigned char m_SMARTErrorLoggingSupported; /* Word84,bit0 */

    unsigned char m_bDeviceResetSupport;        /* Word82, 9 */
} tsHarddiskInfo;

extern tsHarddiskInfo tsaHarddiskInfo[2];

/*-----------------------------------------------------------------------------*/
/* Frequently used ATA commands */
/*-----------------------------------------------------------------------------*/

/* Send And Populate tsHarddiskInfo */
int BootIdeSendIdentifyDevice(int nDriveIndex);
int BootIdeSendSoftReset(void);


/*-----------------------------------------------------------------------------*/
/* Generic ATA commands*/
/*-----------------------------------------------------------------------------*/
int sendControlATACommand(int nDriveIndex, ide_command_t command, unsigned long long startLBA, unsigned char FeatureField, unsigned short sectorCount);
int sendATACommandAndSendData(int nDriveIndex, ide_command_t command, unsigned long long startLBA, const unsigned char* dataBuffer, unsigned int sizeInBytes);
int sendATACommandAndReceiveData(int nDriveIndex, ide_command_t command, unsigned long long startLBA, unsigned char* dataBuffer, unsigned int sizeInSectors);
int getATAData(int nDriveIndex, void* output, unsigned int length);
int BootIdeReadData(unsigned uIoBase, void * buf, unsigned int size);
int BootIdeWriteAtapiData(unsigned uIoBase, void * buf, unsigned int size);
int BootIdeIssueAtapiPacketCommandAndPacket(int nDriveIndex, unsigned char *pAtapiCommandPacket12Bytes);
int BootIdeWaitNotBusy(unsigned uIoBase);


/*-----------------------------------------------------------------------------*/
/* ATA controller config*/
/*-----------------------------------------------------------------------------*/
int BootIdeSetTransferMode(int nIndexDrive, int nMode);
int BootIdeEnableWriteCache(int nIndexDrive, bool enable);


#endif /* DRIVERS_IDE_IDEHELPERS_H_ */
