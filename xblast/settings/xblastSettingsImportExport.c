/*
 * xblastSettingsImportExport.c
 *
 *  Created on: Aug 11, 2016
 *      Author: cromwelldev
 */

#ifndef XBLASTSETTINGSIMPORTEXPORT_C_
#define XBLASTSETTINGSIMPORTEXPORT_C_

#include "xblastSettingsImportExport.h"
#include "lpcmod_v1.h"
#include "FatFSAccessor.h"
#include <stddef.h>
#include "LEDMenuActions.h"
#include "string.h"
#include "stdlib.h"
#include "MenuActions.h"
#include "ToolsMenuActions.h"
#include "NetworkMenuActions.h"
#include "Gentoox.h"
#include "menu/misc/ConfirmDialog.h"
#include "lib/LPCMod/BootLCD.h"
#include "lib/cromwell/cromSystem.h"
#include "i2c.h"
#include "xblast/HardwareIdentifier.h"

static const char* const settingsFileLocation = "MASTER_C:\\XBlast\\xblast.cfg";

const char* xblastcfgstrings[NBTXTPARAMS] = {
	//Contains boolean values.
	"quickboot=",
	"tsopcontrol=",
	"tsophide=",
	"runbankscript=",
	"runbootscript=",
	"enablenetwork=",
	"usedhcp=",
	"enablevga=",
	"enable5v=",
	"displaybootmsg=",
	"customtextboot=",
	"displaybiosnameboot=",

	//Contains numerical values
	"bgcolor=",
	"fanspeed=",
	"boottimeout=",
	"nblines=",
	"linelength=",
	"backlight=",
	"contrast=",

	//Contains IP text strings.
	"staticip=",
	"staticgateway=",
	"staticmask=",
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
	"activebank=",
	"altbank=",
	"ledcolor=",
	"lcdtype="
};

int LPCMod_ReadCFGFromHDD(_LPCmodSettings *LPCmodSettingsPtr, _settingsPtrStruct *settingsStruct)
{
    FILEX fileHandle;
    char compareBuf[100];                     //100 character long seems acceptable
    unsigned char i;
    bool settingLoaded[NBTXTPARAMS];
    int stringStartPtr = 0, stringStopPtr = 0, valueStartPtr = 0;
    unsigned char textStringCopyLength;

    //Take what's already properly set and start from there.
    if(LPCmodSettingsPtr != &LPCmodSettings)   //Avoid useless and potentially hazardous memcpy!
        memcpy((unsigned char *)LPCmodSettingsPtr, (unsigned char *)&LPCmodSettings, sizeof(_LPCmodSettings));

    if(isMounted(HDD_Master, Part_C))
    {
        fileHandle = fatxopen(settingsFileLocation, FileOpenMode_OpenExistingOnly | FileOpenMode_Read);
        if(fileHandle)
        {
            //Initially, no setting has been loaded from txt.
            for(i = 0; i < NBTXTPARAMS; i++)
            {
                settingLoaded[i] = false;
            }

            while(fatxgets(fileHandle, compareBuf, 100) && cromwellLoop()){      //We stay in file
                stringStartPtr = 0;
                while(100 > stringStartPtr && '#' != compareBuf[stringStartPtr] && '\0' != compareBuf[stringStartPtr])        //While we don't hit a new line and still in file
                {
                    if(' ' == compareBuf[stringStartPtr] || '\t' == compareBuf[stringStartPtr])
                    {
                        stringStartPtr++;        //Move on to next character in file.
                    }
                    else
                    {
                        break;
                    }
                }
                if('#' == compareBuf[stringStartPtr] || '\0' == compareBuf[stringStartPtr]){       //This is a comment or empty line

                    continue;
                }

                //stringStartPtr is now a beginning of the line and stringStopPtr is at the end of it.
                valueStartPtr = 0;
                //printk("\n       %s", compareBuf); //debug, print the whole file line by line.
                while(compareBuf[valueStartPtr] != '=' && valueStartPtr < 100 && compareBuf[valueStartPtr] != '\n' && compareBuf[valueStartPtr] != '\0'){     //Search for '=' character.
                    valueStartPtr++;
                }

                if(valueStartPtr < MINPARAMLENGTH || valueStartPtr >= 30){    //If line is not properly constructed
                    continue;
                }
                else{
                    for(i = 0; i < NBTXTPARAMS; i++)
                    {
                        if(!strncmp(compareBuf, xblastcfgstrings[i], valueStartPtr) && !settingLoaded[i])   //Match
                        {
                            settingLoaded[i] = true;        //Recurring entries not allowed.
                            valueStartPtr++;
                            if(i < IPTEXTPARAMGROUP)       //Numerical value parse
                            {
                                if(compareBuf[valueStartPtr] >='0' && compareBuf[valueStartPtr] <='9')
                                {
                                    *settingsStruct->settingsPtrArray[i] = (unsigned char)strtol(&compareBuf[valueStartPtr], NULL, 0);
                                }
                                else if(!strncmp(&compareBuf[valueStartPtr], "Y", 1) ||
                                        !strncmp(&compareBuf[valueStartPtr], "y", 1) ||
                                        !strncmp(&compareBuf[valueStartPtr], "T", 1) ||
                                        !strncmp(&compareBuf[valueStartPtr], "t", 1))
                                {
                                    *settingsStruct->settingsPtrArray[i] = 1;
                                }
                                else if(!strncmp(&compareBuf[valueStartPtr], "N", 1) ||
                                        !strncmp(&compareBuf[valueStartPtr], "n", 1) ||
                                        !strncmp(&compareBuf[valueStartPtr], "F", 1) ||
                                        !strncmp(&compareBuf[valueStartPtr], "f", 1))
                                {
                                    *settingsStruct->settingsPtrArray[i] = 0;
                                }
                            }
                            else if(i < TEXTPARAMGROUP)       //IP string value parse
                            {
                                assertCorrectIPString(settingsStruct->IPsettingsPtrArray[i - IPTEXTPARAMGROUP], &compareBuf[valueStartPtr]);
                            }
                            else if(i < SPECIALPARAMGROUP)    //Text value parse
                            {
                                if((stringStopPtr - valueStartPtr) >= 20)
                                {
                                    textStringCopyLength = 20;
                                }
                                else
                                {
                                    textStringCopyLength = stringStopPtr - valueStartPtr;
                                }
                                strncpy(settingsStruct->textSettingsPtrArray[i - TEXTPARAMGROUP], &compareBuf[valueStartPtr], textStringCopyLength);
                                settingsStruct->textSettingsPtrArray[i - TEXTPARAMGROUP][20] = '\0';
                            }
                            else
                            {
                                switch(i)
                                {
                                    case (SPECIALPARAMGROUP):
                                    case (SPECIALPARAMGROUP + 1):
                                        if(!strcmp(&compareBuf[valueStartPtr], "BNK512"))
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNK512;
                                        else if(!strcmp(&compareBuf[valueStartPtr], "BNK256"))
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNK256;
                                        else if(!strcmp(&compareBuf[valueStartPtr], "BNKTSOPSPLIT0"))
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKTSOPSPLIT0;
                                        else if(!strcmp(&compareBuf[valueStartPtr], "BNKTSOPSPLIT1"))
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKTSOPSPLIT1;
                                        else if(!strcmp(&compareBuf[valueStartPtr], "BNKFULLTSOP"))
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKFULLTSOP;
                                        else if(!strcmp(&compareBuf[valueStartPtr], "BNKOS"))
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKOS;
                                        break;
                                    case (SPECIALPARAMGROUP + 2):
                                        if(!strncmp(&compareBuf[valueStartPtr], "Of", 2) || !strncmp(&compareBuf[valueStartPtr], "of", 2))    //LED_OFF
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_OFF;
                                        else if(!strncmp(&compareBuf[valueStartPtr], "G", 1) || !strncmp(&compareBuf[valueStartPtr], "g", 1))    //LED_GREEN
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_GREEN;
                                        if(!strncmp(&compareBuf[valueStartPtr], "R", 1) || !strncmp(&compareBuf[valueStartPtr], "r", 1))    //LED_RED
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_RED;
                                        if(!strncmp(&compareBuf[valueStartPtr], "Or", 2) || !strncmp(&compareBuf[valueStartPtr], "or", 2))    //LED_ORANGE
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_ORANGE;
                                        if(!strncmp(&compareBuf[valueStartPtr], "C", 1) || !strncmp(&compareBuf[valueStartPtr], "c", 1))    //LED_CYCLE
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_CYCLE;
                                        break;
                                    case (SPECIALPARAMGROUP + 3):
                                        if(!strcmp(&compareBuf[valueStartPtr], "HD44780"))
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LCDTYPE_HD44780 ;
                                        else if(!strcmp(&compareBuf[valueStartPtr], "KS0073"))
                                            *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LCDTYPE_KS0073 ;
                                        break;
                                } //switch(i)
                            } //!if(i < IPTEXTPARAMGROUP){
                        } //if(!strncmp(compareBuf, xblastcfgstrings[i], valueStartPtr) && !settingLoaded[i])
                    } //for(i = 0; i < NBTXTPARAMS; i++)
                } //!if(valueStartPtr >= 30)
            } //while(stringStopPtr < fileinfo.fileSize)
            fatxclose(fileHandle);
        } //if(res)
        else
        {
            return 4; //Cannot open file.
        }
    }
    else
    {
        return 2; //Cannot open partition.
    }

    return 0; //Everything went fine.
}

int LPCMod_SaveCFGToHDD(void){
	FATXFILEINFO fileinfo;
    FATXPartition *partition;
    int res = false;
    char * filebuf;
    char tempString[22];
    unsigned int cursorpos, totalbytes = 0;
    int dcluster, i;
    const char *cfgFileName = "\\XBlast\\xblast.cfg";

    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);

    if(partition != NULL){
        dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "XBlast");
        if((dcluster != -1) && (dcluster != 1)) {
            res = FATXFindFile(partition, (char *)cfgFileName, dcluster, &fileinfo);
        }
        if(res){                //File already exist
            if(ConfirmDialog("Overwrite C:\\XBlast\\xblast.cfg?", 1)){
                CloseFATXPartition(partition);
                UiHeader("Saving to C:\\XBlast\\xblast.cfg aborted.");
                cromwellWarning();
                UIFooter();
                initialSetLED(LPCmodSettings.OSsettings.LEDColor);
                return 1;
            }
            filebuf = (char *)malloc(FATX16CLUSTERSIZE);
            memset(filebuf, 0x00, FATX16CLUSTERSIZE);
            for(i = 0; i < NBTXTPARAMS; i++){
                cursorpos = 0;
                strcpy(&filebuf[cursorpos], xblastcfgstrings[i]);
                cursorpos += strlen(xblastcfgstrings[i]);
                //New line inserted in.
                cursorpos += strlen(tempString);
                strncpy(&filebuf[cursorpos],tempString, strlen(tempString));    //Skip terminating character.
                //filebuf[cursorpos] = 0x0A;
                //cursorpos += 1;
                totalbytes += cursorpos;
            }
            filebuf[cursorpos] = 0;     //Terminating character at the end of file.


            free(filebuf);
        }
        UiHeader("Saved settings to C:\\XBlast\\xblast.cfg");

        CloseFATXPartition(partition);
    }
    else{
        UiHeader("Error opening partition. Drive formatted?");
    }

    UIFooter();
    return 0;
}

void setCFGFileTransferPtr(_LPCmodSettings * tempLPCmodSettings, _settingsPtrStruct *settingsStruct)
{
        int i = 0;
        //Boolean values
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.Quickboot);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.TSOPcontrol);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.TSOPhide);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.runBankScript);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.runBootScript);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.enableNetwork);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.useDHCP);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.enableVGA);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.enable5V);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.displayMsgBoot);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.customTextBoot);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.displayBIOSNameBoot);

        //Numerical values
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.backgroundColorPreset);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.fanSpeed);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.bootTimeout);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.nbLines);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.lineLength);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.backlight);
        settingsStruct->settingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.contrast);

        i = 0;
        settingsStruct->IPsettingsPtrArray[i++] = tempLPCmodSettings->OSsettings.staticIP;
        settingsStruct->IPsettingsPtrArray[i++] = tempLPCmodSettings->OSsettings.staticGateway;
        settingsStruct->IPsettingsPtrArray[i++] = tempLPCmodSettings->OSsettings.staticMask;
        settingsStruct->IPsettingsPtrArray[i++] = tempLPCmodSettings->OSsettings.staticDNS1;
        settingsStruct->IPsettingsPtrArray[i++] = tempLPCmodSettings->OSsettings.staticDNS2;

        i = 0;
        settingsStruct->textSettingsPtrArray[i++] = tempLPCmodSettings->OSsettings.biosName512Bank;
        settingsStruct->textSettingsPtrArray[i++] = tempLPCmodSettings->OSsettings.biosName256Bank;
        settingsStruct->textSettingsPtrArray[i++] = tempLPCmodSettings->OSsettings.biosNameTSOPFullSplit0;
        settingsStruct->textSettingsPtrArray[i++] = tempLPCmodSettings->OSsettings.biosNameTSOPSplit1;
        settingsStruct->textSettingsPtrArray[i++] = tempLPCmodSettings->LCDsettings.customString0;
        settingsStruct->textSettingsPtrArray[i++] = tempLPCmodSettings->LCDsettings.customString1;
        settingsStruct->textSettingsPtrArray[i++] = tempLPCmodSettings->LCDsettings.customString2;
        settingsStruct->textSettingsPtrArray[i++] = tempLPCmodSettings->LCDsettings.customString3;

        i = 0;
        settingsStruct->specialCasePtrArray[i++] = &(tempLPCmodSettings->OSsettings.activeBank);
        settingsStruct->specialCasePtrArray[i++] = &(tempLPCmodSettings->OSsettings.altBank);
        settingsStruct->specialCasePtrArray[i++] = &(tempLPCmodSettings->OSsettings.LEDColor);
        settingsStruct->specialCasePtrArray[i++] = &(tempLPCmodSettings->LCDsettings.lcdType);
}

void importNewSettingsFromCFGLoad(_LPCmodSettings* newSettings)
{
    unsigned char vgaAlreadyset = LPCmodSettings.OSsettings.enableVGA;

    memcpy(&LPCmodSettings, newSettings, sizeof(_LPCmodSettings));

    I2CSetFanSpeed(LPCmodSettings.OSsettings.fanSpeed);
    initialSetLED(LPCmodSettings.OSsettings.LEDColor);
    //Stuff to do right after loading persistent settings from file.
    if(isLCDSupported())
    {
        assertInitLCD();                            //Function in charge of checking if a init of LCD is needed.
    }

    if(isFrostySupport())
    {
        if((vgaAlreadyset > 0) != (LPCmodSettings.OSsettings.enableVGA > 0))
        {
            BootVgaInitializationKernelNG((CURRENT_VIDEO_MODE_DETAILS *)&vmode);
        }
    }
    else
    {
        LPCmodSettings.OSsettings.enableVGA = 0;
    }

    if(isTSOPSplitCapable() == false)
    {
        LPCmodSettings.OSsettings.TSOPcontrol = 0;
    }
}

#endif /* XBLASTSETTINGSIMPORTEXPORT_C_ */
