/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"
#include "BootHddKey.h"
#include "BootEEPROM.h"
#include "i2c.h"
#include "rc4.h"
#include "cromwell.h"
#include "lib/LPCMod/xblastDebug.h"
#include "xblast/HardwareIdentifier.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "RecoveryImages.h"
#include "EEPROMStrings.h"

static void putNewChangeInList(EEPROMChangeList* list, EEPROMChangeEntry_t* change);

void BootEepromReadEntireEEPROM()
{
    int i;
    unsigned char *pb=(unsigned char *)&eeprom;
    for(i = 0; i < sizeof(EEPROMDATA); i++)
    {
        *pb++ = I2CTransmitByteGetReturn(0x54, i);
    }
}

void eepromChangeTrackerInit(void)
{
    eepromChangeList.changeCount = 0;
    eepromChangeList.firstChangeEntry = NULL;
}

void BootEepromReloadEEPROM(EEPROMDATA * realeeprom)
{
    memcpy(realeeprom, &origEeprom, sizeof(EEPROMDATA));
}

void BootEepromCompareAndWriteEEPROM(EEPROMDATA * realeeprom)
{
    int i;
    unsigned char *pb = (unsigned char *)&eeprom;
    unsigned char *pc = (unsigned char *)realeeprom;
    for(i = 0; i < sizeof(EEPROMDATA); i++)
    {
        if(pb[i] != pc[i])                //Compare byte by byte.
        {
            WriteToSMBus(0x54,i,1,pb[i]);          //Physical EEPROM's content is different from what's held in memory.
        }
    }
}

void BootEepromPrintInfo()
{
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n           MAC : ");
    VIDEO_ATTR=0xffc8c800;
    printk("%02X%02X%02X%02X%02X%02X  ",
        eeprom.MACAddress[0], eeprom.MACAddress[1], eeprom.MACAddress[2],
        eeprom.MACAddress[3], eeprom.MACAddress[4], eeprom.MACAddress[5]
    );

    VIDEO_ATTR=0xffc8c8c8;
    printk("Vid: ");
    VIDEO_ATTR=0xffc8c800;

    switch(*((EEPROM_VideoStandard *)&eeprom.VideoStandard))
    {
        case EEPROM_VideoStandardInvalid:
            printk(Gameregiontext[EEPROM_XBERegionInvalid]);
            break;
        case EEPROM_VideoStandardNTSC_M:
            printk(Gameregiontext[EEPROM_XBERegionNorthAmerica]);
            break;
        case EEPROM_VideoStandardNTSC_J:
            printk(Gameregiontext[EEPROM_XBERegionJapan]);
            break;
        case EEPROM_VideoStandardPAL_I:
            printk(Gameregiontext[EEPROM_XBERegionEuropeAustralia]);
            break;
        default:
            printk("%08X", (int)*((EEPROM_VideoStandard *)&eeprom.VideoStandard));
            break;
    }

    VIDEO_ATTR=0xffc8c8c8;
    printk("  Serial: ");
    VIDEO_ATTR=0xffc8c800;
    
    {
        char sz[13];
        memcpy(sz, &eeprom.SerialNumber[0], 12);
        sz[12]='\0';
        printk(" %s", sz);
    }

    printk("\n");
    VIDEO_ATTR=0xffc8c8c8;
}

void BootEepromWriteEntireEEPROM(void)
{
    int i;
    unsigned char *pb=(unsigned char *)&eeprom;

    for(i = 0; i < sizeof(EEPROMDATA); i++)
    {
        WriteToSMBus(0x54,i,1,pb[i]);
    }
}

/* The EepromCRC algorithm was obtained from the XKUtils 0.2 source released by 
 * TeamAssembly under the GNU GPL.  
 * Specifically, from XKCRC.cpp 
 *
 * Rewritten to ANSI C by David Pye (dmp@davidmpye.dyndns.org)
 *
 * Thanks! */
void EepromCRC(unsigned char *crc, unsigned char *data, long dataLen)
{
    unsigned char CRC_Data[0x5b + 4];
    int pos=0;
    memset(crc,0x00,4);

    memset(CRC_Data,0x00, dataLen+4);
    //Circle shift input data one byte right
    memcpy(CRC_Data + 0x01 , data, dataLen-1);
    memcpy(CRC_Data, data + dataLen-1, 0x01);

    for (pos=0; pos<4; ++pos)
    {
        unsigned short CRCPosVal = 0xFFFF;
        unsigned long l;
        for (l=pos; l<dataLen; l+=4)
        {
            CRCPosVal -= *(unsigned short*)(&CRC_Data[l]);
        }
        CRCPosVal &= 0xFF00;
        crc[pos] = (unsigned char) (CRCPosVal >> 8);
    }
}

void EepromSetVideoStandard(EEPROM_VideoStandard standard)
{
    //Changing this setting requires that Checksum2
    //be recalculated.

    memcpy(eeprom.VideoStandard, &standard, 0x04);
    EepromCRC(eeprom.Checksum2,eeprom.SerialNumber,0x28);
}

void EepromSetVideoFormat(EEPROM_VidScreenFormat format)
{
    eeprom.VideoFlags[2] &= ~(EEPROM_VidScreenFullScreen | EEPROM_VidScreenWidescreen | EEPROM_VidScreenLetterbox);
    eeprom.VideoFlags[2] |= format;
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void assertWriteEEPROM(void)
{
    EEPROMDATA realeeprom;
    BootEepromReloadEEPROM(&realeeprom);
    BootEepromCompareAndWriteEEPROM(&realeeprom);    //If there is at least one change that requires to write back to physical EEPROM. This function will write it.
}

int getGameRegionValue(EEPROMDATA * eepromPtr)
{
    int result = -1;
    unsigned char baEepromDataLocalCopy[0x30];
    int version = 0;
    unsigned int gameRegion=0;

    version = decryptEEPROMData((unsigned char *)eepromPtr, baEepromDataLocalCopy);

    if(version > EEPROM_EncryptV1_6)
    {
        result = EEPROM_XBERegionInvalid;
    }
    else
    {
        memcpy(&gameRegion,&baEepromDataLocalCopy[28+16],4);
        if(gameRegion == EEPROM_XBERegionJapan)
        {
            result = EEPROM_XBERegionJapan;
        }
        else if(gameRegion == EEPROM_XBERegionEuropeAustralia)
        {
            result = EEPROM_XBERegionEuropeAustralia;
        }
        else if(gameRegion == EEPROM_XBERegionNorthAmerica)
        {
            result = EEPROM_XBERegionNorthAmerica;
        }
        else
        {
            result = EEPROM_XBERegionInvalid;
        }
    }
    return result;
}

int setGameRegionValue(unsigned char value)
{
    int result = -1;
    unsigned char baKeyHash[20];
    unsigned char baDataHashConfirm[20];
    unsigned char baEepromDataLocalCopy[0x30];
    struct rc4_key RC4_key;
    int version = 0;
    int counter;
    unsigned int gameRegion = value;

    version = decryptEEPROMData((unsigned char *)&eeprom, baEepromDataLocalCopy);

    if (version > EEPROM_EncryptV1_6)
    {
        return (-1);    //error, let's not do something stupid here. Leave with dignity.
    }

    //else we know the version
    memcpy(&baEepromDataLocalCopy[28+16],&gameRegion,4);
    
    encryptEEPROMData(baEepromDataLocalCopy, &eeprom, version);

    //Everything went well, return new gameRegion.
    return gameRegion;

}

EEPROM_EncryptVersion EepromSanityCheck(EEPROMDATA * eepromPtr)
{
    EEPROMDATA decryptBuf;
    EEPROM_EncryptVersion version = decryptEEPROMData((unsigned char *)eepromPtr, (unsigned char *)&decryptBuf);
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_EEPROM_DRIVER, "Encrypt version = %u", version);
    if(version >= EEPROM_EncryptV1_0 && version <= EEPROM_EncryptV1_6)
    {
        unsigned int fourBytesParam = *(unsigned int *)(decryptBuf.XBERegion);
        XBlastLogger(DBG_LVL_DEBUG, DEBUG_EEPROM_DRIVER, "XBE Region = %u", fourBytesParam);
        if(fourBytesParam != EEPROM_XBERegionEuropeAustralia &&
           fourBytesParam != EEPROM_XBERegionJapan &&
           fourBytesParam != EEPROM_XBERegionNorthAmerica)
        {
            return EEPROM_EncryptInvalid;
        }

        fourBytesParam = *(unsigned int *)(eepromPtr->VideoStandard);
        XBlastLogger(DBG_LVL_DEBUG, DEBUG_EEPROM_DRIVER, "Video Standard = %08X", fourBytesParam);
        if(fourBytesParam != EEPROM_VideoStandardNTSC_J &&
           fourBytesParam != EEPROM_VideoStandardNTSC_M &&
           fourBytesParam != EEPROM_VideoStandardPAL_I)
        {
            return EEPROM_EncryptInvalid;
        }

        XBlastLogger(DBG_LVL_DEBUG, DEBUG_EEPROM_DRIVER, "Video flags = %02X %02X %02X %02X", eepromPtr->VideoFlags[0],
                                                             eepromPtr->VideoFlags[1],
                                                             eepromPtr->VideoFlags[2],
                                                             eepromPtr->VideoFlags[3]);

        if(eepromPtr->VideoFlags[2] >= (EEPROM_VidResolutionEnable720p |
                                        EEPROM_VidResolutionEnable1080i |
                                        EEPROM_VidResolutionEnable480p |
                                        EEPROM_VidScreenFullScreen |
                                        EEPROM_VidScreenWidescreen |
                                        EEPROM_VidScreenLetterbox))
        {
            return EEPROM_EncryptInvalid;
        }

        XBlastLogger(DBG_LVL_DEBUG, DEBUG_EEPROM_DRIVER, "DVD playback zone = %02X %02X %02X %02X", eepromPtr->DVDPlaybackKitZone[0],
                                                                   eepromPtr->DVDPlaybackKitZone[1],
                                                                   eepromPtr->DVDPlaybackKitZone[2],
                                                                   eepromPtr->DVDPlaybackKitZone[3]);
        if(eepromPtr->DVDPlaybackKitZone[1] != 0 || eepromPtr->DVDPlaybackKitZone[2] != 0 || eepromPtr->DVDPlaybackKitZone[3] != 0)
        {
            return EEPROM_EncryptInvalid;
        }

        if(eepromPtr->DVDPlaybackKitZone[0] > EEPROM_DVDRegionAirlines)
        {
            return EEPROM_EncryptInvalid;
        }

        return version;
    }

    return EEPROM_EncryptInvalid;
}

EEPROM_EncryptVersion decryptEEPROMData(unsigned char* eepromPtr, unsigned char* decryptedBuf)
{
   struct rc4_key RC4_key;
   EEPROM_EncryptVersion version = EEPROM_EncryptInvalid;
   EEPROM_EncryptVersion counter;
   unsigned char baEepromDataLocalCopy[0x30];
   unsigned char baKeyHash[20];
   unsigned char baDataHashConfirm[20];

    // Static Version change not included yet

    for (counter=EEPROM_EncryptV1_0;counter<=EEPROM_EncryptV1_6;counter++)
    {
        memset(&RC4_key,0,sizeof(rc4_key));
        memcpy(&baEepromDataLocalCopy[0], eepromPtr, 0x30);

                // Calculate the Key-Hash
        HMAC_hdd_calculation(counter, baKeyHash, &baEepromDataLocalCopy[0], 20, NULL);

        //initialize RC4 key
        rc4_prepare_key(baKeyHash, 20, &RC4_key);

            //decrypt data (from eeprom) with generated key
        rc4_crypt(&baEepromDataLocalCopy[20],28,&RC4_key);        //confounder, HDDkey and game region decryption in single block
        //rc4_crypt(&baEepromDataLocalCopy[28],20,&RC4_key);        //"real" data

                // Calculate the Confirm-Hash
        HMAC_hdd_calculation(counter, baDataHashConfirm, &baEepromDataLocalCopy[20], 8, &baEepromDataLocalCopy[28], 20, NULL);

        if (!memcmp(baEepromDataLocalCopy,baDataHashConfirm,0x14))
        {
            // Confirm Hash is correct
            // Copy actual Xbox Version to Return Value
            version=counter;
            // exits the loop
            break;
        }
    }

    //copy out decrypted Confounder, HDDKey and gameregion.
    if(decryptedBuf != NULL)
    {
        memcpy(decryptedBuf, baEepromDataLocalCopy, 0x30);
    }

    XBlastLogger(DBG_LVL_INFO, DEBUG_EEPROM_DRIVER, "EEPROM decrypt %s!!   Version value : %u", counter > EEPROM_EncryptV1_6 ? "failure" : "success", version);

    return version;
}

void encryptEEPROMData(unsigned char decryptedInput[0x30], EEPROMDATA * targetEEPROMPtr, EEPROM_EncryptVersion targetVersion)
{
    unsigned char baKeyHash[20];
    unsigned char baDataHashConfirm[20];
    struct rc4_key RC4_key;

    memset(&RC4_key,0,sizeof(rc4_key));
    memset(baKeyHash,0,20);
    memset(baDataHashConfirm,0,20);

    // Calculate the Confirm-Hash
    HMAC_hdd_calculation(targetVersion, baDataHashConfirm, &decryptedInput[20], 8, &decryptedInput[28], 20, NULL);

    memcpy(decryptedInput,baDataHashConfirm,20);

    // Calculate the Key-Hash
    HMAC_hdd_calculation(targetVersion, baKeyHash, &decryptedInput[0], 20, NULL);

    //initialize RC4 key
    rc4_prepare_key(baKeyHash, 20, &RC4_key);

    //decrypt data (from eeprom) with generated key
    rc4_crypt(&decryptedInput[20],28,&RC4_key);

    // Save back to EEprom
    memcpy(targetEEPROMPtr, &decryptedInput[0], 0x30);
}

const EEPROMDATA* getRecoveryImage(void)
{
    XboxMotherboardRevision rev = getMotherboardRevision();

    switch(rev)
    {
    case XboxMotherboardRevision_1_0:
        return EEPROMimg10;
    case XboxMotherboardRevision_1_1:
    case XboxMotherboardRevision_1_2:
    case XboxMotherboardRevision_1_4:
        return EEPROMimg11;
/*    case REV1_2:
        return EEPROMimg12;
    case REV1_4:
        return EEPROMimg14;*/
    case XboxMotherboardRevision_1_6:
        return EEPROMimg16;
    default:
        break;
    }
    return NULL;
}

unsigned char generateEEPROMChangeList(bool genStrings, EEPROMChangeList* out){
    unsigned int tempOrigChecksum2 = 0, tempOrigChecksum3 = 0, tempChecksum2 = 0, tempChecksum3 = 0;
    int origGameRegion, gameRegion;
    unsigned char i, nbChanges = 0, stringLength;
    char origTempItemString[35], tempItemString[35];
    unsigned char origHDDKey[16], HDDKey[16];
    EEPROMChangeEntry_t* currentChangeEntry;

    for(i = 0; i < 4; i++)
    {
        tempChecksum2 |= eeprom.Checksum2[i] << (i * 8);
        tempChecksum3 |= eeprom.Checksum3[i] << (i * 8);

        tempOrigChecksum2 |= origEeprom.Checksum2[i] << (i * 8);
        tempOrigChecksum3 |= origEeprom.Checksum3[i] << (i * 8);
    }

    for(i = 0; i < 20; i++)
    {
    	if(origEeprom.HMAC_SHA1_Hash[i] != eeprom.HMAC_SHA1_Hash[i])
    	{
    		break;
    	}
    }

    if(i < 20) //Encrypted section has changed
    {
    	BootHddKeyGenerateEepromKeyData((unsigned char *)&origEeprom, origHDDKey);
    	BootHddKeyGenerateEepromKeyData((unsigned char *)&eeprom, HDDKey);
    	for(i = 0; i < 16; i++)
    	{
            if(origHDDKey[i] != HDDKey[i])
            {
                if(genStrings)
                {
                    for(stringLength = 0; stringLength < 16; stringLength++)
                    {
                            sprintf(&origTempItemString[i << 1], "%02X", origHDDKey[i]);
                            sprintf(&tempItemString[i << 1], "%02X", HDDKey[i]);
                    }

                    currentChangeEntry = calloc(1, sizeof(EEPROMChangeEntry_t));
                    currentChangeEntry->eepromModItem = EEPROMModItem_HDDKkey;
                    currentChangeEntry->label = SettingChangeLabel[currentChangeEntry->eepromModItem];
                    sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", origTempItemString, tempItemString);

                    putNewChangeInList(&eepromChangeList, currentChangeEntry);
                }
                nbChanges += 1;
                break;
            }
        }

    	origGameRegion = getGameRegionValue(&origEeprom);
    	gameRegion = getGameRegionValue(&eeprom);

    	if(origGameRegion != gameRegion)
    	{
            if(genStrings)
            {
                currentChangeEntry = calloc(1, sizeof(EEPROMChangeEntry_t));
                currentChangeEntry->eepromModItem = EEPROMModItem_XBERegion;
                currentChangeEntry->label = SettingChangeLabel[currentChangeEntry->eepromModItem];
                sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", Gameregiontext[origGameRegion], Gameregiontext[gameRegion]);

                putNewChangeInList(&eepromChangeList, currentChangeEntry);
    	    }
            nbChanges += 1;
    	}
    }

    if(tempChecksum2 != tempOrigChecksum2) //Change in either Serial number, Video standard and/or MAC Address
    {
        //check serial number
        for(i = 0; i < 12; i++)
        {
            if(origEeprom.SerialNumber[i] != eeprom.SerialNumber[i])
            {
                if(genStrings)
                {
                    currentChangeEntry = calloc(1, sizeof(EEPROMChangeEntry_t));
                    currentChangeEntry->eepromModItem = EEPROMModItem_SerialNumber;
                    currentChangeEntry->label = SettingChangeLabel[currentChangeEntry->eepromModItem];
                    sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", origTempItemString, tempItemString);

                    putNewChangeInList(&eepromChangeList, currentChangeEntry);
                }
                nbChanges += 1;
                break;
            }
        }

        //check MAC address
        for(i = 0; i < 6; i++)
        {
            if(origEeprom.MACAddress[i] != eeprom.MACAddress[i])
            {
                if(genStrings)
                {
                    currentChangeEntry = calloc(1, sizeof(EEPROMChangeEntry_t));
                    currentChangeEntry->eepromModItem = EEPROMModItem_MACAddress;
                    currentChangeEntry->label = SettingChangeLabel[currentChangeEntry->eepromModItem];
                    sprintf(currentChangeEntry->changeString, "\"%02X:%02X:%02X:%02X:%02X:%02X\" -> \"%02X:%02X:%02X:%02X:%02X:%02X\"",
                                                origEeprom.MACAddress[0], origEeprom.MACAddress[1], origEeprom.MACAddress[2],
                                                origEeprom.MACAddress[3], origEeprom.MACAddress[4], origEeprom.MACAddress[5],
                                                eeprom.MACAddress[0], eeprom.MACAddress[1], eeprom.MACAddress[2],
                                                eeprom.MACAddress[3], eeprom.MACAddress[4], eeprom.MACAddress[5]);

                    putNewChangeInList(&eepromChangeList, currentChangeEntry);
                }
                nbChanges += 1;
                break;
            }
        }

        if(*((EEPROM_VideoStandard *)&origEeprom.VideoStandard) != *((EEPROM_VideoStandard *)&eeprom.VideoStandard))
        {
            if(genStrings)
            {
                currentChangeEntry = calloc(1, sizeof(EEPROMChangeEntry_t));
                currentChangeEntry->eepromModItem = EEPROMModItem_VideoStandard;
                currentChangeEntry->label = SettingChangeLabel[currentChangeEntry->eepromModItem];
                sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", getVideoStandardText(*((EEPROM_VideoStandard *)&origEeprom.VideoStandard)), getVideoStandardText(*((EEPROM_VideoStandard *)&eeprom.VideoStandard)));

                putNewChangeInList(&eepromChangeList, currentChangeEntry);
            }
            nbChanges += 1;
        }
    }

    if(tempChecksum3 != tempOrigChecksum3) //Change in either Video format and/or DVD region
    {
        if(tempChecksum3 == 0xFFFFFFFF && tempOrigChecksum3 != 0xFFFFFFFF)
        {
            nbChanges += 1;
        }
        else
        {
            //List individual changes
            if((origEeprom.VideoFlags[2] & 0x11) != (eeprom.VideoFlags[2] & 0x11)) //Video format mask
            {
                if(genStrings)
                {
                    EEPROM_VidScreenFormat origFormat;
                    EEPROM_VidScreenFormat newFormat;
                    if(origEeprom.VideoFlags[2] & EEPROM_VidScreenWidescreen)
                    {
                        origFormat = EEPROM_VidScreenWidescreen;
                    }
                    else if(origEeprom.VideoFlags[2] & EEPROM_VidScreenLetterbox)
                    {
                        origFormat = EEPROM_VidScreenLetterbox;
                    }
                    else
                    {
                        origFormat = EEPROM_VidScreenFullScreen;
                    }

                    if(eeprom.VideoFlags[2] & EEPROM_VidScreenWidescreen)
                    {
                        newFormat = EEPROM_VidScreenWidescreen;
                    }
                    else if(eeprom.VideoFlags[2] & EEPROM_VidScreenLetterbox)
                    {
                        newFormat = EEPROM_VidScreenLetterbox;
                    }
                    else
                    {
                        newFormat = EEPROM_VidScreenFullScreen;
                    }

                    currentChangeEntry = calloc(1, sizeof(EEPROMChangeEntry_t));
                    currentChangeEntry->eepromModItem = EEPROMModItem_VideoFlags_Format;
                    currentChangeEntry->label = SettingChangeLabel[currentChangeEntry->eepromModItem];
                    sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", getScreenFormatText(origFormat), getScreenFormatText(newFormat));

                    putNewChangeInList(&eepromChangeList, currentChangeEntry);
                }
                nbChanges += 1;
            }

            if((origEeprom.VideoFlags[2] & EEPROM_VidResolutionEnable480p) != (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable480p))
            {
                if(genStrings)
                {
                    currentChangeEntry = calloc(1, sizeof(EEPROMChangeEntry_t));
                    currentChangeEntry->eepromModItem = EEPROMModItem_VideoFlags_480p;
                    currentChangeEntry->label = SettingChangeLabel[currentChangeEntry->eepromModItem];
                    sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", (origEeprom.VideoFlags[2] & EEPROM_VidResolutionEnable480p) ? "Yes" : "No", (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable480p) ? "Yes" : "No");

                    putNewChangeInList(&eepromChangeList, currentChangeEntry);
                }
                nbChanges += 1;
            }

            if((origEeprom.VideoFlags[2] & EEPROM_VidResolutionEnable720p) != (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable720p))
            {
                if(genStrings)
                {
                    currentChangeEntry = calloc(1, sizeof(EEPROMChangeEntry_t));
                    currentChangeEntry->eepromModItem = EEPROMModItem_VideoFlags_720p;
                    currentChangeEntry->label = SettingChangeLabel[currentChangeEntry->eepromModItem];
                    sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", (origEeprom.VideoFlags[2] & EEPROM_VidResolutionEnable720p) ? "Yes" : "No", (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable720p) ? "Yes" : "No");

                    putNewChangeInList(&eepromChangeList, currentChangeEntry);
                }
                nbChanges += 1;
            }

            if((origEeprom.VideoFlags[2] & EEPROM_VidResolutionEnable1080i) != (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable1080i))
            {
                if(genStrings)
                {
                    currentChangeEntry = calloc(1, sizeof(EEPROMChangeEntry_t));
                    currentChangeEntry->eepromModItem = EEPROMModItem_VideoFlags_1080i;
                    currentChangeEntry->label = SettingChangeLabel[currentChangeEntry->eepromModItem];
                    sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", (origEeprom.VideoFlags[2] & EEPROM_VidResolutionEnable1080i) ? "Yes" : "No", (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable1080i) ? "Yes" : "No");

                    putNewChangeInList(&eepromChangeList, currentChangeEntry);
                }
                nbChanges += 1;
            }


            if(origEeprom.DVDPlaybackKitZone[0] != eeprom.DVDPlaybackKitZone[0])
            {
                if(genStrings)
                {
                    currentChangeEntry = calloc(1, sizeof(EEPROMChangeEntry_t));
                    currentChangeEntry->eepromModItem = EEPROMModItem_DVDPlaybackKitZone;
                    currentChangeEntry->label = SettingChangeLabel[currentChangeEntry->eepromModItem];
                    sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", DVDregiontext[origEeprom.DVDPlaybackKitZone[0]], DVDregiontext[eeprom.DVDPlaybackKitZone[0]]);

                    putNewChangeInList(&eepromChangeList, currentChangeEntry);
                }
                nbChanges += 1;
            }
        }
    }

    return nbChanges;
}

void cleanEEPROMSettingsChangeListStruct(EEPROMChangeList* input)
{
    EEPROMChangeEntry_t* nextEntry;
    EEPROMChangeEntry_t* curEntry = input->firstChangeEntry;

    while(curEntry != NULL)
    {
        nextEntry = curEntry->nextChange;
        free(curEntry);
        curEntry = nextEntry;
    }

    input->changeCount = 0;
    input->firstChangeEntry = NULL;
}

static void putNewChangeInList(EEPROMChangeList* list, EEPROMChangeEntry_t* change)
{
    EEPROMChangeEntry_t* cycler = list->firstChangeEntry;
    EEPROMChangeEntry_t* lastEntry = cycler;

    if(list->firstChangeEntry == NULL)
    {
        list->firstChangeEntry = change;
        return;
    }

    while(cycler->nextChange != NULL)
    {
        cycler = cycler->nextChange;
    }

    list->changeCount += 1;
    cycler->nextChange = change;
}

const char* getGameRegionText(EEPROM_XBERegion gameRegion)
{
    return Gameregiontext[gameRegion];
}

const char* getDVDRegionText(EEPROM_DVDRegion dvdRegion)
{
    return DVDregiontext[dvdRegion];
}

const char* getVideoStandardText(EEPROM_VideoStandard vidStandard)
{
    //Re-use XBE region strings as they are the same
    switch(vidStandard)
    {
    case EEPROM_VideoStandardNTSC_M:
        return Gameregiontext[EEPROM_XBERegionNorthAmerica];
    case EEPROM_VideoStandardNTSC_J:
        return Gameregiontext[EEPROM_XBERegionJapan];
    case EEPROM_VideoStandardPAL_I:
        return Gameregiontext[EEPROM_XBERegionEuropeAustralia];
    case EEPROM_VideoStandardInvalid:
        /* Fall through */
    default:
        break;
    }

    return Gameregiontext[EEPROM_XBERegionInvalid];
}

const char* getScreenFormatText(EEPROM_VidScreenFormat vidFormat)
{
    switch(vidFormat & (EEPROM_VidScreenWidescreen | EEPROM_VidScreenLetterbox | EEPROM_VidScreenFullScreen))
    {
    case EEPROM_VidScreenWidescreen:
        return Vidformattext[1];
    case EEPROM_VidScreenLetterbox:
        return Vidformattext[2];
    case EEPROM_VidScreenFullScreen:
        /* Fall through */
    default:
        break;
    }

    return Vidformattext[0];
}
