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
#include "stdio.h"
#include "boot.h"

// Globals
_LPCmodSettings LPCmodSettings;
_LPCmodSettings LPCmodSettingsOrigFromFlash;
const unsigned char LPCmodSettingsTextFieldsMaxLength = _SettingsMaxTextFieldsLength;

#define HDD4780_DEFAULT_NBLINES    4
#define HDD4780_DEFAULT_LINELGTH    20

#define DEFAULT_FANSPEED    20


// Privates
static const char* const szText_BNK512 = "BNK512";
static const char* const szText_BNK256 = "BNK256";
static const char* const szText_BNKTSOPSPLIT0 = "BNKTSOPSPLIT0";
static const char* const szText_BNKFULLTSOP = "BNKFULLTSOP";
static const char* const szText_BNKTSOPSPLIT1 = "BNKTSOPSPLIT1";
static const char* const szText_BNKOS = "BNKOS";

static const char* const szDisplay_BNK512 = "512KB bank";
static const char* const szDisplay_BNK256 = "256KB bank";
static const char* const szDisplay_BNKTSOPSPLIT0 = "OnBoard Bank0";
static const char* const szDisplay_BNKFULLTSOP = "OnBoard BIOS";
static const char* const szDisplay_BNKTSOPSPLIT1 = "OnBoard Bank1";
static const char* const szDisplay_BNKOS = "XBlast OS";

static const char* const szText_LED_OFF = "Off";
static const char* const szText_LED_GREEN = "Green";
static const char* const szText_LED_RED = "Red";
static const char* const szText_LED_ORANGE = "Orange";
static const char* const szText_LED_CYCLE = "Cycle";

static const char* const szText_LCDTYPE_HD44780 = "HD44780";
static const char* const szText_LCDTYPE_KS0073 = "KS0073";
static const char* const getSpecialSettingStringCommon(unsigned char SpecialSettingindex, unsigned char value);



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

void LPCMod_LCDBankString(char * string, unsigned char bankID)
{
    switch(bankID)
    {
        case BNK512:
            if('\0' != LPCmodSettings.OSsettings.biosName512Bank[0])
            {
                strcpy(string, LPCmodSettings.OSsettings.biosName512Bank);
            }
            else
            {
                strcpy(string, getSpecialSettingDisplayString(SpecialSettingsPtrArrayIndexName_ActiveBank, BNK512));
            }
            break;
        case BNK256:
            if('\0' != LPCmodSettings.OSsettings.biosName256Bank[0])
            {
                strcpy(string, LPCmodSettings.OSsettings.biosName256Bank);
            }
            else
            {
                strcpy(string, getSpecialSettingDisplayString(SpecialSettingsPtrArrayIndexName_ActiveBank, BNK256));
            }
            break;
        case BNKTSOPSPLIT0:
        case BNKFULLTSOP:
            if('\0' != LPCmodSettings.OSsettings.biosNameTSOPFullSplit0[0])
            {
                strcpy(string, LPCmodSettings.OSsettings.biosNameTSOPFullSplit0);
            }
            else
            {
                if(LPCmodSettings.OSsettings.TSOPcontrol)
                {
                    strcpy(string, getSpecialSettingDisplayString(SpecialSettingsPtrArrayIndexName_ActiveBank, BNKTSOPSPLIT0));
                }
                else
                {
                    strcpy(string, getSpecialSettingDisplayString(SpecialSettingsPtrArrayIndexName_ActiveBank, BNKFULLTSOP));
                }
            }
            break;
        case BNKTSOPSPLIT1:
            if('\0' != LPCmodSettings.OSsettings.biosNameTSOPSplit1[0])
            {
                strcpy(string, LPCmodSettings.OSsettings.biosNameTSOPSplit1);
            }
            else
            {
                strcpy(string, getSpecialSettingDisplayString(SpecialSettingsPtrArrayIndexName_ActiveBank, BNKTSOPSPLIT1));
            }
            break;
         default:
             strcpy(string, "Settings");
         	break;
    }
}

const char* const getSpecialSettingTextString(unsigned char SpecialSettingindex, unsigned char value)
{
    switch(SpecialSettingindex)
    {
    case SpecialSettingsPtrArrayIndexName_ActiveBank:
    case SpecialSettingsPtrArrayIndexName_AltBank:
        switch(value)
        {
        case BNK512:
            return szText_BNK512;
        case BNK256:
            return szText_BNK256;
        case BNKTSOPSPLIT0:
            return szText_BNKTSOPSPLIT0;
        case BNKFULLTSOP:
            return szText_BNKFULLTSOP;
        case BNKTSOPSPLIT1:
            return szText_BNKTSOPSPLIT1;
        case BNKOS:
            return szText_BNKOS;
        }
        break;
    default:
        return getSpecialSettingStringCommon(SpecialSettingindex, value);
    break;
    }

    return NULL;
}

const char* const getSpecialSettingDisplayString(unsigned char SpecialSettingindex, unsigned char value)
{
    switch(SpecialSettingindex)
    {
    case SpecialSettingsPtrArrayIndexName_ActiveBank:
    case SpecialSettingsPtrArrayIndexName_AltBank:
        switch(value)
        {
        case BNK512:
            return szDisplay_BNK512;
        case BNK256:
            return szDisplay_BNK256;
        case BNKTSOPSPLIT0:
            return szDisplay_BNKTSOPSPLIT0;
        case BNKFULLTSOP:
            return szDisplay_BNKFULLTSOP;
        case BNKTSOPSPLIT1:
            return szDisplay_BNKTSOPSPLIT1;
        case BNKOS:
            return szDisplay_BNKOS;
        }
        break;
    default:
        return getSpecialSettingStringCommon(SpecialSettingindex, value);
    break;
    }

    return NULL;
}

static const char* const getSpecialSettingStringCommon(unsigned char SpecialSettingindex, unsigned char value)
{
    switch(SpecialSettingindex)
        {
        case SpecialSettingsPtrArrayIndexName_LEDColor:
            switch(value)
            {
            case LED_OFF:
                return szText_LED_OFF;
            case LED_GREEN:
                return szText_LED_GREEN;
            case LED_RED:
                return szText_LED_RED;
            case LED_ORANGE:
                return szText_LED_ORANGE;
            case LED_CYCLE:
                return szText_LED_CYCLE;
            }
            break;
        case SpecialSettingsPtrArrayIndexName_LCDType:
            switch(value)
            {
            case LCDTYPE_HD44780:
                return szText_LCDTYPE_HD44780;
            case LCDTYPE_KS0073:
                return szText_LCDTYPE_KS0073;
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
