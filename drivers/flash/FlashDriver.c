/*
 * FlashDriver.c
 *
 *  Created on: Dec 5, 2016
 *      Author: cromwelldev
 */

#include "FlashDriver.h"
#include "FlashLowLevel.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "md5.h"
#include "lib/misc/crc32.h"
#include "lpcmod_v1.h"
#include "lib/LPCMod/xblastDebug.h"
#include "xblast/scriptEngine/xblastScriptEngine.h"
#include "xblast/settings/xblastSettings.h"
#include "xblast/HardwareIdentifier.h"
#include "lib/cromwell/cromSystem.h"
#include "config.h"
#include "string.h"
#include "boot.h"

// Constants
#define ImageSize256KB 256 * 1024
#define ImageSize512KB 512 * 1024
#define ImageSize1024KB 1024 * 1024

#define FlashSectorSize_4KB 4 * 1024
#define FlashBlockSize_64KB 64 * 1024
#define FlashChipSize_256KB 256 * 1024

#define EraseBusyCountMin 3

typedef enum
{
    EraseSequenceMethod_Sector = 0U,
    EraseSequenceMethod_Block,
    EraseSequenceMethod_Chip
} EraseSequenceMethod;

// Variables
static unsigned char biosBuffer[ImageSize1024KB];
static unsigned int biosBufferSize;
static FlashOp currentFlashOp;
static FlashErrorcodes flashErrorCode;
static unsigned int startingOffset;
static unsigned int currentAddr;
static FlashTask currentFlashTask;
static EraseSequenceMethod eraseSequenceMethod;
static unsigned int eraseBusyCount;
static bool firstEraseTry;


static FlashErrorcodes  Flash_WriteBios(const unsigned char* buf, unsigned int size, unsigned int offset, FlashBank flashBank);
static FlashErrorcodes checkImageSize(unsigned int size);
static void mirrorimage(FlashBank flashBank);
static FlashErrorcodes validateOSImage(const unsigned char* inBuf, unsigned int size);
static void evaluateReadBackRange(void);
static struct BiosIdentifier getBiosIdentifierFromBuffer(const unsigned char* buf, unsigned int size);
static unsigned int getXBlastOSSettingStartingOffset(struct BiosIdentifier biosID);
static unsigned int calculateSettingsStructCRC32Value(const _LPCmodSettings* in);
static void injectSettingsInBuf(unsigned int offset);
static bool canWrite(unsigned char flashByte, unsigned char bufferByte);
static unsigned int getEraseMethodSize(void);

void Flash_Init(void)
{
    FlashLowLevel_Init();

    Flash_freeFlashFSM();
}

void Flash_executeFlashFSM(void)
{
    switch(currentFlashOp)
    {

    case FlashOp_Idle:

        break;
    case FlashOp_PendingOp:
        if(currentFlashTask == FlashTask_WriteBios)
        {
            XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Starting WriteBIOS sequence");
            currentFlashOp = FlashOp_EraseInProgress;
        }
        else if(currentFlashTask == FlashTask_ReadBios)
        {
            XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Starting ReadBIOS sequence");
            currentFlashOp = FlashOp_ReadInProgress;
        }
        else if(currentFlashTask == FlashTask_WriteSettings)
        {
            XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Starting WriteSettings sequence");
            currentFlashOp = FlashOp_ReadInProgress;
        }
        else if(currentFlashTask == FlashTask_ReadSettings)
        {
            XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Starting ReadSettings sequence");
            currentFlashOp = FlashOp_ReadInProgress;
        }
        break;

    case FlashOp_EraseInProgress:
        if(FlashLowLevel_DeviceIsBusy())
        {
            // This is good.
            eraseBusyCount++;
        }
        else
        {
            unsigned char byteFromFlash = FlashLowLevel_ReadByte(startingOffset + currentAddr);
            if(canWrite(byteFromFlash, biosBuffer[currentAddr])) // Byte does not require erase
            {
                currentAddr++; // Moving to next byte
                firstEraseTry = true;
                eraseBusyCount = 0;

                if(currentAddr >= biosBufferSize) // Erase is actually over
                {
                    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Desired range erased! Moving to Write.   biosBufferSize=%u", biosBufferSize);
                    currentFlashOp = FlashOp_WriteInProgress;
                    currentAddr = 0;
                    return;
                }
            }
            else
            {
                XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Require Erase on address: %u", startingOffset + currentAddr);
                if(firstEraseTry == false) // It't not the first time we got here for the same byte.
                {
                    XBlastLogger(DBG_LVL_WARN, DEBUG_FLASH_DRIVER,"Second try for same address: %u", startingOffset + currentAddr);
                    if(eraseBusyCount < EraseBusyCountMin)
                    {
                        firstEraseTry = true;
                        XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Busy flag not set enough times.");
                        eraseBusyCount = 0;
                        // Erasing never happened. Command not supported? Moving to next possible command.
                        switch(eraseSequenceMethod)
                        {
                        case EraseSequenceMethod_Sector:
                            if(flashDevice.flashType.m_support4KBErase)
                            {
                                eraseSequenceMethod = EraseSequenceMethod_Block;
                                XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Switching to 64KB block erase.");
                            }
                            else
                            {
                                eraseSequenceMethod = EraseSequenceMethod_Chip;
                                XBlastLogger(DBG_LVL_WARN, DEBUG_FLASH_DRIVER,"Switching to chip erase.");
                            }

                            if(currentFlashTask == FlashTask_WriteSettings)
                            {
                                evaluateReadBackRange();
                                return;
                            }
                            break;
                        case EraseSequenceMethod_Block:
                            eraseSequenceMethod = EraseSequenceMethod_Chip;
                            XBlastLogger(DBG_LVL_WARN, DEBUG_FLASH_DRIVER,"Switching to chip erase.");
                            if(currentFlashTask == FlashTask_WriteSettings)
                            {
                                evaluateReadBackRange();
                                return;
                            }
                            break;
                        case EraseSequenceMethod_Chip:
                            currentFlashOp = FlashOp_Error;
                            flashErrorCode = FlashErrorcodes_FailedErase;
                            XBlastLogger(DBG_LVL_FATAL, DEBUG_FLASH_DRIVER,"Halt erase. No possible solution.");
                            return;
                        }
                    }
                }

                switch(eraseSequenceMethod)
                {
                case EraseSequenceMethod_Sector:
                    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Block erase on address: %u", startingOffset + currentAddr);
                    FlashLowLevel_InititiateSectorErase(startingOffset + currentAddr);
                    break;
                case EraseSequenceMethod_Block:
                    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Sector erase on address: %u", startingOffset + currentAddr);
                    FlashLowLevel_InititiateBlockErase(startingOffset + currentAddr);
                    break;
                case EraseSequenceMethod_Chip:
                    XBlastLogger(DBG_LVL_WARN, DEBUG_FLASH_DRIVER,"Chip erase");
                    FlashLowLevel_InititiateChipErase();
                    break;
                }
                firstEraseTry = false;
            }
        }
        break;

    case FlashOp_WriteInProgress:
        if(FlashLowLevel_DeviceIsBusy() == false)
        {
            if(currentAddr >= biosBufferSize)
            {
                XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Desired range wrote! Moving to Verify.");
                currentFlashOp = FlashOp_VerifyInProgress;
                currentAddr = 0;
            }
            else
            {
                unsigned char byteFromFlash = FlashLowLevel_ReadByte(startingOffset + currentAddr);
                if(byteFromFlash != biosBuffer[currentAddr])
                {
                    FlashLowLevel_WriteByte(biosBuffer[currentAddr], startingOffset + currentAddr);
                }
                currentAddr++;
            }
        }
        break;

    case FlashOp_VerifyInProgress:
        if(FlashLowLevel_DeviceIsBusy() == false)
        {
            if(currentAddr >= biosBufferSize)
            {
                XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Verify completed.");
                currentFlashOp = FlashOp_Completed;
            }
            else
            {
                unsigned char byteFromFlash = FlashLowLevel_ReadByte(startingOffset + currentAddr);
                if(byteFromFlash == biosBuffer[currentAddr])
                {
                    currentAddr++;
                }
                else
                {
                    XBlastLogger(DBG_LVL_ERROR, DEBUG_FLASH_DRIVER,"Data mismatch.   startingOffset=%u   currentAddr=%u   flash=%02X   buf=%02X", startingOffset, currentAddr, byteFromFlash, biosBuffer[currentAddr]);
                    currentFlashOp = FlashOp_Error;
                    flashErrorCode = FlashErrorcodes_FailedVerify;
                }
            }
        }
        break;

    case FlashOp_ReadInProgress:
        if(FlashLowLevel_DeviceIsBusy() == false)
        {
            biosBuffer[currentAddr] = FlashLowLevel_ReadByte(startingOffset + currentAddr);
            currentAddr++;

            if(currentAddr >= biosBufferSize)
            {
                currentAddr = 0;
                if(currentFlashTask == FlashTask_ReadBios)
                {
                    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Read BIOS completed.");
                    currentFlashOp = FlashOp_Completed;
                }
                else if(currentFlashTask == FlashTask_ReadSettings)
                {
                    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Read Settings completed.");
                    currentFlashOp = FlashOp_Completed;
                }
                else if(currentFlashTask == FlashTask_WriteSettings)
                {
                    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Readback flash for context save.");
                    injectSettingsInBuf(getXBlastOSSettingStartingOffset(getBiosIdentifierFromFlash()));
                    currentFlashOp = FlashOp_EraseInProgress;
                }
            }
        }
        break;

    case FlashOp_Completed:

        break;
    case FlashOp_Error:

        break;
    }
}

void Flash_freeFlashFSM(void)
{
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Reset flash FSM.");
    biosBufferSize = 0;
    currentFlashOp = FlashOp_Idle;
    flashErrorCode = FlashErrorcodes_NoError;
    currentAddr = 0;
    currentFlashTask = FlashTask_NoTask;
    switchOSBank(FlashBank_OSBank);
}

void Flash_forceUserAbort(void)
{
    currentFlashTask = FlashTask_WriteBios;
    currentFlashOp = FlashOp_Error;
    flashErrorCode = FlashErrorcodes_UserAbort;
}

FlashProgress Flash_getProgress(void)
{
    FlashProgress result;

    result.currentFlashOp = currentFlashOp;
    result.currentFlashTask = currentFlashTask;
    result.flashErrorCode = flashErrorCode;
    if(biosBufferSize == 0)
    {
        result.progressInPercent = 0;
    }
    else
    {
        float temp = (float)(currentAddr +1) / (float)biosBufferSize;
        result.progressInPercent = (unsigned char)(temp * 100);
    }

    return result;
}

FlashProgress Flash_ReadDeviceInfo(const OBJECT_FLASH* *const output)
{
    FlashProgress result;

    result.currentFlashOp = FlashOp_Error;
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Enter");

    if(currentFlashOp == FlashOp_Idle)
    {
        XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Flash FSM idle.");
        if(FlashLowLevel_ReadDevice())
        {
            result.currentFlashOp = FlashOp_Completed;
            XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Device read.");
            *output = &flashDevice;
            result.flashErrorCode = FlashErrorcodes_NoError;
            result.progressInPercent = 100;
            Flash_freeFlashFSM();

            return result;
        }
    }

    XBlastLogger(DBG_LVL_ERROR, DEBUG_FLASH_DRIVER,"Error! Flash FSM NOT idle.");

    result.flashErrorCode = FlashErrorcodes_UnknownFlash;
    result.progressInPercent = 0;
    Flash_freeFlashFSM();

    return result;
}

FlashProgress Flash_ReadBIOSBank(FlashBank bank)
{
    if(currentFlashOp == FlashOp_Idle)
    {
        XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Flash FSM idle.");
        if(FlashLowLevel_ReadDevice())
        {
            currentFlashOp = FlashOp_PendingOp;
            currentFlashTask = FlashTask_ReadBios;


            //TODO: BFD - correct read size for XBlast bank sizes.
            switch(bank)
            {
            case FlashBank_OSBank:
            case FlashBank_256Bank:
                biosBufferSize = ImageSize256KB;
                break;
            case FlashBank_512Bank:
            case FlashBank_SplitTSOP0Bank:
            case FlashBank_SplitTSOP1Bank:
                biosBufferSize = ImageSize512KB;
                break;
            default:
                biosBufferSize = flashDevice.flashType.m_dwLengthInBytes;
                break;
            }
            startingOffset = 0;
        }
    }

    return Flash_getProgress();
}

unsigned int getBiosBuffer(const unsigned char* *const output)
{
    *output = biosBuffer;
    return biosBufferSize;
}

FlashProgress Flash_XBlastOSBankFlash(const unsigned char* inBuf, unsigned int size, unsigned int offset, bool overrideChecks)
{
    XBlastLogger(DBG_LVL_WARN, DEBUG_FLASH_DRIVER,"Initiate XBlast OS update.");
    if(overrideChecks == false)
    {
        flashErrorCode = validateOSImage(inBuf, size);
    }
    if(flashErrorCode == FlashErrorcodes_NoError)
    {
        flashErrorCode = Flash_WriteBios(inBuf, size, offset, FlashBank_OSBank);
        if(flashErrorCode == FlashErrorcodes_NoError)
        {
            //save settings in image to write
            //XXX: overrideChecks skips saving settings to image?
            injectSettingsInBuf(getXBlastOSSettingStartingOffset(getBiosIdentifierFromBuffer(biosBuffer, biosBufferSize)));
        }
    }

    return Flash_getProgress();
}

FlashProgress Flash_XBlastUserBankFlash(const unsigned char* inBuf, unsigned int size, unsigned int offset, FlashBank bank)
{
    flashErrorCode = Flash_WriteBios(inBuf, size, offset, bank);

    return Flash_getProgress();
}

FlashProgress Flash_SimpleBIOSBankFlash(const unsigned char* inBuf, unsigned int size, unsigned int offset)
{
    return Flash_XBlastUserBankFlash(inBuf, size, offset, FlashBank_NoBank);
}

FlashProgress Flash_ReadXBlastOSSettingsRequest(void)
{
    if(currentFlashOp == FlashOp_Idle)
    {
        XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Flash FSM idle.");
        if(FlashLowLevel_ReadDevice() || isXecuter3()) // in an attempt to read settings when X3 flash protect is on.
        {
            currentFlashOp = FlashOp_PendingOp;
            currentFlashTask = FlashTask_ReadSettings;

            switchOSBank(FlashBank_OSBank);
            biosBufferSize = sizeof(_LPCmodSettings);
            XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Get settings offset.");
            startingOffset = getXBlastOSSettingStartingOffset(getBiosIdentifierFromFlash());

            XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Size of Settings struct to read = %u", biosBufferSize);
            if(startingOffset == 0)
            {
                XBlastLogger(DBG_LVL_FATAL, DEBUG_FLASH_DRIVER,"Error! Could not locate proper save location.");
                currentFlashOp = FlashOp_Completed;
                *biosBuffer = 0xff;
            }
        }
        else
        {
            currentFlashOp = FlashOp_Error;
        }
    }

    return Flash_getProgress();
}

bool Flash_LoadXBlastOSSettings(_LPCmodSettings* input)
{
    bool returnValue = false;
    if(currentFlashOp == FlashOp_Completed && currentFlashTask == FlashTask_ReadSettings)
    {
        const _LPCmodSettings* seeker = (const _LPCmodSettings*)biosBuffer;

        XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Settings Version in Flash = %u   Expected:%u", seeker->settingsVersion, CurrentSettingsVersionNumber);
        if(seeker->settingsVersion == CurrentSettingsVersionNumber)
        {
            unsigned int calculatedCRC32 = calculateSettingsStructCRC32Value(seeker);
            XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Calculated CRC32 : 0x%08X", calculatedCRC32);
            XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Stored CRC32     : 0x%08X", seeker->crc32Value);

            if(calculatedCRC32 == seeker->crc32Value)
            {
                XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Settings accepted in biosBuf. Size of buf = %u", biosBufferSize);
                memcpy(input, biosBuffer, sizeof(_LPCmodSettings));
                returnValue = true;
            }
        }
#if 0
        else
        {
            // migrate settings to current version
        }
#endif
    }

    if(returnValue == false)
    {
        XBlastLogger(DBG_LVL_WARN, DEBUG_FLASH_DRIVER,"Error? No settings in biosBuf...");
        populateSettingsStructWithDefault(input);
    }
    return returnValue;
}

bool bootReadXBlastOSSettings(void)
{
    bool returnValue = true;
    XBlastLogger(DBG_LVL_INFO, DEBUG_FLASH_DRIVER,"Initiate Read XBlast OS settings.");

    populateSettingsStructWithDefault(&LPCmodSettings);

    if(returnValue)
    {
        FlashProgress progress = Flash_ReadXBlastOSSettingsRequest();

        while(cromwellLoop())
        {
            progress = Flash_getProgress();

            if(progress.currentFlashOp == FlashOp_Completed || progress.currentFlashOp == FlashOp_Error)
            {
                XBlastLogger(DBG_LVL_INFO, DEBUG_FLASH_DRIVER,"Read Settings from flash completed.");
                returnValue = Flash_LoadXBlastOSSettings(&LPCmodSettings);



                break;
            }

            Flash_executeFlashFSM();
        }
        memcpy(&LPCmodSettingsOrigFromFlash, &LPCmodSettings, sizeof(_LPCmodSettings));
    }

    Flash_freeFlashFSM();

    return returnValue;
}

FlashProgress Flash_SaveXBlastOSSettings(void)
{
    if(currentFlashOp == FlashOp_Idle)
    {
        XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Flash FSM idle.");
        if(FlashLowLevel_ReadDevice())
        {
            startingOffset = getXBlastOSSettingStartingOffset(getBiosIdentifierFromFlash());

            if(startingOffset != 0)
            {
                eraseSequenceMethod = EraseSequenceMethod_Sector;
                eraseBusyCount = 0;
                firstEraseTry = true;
                biosBufferSize = getEraseMethodSize();

                currentFlashOp = FlashOp_PendingOp;
                currentFlashTask = FlashTask_WriteSettings;

                XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"WriteSettings param set.");
            }
            else
            {
                XBlastLogger(DBG_LVL_ERROR, DEBUG_FLASH_DRIVER,"Flash does not contain XBlast OS image on selected bank. Aborting.");
                currentFlashOp = FlashOp_Error;
                flashErrorCode = FlashErrorcodes_FlashContentError;
            }
        }
        else
        {
            currentFlashOp = FlashOp_Error;
            flashErrorCode = FlashErrorcodes_UnknownFlash;
            if(isXecuter3())
            {
                flashErrorCode = FlashErrorcodes_WriteProtect;
            }
        }
    }
    return Flash_getProgress();
}

static FlashErrorcodes Flash_WriteBios(const unsigned char* buf, unsigned int size, unsigned int offset, FlashBank flashBank)
{
    if(currentFlashOp == FlashOp_Idle)
    {
        XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Flash FSM idle.");
        if(FlashLowLevel_ReadDevice())
        {
            flashErrorCode = checkImageSize(size);

            if(flashErrorCode == FlashErrorcodes_NoError)
            {
                eraseSequenceMethod = EraseSequenceMethod_Sector;
                eraseBusyCount = 0;
                firstEraseTry = true;

                currentFlashOp = FlashOp_PendingOp;
                currentFlashTask = FlashTask_WriteBios;

                biosBufferSize = size;
                startingOffset = offset;
                memcpy(biosBuffer, buf, size);
                mirrorimage(flashBank);

                XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"WriteBIOS param set. flashBank=%02X    biosBufferSize=%u    startingOffset=%u.", flashBank, biosBufferSize, startingOffset);
            }
            return flashErrorCode;
        }
    }
    XBlastLogger(DBG_LVL_FATAL, DEBUG_FLASH_DRIVER,"Error! Flash FSM **NOT** idle.");

    return FlashErrorcodes_UndefinedError;
}

static FlashErrorcodes checkImageSize(unsigned int size)
{
    FlashErrorcodes returnValue = FlashErrorcodes_FileSizeError;
    if(size > 0 && size <= flashDevice.flashType.m_dwLengthInBytes)
    {
        if(size % ImageSize256KB == 0)
        {
            returnValue = FlashErrorcodes_NoError;
        }
    }

    return returnValue;
}

static void mirrorimage(FlashBank flashBank)
{
    unsigned int targetSize = ImageSize256KB;

    switch(flashBank)
    {

    case FlashBank_OSBank:
    case FlashBank_256Bank:
        // Already properly set
        break;

    case FlashBank_512Bank:
    case FlashBank_SplitTSOP0Bank:
    case FlashBank_SplitTSOP1Bank:
        targetSize = ImageSize512KB;
        break;

    case FlashBank_FullTSOPBank:
    case FlashBank_NoBank:
        targetSize = flashDevice.flashType.m_dwLengthInBytes;
        break;
    }

    while(biosBufferSize < targetSize)
    {
        memcpy(biosBuffer + biosBufferSize, biosBuffer, biosBufferSize);
        biosBufferSize += biosBufferSize;
        XBlastLogger(DBG_LVL_INFO, DEBUG_FLASH_DRIVER,"Mirroring image to fill size. NewSize=%u", biosBufferSize);
    }
}

static FlashErrorcodes validateOSImage(const unsigned char* inBuf, unsigned int size)
{
    unsigned int md5Size;
    unsigned char md5result[16];
    MD5_CTX hashcontext;
    int i;
    FlashErrorcodes exitCode = FlashErrorcodes_NoError;
    XBlastLogger(DBG_LVL_INFO, DEBUG_FLASH_DRIVER,"Validating XBlast OS image in various ways.");

    if(size != ImageSize256KB)
    {
        XBlastLogger(DBG_LVL_ERROR, DEBUG_FLASH_DRIVER,"Incorrect image file size. Aborting.");
        return FlashErrorcodes_FileSizeError;
    }

    const struct BiosIdentifier* biosID = (const struct BiosIdentifier*)(inBuf + size - sizeof(struct BiosIdentifier));

    if(strncmp(biosID->Name, PROG_NAME, strlen(PROG_NAME)))
    {
        XBlastLogger(DBG_LVL_ERROR, DEBUG_FLASH_DRIVER,"Detected device not XBlast Mod compatible. Aborting.");
        return FlashErrorcodes_InvalidUpdateFile;
    }

    //if(crc32buf(biosBuffer,XBlastOSSaveSettingsOffsetInFlash) != *(unsigned int *)&biosBuffer[0x3FDFC])
    //{
          //return FlashErrorcodes_CRCMismatch;
    //}

    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"BiosIdentifier Header v%u.", biosID->HeaderVersion);
    if(biosID->HeaderVersion == HeaderVersionV2)
    {
        md5Size = biosID->BiosSize;
    }
    else if(biosID->HeaderVersion == HeaderVersionV1)
    {
        md5Size = ImageSize256KB - FlashSectorSize_4KB;
        exitCode = FlashErrorcodes_DowngradeWarning;
    }
    else
    {
        XBlastLogger(DBG_LVL_ERROR, DEBUG_FLASH_DRIVER,"Invalid BiosIdentifier Header version. Aborting.");
        return FlashErrorcodes_InvalidUpdateFile;
    }

    MD5Init(&hashcontext);
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Calculating md5 hash on buffer size %u.", md5Size);
    MD5Update(&hashcontext, inBuf, md5Size);
    MD5Final(md5result, &hashcontext);
    for(i = 0; i < 16; i++)
    {
        if(md5result[i] != biosID->MD5Hash[i])
        {
            XBlastLogger(DBG_LVL_ERROR, DEBUG_FLASH_DRIVER,"MD5 value mismatch!");
            exitCode = FlashErrorcodes_MD5Mismatch;
            break;
        }
    }

    XBlastLogger(DBG_LVL_INFO, DEBUG_FLASH_DRIVER,"XBlast OS image appears to be valid.");

    return exitCode;
}

static void evaluateReadBackRange(void)
{
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Re-evaluating readback range.");
    // Previous read back data is obviously not covering the new range we're about to erase.
    startingOffset &= ~(getEraseMethodSize() - 1);
    biosBufferSize = getEraseMethodSize();
    currentAddr = 0;
    currentFlashOp = FlashOp_ReadInProgress;
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"startingOffset=0x%X    erase size=%u", startingOffset, biosBufferSize);
}

struct BiosIdentifier getBiosIdentifierFromFlash(void)
{
    struct BiosIdentifier out;
    unsigned char* ptr = (unsigned char *)&out;
    memset(ptr, 0xFF, sizeof(struct BiosIdentifier));
    unsigned int i;

    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Reading BIOS Identifier from flash.");
    for(i = 0; i < sizeof(struct BiosIdentifier); i++)
    {
        ptr[i] = FlashLowLevel_ReadByte(ImageSize256KB - sizeof(struct BiosIdentifier) + i);
    }

    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Complete.");
    return out;
}

static struct BiosIdentifier getBiosIdentifierFromBuffer(const unsigned char* buf, unsigned int size)
{
    struct BiosIdentifier out;
    struct BiosIdentifier* seeker;
    unsigned char* ptr = (unsigned char *)&out;
    memset(ptr, 0xFF, sizeof(struct BiosIdentifier));

    if(size == ImageSize256KB)
    {
        seeker = (struct BiosIdentifier*)(buf + size - sizeof(struct BiosIdentifier));
        if(memcmp(seeker->Name, PROG_NAME, strlen(PROG_NAME)) == 0)
        {
            memcpy(&out, seeker, sizeof(struct BiosIdentifier));
        }
    }

    return out;
}

static unsigned int getXBlastOSSettingStartingOffset(struct BiosIdentifier biosID)
{
    unsigned int settingsOffset = 0;
    char temp[33];

    memcpy(temp, biosID.Magic, 4);
    temp[4] = '\0';
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"BiosIdentifier content");
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Magic:          %s", temp);
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"HeaderVersion:  %u", biosID.HeaderVersion);
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"XboxVersion:    %u", biosID.XboxVersion);
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"VideoEncoder:   %u", biosID.VideoEncoder);
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Option1:        %u", biosID.Option1);
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Option2:        %u", biosID.Option2);
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Option3:        %u", biosID.Option3);
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"BiosSize:       %u", biosID.BiosSize);
    memcpy(temp, biosID.Name, 32);
    temp[32] = '\0';
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Name:           %s", biosID.Name);
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"MD5Hash:        %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", biosID.MD5Hash[0], biosID.MD5Hash[1], biosID.MD5Hash[2], biosID.MD5Hash[3], biosID.MD5Hash[4], biosID.MD5Hash[5], biosID.MD5Hash[6], biosID.MD5Hash[7], biosID.MD5Hash[8], biosID.MD5Hash[9], biosID.MD5Hash[10], biosID.MD5Hash[11], biosID.MD5Hash[12], biosID.MD5Hash[13], biosID.MD5Hash[14], biosID.MD5Hash[15]);

    //Settings location is calculated the same way for both HeaderVersion 1 and 2
    if(biosID.HeaderVersion == HeaderVersionV2 || biosID.HeaderVersion == HeaderVersionV1)
    {
        if(memcmp(biosID.Name, PROG_NAME, strlen(PROG_NAME)) == 0)
        {
            if(biosID.Option1 & Option1_SaveSettingsLocationBit)
            {
                settingsOffset = biosID.BiosSize;

                // No proper offset in BiosID? No setting save!
                if(settingsOffset >= ImageSize256KB)
                {
                    settingsOffset = 0;
                }
                // Never go below the 192KB mark
                // This is to make sure we're always positioned inside the last 64KB sector for Sector based flash devices.
                else if(settingsOffset < (3 * FlashBlockSize_64KB))
                {
                    settingsOffset = 3 * FlashBlockSize_64KB;
                }
                else
                {
                    settingsOffset = (settingsOffset + FlashSectorSize_4KB - 1) % ((unsigned int)(FlashSectorSize_4KB));
                }
            }
        }
    }
    //TODO: Put else statement for BiosHeader migration from previous to current version.

    XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"XBlast OS settings starting offset is %u", settingsOffset);

    return settingsOffset;
}

static unsigned int calculateSettingsStructCRC32Value(const _LPCmodSettings* in)
{
    unsigned int returnValue = crc32buf((unsigned char *)in, sizeof(_LPCmodSettings) - sizeof(_CRC32SettingsValue));

    return returnValue;
}

static void injectSettingsInBuf(unsigned int offset)
{
    // Adding Settings data to write buffer.
    if(offset != 0)
    {
        LPCmodSettings.crc32Value = calculateSettingsStructCRC32Value(&LPCmodSettings);
        XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Calculated Settings CRC32 value to write : 0x%08X", LPCmodSettings.crc32Value);
        XBlastLogger(DBG_LVL_DEBUG, DEBUG_FLASH_DRIVER,"Copying settings data (%u bytes) at offset %u in biosBuffer", sizeof(_LPCmodSettings), (offset % biosBufferSize));

        memcpy(biosBuffer + (offset % biosBufferSize), (const void*)&LPCmodSettings, sizeof(_LPCmodSettings));
    }
}

static bool canWrite(unsigned char flashByte, unsigned char bufferByte)
{
    unsigned char result = (~flashByte) & bufferByte;
    if (result != 0)
    {
        return false;
    }

    return true;
}

static unsigned int getEraseMethodSize(void)
{
    if(flashDevice.flashType.m_support4KBErase)
    {
        switch(eraseSequenceMethod)
        {
        case EraseSequenceMethod_Sector:
            return FlashSectorSize_4KB;
        case EraseSequenceMethod_Block:
            return FlashBlockSize_64KB;
        case EraseSequenceMethod_Chip:
            return FlashChipSize_256KB;
        }
    }
    else
    {
        switch(eraseSequenceMethod)
        {
        case EraseSequenceMethod_Sector:
        case EraseSequenceMethod_Block:
            return FlashBlockSize_64KB;
        case EraseSequenceMethod_Chip:
            return FlashChipSize_256KB;
        }
    }

    return FlashChipSize_256KB;
}

#ifdef DEV_FEATURES
unsigned int getBiosBufferSize(void) { return biosBufferSize; }
unsigned int getStartingOffset(void) { return startingOffset; }
unsigned int getCurrentAddr(void) { return currentAddr; }
unsigned int getEraseSequenceMethod(void) { return eraseSequenceMethod; }
bool getFirstEraseTry(void) { return firstEraseTry; }
#endif
