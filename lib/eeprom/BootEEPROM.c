#include "boot.h"
#include "VideoInitialization.h"
#include "BootEEPROM.h"
#include "rc4.h"

void BootEepromReadEntireEEPROM() {
    int i;
    u8 *pb=(u8 *)&eeprom;
    for(i = 0; i < 256; i++) {
        *pb++ = I2CTransmitByteGetReturn(0x54, i);
    }
}

void BootEepromReloadEEPROM(EEPROMDATA * realeeprom) {
    int i;
    u8 *pb=(u8 *)realeeprom;
    for(i = 0; i < 256; i++) {
        *pb++ = I2CTransmitByteGetReturn(0x54, i);
    }
}

void BootEepromCompareAndWriteEEPROM(EEPROMDATA * realeeprom){
    int i;
    u8 *pb = (u8 *)&eeprom;
    u8 *pc = (u8 *)realeeprom;
    for(i = 0; i < sizeof(EEPROMDATA); i++){
        if(memcmp(pb + i,pc + i,1)){                //Compare byte by byte.
            WriteToSMBus(0x54,i,1,pb[i]);                        //Physical EEPROM's content is different from what's held in memory.
            //setLED("rgrg");
            //while(1);                        //Debug, no real write for now. Just hang with led sequence.
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

void EepromSetWidescreen(int enable) {
    //Changing this setting requires that Checksum3 
    //be recalculated.
    
    //Possible values are:
    //0x00 : Full Screen
    //0x01 : Widescreen
    //0x10 : LetterBox
    
    //unsigned char sum[4];
    if (enable) {
        //Enable WS
        //WriteToSMBus(0x54, 0x96, 0, 1);
        eeprom.VideoFlags[2] = 0x01;
    } 
    else {
        //Disable WSS
        //WriteToSMBus(0x54, 0x96, 0, 0);
        eeprom.VideoFlags[2] = 0x00;
    }

    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
/*
    EepromCRC(sum,eeprom.TimeZoneBias,0x5b);
    WriteToSMBus(0x54, 0x60, 1, sum[0]);
    WriteToSMBus(0x54, 0x61, 1, sum[1]);
    WriteToSMBus(0x54, 0x62, 1, sum[2]);
    WriteToSMBus(0x54, 0x63, 1, sum[3]);*/
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

void changeDVDRegion(u8 value){
    eeprom.DVDPlaybackKitZone[0] = value;
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

int getGameRegionValue(void){
    int result = -1;
    u8 baKeyHash[20];
    u8 baDataHashConfirm[20];
    u8 baEepromDataLocalCopy[0x30];
    struct rc4_key RC4_key;
    int version = 0;
    int counter;
    u8 gameRegion=0;

    for (counter=7;counter<13;counter++)
    {
           memset(&RC4_key,0,sizeof(rc4_key));
           memcpy(&baEepromDataLocalCopy[0], &eeprom, 0x30);

                   // Calculate the Key-Hash
        HMAC_hdd_calculation(counter, baKeyHash, &baEepromDataLocalCopy[0], 20, NULL);

        //initialize RC4 key
        rc4_prepare_key(baKeyHash, 20, &RC4_key);

        //decrypt data (from eeprom) with generated key
        rc4_crypt(&baEepromDataLocalCopy[20],28,&RC4_key);        //Whole crypted block
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
    if(version == 13)
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

    for (counter=9;counter<13;counter++)
    {
        memset(&RC4_key,0,sizeof(rc4_key));
        memcpy(&baEepromDataLocalCopy[0], &eeprom, 0x30);

        // Calculate the Key-Hash
        HMAC_hdd_calculation(counter, baKeyHash, &baEepromDataLocalCopy[0], 20, NULL);

        //initialize RC4 key
        rc4_prepare_key(baKeyHash, 20, &RC4_key);

          //decrypt data (from eeprom) with generated key
        rc4_crypt(&baEepromDataLocalCopy[20],28,&RC4_key);        //Whole crypted block
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
    if (version == 13) return (-1);    //error, let's not do something stupid here. Leave with dignity.
    //else we know the version
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
