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
#include "LEDMenuActions.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "MenuActions.h"
#include "NetworkMenuActions.h"
#include "BootVideo.h"
#include "lib/LPCMod/BootLCD.h"
#include "i2c.h"
#include "xblast/HardwareIdentifier.h"
#include "lib/minIni/dev/minIni.h"
#include "xblastSettings.h"
#include <stddef.h>

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
    unsigned char i, j;
    _settingsPtrStruct *settingsStruct = (_settingsPtrStruct *)userdata;

#define BankSettingArraySize 6
    const unsigned char bankSettingArray[BankSettingArraySize] =
    {
     BNK512,
     BNK256,
     BNKTSOPSPLIT0,
     BNKTSOPSPLIT1,
     BNKFULLTSOP,
     BNKOS
    };

#define LedSettingArraySize 5
    const unsigned char ledArraySetting[LedSettingArraySize] =
    {
     LED_OFF,
     LED_GREEN,
     LED_RED,
     LED_ORANGE,
     LED_CYCLE
    };

#define LcdSettingArraySize 2
    const unsigned char lcdArraySetting[LcdSettingArraySize] =
    {
     LCDTYPE_HD44780,
     LCDTYPE_KS0073
    };

    debugSPIPrint(DEBUG_SETTINGS, "key:%s  value:%s  len:%u\n", key, value, strlen(value));

    for(i = 0; i < BoolParamGroup; i++)
    {
        if(0 == strcasecmp(xblastCfgStringsStruct.boolSettingsStringArray[i], key))
        {
            debugSPIPrint(DEBUG_SETTINGS, "found bool setting: %s\n", xblastCfgStringsStruct.boolSettingsStringArray[i]);
            if(*value >='0' && *value <='9')
            {
                *settingsStruct->boolSettingsPtrArray[i] = (unsigned char)strtol(value, NULL, 0);
            }
            else if(!strncasecmp(value, "y", 1) ||
                    !strncasecmp(value, "t", 1))
            {
                *settingsStruct->boolSettingsPtrArray[i] = 1;
            }
            else if(!strncasecmp(value, "n", 1) ||
                    !strncasecmp(value, "f", 1))
            {
                *settingsStruct->boolSettingsPtrArray[i] = 0;
            }
            return 1;
        }
    }

    for(i = 0; i < NumParamGroup; i++)
    {
        if(0 == strcasecmp(xblastCfgStringsStruct.numSettingsStringArray[i], key))
        {
            debugSPIPrint(DEBUG_SETTINGS, "found num setting: %s\n", xblastCfgStringsStruct.numSettingsStringArray[i]);
            if(*value >='0' && *value <='9')
            {
                *settingsStruct->numSettingsPtrArray[i] = (unsigned char)strtol(value, NULL, 0);
            }
            else
            {
                *settingsStruct->numSettingsPtrArray[i] = 0;
            }
            return 1;
        }
    }

    for(i = 0; i < IPParamGroup; i++)
    {
        if(0 == strcasecmp(xblastCfgStringsStruct.IPsettingsStringArray[i], key))
        {
            debugSPIPrint(DEBUG_SETTINGS, "found IP setting: %s\n", xblastCfgStringsStruct.IPsettingsStringArray[i]);
            if(strlen("255.255.255.255") > strlen(value))
            {
                assertCorrectIPString(settingsStruct->IPsettingsPtrArray[i], value);
            }
            return 1;
        }
    }

    for(i = 0; i < TextParamGroup; i++)
    {
        if(0 == strcasecmp(xblastCfgStringsStruct.textSettingsStringArray[i], key))
        {
            debugSPIPrint(DEBUG_SETTINGS, "found text setting: %s len:%u\n", xblastCfgStringsStruct.textSettingsStringArray[i]);
            if(LPCmodSettingsTextFieldsMaxLength > strlen(value))
            {
                strcpy(settingsStruct->textSettingsPtrArray[i], value);
            }
            return 1;
        }
    }

    for(i = 0; i < SpecialParamGroup; i++)
    {
      if(0 == strcasecmp(xblastCfgStringsStruct.specialSettingsStringArray[i], key))
      {
          debugSPIPrint(DEBUG_SETTINGS, "found special setting: %s len:%u\n", xblastCfgStringsStruct.specialSettingsStringArray[i]);
          switch(i)
          {
              case (0):
              case (1):
                  for(j = 0; j < BankSettingArraySize; j++)
                  {
                      if(!strcasecmp(value, getSpecialSettingString(i, bankSettingArray[j])))
                      {
                            *settingsStruct->specialCasePtrArray[i] = bankSettingArray[j];
                            break;
                      }
                  }
              break;
              case (2):
                  for(j = 0; j < LedSettingArraySize; j++)
                  {
                      if(!strncasecmp(value, getSpecialSettingString(i, ledArraySetting[j]), 2))
                      {
                            *settingsStruct->specialCasePtrArray[i] = ledArraySetting[j];
                            break;
                      }
                  }
                  break;
              case (3):
                  for(j = 0; j < LcdSettingArraySize; j++)
                  {
                      if(!strncasecmp(value, getSpecialSettingString(i, lcdArraySetting[j]), 2))
                      {
                            *settingsStruct->specialCasePtrArray[i] = lcdArraySetting[j];
                            break;
                      }
                  }
              } //switch(i)

              return 1;
        }
    }

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
        setCFGFileTransferPtr(LPCmodSettingsPtr, settingsStruct);
        if(1 != ini_browse(iniCallback, settingsStruct, settingsFileLocation))
        {
            debugSPIPrint(DEBUG_SETTINGS, "ini_browse fail\n");
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
            ini_putl(NULL, xblastCfgStringsStruct.boolSettingsStringArray[i], *settingsStruct->boolSettingsPtrArray[i], settingsFileLocation);
        }

        for(i = 0; i < NumParamGroup; i++)
        {
            ini_putl(NULL, xblastCfgStringsStruct.numSettingsStringArray[i], *settingsStruct->numSettingsPtrArray[i], settingsFileLocation);
        }

        for(i = 0; i < IPParamGroup; i++)
        {
            sprintf(tempStringBuf, "%u.%u.%u.%u", settingsStruct->IPsettingsPtrArray[i][0], settingsStruct->IPsettingsPtrArray[i][1], settingsStruct->IPsettingsPtrArray[i][2], settingsStruct->IPsettingsPtrArray[i][3]);
            ini_puts(NULL, xblastCfgStringsStruct.IPsettingsStringArray[i], tempStringBuf, settingsFileLocation);
        }

        for(i = 0; i < TextParamGroup; i++)
        {
            ini_puts(NULL, xblastCfgStringsStruct.textSettingsStringArray[i], settingsStruct->textSettingsPtrArray[i], settingsFileLocation);
        }

        for(i = 0; i < SpecialParamGroup; i++)
        {
            ini_puts(NULL, xblastCfgStringsStruct.specialSettingsStringArray[i], getSpecialSettingString(i, *settingsStruct->specialCasePtrArray[i]), settingsFileLocation);
        }
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

        i = 0;
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
