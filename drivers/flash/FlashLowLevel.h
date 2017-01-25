/*
 * FlashLowLevel.h
 *
 *  Created on: Oct 18, 2016
 *      Author: cromwelldev
 */

#ifndef FLASHLOWLEVEL_H_
#define FLASHLOWLEVEL_H_

#include "stdbool.h"

typedef struct {
    unsigned char m_bManufacturerId;
    unsigned char m_bDeviceId;
     char m_szFlashDescription[32];
    unsigned int m_dwLengthInBytes;
    unsigned char m_support4KBErase;
} KNOWN_FLASH_TYPE;

typedef struct {
 	volatile unsigned char * volatile m_pbMemoryMappedStartAddress; // fill on entry
 	KNOWN_FLASH_TYPE flashType;
 	char m_szAdditionalErrorInfo[256];
	bool m_fIsBelievedCapableOfWriteAndErase;
} OBJECT_FLASH;

typedef enum
{
    FlashOp_Idle,
    FlashOp_PendingOp,
    FlashOp_EraseInProgress,
    FlashOp_WriteInProgress,
    FlashOp_VerifyInProgress,
    FlashOp_ReadInProgress,
    FlashOp_Completed,
    FlashOp_Error
} FlashOp;

typedef enum
{
    FlashErrorcodes_NoError = 0U,
    FlashErrorcodes_UserAbort,
    FlashErrorcodes_FailedErase,
    FlashErrorcodes_FailedProgram,
    FlashErrorcodes_FailedVerify,
    FlashErrorcodes_UnknownFlash,
    FlashErrorcodes_WriteProtect,
    FlashErrorcodes_FileSizeError,
    FlashErrorcodes_InvalidUpdateFile,
    FlashErrorcodes_MD5Mismatch,
    FlashErrorcodes_FlashContentError,
    FlashErrorcodes_DowngradeWarning,
    FlashErrorcodes_UndefinedError
} FlashErrorcodes;

OBJECT_FLASH flashDevice;

void FlashLowLevel_Init(void);
bool FlashLowLevel_ReadDevice(void);
bool FlashLowLevel_DeviceIsBusy(void);
void FlashLowLevel_InititiateSectorErase(unsigned int addr); // 4KB
void FlashLowLevel_InititiateBlockErase(unsigned int addr);  // 64KB
void FlashLowLevel_InititiateChipErase(void);
void FlashLowLevel_WriteByte(unsigned char byte, unsigned int addr);
unsigned char FlashLowLevel_ReadByte(unsigned int addr);

#endif /* FLASHLOWLEVEL_H_ */
