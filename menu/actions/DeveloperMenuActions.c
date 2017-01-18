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
#include "lib/time/timeManagement.h"
#include "MenuActions.h"
#include "string.h"

void LPCIOWrite(void * ignored){
    unsigned short address = 0x00FF;
    unsigned char data = 0x00;
    unsigned char line = 0;
    bool refresh = true;
    while((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) != 1) &&
          (risefall_xpad_STATE(XPAD_STATE_BACK) != 1)){
        if(refresh){
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_ATTR=0xffffffff;
            UiHeader("Write LPC I/O");
            if(line == 0)
       	        VIDEO_ATTR=0xffffef37;    //In yellow
       	    else
       	        VIDEO_ATTR=0xffffffff;
            printk("\n\n\2           Address : 0x%04X", address);
            if(line == 1)
       	        VIDEO_ATTR=0xffffef37;    //In yellow
       	    else
       	        VIDEO_ATTR=0xffffffff;
            printk("\n\n\2           Data    : 0x%02X", data);
            if(line == 2)
       	        VIDEO_ATTR=0xffffef37;    //In yellow
       	    else
       	        VIDEO_ATTR=0xffffffff;
            printk("\n\n\2           Send");
            
            refresh = false;
        }
        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1 || risefall_xpad_STATE(XPAD_STATE_START) == 1){
            if(line == 0)
                address += 16;
            else if(line == 1)
                data += 16;
            else if(line == 2)
                WriteToIO(address, data);
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1 ||
                 risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) == 1) {
            if(line == 0)
                address += 1;
            else if(line == 1 || line == 2)
                data += 1;
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1 ||
                 risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) == 1) {
            if(line == 0)
                address -= 1;
            else if(line == 1 || line == 2)
                data -= 1;
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_Y)) {
            address += 0x1000;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_X)) {
            address -= 0x1000;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1) {
            if(line < 2)
                line += 1;
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1) {
            if(line > 0)
            	line -= 1;
            	
            refresh = true;
        }
    }
    return;
}

void LPCIORead(void * ignored){
    unsigned short address = 0x00FF;
    unsigned char data = 0x00;
    unsigned char line = 0;
    bool refresh = true;
    while((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) != 1) &&
          (risefall_xpad_STATE(XPAD_STATE_BACK) != 1)){
        if(refresh){
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_ATTR=0xffffffff;
            UiHeader("Read LPC I/O");
            if(line == 0)
       	        VIDEO_ATTR=0xffffef37;    //In yellow
       	    else
       	        VIDEO_ATTR=0xffffffff;
            printk("\n\n\2           Address : 0x%04X", address);
            if(line == 1)
       	        VIDEO_ATTR=0xffffef37;    //In yellow
       	    else
       	        VIDEO_ATTR=0xffffffff;
            printk("\n\n\2           Send");
        

 	    VIDEO_ATTR=0xffffffff;
            printk("\n\n\n\n\n\2           Data    : 0x%02X", data);
            refresh = false;
        }
        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1 || risefall_xpad_STATE(XPAD_STATE_START) == 1){
            if(line == 0)
                address += 16;
            else if(line == 1)
                data = ReadFromIO(address);
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1 ||
                 risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) == 1) {
            address += 1;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1 ||
                 risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) == 1) {
            address -= 1;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_Y)) {
            address += 0x1000;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_X)) {
            address -= 0x1000;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1) {
            if(line < 1)
                line += 1;
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1) {
            if(line > 0)
            	line -= 1;
            	
            refresh = true;
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
    bool refresh = true;

    memset(data, 0xff, 16);

    while((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) != 1) &&
          (risefall_xpad_STATE(XPAD_STATE_BACK) != 1))
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
                for(unsigned char i = 0; i < byteCount; i++)
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
            for(unsigned char i = 0; i < byteCount; i++)
            {
                error = ReadfromSMBus(address, registerAddr + i, 1, data + i);
                if(error != ERR_SUCCESS)
                {
                    break;
                }
            }

            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1 ||
                 risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) == 1)
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
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1 ||
                 risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) == 1)
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
    
    for(i = 0; i < NBTXTPARAMS; i++){
        if(i < IPTEXTPARAMGROUP){
            if((i%2) == 0){ //Pair increments
                printk("\n");
            }
                printk("           %s%u", xblastcfgstrings[i], *settingsPtrStruct.settingsPtrArray[i]);
        }
        else if(i < TEXTPARAMGROUP)
        {
            if((i%2) == 0)
            {
                printk("\n");
            }
            printk("           %s%u.%u.%u.%u", xblastcfgstrings[i], settingsPtrStruct.IPsettingsPtrArray[i-IPTEXTPARAMGROUP][0], settingsPtrStruct.IPsettingsPtrArray[i-IPTEXTPARAMGROUP][1], settingsPtrStruct.IPsettingsPtrArray[i-IPTEXTPARAMGROUP][2], settingsPtrStruct.IPsettingsPtrArray[i-IPTEXTPARAMGROUP][3]);
        }
        else if(i < SPECIALPARAMGROUP)
            printk("\n           %s%s", xblastcfgstrings[i],  settingsPtrStruct.textSettingsPtrArray[i-TEXTPARAMGROUP]);
        else{
            switch(i){
                case (SPECIALPARAMGROUP):
                    printk("\n");
                /* Fall through */
                case (SPECIALPARAMGROUP + 1):
                    switch(*settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP]){
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
                    printk("           %s%s", xblastcfgstrings[i], specialCasesBuf);
                    break;
                case (SPECIALPARAMGROUP + 2):
                    switch(*settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP]){
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
                    printk("\n           %s%s", xblastcfgstrings[i], specialCasesBuf);
                    break;
                case (SPECIALPARAMGROUP + 3):
                    if(*settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP] == LCDTYPE_HD44780)
                        printk("\n           %s%s", xblastcfgstrings[i], "HD44780");
                    else if(*settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP] == LCDTYPE_KS0073)
                        printk("\n           %s%s", xblastcfgstrings[i], "KS0073");
                    else
                        printk("\n           %s%s", xblastcfgstrings[i], "Error!");
                break;
            }
        }
    }
    UIFooter();
}
