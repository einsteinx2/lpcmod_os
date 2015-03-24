/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ToolsMenuActions.h"
#include "LEDMenuActions.h"
#include "NetworkMenuActions.h"
#include "lpcmod_v1.h"
#include "boot.h"
#include "BootIde.h"
#include "video.h"
#include "BootFATX.h"

bool replaceEEPROMContentFromBuffer(EEPROMDATA * eepromPtr);

void saveEEPromToFlash(void *whatever){
    u8 unusedBuf[0x30];
    int version;

    version = decryptEEPROMData((u8 *)&(LPCmodSettings.bakeeprom), unusedBuf);
    if(version >= V1_0 && version <= V1_6){   //Current content in eeprom is valid.
        if(ConfirmDialog("       Overwrite back up EEProm content?", 1)){
            return;
        }
    }
    memcpy(&(LPCmodSettings.bakeeprom),&eeprom,sizeof(EEPROMDATA));
    ToolHeader("Back up to flash successful");
    UIFooter();
}

void restoreEEPromFromFlash(void *whatever){

    editeeprom = (EEPROMDATA *)malloc(sizeof(EEPROMDATA));
    if(!updateEEPROMEditBufferFromInputBuffer(&(LPCmodSettings.bakeeprom), sizeof(EEPROMDATA), false)){
        if(replaceEEPROMContentFromBuffer(editeeprom)){
            ToolHeader("No valid EEPROM backup on modchip.");
            printk("\n           Nothing to restore. Xbox EEPROM is unchanged.");
            UIFooter();
        }
    }

    free(editeeprom);
    editeeprom = NULL;
}

void warningDisplayEepromEditMenu(void *ignored){
    if(ConfirmDialog("         Use these tools at your own risk!", 1))
            return;
    editeeprom = (EEPROMDATA *)malloc(sizeof(EEPROMDATA));
    memcpy(editeeprom, &eeprom, sizeof(EEPROMDATA));   //Initial copy into edition buffer.
    ResetDrawChildTextMenu(eepromEditMenuInit());
    free(editeeprom);
    editeeprom = NULL;
}

void wipeEEPromUserSettings(void *whatever){
    if(ConfirmDialog("        Reset user EEProm settings(safe)?", 1))
        return;
    memset(eeprom.Checksum3,0xFF,4);    //Checksum3 need to be 0xFFFFFFFF
    memset(eeprom.TimeZoneBias,0x00,0x5b);    //Start from Checksum3 address in struct and write 0x00 up to UNKNOWN6.
    ToolHeader("Reset user EEProm settings successful");
    UIFooter();
}

void showMemTest(void *whatever){
    ToolHeader("128MB  RAM test");
    memtest();
    UIFooter();
}

void memtest(void){
    u8 bank = 0;
//    char Bank1Text[20];
//    char Bank2Text[20];
//    char Bank3Text[20];
//    char Bank4Text[20];
//    char *BankText[4] = {Bank1Text, Bank2Text, Bank3Text, Bank4Text};

//    strcpy(Bank1Text,"Untested");
//    strcpy(Bank2Text,"Untested");
//    strcpy(Bank3Text,"Untested");
//    strcpy(Bank4Text,"Untested");

    if (xbox_ram == 64){
        //Unknown why this is done but has to be executed
        //It probably has to do with video memory allocation.
        (*(unsigned int*)(0xFD000000 + 0x100200)) = 0x03070103 ;
        (*(unsigned int*)(0xFD000000 + 0x100204)) = 0x11448000 ;

        PciWriteDword(BUS_0, DEV_0, FUNC_0, 0x84, 0x7FFFFFF);  //Force 128 MB
    }
    DisplayProgressBar(0, 4, 0xffff00ff);                      //Draw ProgressBar frame.
    for(bank = 0; bank < 4; bank++)    {
//        sprintf(BankText[bank], "%s", testBank(bank)? "Failed" : "Success");
        printk("\n           Ram chip %u : %s",bank+1, testBank(bank)? "Failed" : "Success");
        DisplayProgressBar(bank + 1, 4, 0xffff00ff);                   //Purple progress bar.
    }
//    sprintf(BankText[1], "%s", testBank(1)? "Failed" : "Success");
    VIDEO_ATTR=0xffc8c8c8;
//    printk("\n           Ram chip 1 : %s",Bank1Text);
//    printk("\n           Ram chip 2 : %s",Bank2Text);
//    printk("\n           Ram chip 3 : %s",Bank3Text);
//    printk("\n           Ram chip 4 : %s",Bank4Text);
    if (xbox_ram == 64) {    //Revert to 64MB RAM if previously set.
        PciWriteDword(BUS_0, DEV_0, FUNC_0, 0x84, 0x3FFFFFF);  // 64 MB
    }
    return;
}


void ToolHeader(char *title) {
    printk("\n\n\n\n\n");
    VIDEO_ATTR=0xffffef37;
    printk("\2%s\2\n\n\n\n           ", title);
    VIDEO_ATTR=0xffc8c8c8;
}

int testBank(int bank){
    u32 counter, subCounter, lastValue;
    u32 *membasetop = (u32*)((64*1024*1024));
//    u32 startBad = 0, stopBad = 0;
    u8 result=0;    //Start assuming everything is good.

    lastValue = 1;
    //Clear Upper 64MB
    for (counter= 0; counter < (64*1024*1024/4);counter+=16) {
        for(subCounter = 0; subCounter < 3; subCounter++)
            membasetop[counter+subCounter+bank*4] = lastValue;                         //Set it all to 0x1
    }

    while(lastValue < 0x80000000){                                      //Test every data bit pins.
        for (counter= 0; counter < (64*1024*1024/4);counter+=16) {     //Test every address bit pin. 4194304 * 8 = 32MB
            for(subCounter = 0; subCounter < 3; subCounter++){
            if(membasetop[counter+subCounter+bank*4]!=lastValue){
                result = 1;    //1=no no
//                if(startBad == 0){
//                    startBad = counter+subCounter+bank*4;
//                    printk("\n           StartBad = 0x%08X , ",startBad);
//                }
                lastValue = 0x80000000;
                return result;        //No need to go further. Bank is broken.
            }
//            else{
//                if(startBad){
//                    startBad = 0;
//                    stopBad = counter+subCounter+bank*4 - 1;
//                    printk("StopBad = 0x%08X , ",stopBad);
//                }
//            }
            membasetop[counter+subCounter+bank*4] = lastValue<<1;                  //Prepare for next read.
        }
        }
        lastValue = lastValue << 1;                                     //Next data bit pin.
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
void saveXBlastcfg(void * ignored){
    LPCMod_SaveCFGToHDD();
}

void loadXBlastcfg(void * ignored){
    int result;
    if(ConfirmDialog("        Restore settings from \"xblast.cfg\"?", 1)){
        ToolHeader("Loading from C:\\xblast.cfg aborted.");
        result = 1;
    }
    else{
        result = LPCMod_ReadCFGFromHDD(&LPCmodSettings);
    }
    
    if(!result){
        ToolHeader("Success.");
        printk("\n           Settings loaded from \"C:\\XBlast\\xblast.cfg\".");
    }
    else{
        ToolHeader("Error!!!");
        switch(result){
            default:
                break;
            case 2:
                printk("\n           Unable to open partition. Is drive formatted?");
                break;
            case 3:
                printk("\n           File \"C:\\XBlast\\xblast.cfg\" not found.");
                break;
            case 4:
                printk("\n           Unable to open \"C:\\XBlast\\xblast.cfg\".");
                break;
        }
    }
    UIFooter();
}

void nextA19controlModBootValue(void * itemPtr){
    switch(A19controlModBoot){
        case BNKFULLTSOP:
            A19controlModBoot = BNKTSOPSPLIT0;
            sprintf(itemPtr, "%s", "Bank0");
            break;
        case BNKTSOPSPLIT0:
            A19controlModBoot = BNKTSOPSPLIT1;
            sprintf(itemPtr, "%s", "Bank1");
            break;
        case BNKTSOPSPLIT1:
        default:
            A19controlModBoot = BNKFULLTSOP;
            sprintf(itemPtr, "%s", "No");
            break;
    }
}

void prevA19controlModBootValue(void * itemPtr){
    switch(A19controlModBoot){
        case BNKTSOPSPLIT1:
            A19controlModBoot = BNKTSOPSPLIT0;
            sprintf(itemPtr, "%s", "Bank0");
            break;
        case BNKFULLTSOP:
            A19controlModBoot = BNKTSOPSPLIT1;
            sprintf(itemPtr, "%s", "Bank1");
            break;
        case BNKTSOPSPLIT0:
        default:
            A19controlModBoot = BNKFULLTSOP;
            sprintf(itemPtr, "%s", "No");
            break;
    }
}

bool replaceEEPROMContentFromBuffer(EEPROMDATA * eepromPtr){
    u8 i, eepromVersion = 0, unlockConfirm[2];
    bool cancelChanges = false;
    char unused[20];

    eepromVersion = BootHddKeyGenerateEepromKeyData(eepromPtr, unused);
    if(eepromVersion >= V1_0 && eepromVersion <= V1_6){            //Make sure eeprom is properly decrypted.
        for(i = 0; i < 2; i++){               //Probe 2 possible drives
            if(tsaHarddiskInfo[i].m_fDriveExists && !tsaHarddiskInfo[i].m_fAtapi){      //If there's a HDD plugged on specified port
                if((tsaHarddiskInfo[i].m_securitySettings &0x0002)==0x0002) {       //If drive is locked
                    if(UnlockHDD(i, 0, (unsigned char *)&eeprom, true))                   //0 is for silent
                        unlockConfirm[i] = 1;                                   //Everything went well, we'll relock after eeprom write.
                    else{
                        unlockConfirm[0] = 255;       //error
                        unlockConfirm[1] = 255;       //error
                        break;
                    }
                }
                else{
                    unlockConfirm[i] = 0;                                         //Drive not locked, won't relock after eeprom write.
                }
            }
            else{
                unlockConfirm[i] = 0;       //Won't relock as no HDD was detected on that port.
            }
        }

        if(unlockConfirm[0] == 255 && unlockConfirm[1] == 255){      //error in unlocking one of 2 drives.
            cancelChanges = ConfirmDialog("        Drive(s) still locked! Continue anyway?", 1);
        }
        if(!cancelChanges){
            memcpy(&eeprom, eepromPtr, sizeof(EEPROMDATA));
            for(i = 0; i < 2; i++){               //Probe 2 possible drives
                if(unlockConfirm[i] == 1){
                    LockHDD(i, 0, (unsigned char *)&eeprom);                                //0 is for silent mode.
                }
            }
            ToolHeader("Saved EEPROM image");
            printk("\n\n           Modified buffer has been saved to main EEPROM buffer.\n           Pressing \'B\' will program EEPROM chip and restart the console.\n           Pressing Power button will cancel EEPROM chip write.\n\n\n");
            UIFooter();
            SlowReboot(NULL);   //This function will take care of saving eeprom image to chip.
            while(1);
            return false;
        }
        else{
            ToolHeader("Operation aborted");
            printk("\n\n           Error unlocking drives with previous key.");
            printk("\n           Actual EEPROM has NOT been changed.");
            printk("\n           Please Manually unlock all connected HDDs before modifying EEPROM content.");
            UIFooter();
            return false;
        }
    }
    return true;        //Error
}
