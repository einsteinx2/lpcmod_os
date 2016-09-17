/*
 * Parts are from the Team-Assembly XKUtils thx.
 *
 */

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 
#ifndef _BootEEPROM_H_
#define _BootEEPROM_H_

#include "VideoInitialization.h"
#include <stdbool.h>
//Defines for Data structure sizes..
#define EEPROM_SIZE        0x100
#define CONFOUNDER_SIZE        0x008
#define HDDKEY_SIZE        0x010
#define XBEREGION_SIZE        0x001
#define SERIALNUMBER_SIZE    0x00C
#define MACADDRESS_SIZE        0x006
#define ONLINEKEY_SIZE        0x010
#define DVDREGION_SIZE        0x001
#define VIDEOSTANDARD_SIZE    0x004

//EEPROM Data struct value enum
typedef enum { 
    ZONE_NONE = 0x00,    //Region Clear
    ZONE1 = 0x01,        //USA
    ZONE2 = 0x02,        //Europe
    ZONE3 = 0x03,        //India
    ZONE4 = 0x04,        //Australia
    ZONE5 = 0x05,        //USSR
    ZONE6 = 0x06,        //China
    ZONE_FREE = 0x07,    //Free
    ZONE_AIRLINES = 0x08//Airlines
} DVD_ZONE;

typedef enum {
    VID_INVALID    = 0x00000000,
    NTSC_M        = 0x00400100,
    NTSC_J        = 0x00400200, 
    PAL_I        = 0x00800300
} VIDEO_STANDARD;

typedef enum {
    XBE_INVALID = 0x00,
    NORTH_AMERICA = 0x01,
    JAPAN = 0x02,
    EURO_AUSTRALIA = 0x04
} XBE_REGION;

typedef enum {
    FULLSCREEN = 0x00,
    WIDESCREEN = 0x01,
    LETTERBOX = 0x10
} VID_FORMAT;


typedef enum {
    R720p = 0x02,
    R1080i = 0x04,
    R480p = 0x08
}VID_RESOLUTION;

typedef enum {
    V1_0 = 0x0A,
    V1_1 = 0x0B,
    V1_6 = 0x0C
} EEPROM_VERSION;

//Structure that holds contents of 256 byte EEPROM image..
typedef struct _EEPROMDATA {
   unsigned char        HMAC_SHA1_Hash[20];            // 0x00 - 0x13 HMAC_SHA1 Hash
   unsigned char        Confounder[8];                // 0x14 - 0x1B RC4 Encrypted Confounder ??
   unsigned char        HDDKkey[16];                // 0x1C - 0x2B RC4 Encrypted HDD key
   unsigned char        XBERegion[4];                // 0x2C - 0x2F RC4 Encrypted Region code (0x01 North America, 0x02 Japan, 0x04 Europe)

   unsigned char        Checksum2[4];                // 0x30 - 0x33 Checksum of next 44 bytes
   unsigned char        SerialNumber[12];            // 0x34 - 0x3F Xbox serial number 
   unsigned char        MACAddress[6];                // 0x40 - 0x45 Ethernet MAC address
   unsigned char        UNKNOWN2[2];                    // 0x46 - 0x47  Unknown Padding ?

   unsigned char        OnlineKey[16];                // 0x48 - 0x57 Online Key ?

   unsigned char        VideoStandard[4];            // 0x58 - 0x5B  ** 0x00014000 = NTSC, 0x00038000 = PAL, 0x00400100 = NTSC_J

   unsigned char        UNKNOWN3[4];                    // 0x5C - 0x5F  Unknown Padding ?
  
   //Comes configured up to here from factory..  everything after this can be zero'd out...
   //To reset XBOX to Factory settings, Make checksum3 0xFFFFFFFF and zero all data below (0x64-0xFF)
   //Doing this will Reset XBOX and upon startup will get Language & Setup screen...
   unsigned char        Checksum3[4];                // 0x60 - 0x63  other Checksum of next  

   unsigned char        TimeZoneBias[4];            // 0x64 - 0x67 Zone Bias?
   unsigned char        TimeZoneStdName[4];            // 0x68 - 0x6B Standard timezone
   unsigned char        TimeZoneDltName[4];            // 0x5C - 0x6F Daylight timezone
   unsigned char        UNKNOWN4[8];                // 0x70 - 0x77 Unknown Padding ?
   unsigned char        TimeZoneStdDate[4];                // 0x78 - 0x7B 10-05-00-02 (Month-Day-DayOfWeek-Hour) 
   unsigned char        TimeZoneDltDate[4];                // 0x7C - 0x7F 04-01-00-02 (Month-Day-DayOfWeek-Hour) 
   unsigned char        UNKNOWN5[8];                // 0x80 - 0x87 Unknown Padding ?
   unsigned char        TimeZoneStdBias[4];            // 0x88 - 0x8B Standard Bias?
   unsigned char        TimeZoneDltBias[4];            // 0x8C - 0x8F Daylight Bias?

   unsigned char        LanguageID[4];                // 0x90 - 0x93 Language ID
  
   unsigned char        VideoFlags[4];                // 0x94 - 0x97 Video Settings - 0x96 b0==widescreen 0x96 b4 == letterbox
   unsigned char        AudioFlags[4];                // 0x98 - 0x9B Audio Settings
  
   unsigned char        ParentalControlGames[4];        // 0x9C - 0x9F 0=MAX rating
   unsigned char        ParentalControlPwd[4];            // 0xA0 - 0xA3 7=X, 8=Y, B=LTrigger, C=RTrigger
   unsigned char        ParentalControlMovies[4];           // 0xA4 - 0xA7 0=Max rating
  
   unsigned char        XBOXLiveIPAddress[4];            // 0xA8 - 0xAB XBOX Live IP Address..
   unsigned char        XBOXLiveDNS[4];                // 0xAC - 0xAF XBOX Live DNS Server..
   unsigned char        XBOXLiveGateWay[4];            // 0xB0 - 0xB3 XBOX Live Gateway Address..
   unsigned char        XBOXLiveSubNetMask[4];            // 0xB4 - 0xB7 XBOX Live Subnet Mask..
   unsigned char        OtherSettings[4];            // 0xA8 - 0xBB Other XBLive settings ?

   unsigned char        DVDPlaybackKitZone[4];            // 0xBC - 0xBF DVD Playback Kit Zone

   unsigned char        UNKNOWN6[64];                // 0xC0 - 0xFF Unknown Codes / Memory timing data ?
}__attribute__((packed)) EEPROMDATA;

EEPROMDATA eeprom;
EEPROMDATA origEeprom;
EEPROMDATA *editeeprom;

#define MAXEDITABLEPARAMSINEEPROM 10    //Modifiable fields in EEPROM by XBlast OS are:
                                        //HDDKkey (not yet)
                                        //XBERegion
                                        //SerialNumber (not yet)
                                        //MACAddress
                                        //VideoStandard
                                        //VideoFlags (Widescreen, Fullscreen, Letterbox)
                                        //VideoFlags (480p)
                                        //VideoFlags (720p)
                                        //VideoFlags (1080i)
                                        //DVDPlaybackKitZone

typedef enum {
    HDDKkey = 0,
    XBERegion = 1,
    //Below is checksum2
    SerialNumber = 2,
    MACAddress,
    VideoStandard,
    //Below is checksum3
    VideoFlags_Format,
    VideoFlags_480p,
    VideoFlags_720p,
    VideoFlags_1080i,
    DVDPlaybackKitZone
} EEPROM_MODIFIABLE_ITEMS;

bool eepromChangesFlag[MAXEDITABLEPARAMSINEEPROM];      //Maybe will be changed for something better
char *eepromChangesStringArray[MAXEDITABLEPARAMSINEEPROM];


//String enum for DVD_ZONE
static const char * const DVDregiontext[] = {
    "Region Clear",
    "USA (1)",
    "Europe (2)",
    "India (3)",
    "Australia (4)",
    "USSR (5)",
    "China (6)",
    "Free (7)",
    "Airlines (8)"
};

//String enum for XBE_REGSION
static const char * const Gameregiontext[] = {
    "Unknown/Error",
    "NTSC-U",
    "NTSC-J",
    "n/a",
    "PAL"
};

void BootEepromReadEntireEEPROM(void);
void BootEepromReloadEEPROM(EEPROMDATA * realeeprom);
void BootEepromCompareAndWriteEEPROM(EEPROMDATA * realeeprom);
void BootEepromPrintInfo(void);
void BootEepromWriteEntireEEPROM(void);
void EepromCRC(unsigned char *crc, unsigned char *data, long dataLen);
void EepromSetVideoStandard(VIDEO_STANDARD standard);
unsigned char decryptEEPROMData(unsigned char* eepromPtr, unsigned char* decryptedBuf);
unsigned char generateStringsForEEPROMChanges(bool genStrings);


void assertWriteEEPROM(void);
int getGameRegionValue(EEPROMDATA * eepromPtr);
int setGameRegionValue(unsigned char value);
#endif // _BootEEPROM_H_
