/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 /* 2003-01-06  andy@warmcat.com  Created
 */
 
 // header for BootFlash.c
#include "xblast/settings/xblastSettingsDefs.h"

#define SHOWGUI 1
#define NOGUI 0

// callback events
// note that if you receive *_START, you will always receive *_END even if an error is detected
 typedef enum {
     EE_ERASE_START=1,
    EE_ERASE_UPDATE,  // dwPos runs from 0 to dwExtent-1
    EE_ERASE_END,
    EE_ERASE_ERROR,  // dwPos indicates error offset from start of flash
    EE_PROGRAM_START,
    EE_PROGRAM_UPDATE,  // dwPos runs from 0 to dwExtent-1
    EE_PROGRAM_END,
    EE_PROGRAM_ERROR,  // dwPos indicates error offset from start of flash, b7..b0 = read data, b15..b8 = written data
    EE_VERIFY_START,
    EE_VERIFY_UPDATE,  // dwPos runs from 0 to dwExtent-1
    EE_VERIFY_END,
    EE_VERIFY_ERROR  // dwPos indicates error offset from start of flash, b7..b0 = read data, b15..b8 = written data
 } ENUM_EVENTS;

     // callback typedef
typedef bool (*CALLBACK_FLASH)(void * pvoidObjectFlash, ENUM_EVENTS ee, unsigned int dwPos, unsigned int dwExtent);

typedef struct {
     volatile unsigned char * volatile m_pbMemoryMappedStartAddress; // fill on entry
    unsigned char m_bManufacturerId;
    unsigned char m_bDeviceId;
    char m_szFlashDescription[256];
     char m_szAdditionalErrorInfo[256];
    unsigned int m_dwLengthInBytes;
    unsigned int m_dwStartOffset;
    unsigned int m_dwLengthUsedArea;
    CALLBACK_FLASH m_pcallbackFlash;
    bool m_fIsBelievedCapableOfWriteAndErase;
} OBJECT_FLASH;


typedef struct {
    unsigned char m_bManufacturerId;
    unsigned char m_bDeviceId;
     char m_szFlashDescription[32];
    unsigned int m_dwLengthInBytes;
} KNOWN_FLASH_TYPE;


// requires pof->m_pbMemoryMappedStartAddress set to start address of flash in memory on entry

bool FlashFileFromBuffer(unsigned char *fileBuf, unsigned int fileSize, bool askConfirm);

int BootReflashAndReset(unsigned char *pbNewData, unsigned int dwStartOffset, unsigned int dwLength);
int BootReflash(unsigned char *pbNewData, unsigned int dwStartOffset, unsigned int dwLength);
int BootFlashSettings(unsigned char *pbNewData, unsigned int dwStartOffset, unsigned int dwLength);
void BootShowFlashDevice(void);
bool BootFlashPrintResult(int res, unsigned int fileSize);

bool BootFlashGetDescriptor( OBJECT_FLASH *pof, KNOWN_FLASH_TYPE * pkft );
bool BootFlashEraseMinimalRegion( OBJECT_FLASH *pof);
bool BootFlashErase4KSector( OBJECT_FLASH *pof);
bool BootFlashProgram( OBJECT_FLASH *pof, unsigned char *pba);

void WriteToIO(unsigned short address, unsigned char data);
unsigned char ReadFromIO(unsigned short address);
unsigned char xGetByteFromFlash( OBJECT_FLASH * myflash, int myaddress);

//Copy into memory 3*256 bytes of settings for LPCMod OS from flash and place it in LPCmodSettings struct.
void BootFlashGetOSSettings(_LPCmodSettings *LPCmodSettings);

//Copy into flash 3*256 bytes of settings for LPCMod OS from memory and place it in the last 4KB block of the flash chip.
void BootFlashSaveOSSettings(void);


int assertOSUpdateValidInput(unsigned char * inputFile);
bool assert4KBErase(OBJECT_FLASH *pof);

void mirrorImage(unsigned char *pbNewData, unsigned int dwLength, OBJECT_FLASH* of);

int fetchBootScriptFromFlash(unsigned char ** buffer);
