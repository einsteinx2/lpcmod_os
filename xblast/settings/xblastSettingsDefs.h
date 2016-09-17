/*
 * xblastSettingsDefs.h
 *
 *  Created on: Aug 11, 2016
 *      Author: cromwelldev
 */

#ifndef XBLASTSETTINGSDEFS_H_
#define XBLASTSETTINGSDEFS_H_

#include "BootEEPROM.h"

#define DEFAULT_FANSPEED    20

//Configuration parameters saved in flash
typedef struct _OSsettings {
    unsigned char    migrateSetttings;    //Flag to indicate if settings in this struct should be carried over a OS update.
    unsigned char    reserved[15];
    unsigned char    activeBank;        //Default Flash bank to load BIOS from.
    unsigned char    altBank;          //Alternative BIOS bank to boot holding black button
    unsigned char    Quickboot;        //Bypass OS and load selected bank in "activeBank"
    unsigned char    selectedMenuItem;    //Default selected item in OS menu when booting into it.
    unsigned char    fanSpeed;        //Why not
    unsigned char    bootTimeout;
    unsigned char    LEDColor;
    unsigned char    TSOPcontrol;        //variable contains the following settings: bit0=active
    unsigned char    TSOPhide;           //Hide boot from TSOP option in icon menu when set.
    unsigned char    runBankScript;      //Will execute script at BIOS bank boot.
    unsigned char    runBootScript;      //Will execute script at OS boot.
    unsigned char	  backgroundColorPreset;
    unsigned char    reserved1[8];
    char    biosName0[21];        //512KB bank name. 20 characters max to properly display on LCD.
    char    biosName1[21];        //256KB bank name
    char    biosName2[21];        //Reserved for future use.
    char    biosName3[21];        //Reserved for future use.
    char    biosName4[21];        //Reserved for future use.
    char    biosName5[21];        //Reserved for future use.
    char    biosName6[21];        //Reserved for future use.
    char    biosName7[21];        //Reserved for future use.
    unsigned char    reserved2[30];
    unsigned char    enableNetwork;        //Future use. For now, network is enabled only by NetFlash or WebUpdate
    unsigned char    useDHCP;        //Self Explanatory
    unsigned char    staticIP[4];        //Only useful when useDHCP is set to false.
    unsigned char    staticGateway[4];    //Only useful when useDHCP is set to false.
    unsigned char    staticMask[4];       //Only useful when useDHCP is set to false.
    unsigned char    staticDNS1[4];        //Only useful when useDHCP is set to false.
    unsigned char    staticDNS2[4];        //Only useful when useDHCP is set to false.
}__attribute__((packed))_OSsettings;                //For a total of 256 bytes

typedef struct _LCDsettings {
    unsigned char migrateLCD;            //Flag to indicate if settings in this struct should be carried over a OS update.
    unsigned char enable5V;            //Flag to indicate if +5V rail should be enabled(for LCD power)
    unsigned char lcdType;            //HD44780, KS0073 only for now
    unsigned char nbLines;            //User puts 4, means 2 lines from HD44780 POV
    unsigned char lineLength;            //Should be 16 or 20 most of the time.
    unsigned char backlight;            //7-bit value
    unsigned char contrast;            //7-bit value
    unsigned char displayMsgBoot;        //Display text on LCD while booting
    unsigned char customTextBoot;        //Display custom text instead of default text.
    unsigned char displayBIOSNameBoot;        //Display BIOS name of active bank when booting
    unsigned char reserved0[5];
    char customString0[21];        //1 of 4 strings to be displayed either when in OS or while booting.
    char customString1[21];        //20 characters max to properly display on LCD.
    char customString2[21];
    char customString3[21];
    unsigned char reserved1[157];
}__attribute__((packed))_LCDsettings;                //For a total of 256 bytes

typedef struct _scriptEntry {
    unsigned short ScripMagicNumber;       //Must be set to 0xFAF* , * is script number, starting at 1.
    unsigned short nextEntryPosition;      //Relative position from 0x3f00 in flash. 0 means no other saved script in flash.
}__attribute__((packed))_scriptEntry;                //For a total of 4 bytes

typedef struct _LPCmodSettings {
    _OSsettings OSsettings;
    _LCDsettings LCDsettings;
    EEPROMDATA bakeeprom;
    _scriptEntry firstScript;
}__attribute__((packed)) _LPCmodSettings;	//For a total size of 0x300.


// Globals
_LPCmodSettings LPCmodSettings;
_LPCmodSettings LPCmodSettingsOrigFromFlash;
_scriptEntry *scriptEntryList;




#define NBTXTPARAMS 35
#define MINPARAMLENGTH 7
#define IPTEXTPARAMGROUP 18
#define TEXTPARAMGROUP (IPTEXTPARAMGROUP + 5)
#define SPECIALPARAMGROUP (TEXTPARAMGROUP + 8)

#define NBBOOLEANPARAMS 11
#define NBNUMERICVALUEPARAMS (IPTEXTPARAMGROUP - NBBOOLEANPARAMS)

typedef struct {
    unsigned char *settingsPtrArray[IPTEXTPARAMGROUP];
    unsigned char *IPsettingsPtrArray[TEXTPARAMGROUP-IPTEXTPARAMGROUP];
    char *textSettingsPtrArray[SPECIALPARAMGROUP - TEXTPARAMGROUP];
    unsigned char *specialCasePtrArray[NBTXTPARAMS - SPECIALPARAMGROUP];
}_settingsPtrStruct;

_settingsPtrStruct settingsPtrStruct;

const char *xblastcfgstrings[NBTXTPARAMS];

#endif /* XBLASTSETTINGSDEFS_H_ */
