/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ToolsMenuActions.h"
#include "lpcmod_v1.h"
#include "boot.h"
#include "video.h"


void saveEEPromToFlash(void *whatever){
    u8 i;
    u8 emptyCount = 0;
    for(i = 0; i < 4; i++) {    //Checksum2 is 4 bytes long.
        if(LPCmodSettings.bakeeprom.Checksum2[i] == 0xFF)
            emptyCount++;
    }
        if(emptyCount < 4)            //Make sure checksum2 is not 0xFFFFFFFF.
            if(ConfirmDialog("       Overwrite back up EEProm content?", 1))
                return;
    memcpy(&(LPCmodSettings.bakeeprom),&eeprom,sizeof(EEPROMDATA));
    ToolHeader("Back up to flash successful");
    ToolFooter();
}

void restoreEEPromFromFlash(void *whatever){
    u8 i;
    u8 emptyCount = 0;
    for(i = 0; i < 4; i++) {    //Checksum2 is 4 bytes long.
        if(LPCmodSettings.bakeeprom.Checksum2[i] == 0xFF)
            emptyCount++;
    }
    if(emptyCount < 4){            //Make sure checksum2 is not 0xFFFFFFFF.
                        //It is practically impossible to get such value in this checksum field.
        if(ConfirmDialog("       Restore backed up EEProm content?", 1))
            return;
        memcpy(&eeprom,&(LPCmodSettings.bakeeprom),sizeof(EEPROMDATA));
        ToolHeader("Restored back up to Xbox");
    }
    else {
        ToolHeader("ERROR: No back up data on modchip");
    }
    ToolFooter();
}

void wipeEEPromUserSettings(void *whatever){
    if(ConfirmDialog("        Reset user EEProm settings(safe)?", 1))
        return;
    memset(eeprom.Checksum3,0xFF,4);    //Checksum3 need to be 0xFFFFFFFF
    memset(eeprom.TimeZoneBias,0x00,0x5b);    //Start from Checksum3 address in struct and write 0x00 up to UNKNOWN6.
    ToolHeader("Reset user EEProm settings succesful");
    ToolFooter();
}

void showMemTest(void *whatever){
    ToolHeader("128MB  RAM test");
    memtest();
    ToolFooter();
}

void memtest(void){
    u8 bank = 0;
    char Bank1Text[20];
    char Bank2Text[20];
    char Bank3Text[20];
    char Bank4Text[20];
    char *BankText[4] = {Bank1Text, Bank2Text, Bank3Text, Bank4Text};
/*
    strcpy(Bank1Text,"Untested");
    strcpy(Bank2Text,"Untested");
    strcpy(Bank3Text,"Untested");
    strcpy(Bank4Text,"Untested");
*/
    if (xbox_ram == 64){
        //Unknown why this is done but has to be executed
        //It probably has to do with video memory allocation.
        (*(unsigned int*)(0xFD000000 + 0x100200)) = 0x03070103 ;
        (*(unsigned int*)(0xFD000000 + 0x100204)) = 0x11448000 ;

        PciWriteDword(BUS_0, DEV_0, FUNC_0, 0x84, 0x7FFFFFF);  //Force 128 MB
    }
    DisplayProgressBar(0, 4, 0xffff00ff);                      //Draw ProgressBar frame.
    for(bank = 0; bank < 4; bank++)    {
        sprintf(BankText[bank], "%s", testBank(bank)? "Failed" : "Success");
        DisplayProgressBar(bank + 1, 4, 0xffff00ff);                   //Purple progress bar.
    }
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n           Bank1 : %s",Bank1Text);
    printk("\n           Bank2 : %s",Bank2Text);
    printk("\n           Bank3 : %s",Bank3Text);
    printk("\n           Bank4 : %s",Bank4Text);
    if (xbox_ram == 64) {    //Revert to 64MB RAM if previously set.
        PciWriteDword(BUS_0, DEV_0, FUNC_0, 0x84, 0x3FFFFFF);  // 64 MB
    }
    return;
}

void ToolFooter(void) {
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n\n           Press Button 'A' to continue.");
    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}

void ToolHeader(char *title) {
    printk("\n\n\n\n\n           ");
    VIDEO_ATTR=0xffffef37;
    printk("\2%s\2\n\n\n\n           ", title);
    VIDEO_ATTR=0xffc8c8c8;
}

int testBank(int bank){
    u32 counter, lastValue;
    u32 *membasetop = (u32*)((64*1024*1024));
    u8 result=0;    //Start assuming everything is good.

    lastValue = 1;
    //Clear Upper 64MB
    for (counter= 0; counter < (64*1024*1024/(4*4));counter++) {
        membasetop[counter*4+bank] = lastValue;                         //Set it all to 0x1
    }

    while(lastValue < 0x80000000){                                      //Test every data bit pins.
        for (counter= 0; counter< (64*1024*1024/(4*4));counter++) {     //Test every address bit pin
            if(membasetop[counter*4+bank]!=lastValue){
                result = 1;    //1=no no
                lastValue = 0x80000000;
                return result;        //No need to go further. Bank is broken.
            }
            membasetop[counter*4+bank] = lastValue<<1;                  //Prepare for next read.
        }
        lastValue = lastValue << 1;                                     //Next data bit pin.
    }
    return result;
}
