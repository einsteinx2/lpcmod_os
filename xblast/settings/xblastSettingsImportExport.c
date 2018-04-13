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

const _xblastCfgStringsStruct xblastCfgStringsStruct =
{
 //Contains boolean values.
 {
  "quickboot",
  "tsopcontrol",
  "tsophide",
  "runbankscript",
  "runbootscript",
  "enablenetwork",
  "usedhcp",
  "enablevga",
  "enable5v",
  "displaybootmsg",
  "customtextboot",
  "displaybiosnameboot"

 },

 //Contains numerical values
 {
  "bgcolor",
  "fanspeed",
  "boottimeout",
  "nblines",
  "linelength",
  "backlight",
  "contrast"
 },

 //Contains IP text strings.
 {
  "staticip",
  "staticgateway",
  "staticmask",
  "staticdns1",
  "staticdns2"
 },

 //Contains text strings.
 {
  "512kbname",
  "256kbname",
  "tsop0name",
  "tsop1name",
  "customstring0",
  "customstring1",
  "customstring2",
  "customstring3"
 },

 //Special case.
 {
     "activebank",
     "altbank",
     "ledcolor",
     "lcdtype"
 }
};

int iniCallback(const char *section, const char *key, const char *value, void *userdata)
{
    unsigned char i;
    _settingsPtrStruct *settingsStruct = (_settingsPtrStruct *)userdata;

    for(i = 0; i < NBTXTPARAMS; i++)
    {
      if(0 == strcmp(key, xblastCfgStringsStruct[i]))   //Match
      {
          if(i < IPTEXTPARAMGROUP)       //Numerical value parse
          {
              if(*value >='0' && *value <='9')
              {
                  *settingsStruct->settingsPtrArray[i] = (unsigned char)strtol(&value, NULL, 0);
              }
              else if(!strncmp(value, "Y", 1) ||
                      !strncmp(value, "y", 1) ||
                      !strncmp(value, "T", 1) ||
                      !strncmp(value, "t", 1))
              {
                  *settingsStruct->settingsPtrArray[i] = 1;
              }
              else if(!strncmp(value, "N", 1) ||
                      !strncmp(value, "n", 1) ||
                      !strncmp(value, "F", 1) ||
                      !strncmp(value, "f", 1))
              {
                  *settingsStruct->settingsPtrArray[i] = 0;
              }
          }
          else if(i < TEXTPARAMGROUP)       //IP string value parse
          {
              assertCorrectIPString(settingsStruct->IPsettingsPtrArray[i - IPTEXTPARAMGROUP], value);
          }
          else if(i < SPECIALPARAMGROUP)    //Text value parse
          {
              strncpy(settingsStruct->textSettingsPtrArray[i - TEXTPARAMGROUP], value, 20);
              settingsStruct->textSettingsPtrArray[i - TEXTPARAMGROUP][20] = '\0';
          }
          else
          {
              switch(i)
              {
                  case (SPECIALPARAMGROUP):
                  case (SPECIALPARAMGROUP + 1):
                      if(!strcmp(value, "BNK512"))
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNK512;
                      else if(!strcmp(value, "BNK256"))
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNK256;
                      else if(!strcmp(value, "BNKTSOPSPLIT0"))
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKTSOPSPLIT0;
                      else if(!strcmp(value, "BNKTSOPSPLIT1"))
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKTSOPSPLIT1;
                      else if(!strcmp(value, "BNKFULLTSOP"))
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKFULLTSOP;
                      else if(!strcmp(value, "BNKOS"))
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKOS;
                      break;
                  case (SPECIALPARAMGROUP + 2):
                      if(!strncmp(value, "Of", 2) || !strncmp(value, "of", 2))    //LED_OFF
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_OFF;
                      else if(!strncmp(value, "G", 1) || !strncmp(value, "g", 1))    //LED_GREEN
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_GREEN;
                      if(!strncmp(value, "R", 1) || !strncmp(value, "r", 1))    //LED_RED
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_RED;
                      if(!strncmp(value, "Or", 2) || !strncmp(value, "or", 2))    //LED_ORANGE
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_ORANGE;
                      if(!strncmp(value, "C", 1) || !strncmp(value, "c", 1))    //LED_CYCLE
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_CYCLE;
                      break;
                  case (SPECIALPARAMGROUP + 3):
                      if(!strcmp(value, "HD44780"))
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LCDTYPE_HD44780 ;
                      else if(!strcmp(value, "KS0073"))
                          *settingsStruct->specialCasePtrArray[i - SPECIALPARAMGROUP] = LCDTYPE_KS0073 ;
                      break;
              } //switch(i)
          } //!if(i < IPTEXTPARAMGROUP){
      } //if(!strncmp(compareBuf, xblastcfgstrings[i], valueStartPtr) && !settingLoaded[i])
    } //for(i = 0; i < NBTXTPARAMS; i++)

    return 1; /* OK */
}

int LPCMod_ReadCFGFromHDD(_LPCmodSettings *LPCmodSettingsPtr, _settingsPtrStruct *settingsStruct)
{
    //Take what's already properly set and start from there.
    if(LPCmodSettingsPtr != &LPCmodSettings)   //Avoid useless and potentially hazardous memcpy!
    {
        memcpy((unsigned char *)LPCmodSettingsPtr, (unsigned char *)&LPCmodSettings, sizeof(_LPCmodSettings));
    }

    if(isMounted(HDD_Master, Part_C))
    {
        if(1 != ini_browse(iniCallback, settingsStruct, settingsFileLocation))
        {
            return 4;
        }
    }
    else
    {
        return 2; //Cannot open partition.
    }

    return 0; //Everything went fine.
}

int LPCMod_SaveCFGToHDD(_settingsPtrStruct *settingsStruct)
{
    unsigned char i;
    char tempStringBuf[50];

    if(isMounted(HDD_Master, Part_C))
    {
        for(i = 0; i < BoolParamGroup; i++)
        {
            ini_putl(NULL, xblastCfgStringsStruct.boolSettingsStringArray[i], settingsStruct->boolSettingsPtrArray[i], settingsFileLocation);
        }

        for(i = 0; i < NumParamGroup; i++)
        {
            ini_putl(NULL, xblastCfgStringsStruct.numSettingsStringArray[i], settingsStruct->numSettingsPtrArray[i], settingsFileLocation);
        }

        //TODO: IP, Text and Special params
    }
    else
    {
        UiHeader("Error opening partition.");
    }

    UIFooter();
    return 0;
}

void setCFGFileTransferPtr(_LPCmodSettings * tempLPCmodSettings, _settingsPtrStruct *settingsStruct)
{
        int i = 0;
        //Boolean values
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.Quickboot);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.TSOPcontrol);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.TSOPhide);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.runBankScript);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.runBootScript);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.enableNetwork);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.useDHCP);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.enableVGA);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.enable5V);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.displayMsgBoot);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.customTextBoot);
        settingsStruct->boolSettingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.displayBIOSNameBoot);

        int i = 0;
        //Numerical values
        settingsStruct->numSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.backgroundColorPreset);
        settingsStruct->numSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.fanSpeed);
        settingsStruct->numSettingsPtrArray[i++] = &(tempLPCmodSettings->OSsettings.bootTimeout);
        settingsStruct->numSettingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.nbLines);
        settingsStruct->numSettingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.lineLength);
        settingsStruct->numSettingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.backlight);
        settingsStruct->numSettingsPtrArray[i++] = &(tempLPCmodSettings->LCDsettings.contrast);

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
