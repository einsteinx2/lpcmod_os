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
#include "video.h"
#include "lpcmod_v1.h"

void LPCIOWrite(void * ignored){
    u16 address = 0x00FF;
    u8 data = 0x00;
    u8 line = 0;
    bool refresh = true;
    while((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) != 1) &&
          (risefall_xpad_STATE(XPAD_STATE_BACK) != 1)){
        if(refresh){
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_ATTR=0xffffffff;
            ToolHeader("Write LPC I/O");
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
    u16 address = 0x00FF;
    u8 data = 0x00;
    u8 line = 0;
    bool refresh = true;
    while((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) != 1) &&
          (risefall_xpad_STATE(XPAD_STATE_BACK) != 1)){
        if(refresh){
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_ATTR=0xffffffff;
            ToolHeader("Read LPC I/O");
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

void GPIORead(void * ignored){

    ToolHeader("Read GPI/O");
    LPCMod_ReadIO(NULL);
    printk("\n\n\2           GPO : 0b%u%u%u%u", GenPurposeIOs.GPO3,  GenPurposeIOs.GPO2, GenPurposeIOs.GPO1, GenPurposeIOs.GPO0);
    printk("\n\n\2           GPI : 0b%u%u", GenPurposeIOs.GPI1, GenPurposeIOs.GPI0);
    printk("\n\n\2           A19Ctrl : 0b%u", GenPurposeIOs.A19BufEn);
    printk("\n\n\2           EN_5V : 0b%u", GenPurposeIOs.EN_5V);
    ToolFooter();
    return;
}

void settingsPrintData(void * ignored){
    u8 i;
    char specialCasesBuf[15];
    ToolHeader("Persistent settings");
    for(i = 0; i < NBTXTPARAMS; i++){
        if(i < IPTEXTPARAMGROUP)
            printk("\n           %s%u", xblastcfgstrings[i], *settingsPtrArray[i]);
        else if(i < TEXTPARAMGROUP)
            printk("\n           %s%u.%u.%u.%u", xblastcfgstrings[i], settingsPtrArray[i][0], settingsPtrArray[i][1], settingsPtrArray[i][2], settingsPtrArray[i][3]);
        else if(i < SPECIALPARAMGROUP)
            printk("\n           %s%s", xblastcfgstrings[i], textSettingsPtrArray[i]);
        else{
            switch(i){
                case 27:
                case 28:
                    switch(*specialCasePtrArray[i]){
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
                        default:
                            sprintf(specialCasesBuf, "Error!");
                            break;
                    }
                    printk("\n           %s%s", xblastcfgstrings[i], specialCasesBuf);
                    break;
                case 29:
                    switch(*specialCasePtrArray[i]){
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
                case 30:
                    if(*specialCasePtrArray[i] == 0)
                        printk("\n           %s%s", xblastcfgstrings[i], "HD44780");
                    else
                        printk("\n           %s%s", xblastcfgstrings[i], "Error!");
            }
        }
    }
}
