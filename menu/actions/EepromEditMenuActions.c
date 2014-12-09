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
#include "lpcmod_v1.h"

void LastResortRecovery(void *ignored){
    const char *xbox_mb_rev[8] = {
        "DevKit",
        "DebugKit",
        "1.0",
        "1.1",
        "1.2/1.3",
        "1.4/1.5",
        "1.6/1.6b",
        "Unknown"
    };
    char revString[50];
    u8 nIndexDrive, unlockConfirm[2], mbVersion = I2CGetXboxMBRev();
    if(ConfirmDialog("        Warning: overwrite EEPROM with generic image?", 1))       //First generic warning
        return;
    sprintf(revString, "        Overwriting with %s EEPROM image. OK?", xbox_mb_rev[mbVersion]);
    if(ConfirmDialog(revString, 1))             //Second warning for user to assert if Xbox revision detection is right.
        return;
    for(nIndexDrive = 0; nIndexDrive < 2; nIndexDrive++){               //Probe 2 possible drives
        if(tsaHarddiskInfo[nIndexDrive].m_fDriveExists && !tsaHarddiskInfo[nIndexDrive].m_fAtapi){      //If there's a HDD plugged on specified port
            if((tsaHarddiskInfo[nIndexDrive].m_securitySettings &0x0002)==0x0002) {       //If drive is locked
                    if(UnlockHDD(nIndexDrive, 0))                                             //0 is for silent
                        unlockConfirm[nIndexDrive] = 1;                                   //Everything went well, we<ll relock after eeprom write.
                    else{
                        unlockConfirm[0] = 255;       //error
                        unlockConfirm[1] = 255;       //error
                        break;
                    }
            }
            else{
                unlockConfirm[nIndexDrive] = 0;                                         //Drive not locked, won't relock after eeprom write.
            }
        }
        else{
            unlockConfirm[nIndexDrive] = 0;       //Won't relock as no HDD was detected on that port.
        }
    }
    if(unlockConfirm[0] != 255 && unlockConfirm[1] != 255){      //No reported error
        switch(mbVersion){
            //TODO: Validate if all DevKits and DebugKits are 1.0!
            case DEVKIT:
            case DEBUGKIT:
            case REV1_0:
                memcpy(&eeprom, EEPROMimg10, sizeof(EEPROMDATA));
                break;
            case REV1_1:
                memcpy(&eeprom, EEPROMimg11, sizeof(EEPROMDATA));
                break;
            case REV1_2:
                memcpy(&eeprom, EEPROMimg12, sizeof(EEPROMDATA));
                break;
            case REV1_4:
                memcpy(&eeprom, EEPROMimg14, sizeof(EEPROMDATA));
                break;
            case REV1_6:
                memcpy(&eeprom, EEPROMimg16, sizeof(EEPROMDATA));
                break;
        }
        for(nIndexDrive = 0; nIndexDrive < 2; nIndexDrive++){               //Probe 2 possible drives
            if(unlockConfirm[nIndexDrive] == 1){
                LockHDD(nIndexDrive, 0);                                //0 is for silent mode.
            }
        }

        SlowReboot(NULL);
        while(1);
    }
}

void bruteForceFixDisplayresult(void *ignored){
    u8 eepromVersion;
    char unused[20];
    ToolHeader("Brute Force Fix EEPROM");
    eepromVersion = BootHddKeyGenerateEepromKeyData(eeprom,unused);

    if(eepromVersion == 13){
        if(bruteForceFixEEprom()){
            //success
            printk("\n\n\2           Successfully fixed!");
        }
        else{
            printk("\n\n\2           EEPROM could be fixed...");
        }
    }
    else{
        printk("\n\n\2           Brute force fix not useful here. Aborting.");
    }
    ToolFooter();
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
            memcpy(teeprom,&eeprom,256);
            teeprom[bytepos]=bytecombinations;
            ver = BootHddKeyGenerateEepromKeyData(teeprom,unused);
            if (ver!=13) {
                memcpy(&eeprom,teeprom,256);
                free(teeprom);
                return true;
            }
        }
    }

    free(teeprom);
    return false;      // No Match found
}

void confirmSaveToEEPROMChip(void *ignored){
    if(ConfirmDialog("        Save edited EEPROM buffer to physical chip?", 1))
            return;
    memcpy(&eeprom, editeeprom, sizeof(EEPROMDATA));   //Copy back edition buffer to main eeprom buffer.
    ToolHeader("Saved EEPROM edition buffer to physical EEPROM.");
    ToolFooter();
    SlowReboot(NULL);   //This function will take care of saving eeprom image to chip.
    while(1);
}
