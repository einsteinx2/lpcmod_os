/*
 * IdeConfig.c
 *
 *  Created on: Apr 7, 2018
 *      Author: cromwelldev
 */

#include "IdeDriver.h"
#include "IdeHelpers.h"
#include "string.h"
#include "boot.h"
#include "xblast/HardwareIdentifier.h"
#include "BootHddKey.h"
#include "lib/cromwell/cromString.h"

static int BootIdeSendSetFeatures(int nIndexDrive, unsigned char featureSelect, unsigned short valueInSectorCount);

static void invalidateDeviceInfoStruct(unsigned char deviceIndex)
{
    memset(&tsaHarddiskInfo[deviceIndex], 0x00, sizeof(struct tsHarddiskInfo));
    tsaHarddiskInfo[deviceIndex].m_fwPortBase = IDE_BASE1;
    tsaHarddiskInfo[deviceIndex].m_bCableConductors = 40;
}

static unsigned char convertUltraDMAMode(UltraDMAMode inMode)
{
    unsigned char result = 0;
    if(0 != inMode)
    {
        while(0 == (inMode & 0x01))
        {
            inMode >>= 1;
            result++;
        }
    }

    return result;
}


void IdeDriver_Init(void)
{
    unsigned char i;
    int result;
    UltraDMAMode targetMode;

    // Disable IRQ
    // TODO: reset IDE controller if not from XBE
    //BootIdeSendSoftReset();
    IoOutputByte(IDE_REG_CONTROL(IDE_BASE1), ATA_CTRL_nIEN); // kill interrupt, only bit1 needs to be set
    // Why disable DMA?
    IoOutputByte(IDE_REG_FEATURE(IDE_BASE1), 0x00); // kill DMA

    for(i = 0; 1 >= i; i++)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "IDE init for drive %d", i);
        invalidateDeviceInfoStruct(i);
        result = BootIdeSendIdentifyDevice(i);

        if(0 == result)
        {
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "ATA%s device at position %d", IdeDriver_DeviceIsATAPI(i) ? "PI" : "", i);
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "hd%c:", i+'a');
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "    %s",tsaHarddiskInfo[i].m_szIdentityModelNumber);
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "    %s", tsaHarddiskInfo[i].m_szIdentityModelNumber);
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "    %s", tsaHarddiskInfo[i].m_szSerial);
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "    %s", tsaHarddiskInfo[i].m_szFirmware);
            // Additional init if from coldboot
            if(IdeDriver_DeviceIsATAPI(i))
            {
                if(0 == isXBE())
                {
                    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_DEBUG, "Reset ATAPI device specific requirement from cold boot.");
                    // this is the only way to clear the ATAPI ''I have been reset'' error indication
                    unsigned char ba[128];
                    ba[2]=0x06;
                    while (ba[2]==0x06)
                    {
                        // while bitching that it 'needs attention', give it REQUEST SENSE
                        int nPacketLength=IdeDriver_AtapiAdditionalSenseCode(i, ba, sizeof(ba));
                        if(nPacketLength<12)
                        {
                            ba[2]=0;
                        }
                    }
                }
            }
            else
            {
                // Only support ATA device in case not ATAPI
                if(IdeDriver_DeviceIsLocked(i)) //Drive in locked status
                {
                    char userPassword[21];
                    CalculateDrivePassword(i, userPassword, (char *)&eeprom);
                    userPassword[20] = '\0';
                    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "Drive requires SECURITY_UNLOCK");
                    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "Calculated password: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                      userPassword[0], userPassword[1], userPassword[2], userPassword[3], userPassword[4], userPassword[5], userPassword[6], userPassword[7],
                      userPassword[8], userPassword[9], userPassword[10], userPassword[11], userPassword[12], userPassword[13], userPassword[14], userPassword[15],
                      userPassword[16], userPassword[17], userPassword[18], userPassword[19]);
                    IdeDriver_SecurityUnlock(i, userPassword, false);
                }
                if(BootIdeEnableWriteCache(i, true))
                {
                    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_WARN, "Failed enabling drive%u write cache", i);
                }
            }
            if(tsaHarddiskInfo[i].m_bDMASupported)
            {
                targetMode = tsaHarddiskInfo[i].m_UltraDMAmodeSupported;
                XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "Device %u support UltraDMA Mode %u & below", i, convertUltraDMAMode(targetMode));
                if(tsaHarddiskInfo[0].m_bCableConductors != 80 && UltraDMAMode2 < targetMode)
                {
                    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "Not 80 conductors cable. Limiting to UltraDMA Mode 2");
                    targetMode = UltraDMAMode2;
                }
                /* As per ACS-2 doc, check table 55 for bit config. */
                if(BootIdeSetTransferMode(i, targetMode))
                {
                    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_WARN, "Failed setting drive%u UltraDMA mode %u", i, convertUltraDMAMode(UltraDMAMode2));
                }
            }
            else
            {
                XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_WARN, "Device %d does not support DMA", i);
            }
        }
        else
        {
            XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "No device at position %d", i);
        }
    }
}

int BootIdeSetMultimodeSectors(unsigned char nIndexDrive, unsigned char nbSectors)
{
    sendControlATACommand(nIndexDrive, IDE_CMD_SET_MULTIPLE_MODE, NoStartLBA, NoFeatureField, nbSectors);

    //check if it worked.
    BootIdeSendIdentifyDevice(nIndexDrive);
    return tsaHarddiskInfo[nIndexDrive].m_bCurrentBlockPerDRQValid;  //No error if >0
}

int BootIdeEnableWriteCache(int nIndexDrive, bool enable)
{
    return BootIdeSendSetFeatures(nIndexDrive, enable ? 0x02 : 0x82, NoSectorCount);
}

int BootIdeSetTransferMode(int nIndexDrive, UltraDMAMode nMode)
{
    unsigned char mode;

    if(0 == nMode)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_FATAL, "Invalid DMA mode request: %u", nMode);
        return 1;
    }
    tsaHarddiskInfo[nIndexDrive].m_fUseDMA = 0;
    mode = convertUltraDMAMode(nMode);


    XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_INFO, "Trying to set IDE to UltraDMA Mode %u", mode);
    //TODO: make it more robust, check back if settings stuck
    IoOutputByte(DMA_BUSMASTER_BASE+2, 0x62); // DMA possible for both drives

    //IoOutputByte(IDE_REG_CONTROL(tsaHarddiskInfo[nIndexDrive].m_fwPortBase), 0x00); // enable interrupt
    IoOutputByte(IDE_REG_FEATURE(tsaHarddiskInfo[nIndexDrive].m_fwPortBase), 0x01); // enable DMA
#define ULTRA_DMA_MODE 0x40u
    if(BootIdeSendSetFeatures(nIndexDrive, 0x03, ULTRA_DMA_MODE | mode))
    {
        return 1;
    }

    if(BootIdeSendIdentifyDevice(nIndexDrive))
    {
        return 1;
    }

    if(tsaHarddiskInfo[nIndexDrive].m_UltraDMAmodeSelected != nMode)
    {
        XBlastLogger(DEBUG_IDE_DRIVER, DBG_LVL_ERROR, "Error setting UltraDMA Mode %u", mode);
        return 1;
    }

    tsaHarddiskInfo[nIndexDrive].m_fUseDMA = 1;

    return 0;
}

static int BootIdeSendSetFeatures(int nIndexDrive, unsigned char featureSelect, unsigned short valueInSectorCount)
{
    //TODO: make it more robust, check back if settings stuck
    IoOutputByte(IDE_REG_DRIVEHEAD(tsaHarddiskInfo[nIndexDrive].m_fwPortBase), IDE_DH_DEFAULT| IDE_DH_DRIVE(nIndexDrive));

    if(sendControlATACommand(nIndexDrive, IDE_CMD_SET_FEATURES, NoStartLBA, featureSelect, valueInSectorCount))
    {
        return 1;
    }
    return BootIdeSendIdentifyDevice(nIndexDrive);
}
