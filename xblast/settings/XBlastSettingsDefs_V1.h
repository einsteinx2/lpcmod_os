/*
 * XBlastSettingsDefs_v1.h
 *
 *  Created on: Dec 9, 2016
 *      Author: cromwelldev
 */

#ifndef XBLASTSETTINGSDEFS_V1_H_
#define XBLASTSETTINGSDEFS_V1_H_

#include "BootEEPROM.h"

#define _TextFieldsMaxLength 20

//Configuration parameters saved in flash
typedef struct {
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
    unsigned char    enableVGA;          //Display as per Frosty's VGA settings.
    unsigned char     backgroundColorPreset;
    char    biosName512Bank[_TextFieldsMaxLength + sizeof('\0')];        //512KB bank name. 20 characters max to properly display on LCD.
    char    biosName256Bank[_TextFieldsMaxLength + sizeof('\0')];        //256KB bank name
    char    biosNameTSOPFullSplit0[_TextFieldsMaxLength + sizeof('\0')];        //Reserved for future use.
    char    biosNameTSOPSplit1[_TextFieldsMaxLength + sizeof('\0')];        //Reserved for future use.
    unsigned char    enableNetwork;        //Future use. For now, network is enabled only by NetFlash or WebUpdate
    unsigned char    useDHCP;        //Self Explanatory
    unsigned char    staticIP[4];        //Only useful when useDHCP is set to false.
    unsigned char    staticGateway[4];    //Only useful when useDHCP is set to false.
    unsigned char    staticMask[4];       //Only useful when useDHCP is set to false.
    unsigned char    staticDNS1[4];        //Only useful when useDHCP is set to false.
    unsigned char    staticDNS2[4];        //Only useful when useDHCP is set to false.
    unsigned char    reserved[137];
}__attribute__((packed))_OSsettings_V1;                //For a total of 256 bytes

typedef struct {
    unsigned char enable5V;            //Flag to indicate if +5V rail should be enabled(for LCD power)
    unsigned char lcdType;            //HD44780, KS0073 only for now
    unsigned char nbLines;            //User puts 4, means 2 lines from HD44780 POV
    unsigned char lineLength;            //Should be 16 or 20 most of the time.
    unsigned char backlight;            //7-bit value
    unsigned char contrast;            //7-bit value
    unsigned char displayMsgBoot;        //Display text on LCD while booting
    unsigned char customTextBoot;        //Display custom text instead of default text.
    unsigned char displayBIOSNameBoot;        //Display BIOS name of active bank when booting
    char customString0[_TextFieldsMaxLength + sizeof('\0')];        //1 of 4 strings to be displayed either when in OS or while booting.
    char customString1[_TextFieldsMaxLength + sizeof('\0')];        //20 characters max to properly display on LCD.
    char customString2[_TextFieldsMaxLength + sizeof('\0')];
    char customString3[_TextFieldsMaxLength + sizeof('\0')];
    unsigned char reserved[163];
}__attribute__((packed))_LCDsettings_V1;                //For a total of 256 bytes

typedef unsigned char _SettingsVersion;
typedef unsigned int _CRC32SettingsValue;
typedef unsigned short _ScriptSize;

typedef struct {
    _ScriptSize scriptSize;
#define Flash4KBSectorSizeInBytes 4 * 1024
#define BIOSFooterReservedSpace 0x100
#define ScriptSavedInFlashMaxSizeInBytes Flash4KBSectorSizeInBytes \
                                         - BIOSFooterReservedSpace \
                                         - sizeof(_SettingsVersion) \
                                         - sizeof(_OSsettings_V1) \
                                         - sizeof(_LCDsettings_V1) \
                                         - sizeof(EEPROMDATA) \
                                         - sizeof(_ScriptSize) \
                                         - sizeof(_CRC32SettingsValue)

    unsigned char scriptData[ScriptSavedInFlashMaxSizeInBytes];
}__attribute__((packed))_scriptEntry_V1;


typedef struct {
    _SettingsVersion settingsVersion; //Must always be first field
    _OSsettings_V1 OSsettings;
    _LCDsettings_V1 LCDsettings;
    EEPROMDATA bakeeprom;
    _scriptEntry_V1 flashScript;
    _CRC32SettingsValue crc32Value;
}__attribute__((packed)) _LPCmodSettings_V1;

#define _SettingsMaxTextFieldsLength_V1  _TextFieldsMaxLength;


#endif /* XBLASTSETTINGSDEFS_V1_H_ */
