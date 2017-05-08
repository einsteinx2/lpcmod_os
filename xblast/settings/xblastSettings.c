/*
 * xblastSettings.c
 *
 *  Created on: Aug 11, 2016
 *      Author: cromwelldev
 */

#include "xblastSettings.h"
#include "lpcmod_v1.h"
#include "config.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "LEDMenuActions.h"
#include "xblast/HardwareIdentifier.h"
#include "string.h"
#include "boot.h"

#define HDD4780_DEFAULT_NBLINES    4
#define HDD4780_DEFAULT_LINELGTH    20

#define DEFAULT_FANSPEED    20

//Sets default values to most important settings.
void populateSettingsStructWithDefault(_LPCmodSettings *inout)
{
    unsigned char i;

    memset(inout, 0xFF, sizeof(_LPCmodSettings));

    inout->settingsVersion = CurrentSettingsVersionNumber;

    if(isXBlastCompatible())
    {
        inout->OSsettings.activeBank = BNK512;
    }
    else
    {
        inout->OSsettings.activeBank = NOBNKID;
    }
    inout->OSsettings.altBank = BNKOS;
    inout->OSsettings.Quickboot = 0;
    inout->OSsettings.selectedMenuItem = BNK512;
    inout->OSsettings.fanSpeed = DEFAULT_FANSPEED;
    inout->OSsettings.bootTimeout = BOOT_TIMEWAIT;
    inout->OSsettings.LEDColor = LED_GREEN;    //Set for next boot
    inout->OSsettings.TSOPcontrol = 0;
    inout->OSsettings.TSOPhide = 0;
    inout->OSsettings.runBankScript = 0;
    inout->OSsettings.runBootScript = 0;
#ifdef DEFAULT_ENABLE_VGA
    inout->OSsettings.enableVGA = 1;
#else
    inout->OSsettings.enableVGA = 0;
#endif
    switch(fSpecialEdition)
    {
    case SYSCON_ID_V1_PRE_EDITION:
        inout->OSsettings.backgroundColorPreset = 1;
        break;
    default:
        inout->OSsettings.backgroundColorPreset = 0;
        break;
    }
    inout->OSsettings.enableNetwork = 1;
    inout->OSsettings.useDHCP = 1;
    inout->OSsettings.staticIP[0] = 192;
    inout->OSsettings.staticIP[1] = 168;
    inout->OSsettings.staticIP[2] = 0;
    inout->OSsettings.staticIP[3] = 250;
    inout->OSsettings.staticGateway[0] = 192;
    inout->OSsettings.staticGateway[1] = 168;
    inout->OSsettings.staticGateway[2] = 0;
    inout->OSsettings.staticGateway[3] = 1;
    inout->OSsettings.staticMask[0] = 255;
    inout->OSsettings.staticMask[1] = 255;
    inout->OSsettings.staticMask[2] = 255;
    inout->OSsettings.staticMask[3] = 0;
    inout->OSsettings.staticDNS1[0] = 192;
    inout->OSsettings.staticDNS1[1] = 168;
    inout->OSsettings.staticDNS1[2] = 0;
    inout->OSsettings.staticDNS1[3] = 1;
    inout->OSsettings.staticDNS2[0] = 192;
    inout->OSsettings.staticDNS2[1] = 168;
    inout->OSsettings.staticDNS2[2] = 0;
    inout->OSsettings.staticDNS2[3] = 1;


    inout->LCDsettings.enable5V = 0;
    inout->LCDsettings.lcdType = 0;
    inout->LCDsettings.nbLines = HDD4780_DEFAULT_NBLINES;
    inout->LCDsettings.lineLength = HDD4780_DEFAULT_LINELGTH;
    inout->LCDsettings.backlight = 50;
    inout->LCDsettings.contrast = 20;
    inout->LCDsettings.displayMsgBoot = 0;
    inout->LCDsettings.customTextBoot = 0;
    inout->LCDsettings.displayBIOSNameBoot = 0;


    for(i = 0; i < (HDD4780_DEFAULT_LINELGTH + 1); i++)
    {
        inout->OSsettings.biosName512Bank[i] = 0;
        inout->OSsettings.biosName256Bank[i] = 0;
        inout->OSsettings.biosNameTSOPFullSplit0[i] = 0;
        inout->OSsettings.biosNameTSOPSplit1[i] = 0;

        inout->LCDsettings.customString0[i] = 0;
        inout->LCDsettings.customString1[i] = 0;
        inout->LCDsettings.customString2[i] = 0;
        inout->LCDsettings.customString3[i] = 0;
    }

    inout->flashScript.scriptSize = 0;
}

void LPCMod_LCDBankString(char * string, unsigned char bankID){
    switch(bankID){
        case BNK512:
            if(LPCmodSettings.OSsettings.biosName512Bank[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosName512Bank);
            }
            else{
                sprintf(string, "%s", getSpecialSettingString(SpecialSettingsPtrArrayIndexName_ActiveBank, BNK512));
            }
            break;
        case BNK256:
            if(LPCmodSettings.OSsettings.biosName256Bank[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosName256Bank);
            }
            else{
                sprintf(string, "%s", getSpecialSettingString(SpecialSettingsPtrArrayIndexName_ActiveBank, BNK256));
            }
            break;
        case BNKTSOPSPLIT0:
        case BNKFULLTSOP:
            if(LPCmodSettings.OSsettings.biosNameTSOPFullSplit0[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosNameTSOPFullSplit0);
            }
            else{
                if(LPCmodSettings.OSsettings.TSOPcontrol)
                    sprintf(string, "%s", getSpecialSettingString(SpecialSettingsPtrArrayIndexName_ActiveBank, BNKTSOPSPLIT0));
                else
                    sprintf(string, "%s", getSpecialSettingString(SpecialSettingsPtrArrayIndexName_ActiveBank, BNKFULLTSOP));
            }
            break;
        case BNKTSOPSPLIT1:
            if(LPCmodSettings.OSsettings.biosNameTSOPSplit1[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosNameTSOPSplit1);
            }
            else{
                sprintf(string, "%s", getSpecialSettingString(SpecialSettingsPtrArrayIndexName_ActiveBank, BNKTSOPSPLIT1));
            }
            break;
         default:
         	sprintf(string, "%s", "Settings");
         	break;
    }
}

const char* getSpecialSettingString(unsigned char SpecialSettingindex, unsigned char value)
{
    switch(SpecialSettingindex)
    {
    case SpecialSettingsPtrArrayIndexName_ActiveBank:
    case SpecialSettingsPtrArrayIndexName_AltBank:
        switch(value)
        {
        case BNK512:
            return "512KB bank";
        case BNK256:
            return "256KB bank";
        case BNKTSOPSPLIT0:
            return "OnBoard Bank0";
        case BNKFULLTSOP:
            return "OnBoard BIOS";
        case BNKTSOPSPLIT1:
            return "OnBoard Bank1";
        case BNKOS:
            return "XBlast OS";
        }
        break;
    case SpecialSettingsPtrArrayIndexName_LEDColor:
        switch(value)
        {
        case LED_OFF:
            return "Off";
        case LED_GREEN:
            return "Green";
        case LED_RED:
            return "Red";
        case LED_ORANGE:
            return "Orange";
        case LED_CYCLE:
            return "Cycle";
        }
        break;
    case SpecialSettingsPtrArrayIndexName_LCDType:
        switch(value)
        {
        case LCDTYPE_HD44780:
            return "HD44780";
        case LCDTYPE_KS0073:
            return "KS0073";
        }
        break;
    }

    return NULL;
}

bool emergencyRecoverSettings(void)
{
    if(risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) &&
       risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) &&
       risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_X) &&
       risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_Y))
    {
        populateSettingsStructWithDefault(&LPCmodSettings);

        return true;
    }

    return false;
}
