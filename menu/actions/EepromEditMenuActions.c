/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "EepromEditMenuActions.h"
#include "ToolsMenuActions.h"
#include "BootFATX.h"
#include "lpcmod_v1.h"
#include "rc4.h"

void displayEditEEPROMBuffer(void *ignored){
    int i;
    u8 decryptedConfounder[8];
    u8 decryptedGameRegion[4];
    u8 decryptedWholeBuffer[0x30];
    u8 version;
    char serialString[13];
    char *Gameregiontext[5] = {
        "Unknown/Error",
        "NTSC-U",
        "NTSC-J",
        "n/a",
        "PAL"
    };
    version = decryptEEPROMData((u8 *)editeeprom, decryptedWholeBuffer);
    ToolHeader("Modified EEPROM buffer content");
    printk("\n\n           EEPROM version: %u", version);
    if(version < V1_0 || version > V1_6){
        printk(" (corrupted!)");
    }
    printk("\n           Decrypted Confounder:");
    for(i = 0; i < 8; i++){
        printk(" %02X", decryptedWholeBuffer[20+i]);
    }
/*
    printk("\n           Encrypted HDDKey:");
    for(i = 0; i < 16; i++){
        printk(" %02X", editeeprom->HDDKkey[i]);
    }
*/
    printk("\n           Decrypted HDDKey:");
    for(i = 0; i < 16; i++){
        printk(" %02X", decryptedWholeBuffer[28+i]);
    }
    printk("\n           Decrypted GameRegion:");
    for(i = 0; i < 4; i++){
        printk(" %02X", decryptedWholeBuffer[44+i]);
    }
    printk(" (%s)", Gameregiontext[(decryptedWholeBuffer[44] <= 4)? decryptedWholeBuffer[44] : 0]);  //Hopefully, everything we need is in first byte.
    printk("\n           MAC address: %02X:%02X:%02X:%02X:%02X:%02X",
    editeeprom->MACAddress[0], editeeprom->MACAddress[1], editeeprom->MACAddress[2],
    editeeprom->MACAddress[3], editeeprom->MACAddress[4], editeeprom->MACAddress[5]);
    memcpy(serialString, editeeprom->SerialNumber, 12);
    serialString[12]='\0';
    printk("\n           Serial Number: %s", serialString);
    printk("\n           Online Key:");
    for(i = 0; i < 16; i++){
        printk(" %02X", editeeprom->OnlineKey[i]);
    }
    UIFooter();
}


void LastResortRecovery(void *ignored){
//Generic 1.0 revision eeprom image. Thanks bunnie!
    const u8 EEPROMimg[] = {
        0x47, 0x83, 0xa2, 0x7d, 0x6a, 0x69, 0x10, 0x8b, 0x2d, 0xb2, 0xe8, 0x90, 0xe1, 0x60, 0xde, 0xed,
        0x02, 0xc2, 0xaa, 0x79, 0x21, 0x47, 0xcd, 0xb0, 0xb7, 0xa8, 0x7a, 0x77, 0x44, 0x9c, 0x5e, 0x6e,
        0xd0, 0xf5, 0xf9, 0xe6, 0x94, 0x68, 0x39, 0xe0, 0xca, 0xa5, 0xd2, 0xe5, 0xfa, 0x02, 0xb9, 0xb7,
        0x9d, 0x19, 0xe6, 0xed, 0x36, 0x30, 0x35, 0x33, 0x37, 0x39, 0x35, 0x32, 0x31, 0x39, 0x30, 0x32,
        0x00, 0x50, 0xf2, 0x41, 0x9e, 0x5f, 0x00, 0x00, 0x2d, 0xaa, 0x6c, 0x23, 0x99, 0x80, 0x11, 0x47,
        0x33, 0xc3, 0xc7, 0x1a, 0x2b, 0xa5, 0x06, 0xb3, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x75, 0x61, 0x57, 0xfb, 0x2c, 0x01, 0x00, 0x00, 0x45, 0x53, 0x54, 0x00, 0x45, 0x44, 0x54, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x05, 0x00, 0x02, 0x04, 0x01, 0x00, 0x02,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc4, 0xff, 0xff, 0xff,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    if(ConfirmDialog("     Overwrite EEPROM with generic image?", 1))       //First generic warning
        return;

    memcpy(editeeprom, EEPROMimg, sizeof(EEPROMDATA));
}

void bruteForceFixDisplayresult(void *ignored){
    u8 eepromVersion;
    char unused[20];
    ToolHeader("Brute Force Fix EEPROM");
    eepromVersion = BootHddKeyGenerateEepromKeyData(editeeprom,unused);

    if(eepromVersion < V1_0 && eepromVersion > V1_6){
        if(bruteForceFixEEprom()){
            //success
            printk("\n\n           Successfully fixed!\n\n\n");
        }
        else{
            printk("\n\n           EEPROM could be fixed...\n           Use \"Last resort recovery\" feature or try something else.\n\n\n");
        }
    }
    else{
        printk("\n\n           Brute force fix not useful here.\n           Aborting.\n\n\n");
    }
    UIFooter();
}

bool bruteForceFixEEprom(void){
    u8 ver, bytepos;
    int bytecombinations;
    u8 *teeprom;
    char unused[20];
    teeprom = malloc(sizeof(EEPROMDATA));

    //Fix attempt to sucessfully decrypt 48 first bytes of EEPROM by
    //trying to change a single byte in the buffer. all possible values
    //of this byte are tried in the calculation. If this yields no
    //positive result, next byte in buffer is given the same treatment.
    //So, effectively, this technique can only recover a corrupt
    //EEPROM with only a single corrupt byte in it's first 48 bytes.
    for (bytepos=0;bytepos<0x30;bytepos++) {
        for (bytecombinations=0;bytecombinations<0x100;bytecombinations++) {
            memcpy(teeprom,editeeprom,sizeof(EEPROMDATA));
            teeprom[bytepos]=bytecombinations;
            ver = BootHddKeyGenerateEepromKeyData(teeprom,unused);
            if (ver!=13) {
                memcpy(editeeprom,teeprom,sizeof(EEPROMDATA));
                free(teeprom);
                return true;
            }
        }
    }

    free(teeprom);
    return false;      // No Match found
}

void confirmSaveToEEPROMChip(void *ignored){
    if(replaceEEPROMContentFromBuffer(editeeprom)){
        ToolHeader("Operation aborted");
        printk("\n           Invalid EEPROM image data.");
        printk("\n           Data not saved!");
        UIFooter();
    }
}

void editMACAddress(void *ignored){
    u8 i, j;
    char macString[13];
    u8 nibble[2];
    sprintf(macString, "%02X%02X%02X%02X%02X%02X",
    editeeprom->MACAddress[0], editeeprom->MACAddress[1], editeeprom->MACAddress[2],
    editeeprom->MACAddress[3], editeeprom->MACAddress[4], editeeprom->MACAddress[5]);
    OnScreenKeyboard(macString, 13, 3, HEX_KEYPAD); //Function will add terminating character.
    if(strlen(macString) == 12){
        for(i = 0; i < 6; i++){
            for(j = 0; j < 2; j++){
                //Dumdum way of converting string of hex into actual hex.
                switch(macString[i*2 + j]){
                    case '0':
                        nibble[j] = 0;
                        break;
                    case '1':
                        nibble[j] = 0x1;
                        break;
                    case '2':
                        nibble[j] = 0x2;
                        break;
                    case '3':
                        nibble[j] = 0x3;
                        break;
                    case '4':
                        nibble[j] = 0x4;
                        break;
                    case '5':
                        nibble[j] = 0x5;
                        break;
                    case '6':
                        nibble[j] = 0x6;
                        break;
                    case '7':
                        nibble[j] = 0x7;
                        break;
                    case '8':
                        nibble[j] = 0x8;
                        break;
                    case '9':
                        nibble[j] = 0x9;
                        break;
                    case 'A':
                        nibble[j] = 0xA;
                        break;
                    case 'B':
                        nibble[j] = 0xB;
                        break;
                    case 'C':
                        nibble[j] = 0xC;
                        break;
                    case 'D':
                        nibble[j] = 0xD;
                        break;
                    case 'E':
                        nibble[j] = 0xE;
                        break;
                    case 'F':
                        nibble[j] = 0xF;
                        break;
                }
            }
            editeeprom->MACAddress[i] = (nibble[0] << 4) | nibble[1];
        }
    }
}

void restoreEEPROMFromFile(void *fname) {
    int res;
    FATXFILEINFO fileinfo;
    FATXPartition *partition;

    partition = OpenFATXPartition (0, SECTOR_SYSTEM, SYSTEM_SIZE);

    res = LoadFATXFile(partition, fname, &fileinfo);
    ToolHeader("Load EEPROM image from HDD");
    if(res){
        updateEEPROMEditBufferFromInputBuffer(fileinfo.buffer, fileinfo.fileSize);
        free(fileinfo.buffer);
    }
    else{
            printk("\n\n           Error!\n           File read error.");
    }
    UIFooter();
}

int updateEEPROMEditBufferFromInputBuffer(u8 *buffer, u32 size){
    int res = 0, version;
    u8 decryptedData[0x30];
    u8 tempBuffer[256];
    u8 confirmHash[20];
    u8 baKeyHash[20];
    struct rc4_key RC4_key;
    
    memcpy(tempBuffer, buffer, size);

    if(size != 256){
        res = -3;
    }
    else{
        memset(decryptedData, 0, 0x30);
        version = decryptEEPROMData(tempBuffer, decryptedData);
        if(version >= V1_0 && version <= V1_6){   //Current content in eeprom is valid.
        
            //Prepare to encrypt for the right motherboard version
            if(mbVersion >= REV1_6)
                version = V1_6;
            else if(mbVersion >= REV1_1)
                version = V1_1;
            else
                version = V1_0;
                
            memset(&RC4_key,0,sizeof(rc4_key));
            memset(confirmHash, 0, 20);
            memset(baKeyHash, 0, 20);
            memcpy(tempBuffer, decryptedData, 0x30);

            // Calculate the Confirm-Hash
            HMAC_hdd_calculation(version, confirmHash, &tempBuffer[20], 8, &tempBuffer[28], 20, NULL);

            memcpy(&tempBuffer[0],confirmHash,20);

            // Calculate the Key-Hash
            HMAC_hdd_calculation(version, baKeyHash, &tempBuffer[0], 20, NULL);

            //initialize RC4 key
            rc4_prepare_key(baKeyHash, 20, &RC4_key);

            //decrypt data (from eeprom) with generated key
            rc4_crypt(&tempBuffer[20],28,&RC4_key);

            // Save back to EEprom
            memcpy((u8 *)editeeprom, tempBuffer, sizeof(EEPROMDATA));
            
            //Recalculate checksums to be nice.
            EepromCRC((u8 *)editeeprom->Checksum2,(u8 *)editeeprom->SerialNumber,0x28);
            EepromCRC((u8 *)editeeprom->Checksum3,(u8 *)editeeprom->TimeZoneBias,0x5b);
            
            res = 1;
    
        }
        else{       //Content of new EEPROM image is not valid.
            res = -1;   //loaded EEPROM image is not valid
        }
    }

    if(res > 0){
        printk("\n\n           Success!.\n           EEPROM image successfully loaded to modified EEPROM buffer.");
    }
    else{
        printk("\n\n           Error!");
        if(res == -1)
            printk("\n           Invalid EEPROM image file.");
        else if(res == -3)
            printk ("\n          EEPROM image file size wrong.");
    }
    return res;
}
