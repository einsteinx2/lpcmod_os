#include "boot.h"
#include "BootEEPROM.h"
#include "rc4.h"

void BootEepromReadEntireEEPROM() {
    int i;
    u8 *pb=(u8 *)&eeprom;
    for(i = 0; i < 256; i++) {
        *pb++ = I2CTransmitByteGetReturn(0x54, i);
    }
    memcpy(&origEeprom, &eeprom, sizeof(EEPROMDATA));
}

void BootEepromReloadEEPROM(EEPROMDATA * realeeprom) {/*
    int i;
    u8 *pb=(u8 *)realeeprom;
    for(i = 0; i < 256; i++) {
        *pb++ = I2CTransmitByteGetReturn(0x54, i);
    }
*/
    memcpy(&eeprom, &origEeprom, sizeof(EEPROMDATA));
}

void BootEepromCompareAndWriteEEPROM(EEPROMDATA * realeeprom){
    int i;
    u8 *pb = (u8 *)&eeprom;
    u8 *pc = (u8 *)realeeprom;
    for(i = 0; i < sizeof(EEPROMDATA); i++){
        if(memcmp(pb + i,pc + i,1)){                //Compare byte by byte.
            WriteToSMBus(0x54,i,1,pb[i]);           //Physical EEPROM's content is different from what's held in memory.
        }
    }
}

void BootEepromPrintInfo() {
    VIDEO_ATTR=0xffc8c8c8;
    printk("MAC : ");
    VIDEO_ATTR=0xffc8c800;
    printk("%02X%02X%02X%02X%02X%02X  ",
        eeprom.MACAddress[0], eeprom.MACAddress[1], eeprom.MACAddress[2],
        eeprom.MACAddress[3], eeprom.MACAddress[4], eeprom.MACAddress[5]
    );

    VIDEO_ATTR=0xffc8c8c8;
    printk("Vid: ");
    VIDEO_ATTR=0xffc8c800;

    switch(*((VIDEO_STANDARD *)&eeprom.VideoStandard)) {
        case VID_INVALID:
            printk("0  ");
            break;
        case NTSC_M:
            printk("NTSC-M  ");
            break;
        case NTSC_J:
            printk("NTSC-J  ");
            break;
        case PAL_I:
            printk("PAL-I  ");
            break;
        default:
            printk("%X  ", (int)*((VIDEO_STANDARD *)&eeprom.VideoStandard));
            break;
    }

    VIDEO_ATTR=0xffc8c8c8;
    printk("Serial: ");
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

void BootEepromWriteEntireEEPROM(void){
    int i;
    u8 *pb=(u8 *)&eeprom;
    for(i = 0; i < 256; i++) {
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
void EepromCRC(unsigned char *crc, unsigned char *data, long dataLen) {
    unsigned char* CRC_Data = (unsigned char *)malloc(dataLen+4);
    int pos=0;
    memset(crc,0x00,4);

    memset(CRC_Data,0x00, dataLen+4);
    //Circle shift input data one byte right
    memcpy(CRC_Data + 0x01 , data, dataLen-1);
    memcpy(CRC_Data, data + dataLen-1, 0x01);

    for (pos=0; pos<4; ++pos) {
        unsigned short CRCPosVal = 0xFFFF;
        unsigned long l;
        for (l=pos; l<dataLen; l+=4) {
            CRCPosVal -= *(unsigned short*)(&CRC_Data[l]);
        }
        CRCPosVal &= 0xFF00;
        crc[pos] = (unsigned char) (CRCPosVal >> 8);
    }
    free(CRC_Data);
}

void EepromSetVideoStandard(VIDEO_STANDARD standard) {
    //Changing this setting requires that Checksum2
    //be recalculated.
    //unsigned char sum[4];
    unsigned int i;



/*
 * EEPROM will be written only once at exit.
 */
    //Write the four bytes to the EEPROM
    //for (i=0; i<4; ++i) {
    //    WriteToSMBus(0x54,0x58+i, 1, (u8)(standard>>(8*i))&0xff);
    //}

    memcpy(eeprom.VideoStandard, &standard, 0x04);
    EepromCRC(eeprom.Checksum2,eeprom.SerialNumber,0x28);
/*
    EepromCRC(sum,eeprom.SerialNumber,0x28);
    WriteToSMBus(0x54, 0x30, 0, sum[0]);
    WriteToSMBus(0x54, 0x31, 0, sum[1]);
    WriteToSMBus(0x54, 0x32, 0, sum[2]);
    WriteToSMBus(0x54, 0x33, 0, sum[3]);
*/
}

void assertWriteEEPROM(void){
    EEPROMDATA realeeprom;
    BootEepromReloadEEPROM(&realeeprom);
    BootEepromCompareAndWriteEEPROM(&realeeprom);    //If there is at least one change that requires to write back to physical EEPROM. This function will write it.
    return;
}

int getGameRegionValue(EEPROMDATA * eepromPtr){
    int result = -1;
    u8 baEepromDataLocalCopy[0x30];
    int version = 0;
    u32 gameRegion=0;

    version = decryptEEPROMData((u8 *)eepromPtr, baEepromDataLocalCopy);

    if(version > V1_6)
        result = XBE_INVALID;
    else {
        memcpy(&gameRegion,&baEepromDataLocalCopy[28+16],4);
        if(gameRegion == JAPAN)
            result = JAPAN;
        else if(gameRegion == EURO_AUSTRALIA)
            result = EURO_AUSTRALIA;
        else if(gameRegion == NORTH_AMERICA)
            result = NORTH_AMERICA;
        else
            result = XBE_INVALID;
    }
    return result;
}

int setGameRegionValue(u8 value){
    int result = -1;
    u8 baKeyHash[20];
    u8 baDataHashConfirm[20];
    u8 baEepromDataLocalCopy[0x30];
    struct rc4_key RC4_key;
    int version = 0;
    int counter;
    u32 gameRegion = value;

    version = decryptEEPROMData((u8 *)&eeprom, baEepromDataLocalCopy);

    if (version > V1_6) return (-1);    //error, let's not do something stupid here. Leave with dignity.
    //else we know the version
    memset(&RC4_key,0,sizeof(rc4_key));
    memcpy(&baEepromDataLocalCopy[28+16],&gameRegion,4);

    // Calculate the Confirm-Hash
    HMAC_hdd_calculation(version, baDataHashConfirm, &baEepromDataLocalCopy[20], 8, &baEepromDataLocalCopy[28], 20, NULL);

    memcpy(baEepromDataLocalCopy,baDataHashConfirm,20);

    // Calculate the Key-Hash
    HMAC_hdd_calculation(version, baKeyHash, &baEepromDataLocalCopy[0], 20, NULL);

    //initialize RC4 key
    rc4_prepare_key(baKeyHash, 20, &RC4_key);

    //decrypt data (from eeprom) with generated key
    rc4_crypt(&baEepromDataLocalCopy[20],28,&RC4_key);

    // Save back to EEprom
    memcpy(&eeprom,&baEepromDataLocalCopy[0], 0x30);
    
    //Everything went well, return new gameRegion.
    return gameRegion;

}

u8 decryptEEPROMData(u8* eepromPtr, u8* decryptedBuf){
   struct rc4_key RC4_key;
   int version = 0;
   int counter;
   u8 baEepromDataLocalCopy[0x30];
   u8 baKeyHash[20];
   u8 baDataHashConfirm[20];

        // Static Version change not included yet

        for (counter=V1_0;counter<=V1_6;counter++)
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

            if (!memcmp(baEepromDataLocalCopy,baDataHashConfirm,0x14)) {
                // Confirm Hash is correct
                // Copy actual Xbox Version to Return Value
                version=counter;
                // exits the loop
                break;
            }
        }

        //copy out decrypted Confounder, HDDKey and gameregion.
        memcpy(decryptedBuf,baEepromDataLocalCopy,0x30);

        return version;
}

u8 generateStringsForEEPROMChanges(bool genStrings){
    u32 tempOrigChecksum2 = 0, tempOrigChecksum3 = 0, tempChecksum2 = 0, tempChecksum3 = 0;
    int origGameRegion, gameRegion;
    u8 i, nbChanges = 0, stringLength;
    char tempString[200];
    char origTempItemString[35], tempItemString[35];
    u8 origHDDKey[16], HDDKey[16];

    for(i = 0; i < 4; i++){
        tempChecksum2 |= eeprom.Checksum2[i] << (i * 8);
        tempChecksum3 |= eeprom.Checksum3[i] << (i * 8);

        tempOrigChecksum2 |= origEeprom.Checksum2[i] << (i * 8);
        tempOrigChecksum3 |= origEeprom.Checksum3[i] << (i * 8);
    }

    for(i = 0; i < 20; i++){
    	if(origEeprom.HMAC_SHA1_Hash[i] != eeprom.HMAC_SHA1_Hash[i]){
    		break;
    	}
    }

    if(i < 20){ //Encrypted section has changed
    	origGameRegion = getGameRegionValue(&origEeprom);
    	gameRegion = getGameRegionValue(&eeprom);
    	if(origGameRegion != gameRegion){
            if(genStrings){
            	sprintf(tempString, "Game region= \"%s\" -> \"%s\"", Gameregiontext[origGameRegion], Gameregiontext[gameRegion]);
            	stringLength = strlen(tempString);
				eepromChangesStringArray[nbChanges] = malloc(stringLength + 1);
				strncpy(eepromChangesStringArray[nbChanges], tempString, stringLength);
				eepromChangesStringArray[nbChanges][stringLength] = '\0';
    	    }
            nbChanges += 1;
    	}

    	BootHddKeyGenerateEepromKeyData(&origEeprom, origHDDKey);
    	BootHddKeyGenerateEepromKeyData(&eeprom, HDDKey);
    	for(i = 0; i < 16; i++){
    		if(origHDDKey[i] != HDDKey[i]){
    			if(genStrings){
    				for(stringLength = 0; stringLength < 16; stringLength++){
    					sprintf(&origTempItemString[i << 1], "%02X", origHDDKey[i]);
    					sprintf(&tempItemString[i << 1], "%02X", HDDKey[i]);
    				}
    				sprintf(tempString, "HDD key= \"%s\" -> \"%s\"", origTempItemString, tempItemString);
                    stringLength = strlen(tempString);
                    eepromChangesStringArray[nbChanges] = malloc(stringLength + 1);
                    strncpy(eepromChangesStringArray[nbChanges], tempString, stringLength);
                    eepromChangesStringArray[nbChanges][stringLength] = '\0';
    			}
    			nbChanges += 1;
    			break;
    		}
    	}

    }

    if(tempChecksum2 != tempOrigChecksum2){//Change in either Serial number, Video standard and/or MAC Address

        //check serial number
        for(i = 0; i < 12; i++){
            if(origEeprom.SerialNumber[i] != eeprom.SerialNumber[i]){
                if(genStrings){
                    memcpy(origTempItemString, &origEeprom.SerialNumber[0], 12);
                    origTempItemString[12]='\0';

                    memcpy(tempItemString, &eeprom.SerialNumber[0], 12);
                    tempItemString[12]='\0';

                    sprintf(tempString, "Serial number= \"%s\" -> \"%s\"", origTempItemString, tempItemString);
                    stringLength = strlen(tempString);
                    eepromChangesStringArray[nbChanges] = malloc(stringLength + 1);
                    strncpy(eepromChangesStringArray[nbChanges], tempString, stringLength);
                    eepromChangesStringArray[nbChanges][stringLength] = '\0';
                }
                nbChanges += 1;

                break;
            }
        }

        //check MAC address
        for(i = 0; i < 12; i++){
            if(origEeprom.MACAddress[i] != eeprom.MACAddress[i]){
                if(genStrings){
                    sprintf(tempString, "MAC address= \"%02X:%02X:%02X:%02X:%02X:%02X\" -> \"%02X:%02X:%02X:%02X:%02X:%02X\"",
                            origEeprom.MACAddress[0], origEeprom.MACAddress[1], origEeprom.MACAddress[2],
                            origEeprom.MACAddress[3], origEeprom.MACAddress[4], origEeprom.MACAddress[5],
                            eeprom.MACAddress[0], eeprom.MACAddress[1], eeprom.MACAddress[2],
                            eeprom.MACAddress[3], eeprom.MACAddress[4], eeprom.MACAddress[5]);
                    stringLength = strlen(tempString);
                    eepromChangesStringArray[nbChanges] = malloc(stringLength + 1);
                    strncpy(eepromChangesStringArray[nbChanges], tempString, stringLength);
                    eepromChangesStringArray[nbChanges][stringLength] = '\0';
                }
                nbChanges += 1;

                break;
            }
        }

        if(*((VIDEO_STANDARD *)&origEeprom.VideoStandard) != *((VIDEO_STANDARD *)&eeprom.VideoStandard)){
            if(genStrings){
                if(*((VIDEO_STANDARD *)&origEeprom.VideoStandard) == NTSC_M)
                    sprintf(origTempItemString, "NTSC-U");
                else if(*((VIDEO_STANDARD *)&origEeprom.VideoStandard) == NTSC_J)
                    sprintf(origTempItemString, "NTSC-J");
                else if(*((VIDEO_STANDARD *)&origEeprom.VideoStandard) == PAL_I)
                    sprintf(origTempItemString, "PAL");
                else
                    sprintf(origTempItemString, "Unknown/Error");

                if(*((VIDEO_STANDARD *)&eeprom.VideoStandard) == NTSC_M)
                    sprintf(tempItemString, "NTSC-U");
                else if(*((VIDEO_STANDARD *)&eeprom.VideoStandard) == NTSC_J)
                    sprintf(tempItemString, "NTSC-J");
                else if(*((VIDEO_STANDARD *)&eeprom.VideoStandard) == PAL_I)
                    sprintf(tempItemString, "PAL");
                else
                    sprintf(tempItemString, "Unknown/Error");

                sprintf(tempString, "Video standard= \"%s\" -> \"%s\"", origTempItemString, tempItemString);
                stringLength = strlen(tempString);
                eepromChangesStringArray[nbChanges] = malloc(stringLength + 1);
                strncpy(eepromChangesStringArray[nbChanges], tempString, stringLength);
                eepromChangesStringArray[nbChanges][stringLength] = '\0';
            }
            nbChanges += 1;
        }
    }

    if(origEeprom.Checksum3 != eeprom.Checksum3){//Change in either Video format and/or DVD region
        if(tempChecksum3 == 0xFFFFFFFF && tempOrigChecksum3 != 0xFFFFFFFF){
            //List as single change
            nbChanges += 1;
        }
        else{
            //List individual changes
            if((origEeprom.VideoFlags[2] & R480p) != (eeprom.VideoFlags[2] & R480p)){
                if(genStrings){
                    sprintf(tempString, "480p= \"%s\" -> \"%s\"", (origEeprom.VideoFlags[2] & R480p) ? "Yes" : "No", (eeprom.VideoFlags[2] & R480p) ? "Yes" : "No");
                    stringLength = strlen(tempString);
                    eepromChangesStringArray[nbChanges] = malloc(stringLength + 1);
                    strncpy(eepromChangesStringArray[nbChanges], tempString, stringLength);
                    eepromChangesStringArray[nbChanges][stringLength] = '\0';
                }
                nbChanges += 1;
            }

            if((origEeprom.VideoFlags[2] & R720p) != (eeprom.VideoFlags[2] & R720p)){
                if(genStrings){
                    sprintf(tempString, "720p= \"%s\" -> \"%s\"", (origEeprom.VideoFlags[2] & R720p) ? "Yes" : "No", (eeprom.VideoFlags[2] & R720p) ? "Yes" : "No");
                    stringLength = strlen(tempString);
                    eepromChangesStringArray[nbChanges] = malloc(stringLength + 1);
                    strncpy(eepromChangesStringArray[nbChanges], tempString, stringLength);
                    eepromChangesStringArray[nbChanges][stringLength] = '\0';
                }
                nbChanges += 1;
            }

            if((origEeprom.VideoFlags[2] & R1080i) != (eeprom.VideoFlags[2] & R1080i)){
                if(genStrings){
                    sprintf(tempString, "1080i= \"%s\" -> \"%s\"", (origEeprom.VideoFlags[2] & R1080i) ? "Yes" : "No", (eeprom.VideoFlags[2] & R1080i) ? "Yes" : "No");
                    stringLength = strlen(tempString);
                    eepromChangesStringArray[nbChanges] = malloc(stringLength + 1);
                    strncpy(eepromChangesStringArray[nbChanges], tempString, stringLength);
                    eepromChangesStringArray[nbChanges][stringLength] = '\0';
                }
                nbChanges += 1;
            }

            if((origEeprom.VideoFlags[2] & 0x11) != (eeprom.VideoFlags[2] & 0x11)){ //Video format mask
                if(genStrings){
                    if(origEeprom.VideoFlags[2] & WIDESCREEN)
                        sprintf(origTempItemString, "Widescreen");
                    else if(origEeprom.VideoFlags[2] & LETTERBOX)
                        sprintf(origTempItemString, "Letterbox");
                    else
                        sprintf(origTempItemString, "Fullscreen");

                    if(eeprom.VideoFlags[2] & WIDESCREEN)
                        sprintf(tempItemString, "Widescreen");
                    else if(eeprom.VideoFlags[2] & LETTERBOX)
                        sprintf(tempItemString, "Letterbox");
                    else
                        sprintf(tempItemString, "Fullscreen");

                    sprintf(tempString, "Video format= \"%s\" -> \"%s\"", origTempItemString, tempItemString);
                    stringLength = strlen(tempString);
                    eepromChangesStringArray[nbChanges] = malloc(stringLength + 1);
                    strncpy(eepromChangesStringArray[nbChanges], tempString, stringLength);
                    eepromChangesStringArray[nbChanges][stringLength] = '\0';
                }
                nbChanges += 1;
            }

            if(origEeprom.DVDPlaybackKitZone[0] != eeprom.DVDPlaybackKitZone[0]){
                if(genStrings){
                    sprintf(tempString, "DVD zone= \"%s\" -> \"%s\"", DVDregiontext[origEeprom.DVDPlaybackKitZone[0]], DVDregiontext[eeprom.DVDPlaybackKitZone[0]]);
                    stringLength = strlen(tempString);
                    eepromChangesStringArray[nbChanges] = malloc(stringLength + 1);
                    strncpy(eepromChangesStringArray[nbChanges], tempString, stringLength);
                    eepromChangesStringArray[nbChanges][stringLength] = '\0';
                }
                nbChanges += 1;
            }
        }
    }

    return nbChanges;
}
