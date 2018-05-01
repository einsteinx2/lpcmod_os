/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "MenuInits.h"
#include "MenuActions.h"
#include "ToolsMenuActions.h"
#include "EepromEditMenuActions.h"
#include "NetworkMenuActions.h"
#include "HDDMenuActions.h"
#include "ResetMenuActions.h"
#include "lpcmod_v1.h"
#include "BootHddKey.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/cromwell/cromString.h"
#include "lib/cromwell/cromSystem.h"
#include "xblast/settings/xblastSettingsImportExport.h"
#include "string.h"
#include "stdio.h"
#include "menu/misc/ConfirmDialog.h"
#include "menu/misc/ProgressBar.h"
#include "Gentoox.h"
TEXTMENUITEM* saveEEPROMPtr;
TEXTMENUITEM* restoreEEPROMPtr;
TEXTMENUITEM* editEEPROMPtr;

void saveEEPromToFlash(void* ignored)
{
    int version;

    version = decryptEEPROMData((unsigned char *)&(LPCmodSettings.bakeeprom), NULL);
    if(version >= EEPROM_EncryptV1_0 && version <= EEPROM_EncryptV1_6)   //Current content in eeprom is valid.
    {
        if(ConfirmDialog("Overwrite back up EEProm content?", 1))
        {
            return;
        }
    }

    saveEEPROMPtr->nextMenuItem = restoreEEPROMPtr;
#ifdef DEV_FEATURES
    editEEPROMPtr->previousMenuItem = eraseEEPROMPtr;
#else
    editEEPROMPtr->previousMenuItem = restoreEEPROMPtr;
#endif

    memcpy(&(LPCmodSettings.bakeeprom),&eeprom,sizeof(EEPROMDATA));
    UiHeader("Back up to flash successful");

    UIFooter();
}

void restoreEEPromFromFlash(void* ignored){

    editeeprom = (EEPROMDATA *)malloc(sizeof(EEPROMDATA));

    if(updateEEPROMEditBufferFromInputBuffer((unsigned char *)&(LPCmodSettings.bakeeprom), sizeof(EEPROMDATA), false) == false)
    {
        if(replaceEEPROMContentFromBuffer(editeeprom))
        {
            UiHeader("No valid EEPROM backup on modchip.");
            printk("\n           Nothing to restore. Xbox EEPROM is unchanged.");
            UIFooter();
        }
    }

    free(editeeprom);
    editeeprom = NULL;
}

#ifdef DEV_FEATURES
void eraseEEPromFromFlash(void* ignored)
{
    memset(&LPCmodSettings.bakeeprom, 0xFF, sizeof(EEPROMDATA));

    saveEEPROMPtr->nextMenuItem = editEEPROMPtr;
    editEEPROMPtr->previousMenuItem = saveEEPROMPtr;

    UiHeader("EEPROM backup on modchip erased.");
    UIFooter();
}
#endif

void warningDisplayEepromEditMenu(void* ignored)
{
    if(ConfirmDialog("Use these tools at your own risk!", 1))
    {
            return;
    }

    editeeprom = (EEPROMDATA *)malloc(sizeof(EEPROMDATA));
    memcpy(editeeprom, &eeprom, sizeof(EEPROMDATA));   //Initial copy into edition buffer.
    dynamicDrawChildTextMenu(eepromEditMenuInit);
    free(editeeprom);
    editeeprom = NULL;
}

void wipeEEPromUserSettings(void* ignored)
{
    if(ConfirmDialog("Reset user EEProm settings(safe)?", 1))
    {
        return;
    }

    memset(eeprom.Checksum3,0xFF,4);    //Checksum3 need to be 0xFFFFFFFF
    memset(eeprom.TimeZoneBias,0x00,0x5b);    //Start from Checksum3 address in struct and write 0x00 up to UNKNOWN6.
    UiHeader("Reset user EEProm settings successful");
    UIFooter();
}

void showMemTest(void* ignored)
{
    UiHeader("128MB  RAM test");
    memtest();
    UIFooter();
}

void memtest(void)
{
    unsigned char bank = 0;

    if (xbox_ram == 64)
    {
        //Unknown why this is done but has to be executed
        //It probably has to do with video memory allocation.
        (*(unsigned int*)(0xFD000000 + 0x100200)) = 0x03070103 ;
        (*(unsigned int*)(0xFD000000 + 0x100204)) = 0x11448000 ;

        PciWriteDword(BUS_0, DEV_0, FUNC_0, 0x84, 0x7FFFFFF);  //Force 128 MB
    }

    DisplayProgressBar(0, 4, 0xffff00ff);                      //Draw ProgressBar frame.
    for(bank = 0; bank < 4; bank++)
    {
        printk("\n           Ram chip %u : %s",bank+1, testBank(bank) ? "Failed" : "Success");
        DisplayProgressBar(bank + 1, 4, 0xffff00ff);                   //Purple progress bar.
    }

    VIDEO_ATTR=0xffc8c8c8;

    if (xbox_ram == 64)     //Revert to 64MB RAM if previously set.
    {
        PciWriteDword(BUS_0, DEV_0, FUNC_0, 0x84, 0x3FFFFFF);  // 64 MB
    }
}

int testBank(int bank)
{
    unsigned int counter, subCounter, lastValue;
    unsigned int *membasetop = (unsigned int*)((64*1024*1024));
    unsigned char result=0;    //Start assuming everything is good.

    lastValue = 1;
    //Clear Upper 64MB
    for (counter= 0; counter < (64*1024*1024/4);counter+=16)
    {
        for(subCounter = 0; subCounter < 3; subCounter++)
            membasetop[counter+subCounter+bank*4] = lastValue;                         //Set it all to 0x1
    }

    while(lastValue < 0x80000000 && cromwellLoop())                                      //Test every data bit pins.
    {
        for (counter= 0; counter < (64*1024*1024/4);counter+=16)       //Test every address bit pin. 4194304 * 8 = 32MB
        {
            for(subCounter = 0; subCounter < 3; subCounter++)
            {
                if(membasetop[counter+subCounter+bank*4]!=lastValue)
                {
                    result = 1;    //1=no no
                    lastValue = 0x80000000;
                    return result;        //No need to go further. Bank is broken.
                }
                membasetop[counter+subCounter+bank*4] = lastValue<<1;        //Prepare for next read.
            }
        }
        lastValue = lastValue << 1;    //Next data bit pin.
    }
    return result;
}
/*
void TSOPRecoveryReboot(void *ignored){
    if(ConfirmDialog("       Confirm reboot in TSOP recovery mode?", 1))
        return;
    WriteToIO(XODUS_CONTROL, RELEASED0 | GROUNDA15);
    WriteToIO(XBLAST_CONTROL, BNKOS);   //Make sure A19 signal is not controlled.
    BootFlashSaveOSSettings();
    assertWriteEEPROM();
    I2CTransmitWord(0x10, 0x1b00 + ( I2CTransmitByteGetReturn(0x10, 0x1b) | 0x04 )); // set noani-bit
    I2CRebootQuick();        //Retry
    while(1);
}
*/
void saveXBlastcfg(void* fileExist)
{
    unsigned char filePresent = *(unsigned char*)fileExist;
    char tempString[50];

    if(filePresent)
    {
        sprintf(tempString, "\"%s\" exists\n\2Overwrite?", getSettingsFileLocation() + strlen("/MASTER_"));
        if(true == ConfirmDialog(tempString, 1))
        {
            return;
        }
    }

    sprintf(tempString, "Saving \"%s\"", getSettingsFileLocation() + strlen("/MASTER_"));
    UiHeader(tempString);

    if(LPCMod_SaveCFGToHDD(&settingsPtrStruct))
    {
        printk("\n           Error!");
        cromwellError();
    }
    else
    {
        printk("\n           Success.");
    }

    UIFooter();
}

void loadXBlastcfg(void* ignored)
{
    int result;
    char tempString[50];
    _LPCmodSettings tempSettings;
    if(ConfirmDialog("Restore settings from \"xblast.cfg\"?", 1))
    {
        sprintf(tempString, "Loading from \"%s\" aborted.", getSettingsFileLocation() + strlen("MASTER_"));
        UiHeader(tempString);
        result = 1;
    }
    else
    {
        result = LPCMod_ReadCFGFromHDD(&tempSettings, &settingsPtrStruct);
    }
    
    if(result == 0)
    {
        importNewSettingsFromCFGLoad(&tempSettings);
        UiHeader("Success.");
        printk("\n           Settings loaded from \"%s\".", getSettingsFileLocation() + strlen("MASTER_"));
    }
    else
    {
        UiHeader("Error!!!");
        switch(result)
        {
        default:
            break;
        case 2:
            printk("\n           Unable to open partition. Is drive formatted?");
            break;
        case 3:
            printk("\n           File \"%s\" not found.", getSettingsFileLocation() + strlen("MASTER_"));
            break;
        case 4:
            printk("\n           Unable to open \"%s\".", getSettingsFileLocation() + strlen("MASTER_"));
            break;
        }
    }
    UIFooter();
}

void nextA19controlModBootValue(void* itemPtr)
{
    switch(A19controlModBoot)
    {
        case BNKFULLTSOP:
            A19controlModBoot = BNKTSOPSPLIT0;
            strcpy(itemPtr, "Bank0");
            break;
        case BNKTSOPSPLIT0:
            A19controlModBoot = BNKTSOPSPLIT1;
            strcpy(itemPtr, "Bank1");
            break;
        case BNKTSOPSPLIT1:
        default:
            A19controlModBoot = BNKFULLTSOP;
            strcpy(itemPtr, "No");
            break;
    }
}

void prevA19controlModBootValue(void* itemPtr)
{
    switch(A19controlModBoot)
    {
        case BNKTSOPSPLIT1:
            A19controlModBoot = BNKTSOPSPLIT0;
            strcpy(itemPtr,  "Bank0");
            break;
        case BNKFULLTSOP:
            A19controlModBoot = BNKTSOPSPLIT1;
            strcpy(itemPtr,  "Bank1");
            break;
        case BNKTSOPSPLIT0:
        default:
            A19controlModBoot = BNKFULLTSOP;
            strcpy(itemPtr, "No");
            break;
    }
}

bool replaceEEPROMContentFromBuffer(EEPROMDATA* eepromPtr)
{
    unsigned char i, unlockConfirm[2];
    bool cancelChanges = false;

    for(i = 0; i < 2; i++)               //Probe 2 possible drives
    {
        if(tsaHarddiskInfo[i].m_fDriveExists && !tsaHarddiskInfo[i].m_fAtapi)  //If there's a HDD plugged on specified port
        {
            if((tsaHarddiskInfo[i].m_securitySettings &0x0002)==0x0002)        //If drive is locked
            {
                if(UnlockHDD(i, 0, (unsigned char *)&eeprom, true))            //0 is for silent
                {
                    unlockConfirm[i] = 255;  //error
                }
                else
                {
                    unlockConfirm[i] = 1; //Everything went well, we'll relock after eeprom write.
                }
            }
            else
            {
                unlockConfirm[i] = 0;   //Drive not locked, won't relock after eeprom write.
            }
        }
        else
        {
            unlockConfirm[i] = 0;       //Won't relock as no HDD was detected on that port.
        }

        XBlastLogger(DEBUG_GENERAL_UI, DBG_LVL_DEBUG, "Drive %u  lock assert result %u", i, unlockConfirm[i]);
    }

    if(unlockConfirm[0] == 255 || unlockConfirm[1] == 255)      //error in unlocking one of 2 drives.
    {
        cancelChanges = ConfirmDialog("Drive(s) still locked\n\2Continue anyway?", 1);
    }

    if(cancelChanges == false)
    {
        memcpy(&eeprom, eepromPtr, sizeof(EEPROMDATA));

        for(i = 0; i < 2; i++)               //Probe 2 possible drives
        {
            if(unlockConfirm[i] == 1)
            {
                XBlastLogger(DEBUG_GENERAL_UI, DBG_LVL_INFO, "Relocking drive %u with new HDDKey", i);
                LockHDD(i, 0, (unsigned char *)&eeprom);    //0 is for silent mode.
            }
        }
        UiHeader("Saved EEPROM image");
        printk("\n\n           Modified buffer has been saved to main EEPROM buffer.\n           Pressing \'B\' will program EEPROM chip and restart the console.\n           Pressing Power button will cancel EEPROM chip write.\n\n\n");
        UIFooter();
        SlowReboot(NULL);   //This function will take care of saving eeprom image to chip.
        while(1);
        return false;
    }
    else
    {
        UiHeader("Operation aborted");
        printk("\n\n           Error unlocking drives with previous key.");
        printk("\n           Actual EEPROM has NOT been changed.");
        printk("\n           Please Manually unlock all connected HDDs before modifying EEPROM content.");
        UIFooter();
        return false;
    }

    return true;        //Error
}
