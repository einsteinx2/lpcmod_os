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
#include "BootIde.h"
#include "video.h"
#include "BootFATX.h"

#define NBTXTPARAMS 30
#define IPTEXTPARAMGROUP 16
#define TEXTPARAMGROUP IPTEXTPARAMGROUP + 4
#define SPECIALPARAMGROUP TEXTPARAMGROUP + 8

char * xblastcfgstrings[NBTXTPARAMS] = {
        //Contains either numerical or boolean values.
	"activebank=",
	"altbank=",
	"quickboot=",
	"fanspeed=",
	"boottimeout=",
	"tsopcontrol=",
	"enablenetwork=",
	"usedhcp=",
	"enable5v=",
	"nblines=",
	"linelength=",
	"backlight=",
	"contrast=",
	"displaybootmsg=",
	"customtextboot=",
	"displaybiosnameboot=",

	//Contains IP text strings.
        "staticip=",
        "staticgateway=",
        "staticdns1=",
        "staticdns2=",

        //Contains text strings.
        "512kbname=",
        "256kbname=",
        "tsop0name=",
        "tsop1name=",
	"customstring0=",
	"customstring1=",
	"customstring2=",
	"customstring3=",

	//Special case.
	"ledcolor=",
        "lcdtype="
};



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
    u8 lockPreference = 0;
    u8 emptyCount = 0;
    char hddString[6];
    for(i = 0; i < 2; i++){
        if (tsaHarddiskInfo[i].m_fDriveExists && !tsaHarddiskInfo[i].m_fAtapi) {
    	    if((tsaHarddiskInfo[i].m_securitySettings &0x0002)==0x0002){        //Drive locked.
    	        lockPreference += (i + 1);             //0=no drive locked, 1=master locked, 2= slave locked
    	    }                                          //3=both drives locked.
        }
    }
    
    for(i = 0; i < 4; i++) {         //Checksum2 is 4 bytes long.
        if(LPCmodSettings.bakeeprom.Checksum2[i] == 0xFF)
            emptyCount++;
    }
    if(emptyCount < 4){            //Make sure checksum2 is not 0xFFFFFFFF.
                        //It is practically impossible to get such value in this checksum field.
        if(ConfirmDialog("       Restore backed up EEProm content?", 1))
            return;
        if((lockPreference&1) == 1){       //Master is locked.
    	    if(!UnlockHDD(0,0)){		     //Silently unlock master.
    	        ToolHeader("ERROR: Could not unlock master HDD");
    	    	goto failed;
    	    }
        }
        if((lockPreference&2) == 2){       //Slave is locked.
            if(!UnlockHDD(1,0)){              //Silently unlock slave.
                ToolHeader("ERROR: Could not unlock slave HDD");
                goto failed;
            }
        }
        memcpy(&eeprom,&(LPCmodSettings.bakeeprom),sizeof(EEPROMDATA));
        ToolHeader("Restored back up to Xbox");
        if((lockPreference&1) == 1){       //Master was initiallylocked.
    	    LockHDD(0,0);		     //Silently lock master.
        }
        if((lockPreference&2) == 2){       //Slave was initially locked.
            LockHDD(1,0);              //Silently lock slave.
        }
    }
    else {
        ToolHeader("ERROR: No back up data on modchip");
    }
failed:
    ToolFooter();
}

void warningDisplayEepromEditMenu(void *ignored){
    if(ConfirmDialog("         Use these tools at your own risk!", 1))
            return;
    editeeprom = (EEPROMDATA *)malloc(sizeof(EEPROMDATA));
    memcpy(editeeprom, &eeprom, sizeof(EEPROMDATA));   //Initial copy into edition buffer.
    ResetDrawChildTextMenu(eepromEditMenuInit());
    free(editeeprom);
}

void wipeEEPromUserSettings(void *whatever){
    if(ConfirmDialog("        Reset user EEProm settings(safe)?", 1))
        return;
    memset(eeprom.Checksum3,0xFF,4);    //Checksum3 need to be 0xFFFFFFFF
    memset(eeprom.TimeZoneBias,0x00,0x5b);    //Start from Checksum3 address in struct and write 0x00 up to UNKNOWN6.
    ToolHeader("Reset user EEProm settings successful");
    ToolFooter();
}

void showMemTest(void *whatever){
    ToolHeader("128MB  RAM test");
    memtest();
    ToolFooter();
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
    FATXFILEINFO fileinfo;
    FATXPartition *partition;
    int res = false;
    char * filebuf;
    char tempString[22];
    u32 cursorpos, totalbytes = 0;
    int dcluster, i;
    const char *path="\\XBlast\\";
    const char *cfgFileName = "\\XBlast\\xblast.cfg";


    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);

    if(partition != NULL){
        dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "XBlast");
        if((dcluster != -1) && (dcluster != 1)) {
            res = FATXFindFile(partition, (char *)cfgFileName, dcluster, &fileinfo);
        }
        if(res){                //File already exist
            if(ConfirmDialog("              Overwrite C:\\XBlast\\xblast.cfg?", 1)){
                CloseFATXPartition(partition);
                ToolHeader("Saving to C:\\XBlast\\xblast.cfg aborted.");
                cromwellWarning();
                ToolFooter();
                initialSetLED(LPCmodSettings.OSsettings.LEDColor);
                return;
            }
            filebuf = (char *)malloc(FATX16CLUSTERSIZE);
            memset(filebuf, 0x00, FATX16CLUSTERSIZE);
            for(i = 0; i < NBTXTPARAMS; i++){
                cursorpos = 0;
                strcpy(&filebuf[cursorpos], xblastcfgstrings[i]);
                cursorpos += strlen(xblastcfgstrings[i]);
                //New line inserted in.
                sprintf(tempString, "%d\n", LPCmodSettings.OSsettings.fanSpeed);
                cursorpos += strlen(tempString);
                strncpy(&filebuf[cursorpos],tempString, strlen(tempString));    //Skip terminating character.
                //filebuf[cursorpos] = 0x0A;
                //cursorpos += 1;
                totalbytes += cursorpos;
            }
            filebuf[cursorpos] = 0;     //Terminating character at the end of file.


            free(filebuf);
        }
        ToolHeader("Saved settings to C:\\XBlast\\xblast.cfg");

        CloseFATXPartition(partition);
    }
    else{
        ToolHeader("Error opening partition. Drive formatted?");
    }

    ToolFooter();
}

void loadXBlastcfg(void * ignored){
    FATXFILEINFO fileinfo;
    FATXPartition *partition;
    int res = false;
    int dcluster;
    const char *cfgFileName = "\\XBlast\\xblast.cfg";
    const char *path="\\XBlast\\";
    char compareBuf[100];                     //100 character long seems acceptable
    u8 * fileBuf;
    u8 i;
    bool settingLoaded[NBTXTPARAMS];
    int stringStartPtr = 0, stringStopPtr = 0, valueStartPtr = 0;
    bool CRdetected;
    u8 textStringCopyLength;
    u8 *settingsPtrArray[] = {
            &(LPCmodSettings.OSsettings.activeBank),
            &(LPCmodSettings.OSsettings.altBank),
            &(LPCmodSettings.OSsettings.Quickboot),
            &(LPCmodSettings.OSsettings.fanSpeed),
            &(LPCmodSettings.OSsettings.bootTimeout),
            &(LPCmodSettings.OSsettings.TSOPcontrol),
            &(LPCmodSettings.OSsettings.enableNetwork),
            &(LPCmodSettings.OSsettings.useDHCP),
            &(LPCmodSettings.LCDsettings.enable5V),
            &(LPCmodSettings.LCDsettings.nbLines),
            &(LPCmodSettings.LCDsettings.lineLength),
            &(LPCmodSettings.LCDsettings.backlight),
            &(LPCmodSettings.LCDsettings.contrast),
            &(LPCmodSettings.LCDsettings.displayMsgBoot),
            &(LPCmodSettings.LCDsettings.customTextBoot),
            &(LPCmodSettings.LCDsettings.displayBIOSNameBoot)
    };
    char *textSettingsPtrArray[] = {
            LPCmodSettings.OSsettings.biosName0,
            LPCmodSettings.OSsettings.biosName1,
            LPCmodSettings.OSsettings.biosName2,
            LPCmodSettings.OSsettings.biosName3,
            LPCmodSettings.LCDsettings.customString0,
            LPCmodSettings.LCDsettings.customString1,
            LPCmodSettings.LCDsettings.customString2,
            LPCmodSettings.LCDsettings.customString3
    };
    
    u8 *specialCasePtrArray[] = {
            &(LPCmodSettings.OSsettings.LEDColor),
            &(LPCmodSettings.LCDsettings.lcdType)
    };

    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);
    if(partition != NULL){
        dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "XBlast");
        if((dcluster != -1) && (dcluster != 1)) {
        
            res = FATXFindFile(partition, (char *)cfgFileName, FATX_ROOT_FAT_CLUSTER, &fileinfo);
        }
        else
            LEDOff(NULL);
        if(res){
             if(ConfirmDialog("        Restore settings from C:\\XBlast\\xblast.cfg?", 1)){
                CloseFATXPartition(partition);
                ToolHeader("Loading from C:\\xblast.cfg aborted.");
                cromwellWarning();
                ToolFooter();
                initialSetLED(LPCmodSettings.OSsettings.LEDColor);
                return;
            }
            //Initially, no setting has been loaded from txt.
            for(i = 0; i < NBTXTPARAMS; i++)
            {
                settingLoaded[i] = false;
            }

            if(LoadFATXFilefixed (partition, (char *)cfgFileName, &fileinfo, fileBuf)){
                while(stringStopPtr < (fileinfo.fileSize - 1)){      //We stay in file
                    while(fileBuf[stringStopPtr] != '\n' && stringStopPtr < (fileinfo.fileSize - 1)){        //While we don't hit a new line and still in file
                        stringStopPtr++;        //Move on to next character in file.
                    }
                    valueStartPtr = 0;
                    CRdetected = fileBuf[stringStopPtr - 1] == '\r' ? 1 : 0;    //Dos formatted line?
                    strncpy(compareBuf, &fileBuf[stringStartPtr], stringStopPtr - CRdetected - stringStartPtr);
                    compareBuf[stringStopPtr - CRdetected - stringStartPtr] = '\0';
                    printk("\n       %s", compareBuf);
                    while(compareBuf[valueStartPtr] != '=' && valueStartPtr < 100 && compareBuf[valueStartPtr] != '\0'){     //Search for '=' character.
                        valueStartPtr++;
                    }
                    stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
                    if(valueStartPtr >= 30){    //If line is not properly constructed
                        continue;
                    }
                    else{
                        for(i = 0; i < NBTXTPARAMS; i++)
                        {
                            if(!strncmp(compareBuf, xblastcfgstrings[i], valueStartPtr) && !settingLoaded[i]){   //Match
                                settingLoaded[i] = true;        //Don't know why I do this.
                                valueStartPtr++;
                                if(i < IPTEXTPARAMGROUP){       //Numerical value parse
                                    if(compareBuf[valueStartPtr] >='0' && compareBuf[valueStartPtr] <='9'){
                                        *settingsPtrArray[i] = (u8)atoi(&compareBuf[valueStartPtr]);
                                    }
                                    else if(!strncmp(&compareBuf[valueStartPtr], "Y", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "y", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "T", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "t", 1)){
                                        *settingsPtrArray[i] = 1;
                                    }
                                    else if(!strncmp(&compareBuf[valueStartPtr], "N", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "n", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "F", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "f", 1)){
                                        *settingsPtrArray[i] = 0;
                                    }
                                }
                                else if(i < TEXTPARAMGROUP){       //IP string value parse

                                }
                                else if(i < SPECIALPARAMGROUP){    //Text value parse
                                    if((stringStopPtr - valueStartPtr) >= 20)
                                        textStringCopyLength = 20;
                                    else
                                        textStringCopyLength = stringStopPtr - valueStartPtr;
                                    strncpy(textSettingsPtrArray[i - TEXTPARAMGROUP], &compareBuf[valueStartPtr], textStringCopyLength);
                                    textSettingsPtrArray[i - TEXTPARAMGROUP][20] = '\0';
                                }
                                else{
                                    //Only one case for now.

                                }
                            }
                        }
                    }
                }


                ToolHeader("Success.");
                printk("\n           Settings loaded from \"C:\\XBlast\\xblast.cfg\".");
                printk("\n          fan : %u%%,   start=%u   stop=%u", LPCmodSettings.OSsettings.fanSpeed, stringStartPtr, stringStopPtr);
                
            }
            else{
                ToolHeader("Error!!!");
                printk("\n           Unable to open \"C:\\XBlast\\xblast.cfg\".");
            }

        }
        else{
            ToolHeader("Error!!!");
            printk("\n           File \"C:\\XBlast\\xblast.cfg\" not found.");
        }

        //CloseFATXPartition(partition);
    }
    else{
        ToolHeader("Error opening partition.");
    }
    ToolFooter();
}

void nextA19controlModBootValue(void * itemPtr){
    switch(A19controlModBoot){
        case TSOPFULLBOOT:
            A19controlModBoot = BNKTSOPSPLIT0;
            sprintf(itemPtr, "%s", "Bank0");
            break;
        case BNKTSOPSPLIT0:
            A19controlModBoot = BNKTSOPSPLIT1;
            sprintf(itemPtr, "%s", "Bank1");
            break;
        case BNKTSOPSPLIT1:
        default:
            A19controlModBoot = TSOPFULLBOOT;
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
        case TSOPFULLBOOT:
            A19controlModBoot = BNKTSOPSPLIT1;
            sprintf(itemPtr, "%s", "Bank1");
            break;
        case BNKTSOPSPLIT0:
        default:
            A19controlModBoot = TSOPFULLBOOT;
            sprintf(itemPtr, "%s", "No");
            break;
    }
}
