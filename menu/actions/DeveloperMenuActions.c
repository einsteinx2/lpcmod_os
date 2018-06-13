/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "DeveloperMenuActions.h"
#include "ToolsMenuActions.h"
#include "LEDMenuActions.h"
#include "boot.h"
#include "i2c.h"
#include "video.h"
#include "lpcmod_v1.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/cromwell/cromString.h"
#include "lib/cromwell/cromSystem.h"
#include "lib/time/timeManagement.h"
#include "FatFSAccessor.h"
#include "MenuActions.h"
#include "FlashDriver.h"
#include "string.h"
#include "stdio.h"

void LPCIOWrite(void* ignored)
{
    unsigned short address = 0x00FF;
    unsigned char data = 0x00;
    unsigned char line = 0;
    bool refresh = true;

    while(cromwellLoop())
    {
        if(refresh)
        {
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_ATTR=0xffffffff;
            UiHeader("Write LPC I/O");
            if(line == 0)
            {
       	        VIDEO_ATTR=0xffffef37;    //In yellow
            }
       	    else
       	    {
       	        VIDEO_ATTR=0xffffffff;
       	    }
            printk("\n\n\2           Address : 0x%04X", address);
            if(line == 1)
            {
       	        VIDEO_ATTR=0xffffef37;    //In yellow
            }
       	    else
       	    {
       	        VIDEO_ATTR=0xffffffff;
       	    }
            printk("\n\n\2           Data    : 0x%02X", data);
            if(line == 2)
            {
       	        VIDEO_ATTR=0xffffef37;    //In yellow
            }
       	    else
       	    {
       	        VIDEO_ATTR=0xffffffff;
       	    }
            printk("\n\n\2           Send");
            
            refresh = false;
        }

        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1 || risefall_xpad_STATE(XPAD_STATE_START) == 1)
        {
            if(line == 0)
                address += 16;
            else if(line == 1)
                data += 16;
            else if(line == 2)
                WriteToIO(address, data);
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1 || risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) == 1) {
            if(line == 0)
            {
                address += 1;
            }
            else if(line == 1 || line == 2)
            {
                data += 1;
            }
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1 || risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) == 1)
        {
            if(line == 0)
            {
                address -= 1;
            }
            else if(line == 1 || line == 2)
            {
                data -= 1;
            }
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_Y))
        {
            address += 0x1000;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_X))
        {
            address -= 0x1000;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1)
        {
            if(line < 2)
            {
                line += 1;
            }
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1)
        {
            if(line > 0)
            {
            	line -= 1;
            }
            	
            refresh = true;
        }
        else if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) == 1 || risefall_xpad_STATE(XPAD_STATE_BACK) == 1)
        {
            break;
        }
    }
}

void LPCIORead(void* ignored)
{
    unsigned short address = 0x00FF;
    unsigned char data = 0x00;
    unsigned char line = 0;
    bool refresh = true;
    while(cromwellLoop())
    {
        if(refresh)
        {
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_ATTR=0xffffffff;
            UiHeader("Read LPC I/O");
            if(line == 0)
            {
       	        VIDEO_ATTR=0xffffef37;    //In yellow
            }
       	    else
       	    {
       	        VIDEO_ATTR=0xffffffff;
       	    }
            printk("\n\n\2           Address : 0x%04X", address);
            if(line == 1)
            {
       	        VIDEO_ATTR=0xffffef37;    //In yellow
            }
       	    else
       	    {
       	        VIDEO_ATTR=0xffffffff;
       	    }
            printk("\n\n\2           Send");

            VIDEO_ATTR=0xffffffff;
            printk("\n\n\n\n\n\2           Data    : 0x%02X", data);
            refresh = false;
        }

        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1 || risefall_xpad_STATE(XPAD_STATE_START) == 1)
        {
            if(line == 0)
            {
                address += 16;
            }
            else if(line == 1)
            {
                data = ReadFromIO(address);
            }
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1 || risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) == 1)
        {
            address += 1;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1 || risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) == 1)
        {
            address -= 1;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_Y))
        {
            address += 0x1000;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_X))
        {
            address -= 0x1000;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1)
        {
            if(line < 1)
            {
                line += 1;
            }
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1)
        {
            if(line > 0)
            {
            	line -= 1;
            }
            	
            refresh = true;
        }
        else if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) == 1 || risefall_xpad_STATE(XPAD_STATE_BACK) == 1)
        {
            break;
        }
    }
    return;
}

void SMBusRead(void * ignored)
{
    unsigned char address = 0x00;
    unsigned char registerAddr = 0x00;
    unsigned char byteCount = 1;
    unsigned int data[16];
    unsigned char line = 0;
    unsigned int error = ERR_SUCCESS;
    unsigned char i;
    bool refresh = true;

    memset(data, 0xff, 16);

    while(cromwellLoop())
    {
        if(refresh)
        {
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_ATTR=0xffffffff;
            UiHeader("Read SMBus");

            if(line == 0)
            {
                VIDEO_ATTR=0xffffef37;    //In yellow
            }
            else
            {
                VIDEO_ATTR=0xffffffff;
            }
            printk("\n\n\2           Address  : 0x%02X", address);

            if(line == 1)
            {
                VIDEO_ATTR=0xffffef37;    //In yellow
            }
            else
            {
                VIDEO_ATTR=0xffffffff;
            }
            printk("\n\n\2           Register  : 0x%02X", registerAddr);

            if(line == 2)
            {
                VIDEO_ATTR=0xffffef37;    //In yellow
            }
            else
            {
                VIDEO_ATTR=0xffffffff;
            }
            printk("\n\n\2           Count     : 0x%02X", byteCount);

            if(line == 3)
            {
                VIDEO_ATTR=0xffffef37;    //In yellow
            }
            else
            {
                VIDEO_ATTR=0xffffffff;
            }
            printk("\n\n\2           Press A to execute.");

            if(error == ERR_SUCCESS)
            {
                VIDEO_ATTR=0xffffffff;
                char temp[100];
                unsigned char wroteCount = 0;
                for(i = 0; i < byteCount; i++)
                {
                    wroteCount += sprintf(temp + wroteCount, "%02X ", data[i]);
                    data[i] = '\0';
                }
                printk("\n\n\n\n\n\2           Data      : \2%s", temp);
            }
            else
            {
                VIDEO_ATTR=0xffff0000;
                if(error == ERR_I2C_ERROR_BUS)
                {
                    printk("\n\n\n\n\n\2           Data    : i2c bus error!");
                }
                else
                {
                    printk("\n\n\n\n\n\2           Data    : i2c timeout error!");
                }
            }

            refresh = false;
        }
        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1 || risefall_xpad_STATE(XPAD_STATE_START) == 1)
        {
            for(i = 0; i < byteCount; i++)
            {
                error = ReadfromSMBus(address, registerAddr + i, 1, data + i);
                if(error != ERR_SUCCESS)
                {
                    break;
                }
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1 || risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) == 1)
        {
            if(line == 0)
            {
                address = (address + 0x01) % 0x7f;
            }
            else if(line == 1)
            {
                registerAddr = (registerAddr + 0x01);
            }
            else if(line == 2)
            {
                if(byteCount < 16)
                {
                    byteCount += 1;
                }
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1 || risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) == 1)
        {
            if(line == 0)
            {
                address = (address - 0x01) % 0x7f;
            }
            else if(line == 1)
            {
                registerAddr = (registerAddr - 0x01);
            }
            else if(line == 2)
            {
                if(byteCount > 1)
                {
                    byteCount = byteCount - 0x01;
                }
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_Y))
        {
            if(line == 0)
            {
                address = (address + 0x10) % 0x7f;
            }
            else if(line == 1)
            {
                registerAddr = (registerAddr + 0x10) % 0x7f;
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_X))
        {
            if(line == 0)
            {
                address = (address - 0x10) % 0x7f;
            }
            else if(line == 1)
            {
                registerAddr = (registerAddr - 0x10) % 0x7f;
            }
            else if(line == 2)
            {
                byteCount = (byteCount - 0x04) % 16;
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1)
        {
            if(line < 2)
            {
                line += 1;
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1)
        {
            if(line > 0)
            {
                line -= 1;
            }

            refresh = true;
        }
        else if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) == 1 || risefall_xpad_STATE(XPAD_STATE_BACK) == 1)
        {
            break;
        }
    }
}

void GPIORead(void * ignored){

    UiHeader("Read GPI/O");
    LPCMod_ReadIO(NULL);
    printk("\n\n\2           GPO : 0b%u%u%u%u", GenPurposeIOs.GPO3,  GenPurposeIOs.GPO2, GenPurposeIOs.GPO1, GenPurposeIOs.GPO0);
    printk("\n\n\2           GPI : 0b%u%u", GenPurposeIOs.GPI1, GenPurposeIOs.GPI0);
    printk("\n\n\2           A19Ctrl : 0b%u", GenPurposeIOs.A19BufEn);
    printk("\n\n\2           EN_5V : 0b%u", GenPurposeIOs.EN_5V);
    UIFooter();
    return;
}

void settingsPrintData(void * ignored){
    unsigned char i;
    char specialCasesBuf[15];
    VIDEO_ATTR=0xffffef37;
    printk("\n           Persistent settings\n");
    VIDEO_ATTR=0xffc8c8c8;
    
    for(i = 0; i < BoolParamGroup; i++)
    {
        if((i%2) == 0) //Pair increments
        {
            printk("\n");
        }
        printk("           %s=%u", xblastCfgStringsStruct.boolSettingsStringArray[i], *settingsPtrStruct.boolSettingsPtrArray[i]);
    }
    printk("\n");

    for(i = 0; i < NumParamGroup; i++)
    {
        if((i%2) == 0) //Pair increments
        {
            printk("\n");
        }
        printk("           %s=%u", xblastCfgStringsStruct.numSettingsStringArray[i], *settingsPtrStruct.numSettingsPtrArray[i]);
    }
    printk("\n");

    for(i = 0; i < IPParamGroup; i++)
    {
        if((i%2) == 0) //Pair increments
        {
            printk("\n");
        }
        printk("           %s=%u.%u.%u.%u", xblastCfgStringsStruct.IPsettingsStringArray[i], settingsPtrStruct.IPsettingsPtrArray[i][0], settingsPtrStruct.IPsettingsPtrArray[i][1], settingsPtrStruct.IPsettingsPtrArray[i][2], settingsPtrStruct.IPsettingsPtrArray[i][3]);
    }

    for(i = 0; i < TextParamGroup; i++)
    {
        printk("\n           %s=%s", xblastCfgStringsStruct.textSettingsStringArray[i], settingsPtrStruct.textSettingsPtrArray[i]);
    }

    for(i = 0; i< SpecialParamGroup; i++)
    {
        switch(i)
        {
            case SpecialSettingsPtrArrayIndexName_ActiveBank:
                printk("\n");
            /* Fall through */
            case SpecialSettingsPtrArrayIndexName_AltBank:
                switch(*settingsPtrStruct.specialCasePtrArray[i])
                {
                    case BNK512:
                        sprintf(specialCasesBuf, "BNK512");
                        break;
                    case BNK256:
                        sprintf(specialCasesBuf, "BNK256");
                        break;
                    case BNKTSOPSPLIT0:
                        sprintf(specialCasesBuf, "BNKTSOPSPLIT0");
                        break;
                    case BNKTSOPSPLIT1:
                        sprintf(specialCasesBuf, "BNKTSOPSPLIT1");
                        break;
                    case BNKFULLTSOP:
                        sprintf(specialCasesBuf, "BNKFULLTSOP");
                        break;
                    case BNKOS:
                        sprintf(specialCasesBuf, "BNKOS");
                        break;
                    default:
                        sprintf(specialCasesBuf, "Error!");
                        break;
                }
                printk("           %s=%s", xblastCfgStringsStruct.specialSettingsStringArray[i], specialCasesBuf);
                break;
            case SpecialSettingsPtrArrayIndexName_LEDColor:
                switch(*settingsPtrStruct.specialCasePtrArray[i])
                {
                    case LED_OFF:
                        sprintf(specialCasesBuf, "LED_OFF");
                        break;
                    case LED_GREEN:
                        sprintf(specialCasesBuf, "LED_GREEN");
                        break;
                    case LED_RED:
                        sprintf(specialCasesBuf, "LED_RED");
                        break;
                    case LED_ORANGE:
                        sprintf(specialCasesBuf, "LED_ORANGE");
                        break;
                    case LED_CYCLE:
                        sprintf(specialCasesBuf, "LED_CYCLE");
                        break;
                    default:
                        sprintf(specialCasesBuf, "LED_FIRSTBOOT?");
                        break;
                }
                printk("\n           %s=%s", xblastCfgStringsStruct.specialSettingsStringArray[i], specialCasesBuf);
                break;
            case SpecialSettingsPtrArrayIndexName_LCDType:
                if(*settingsPtrStruct.specialCasePtrArray[i] == LCDTYPE_HD44780)
                    printk("\n           %s=%s", xblastCfgStringsStruct.specialSettingsStringArray[i], "HD44780");
                else if(*settingsPtrStruct.specialCasePtrArray[i] == LCDTYPE_KS0073)
                    printk("\n           %s=%s", xblastCfgStringsStruct.specialSettingsStringArray[i], "KS0073");
                else
                    printk("\n           %s=%s", xblastCfgStringsStruct.specialSettingsStringArray[i], "Error!");
            break;
        }
    }

    UIFooter();
}

void printBiosIdentifier(void * ignored)
{
    unsigned char i;
    UiHeader("Print BIOS Identifier");

    struct BiosIdentifier biosID = getBiosIdentifierFromFlash();

    printk("\n\n           Magic : %s", biosID.Magic);
    printk("\n           Header Version : %u", biosID.HeaderVersion);
    printk("\n           Xbox Version : %02X", biosID.XboxVersion);
    printk("\n           Xbox Version : %02X", biosID.VideoEncoder);
    printk("\n           Header Patch : %02X", biosID.HeaderPatch);
    printk("\n           Option1 : %02X", biosID.Option1);
    printk("\n           Option2 : %02X", biosID.Option2);
    printk("\n           Option3 : %02X", biosID.Option3);
    printk("\n           Bios Size : %u", biosID.BiosSize);
    printk("\n           Bios Size : %s", biosID.Name);
    printk("\n           MD5 :");
    for(i = 0; i < 16; i++)
    {
        printk(" %02X", biosID.MD5Hash[i]);
    }

    UIFooter();
}

void WriteToYDrive(void * drive)
{
#define MaxTestFileSizeInMb 700
    unsigned short sizeInMB = 10;
    unsigned char dataByteValue = 0x00;
    unsigned char line = 0;
    unsigned int i;
    unsigned int startTime, endTime;
    unsigned char repeatingBuf[1024 * 1024]; // 1MB
#define fileName "XBlastTestFile.bin"
    char fullPath[50];
    const char* drivename;
    bool refresh = true;
#define ErrorSucess 1
#define ErrorFail   2
    unsigned int error = 0;
    unsigned char nDriveIndex = *(unsigned char *)drive;
    FILEX handle;

    while(cromwellLoop())
    {
        if(refresh)
        {
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_ATTR=0xffffffff;
            UiHeader("Write To Y:");
            if(line == 0)
            {
                VIDEO_ATTR=0xffffef37;    //In yellow
            }
            else
            {
                VIDEO_ATTR=0xffffffff;
            }
            printk("\n\n\2           Size   : %uMB", sizeInMB);
            if(line == 1)
            {
                VIDEO_ATTR=0xffffef37;    //In yellow
            }
            else
            {
                VIDEO_ATTR=0xffffffff;
            }
            printk("\n\n\2           Data seed : 0x%02X", dataByteValue);
            if(line == 2)
            {
                VIDEO_ATTR=0xffffef37;    //In yellow
            }
            else
            {
                VIDEO_ATTR=0xffffffff;
            }
            printk("\n\n\2           Send");

            if(error == ErrorSucess)
            {
                VIDEO_ATTR=0xffffffff;

                printk("\n\n\n\n\n\2           Speed     : \2%fMB/s", (float)sizeInMB / ((float)endTime / 1000.0));
            }
            else if(error == ErrorFail)
            {
                VIDEO_ATTR=0xffff0000;
                printk("\n\n\n\n\n\2           Operation fail");
            }

            refresh = false;
        }

        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1 || risefall_xpad_STATE(XPAD_STATE_START) == 1)
        {
            if(line == 0)
            {
                if(sizeInMB + 10 <= MaxTestFileSizeInMb)
                {
                    sizeInMB += 10;
                }
            }
            else if(line == 1)
            {
                dataByteValue += 16;
            }
            else if(line == 2)
            {
                for(i = 0; i < 1024 * 1024; i++)
                {
                    repeatingBuf[i] = dataByteValue + i;
                }
                if(-1 != fatxgetActivePartName((nDriveIndex * NbFATXPartPerHDD) + Part_X, &drivename))
                {
                    sprintf(fullPath, PathSep"%s"PathSep fileName, drivename);
                    fatxdelete(fullPath);
                    handle = fatxopen(fullPath, FileOpenMode_CreateAlways | FileOpenMode_Write);
                    startTime = getMS();
                    if(handle)
                    {
                        for(i = 0; i < sizeInMB; i++)
                        {
                            if(-1 == fatxwrite(handle, repeatingBuf, 1024 * 1024))
                            {
                                // Error
                                error = ErrorFail;
                                break;
                            }
                        }
                        fatxclose(handle);
                        error = ErrorSucess;
                    }
                    else
                    {
                        error = ErrorFail;
                    }
                    endTime = getElapsedTimeSince(startTime);
                }
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1 || risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) == 1) {
            if(line == 0)
            {
                if(sizeInMB < MaxTestFileSizeInMb)
                {
                    sizeInMB += 1;
                }
            }
            else if(line == 1 || line == 2)
            {
                dataByteValue += 1;
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1 || risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) == 1)
        {
            if(line == 0)
            {
                if(sizeInMB > 1)
                {
                    sizeInMB -= 1;
                }
            }
            else if(line == 1 || line == 2)
            {
                dataByteValue -= 1;
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_Y))
        {
            if(sizeInMB + 100 <= MaxTestFileSizeInMb)
            {
                sizeInMB += 100;
            }
            else
            {
                sizeInMB = MaxTestFileSizeInMb;
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_X))
        {
            if(sizeInMB < 101)
            {
                sizeInMB = 1;
            }
            else
            {
                sizeInMB -= 100;
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1)
        {
            if(line < 2)
            {
                line += 1;
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1)
        {
            if(line > 0)
            {
                line -= 1;
            }

            refresh = true;
        }
        else if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) == 1 || risefall_xpad_STATE(XPAD_STATE_BACK) == 1)
        {
            break;
        }
    }
}
