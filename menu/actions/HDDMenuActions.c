/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "HDDMenuActions.h"
#include "boot.h"
#include "TextMenu.h"

void AssertLockUnlock(void *itemPtr){
    TEXTMENUITEM * tempItemPtr = (TEXTMENUITEM *)itemPtr;
    int nIndexDrive = 1;                                //Toggle master by default.

    //Not that cool to do but I don't want to change the function call in textmenu.c...
    nIndexDrive = tempItemPtr->szParameter[50];

    if((tsaHarddiskInfo[nIndexDrive].m_securitySettings &0x0002)==0x0002) {       //Drive is already locked
        UnlockHDD(nIndexDrive, 1);                                        //1 is for verbose
    }
    else {
        LockHDD(nIndexDrive, 1);                                                 //1 is for verbose
    }
    if((tsaHarddiskInfo[nIndexDrive].m_securitySettings &0x0002)==0x0002) {
        sprintf(tempItemPtr->szCaption, "%s", "Lock HDD");
    }
    else{
        sprintf(tempItemPtr->szCaption, "%s", "Unlock HDD");
    }
}

bool LockHDD(int nIndexDrive, bool verbose) {
    u8 password[20];
    unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    int i;

    if(verbose){
        if (ConfirmDialog("                      Confirm Lock HDD?", 1)) return false;
    }
    
    if (CalculateDrivePassword(nIndexDrive,password)) {
        printk("           Unable to calculate drive password - eeprom corrupt?");
        while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
        return false;
    }
    if(verbose){
        printk("\n\n\n\n\n           XBlast OS locks drives with a master password of\n\n           \"\2TEAMASSEMBLY\2\"\n\n\n           Please remember this ");
        printk("as it could save your drive!\n\n");
        printk("           The normal password (user password) the drive is\n           being locked with is as follows:\n\n");
        printk("                              ");
        VIDEO_ATTR=0xffef37;
        for (i=0; i<20; i++) {
            printk("\2%02x \2",password[i]);
            if ((i+1)%5 == 0) {
                printk("\n\n                              ");
            }
        }    
        VIDEO_ATTR=0xffffff;
        printk("\n           Locking drive");
        dots();
    }
    if (DriveSecurityChange(uIoBase, nIndexDrive, IDE_CMD_SECURITY_SET_PASSWORD, password)) {
        printk("\n           Locking drive failed");
        cromwellError();
        while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
        return false;
    }
    if(verbose){
        cromwellSuccess();
        printk("           Make a note of the password above.\n");
        printk("           Press Button A to continue");

        while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
    }
    return true;
}

bool UnlockHDD(int nIndexDrive, bool verbose) {
    u8 password[20];
    bool result = false; //Start assuming not good.
    unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    if(tsaHarddiskInfo[nIndexDrive].m_securitySettings & 0x0010){            //Unlock attempt counter expired
        printk("\n\n\n\n\n           \2Drive is now locked out.\n           \2Reboot system to reset HDD unlock capabilities.\n\n");
        printk("           \2Press Button A to continue");
        while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
        return false;
    }
    if(verbose){
        if (ConfirmDialog("                    Confirm Unlock HDD?", 1)) return false;
    }
    if((tsaHarddiskInfo[nIndexDrive].m_securitySettings&0x0004)==0x0004){
        printk("\n\n           Something's wrong with the drive!\n           Jumping to Master Password Unlock sequence.");
        if(!masterPasswordUnlockSequence(nIndexDrive)){
            result = false;
            verbose = true;
        }
    }
    else{
        if (CalculateDrivePassword(nIndexDrive,password)) {
            printk("\n\n           Unable to calculate drive password - eeprom corrupt?");
            if(!masterPasswordUnlockSequence(nIndexDrive)){
                result = false;
                verbose = true;
            }
            else{
                result = true;
            }
        }
        else{
            if (DriveSecurityChange(uIoBase, nIndexDrive, IDE_CMD_SECURITY_DISABLE, password)) {
                printk("\n           Security disable failed!");
                if(!masterPasswordUnlockSequence(nIndexDrive)){
                    result = false;
                    verbose = true;
                }
            }
            else{
                    result = true;
            }
        }
    }
    if(verbose && result)
        printk("\n\n\n\n\n           \2This drive is now unlocked.\n\n");
    if(verbose){
        printk("           \2Press Button A to continue");
        while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
    }
    return result;
}

bool masterPasswordUnlockSequence(int nIndexDrive){
    bool result = false;    //Assume not working.
    u8 i;
    unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    const char * MasterPasswordList[] = {
            "TEAMASSEMBLY",
            "XBOXSCENE",
            "Seagate                         ",
            "WDCWDCWDCWDCWDCWDCWDCWDCWDCWDCWD"
    };
    printk("\n           Trying Master Password unlock.");
    for(i = 0; i < 4; i++){
        if(!(tsaHarddiskInfo[nIndexDrive].m_securitySettings&0x0010)){           //Drive is not locked out.
            if(!driveMasterPasswordUnlock(uIoBase, nIndexDrive, MasterPasswordList[i])){
                printk("\n           Master Password(%s) Unlock failed...", MasterPasswordList[i]);
            }
            else{
                printk("\n           Unlock Using Master Password %s successful.", MasterPasswordList[i]);
                result = true;
                i = 4;
                break;
            }
        }
        else{
            printk("\n           Drive is locked out. No further unlock attempts possible.\n           Power cycle console to reset HDD state.");
            i = 4;
            break;
        }
    }
    printk("\n          Master Password Unlock failed.\n          No suitable password found.");
    return result;
}


void DisplayHDDPassword(void *driveId) {
    int nIndexDrive = *(int *)driveId;
    u8 password[20];
    int i;
    
    printk("\n\n\n\n\n           Calculating password");
    dots();
    if (CalculateDrivePassword(nIndexDrive,password)) {
        cromwellError();
        wait_ms(2000);
        return;
    }
    
    cromwellSuccess();

    printk("           The normal password (user password) for this drive is as follows:\n\n");
    printk("                              ");
    VIDEO_ATTR=0xffef37;
    for (i=0; i<20; i++) {
        printk("\2%02x \2",password[i]);
        if ((i+1)%5 == 0) {
            printk("\n\n                              ");
        }
    }    
    VIDEO_ATTR=0xffffff;
    printk("\n\n           Press Button A to continue.");

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}

void FormatCacheDrives(void *driveId){
    int nIndexDrive = *(int *)driveId;

    if(ConfirmDialog("             Confirm format cache drives?", 1))
        return;                                 //Cancel operation.

    HDDMenuHeader("Format cache drives");
    FATXFormatCacheDrives(nIndexDrive, 1);      //'1' for verbose
    HDDMenuFooter();
}

void FormatDriveC(void *driveId){
    int nIndexDrive = *(int *)driveId;

    if(ConfirmDialog("                  Confirm format C: drive?", 1))
        return;                                 //Cancel operation.
        
    HDDMenuHeader("Format C: drive");      //'1' for verbose
    FATXFormatDriveC(nIndexDrive, 1);
    HDDMenuFooter();
}

void FormatDriveE(void *driveId){
    int nIndexDrive = *(int *)driveId;

    if(ConfirmDialog("                  Confirm format E: drive?", 1))
        return;                                 //Cancel operation.

    HDDMenuHeader("Format E: drive");      //'1' for verbose
    FATXFormatDriveE(nIndexDrive, 1);
    HDDMenuFooter();
}

void HDDMenuFooter(void) {
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n\n           Press Button 'A' to continue.");
    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}

void HDDMenuHeader(char *title) {
    BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
    printk("\n\n\n\n\n           ");
    VIDEO_ATTR=0xffffef37;
    printk("\2%s\2\n           ", title);
    VIDEO_ATTR=0xffc8c8c8;
}

void DisplayHDDInfo(void *driveId) {
    int nIndexDrive = *(int *)driveId;
    u8 MBRBuffer[512];
    u8 i;
    XboxPartitionTable * mbr = (XboxPartitionTable *)MBRBuffer;
    u8 clusterSize;
    u32 partSize;

    printk("\n           Hard Disk Drive(%s)", nIndexDrive ? "slave":"master");

    printk("\n\n\1           Model : %s", tsaHarddiskInfo[nIndexDrive].m_szIdentityModelNumber);
    printk("\n\1           Serial : %s", tsaHarddiskInfo[nIndexDrive].m_szSerial);
    printk("\n\1           Firmware : %s", tsaHarddiskInfo[nIndexDrive].m_szFirmware);
    printk("\n\1           Capacity : %uGB", tsaHarddiskInfo[nIndexDrive].m_dwCountSectorsTotal / (2*1024*1024));     //In GB
    printk("\n\1           Sectors : %u ", tsaHarddiskInfo[nIndexDrive].m_dwCountSectorsTotal);
    printk("\n\1           # conductors : %u ", tsaHarddiskInfo[nIndexDrive].m_bCableConductors);
//    printk("\n\1           Sectors-blocks : %u ", tsaHarddiskInfo[nIndexDrive].m_maxBlockTransfer);    
//    printk("\n\1           PIO cycletime : %u ", tsaHarddiskInfo[nIndexDrive].m_minPIOcycle);           //Mostly useful for debug. Will not print.
    printk("\n\1           Lock Status : %s ", ((tsaHarddiskInfo[nIndexDrive].m_securitySettings &0x0002)==0x0002) ? "Locked" : "Unlocked");
    printk("\n\1           FATX Formatted? : %s ", tsaHarddiskInfo[nIndexDrive].m_enumDriveType==EDT_XBOXFS ? "Yes" : "No");
    printk("\n\1           Xbox MBR on HDD? : %s", tsaHarddiskInfo[nIndexDrive].m_fHasMbr ? "Yes" : "No");
    if(tsaHarddiskInfo[nIndexDrive].m_fHasMbr){
        if(BootIdeReadSector(nIndexDrive, &MBRBuffer[0], 0x00, 0, 512)) {
            VIDEO_ATTR=0xffff0000;
            printk("\n\1                Unable to read MBR sector...\n");
        }
        else{
            for(i = 0; i < 7; i++){     //Print only info for C, E, F, G, X, Y and Z
                if(mbr->TableEntries[i].Name[0] != ' ' && mbr->TableEntries[i].LBAStart != 0){          //Valid partition entry only
                    printk("\n\1                 %s", mbr->TableEntries[i].Name);
                    printk("\n\1                     Active: %s", mbr->TableEntries[i].Flags == PE_PARTFLAGS_IN_USE ? "Yes" : "No");
                    if(mbr->TableEntries[i].LBASize >= LBASIZE_512GB)           //Need 64K clusters
                        clusterSize = 64;                                      //Clustersize in number of 512-byte sectors
                    else if(mbr->TableEntries[i].LBASize >= LBASIZE_256GB)
                        clusterSize = 32;
                    else if(mbr->TableEntries[i].LBASize >= 1)
                        clusterSize = 16;
                    else
                    	clusterSize = 0;
                    partSize = mbr->TableEntries[i].LBASize / 2048;      //in MB
                        printk("    Size: %uMB   Cluster: %uKB", partSize, clusterSize);
                }
            }
        }
    }
    HDDMenuFooter();
}

void FormatDriveFG(void *driveId) {
    u8 nDriveIndex = (*(u8 *)driveId) & 0x0f;
    u8 formatOption = (*(u8 *)driveId) & 0xf0;
    u32 fsize,gstart = SECTOR_EXTEND,gsize = 0;
    u8 buffer[512];                                             //Multi purpose
    XboxPartitionTable * mbr = (XboxPartitionTable *)buffer;

    u32 nExtendSectors = tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal - SECTOR_EXTEND;

    switch(formatOption){
        case F_GEQUAL:                                          //Split amount of sectors evenly on 2 partitions
            if(nExtendSectors % 2){                             //Odd number of sectors
                fsize = (nExtendSectors + 1) >> 1;              //F: will be 1 sector bigger than G:            //Sorry G:
            }
            else{
                fsize = nExtendSectors >> 1;
            }
            if(fsize >= LBASIZE_1024GB)
                fsize = LBASIZE_1024GB - 1;
            sprintf(buffer, "                         %s", "Confirm format:\n\n\2                      F:, G: Split evenly?");
            break;
        case FMAX_G:            //F = LBASIZE_1024GB - 1 and G: takes the rest
            fsize = LBASIZE_1024GB - 1;
            sprintf(buffer, "                         %s", "Confirm format:\n\n\2                 Max F:, G: takes the rest?");
            break;
        case F137_G:            //F = LBASIZE_137GB and G takes the rest
            fsize = LBASIZE_137GB;
            sprintf(buffer, "                         %s", "Confirm format:\n\n\2            F: = 120GB, G: takes the rest?");
            break;
        case F_NOG:             //F < LBASIZE_1024GB - 1.
            fsize = nExtendSectors;
            sprintf(buffer, "                         %s", "Confirm format:\n\n\2                       F: take all, no G:?");
            break;
        default:
            return;
            break;
    }
    gstart = SECTOR_EXTEND + fsize + 1;
    gsize = nExtendSectors - fsize;
    if(gsize >= LBASIZE_1024GB)
        gsize = LBASIZE_1024GB - 1;
    if(!ConfirmDialog(buffer, 1)){
        HDDMenuHeader("Format F: drive");
        FATXFormatExtendedDrive(nDriveIndex, 5, SECTOR_EXTEND, fsize);          //F: drive is partition 5 in table
        HDDMenuFooter();

        if(formatOption != F_NOG){
            HDDMenuHeader("Format G: drive");
            FATXFormatExtendedDrive(nDriveIndex, 6, gstart, gsize);             //G: drive is partition 6 in table
            HDDMenuFooter();
        }
        else{       //Print G drive entry in partition table being inactive and of null size.
            if(tsaHarddiskInfo[nDriveIndex].m_fHasMbr == 1) {       //No need to do anything if no MBR is on disk.
               if(BootIdeReadSector(nDriveIndex, &buffer[0], 0x00, 0, 512)) {
                    VIDEO_ATTR=0xffff0000;
                    printk("\n\1                Unable to read MBR sector...\n");
                    HDDMenuFooter();
                    return;
                }
                else{
                    mbr->TableEntries[6].Flags = 0;
                    mbr->TableEntries[6].LBAStart = SECTOR_EXTEND;
                    mbr->TableEntries[6].LBASize = 0;
                    FATXSetMBR(nDriveIndex, mbr);
                }
            }
        }
    }
}

void AssertSMARTEnableDisable(void *itemPtr){
    TEXTMENUITEM * tempItemPtr = (TEXTMENUITEM *)itemPtr;
    int nIndexDrive = 1;                                //Toggle master by default.

    //Not that cool to do but I don't want to change the function call in textmenu.c...
    nIndexDrive = tempItemPtr->szParameter[50];

    if(tsaHarddiskInfo[nIndexDrive].m_fSMARTEnabled) {       //Drive is already locked
        driveToggleSMARTFeature(nIndexDrive, 0xD9);          //0xD9 is subcommand for disabling SMART.
    }
    else {
        driveToggleSMARTFeature(nIndexDrive, 0xD8);          //0xD8 is subcommand for disabling SMART.                                        //1 is for verbose
    }
    if(tsaHarddiskInfo[nIndexDrive].m_fSMARTEnabled) {
        sprintf(tempItemPtr->szCaption, "%s", "Disable");
    }
    else{
        sprintf(tempItemPtr->szCaption, "%s", "Enable");
    }
    sprintf(tempItemPtr->szParameter, " S.M.A.R.T.");
}
