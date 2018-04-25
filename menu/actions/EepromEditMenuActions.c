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
#include "FatFSAccessor.h"
#include "lpcmod_v1.h"
#include "lib/cromwell/cromString.h"
#include "lib/LPCMod/xblastDebug.h"
#include "MenuActions.h"
#include "string.h"
#include "stdio.h"
#include "boot.h"
#include "menu/misc/OnScreenKeyboard.h"
#include "menu/misc/ConfirmDialog.h"
#include "xblast/HardwareIdentifier.h"

static const char* const eepromDirectoryLocation = "MASTER_C:"PathSep"XBlast"PathSep"eeproms";

void displayEditEEPROMBuffer(void *ignored)
{
    int i;
    unsigned char decryptedWholeBuffer[0x30];
    unsigned char version;
    char serialString[13];

    version = decryptEEPROMData((unsigned char *)editeeprom, decryptedWholeBuffer);
    UiHeader("Modified EEPROM buffer content");

    printk("\n\n           EEPROM version: %u", version);
    if(version < EEPROM_EncryptV1_0 || version > EEPROM_EncryptV1_6)
    {
        printk(" (corrupted!)");
    }

    printk("\n           Decrypted Confounder:");
    for(i = 0; i < 8; i++)
    {
        printk(" %02X", decryptedWholeBuffer[20+i]);
    }
/*
    printk("\n           Encrypted HDDKey:");
    for(i = 0; i < 16; i++){
        printk(" %02X", editeeprom->HDDKkey[i]);
    }
*/
    printk("\n           Decrypted HDDKey:");
    for(i = 0; i < 16; i++)
    {
        printk(" %02X", decryptedWholeBuffer[28+i]);
    }

    printk("\n           Decrypted GameRegion:");
    for(i = 0; i < 4; i++){
        printk(" %02X", decryptedWholeBuffer[44+i]);
    }

    printk(" (%s)", getGameRegionText((decryptedWholeBuffer[44] <= 4)? decryptedWholeBuffer[44] : EEPROM_XBERegionInvalid));  //Hopefully, everything we need is in first byte.

    printk("\n           MAC address: %02X:%02X:%02X:%02X:%02X:%02X",
    editeeprom->MACAddress[0], editeeprom->MACAddress[1], editeeprom->MACAddress[2],
    editeeprom->MACAddress[3], editeeprom->MACAddress[4], editeeprom->MACAddress[5]);

    memcpy(serialString, editeeprom->SerialNumber, 12);
    serialString[12]='\0';
    printk("\n           Serial Number: %s", serialString);

    printk("\n           Online Key:");
    for(i = 0; i < 16; i++)
    {
        printk(" %02X", editeeprom->OnlineKey[i]);
    }

    UIFooter();
}


void LastResortRecovery(void *ignored)
{
    if(ConfirmDialog("Overwrite EEPROM with generic image?", 1))       //First generic warning
    {
        return;
    }

    const EEPROMDATA* data = getRecoveryImage();

    updateEEPROMEditBufferFromInputBuffer((unsigned char *)data, sizeof(EEPROMDATA), true);
}

void bruteForceFixDisplayresult(void *ignored)
{
    unsigned char eepromVersion;

    UiHeader("Brute Force Fix EEPROM");
    eepromVersion = decryptEEPROMData((unsigned char *)editeeprom, NULL);

    if(eepromVersion < EEPROM_EncryptV1_0 && eepromVersion > EEPROM_EncryptV1_6)
    {
        if(bruteForceFixEEprom())
        {
            //success
            printk("\n\n           Successfully fixed!\n\n\n");
        }
        else
        {
            printk("\n\n           EEPROM could be fixed...\n           Use \"Last resort recovery\" feature or try something else.\n\n\n");
        }
    }
    else
    {
        printk("\n\n           Brute force fix not useful here.\n           Aborting.\n\n\n");
    }
    UIFooter();
}

bool bruteForceFixEEprom(void)
{
    unsigned char ver, bytepos;
    int bytecombinations;
    unsigned char *teeprom;
    teeprom = malloc(sizeof(EEPROMDATA));

    //Fix attempt to sucessfully decrypt 48 first bytes of EEPROM by
    //trying to change a single byte in the buffer. all possible values
    //of this byte are tried in the calculation. If this yields no
    //positive result, next byte in buffer is given the same treatment.
    //So, effectively, this technique can only recover a corrupt
    //EEPROM with only a single corrupt byte in it's first 48 bytes.
    for (bytepos=0;bytepos<0x30;bytepos++)
    {
        for (bytecombinations=0;bytecombinations<0x100;bytecombinations++)
        {
            memcpy(teeprom,editeeprom,sizeof(EEPROMDATA));
            teeprom[bytepos]=bytecombinations;
            ver = decryptEEPROMData(teeprom,NULL);
            if(ver >= EEPROM_EncryptV1_0 && ver <= EEPROM_EncryptV1_6)
            {
                memcpy(editeeprom,teeprom,sizeof(EEPROMDATA));
                free(teeprom);
                return true;
            }
        }
    }

    free(teeprom);
    return false;      // No Match found
}

void confirmSaveToEEPROMChip(void *ignored)
{
    if(replaceEEPROMContentFromBuffer(editeeprom))
    {
        UiHeader("Operation aborted");
        printk("\n           Invalid EEPROM image data.");
        printk("\n           Data not saved!");
        UIFooter();
    }
}

void editMACAddress(void *ignored)
{
    unsigned char i, j;
    char macString[13];
    unsigned char nibble[2];

    sprintf(macString, "%02X%02X%02X%02X%02X%02X",
    editeeprom->MACAddress[0], editeeprom->MACAddress[1], editeeprom->MACAddress[2],
    editeeprom->MACAddress[3], editeeprom->MACAddress[4], editeeprom->MACAddress[5]);

    OnScreenKeyboard(macString, 13, 3, HEX_KEYPAD); //Function will add terminating character.

    if(strlen(macString) == 12)
    {
        for(i = 0; i < 6; i++)
        {
            for(j = 0; j < 2; j++)
            {
                //Dumdum way of converting string of hex into actual hex.
                switch(macString[i*2 + j])
                {
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

const char* const getEEPROMDirectoryLocation(void)
{
    return eepromDirectoryLocation;
}

void restoreEEPROMFromFile(void *fname)
{
    int res = 0;
    unsigned char fileBuf[sizeof(EEPROMDATA)];
    unsigned int size;
    const char* filename = (const char *)fname;
    char fullPathName[255 + sizeof('\0')];
    FILEX fileHandle;

    if(255 < (strlen(getEEPROMDirectoryLocation()) + sizeof(cPathSep) + strlen(filename)))
    {
        return;
    }

    sprintf(fullPathName, "%s"PathSep"%s", getEEPROMDirectoryLocation(), filename);

    fileHandle = fatxopen(fullPathName, FileOpenMode_OpenExistingOnly | FileOpenMode_Read);
    if(fileHandle)
    {
        size = fatxsize(fileHandle);
        if(sizeof(EEPROMDATA) == size)
        {
            if(size == fatxread(fileHandle, fileBuf, size))
            {
                res = 1;
            }
        }
        fatxclose(fileHandle);
    }
    UiHeader("Load EEPROM image from HDD");
    if(res)
    {
        updateEEPROMEditBufferFromInputBuffer(fileBuf, size, true);
    }
    else
    {
            printk("\n\n           Error!\n           File read error.");
    }
    UIFooter();
}

int updateEEPROMEditBufferFromInputBuffer(unsigned char *buffer, unsigned int size, bool verbose)
{
    int res = 0;
    EEPROM_EncryptVersion newVersion, hostVersion;
    unsigned char decryptedData[0x30];
    EEPROMDATA result;

    if(buffer == NULL)
    {
        res = -4;
    }
    else if(size != sizeof(EEPROMDATA))
    {
        res = -3;
    }
    else
    {
        newVersion = EepromSanityCheck((EEPROMDATA *)buffer);
        XBlastLogger(DEBUG_GENERAL_UI, DBG_LVL_DEBUG, "Input EEPROM image version : %u", newVersion);

        if(newVersion >= EEPROM_EncryptV1_0 && newVersion <= EEPROM_EncryptV1_6)   //Current content in eeprom is valid.
        {
            memcpy(&result, buffer, sizeof(EEPROMDATA));
            memset(decryptedData, 0, 0x30);
            decryptEEPROMData((unsigned char *)&result, decryptedData);

            //Prepare to encrypt for the right motherboard version
            switch(getMotherboardRevision())
            {
            case XboxMotherboardRevision_DEBUGKIT:
            case XboxMotherboardRevision_DEVKIT:
            case XboxMotherboardRevision_1_0:
                hostVersion = EEPROM_EncryptV1_0;
                break;
            case XboxMotherboardRevision_1_1:
            case XboxMotherboardRevision_1_2:
            case XboxMotherboardRevision_1_4:
                hostVersion = EEPROM_EncryptV1_1;
                break;
            case XboxMotherboardRevision_1_6:
                hostVersion = EEPROM_EncryptV1_6;
                break;
            case XboxMotherboardRevision_UNKNOWN:
                hostVersion = EEPROM_EncryptInvalid;
                res = -4;
                goto endExec;
                break;
            }

            if(hostVersion != newVersion)
            {
                if(ConfirmDialog("EEPROM versions mismatch!\n\2Continue anyway?", true))
                {
                    res = -2;
                    goto endExec;
                }
            }

            XBlastLogger(DEBUG_GENERAL_UI, DBG_LVL_DEBUG, "Encrypt new image into version : %u", hostVersion);
            encryptEEPROMData(decryptedData, &result, hostVersion);

            // Save back to EEprom
            memcpy((unsigned char *)editeeprom, &result, sizeof(EEPROMDATA));

            //Recalculate checksums to be nice.
            EepromCRC((unsigned char *)editeeprom->Checksum2,(unsigned char *)editeeprom->SerialNumber,0x28);
            EepromCRC((unsigned char *)editeeprom->Checksum3,(unsigned char *)editeeprom->TimeZoneBias,0x5b);
        }
        else       //Content of new EEPROM image is not valid.
        {
            res = -1;   //loaded EEPROM image is not valid
        }
    }
endExec:
    if(verbose)
    {
        if(res == 0)
        {
            printk("\n\n           Success!.\n           EEPROM image successfully loaded to modified EEPROM buffer.");
        }
        else
        {
            printk("\n\n           Error!");
            if(res == -1)
            {
                printk("\n           Invalid EEPROM image file.");
            }
            else if(res == -2)
            {
                printk("\n           Aborted operation.");
            }
            else if(res == -3)
            {
                printk ("\n          EEPROM image file size wrong.");
            }
            else
            {
                printk ("\n          Unknown EEPROM error.");
            }
        }
    }

    return res;
}
