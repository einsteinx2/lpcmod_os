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
    TEXTMENUITEM * tempItemPtr = (TEXTMENUITEM *)&itemPtr;
    int nIndexDrive = 0;                                //Toggle master by default.

    //Not that cool to do but I don't want to change the function call in textmenu.c...
    if((strncmp(tempItemPtr->szParameter, "slave", 5)))    //If string in szParameter is different from "master"
        nIndexDrive = 1;                                //It means we need to change the slave lock status.

    if((tsaHarddiskInfo[nIndexDrive].m_securitySettings &0x0004)==0x0004) {       //Drive is already locked
        if(UnlockHDD(nIndexDrive))
            sprintf(tempItemPtr->szCaption, "%s", tempItemPtr->szParameter);                     //Next action will be to lock it
    }
    else {
        if(LockHDD(nIndexDrive))
            sprintf(tempItemPtr->szCaption, "%s", tempItemPtr->szParameter);
    }
}

bool LockHDD(int nIndexDrive) {
    u8 password[20];
    unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    int i;

    if (!Confirm("Confirm locking", "Yes", "No", 0)) return false;
    
    if (CalculateDrivePassword(nIndexDrive,password)) {
        printk("           Unable to calculate drive password - eeprom corrupt?");
        return false;
    }
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
    if (DriveSecurityChange(uIoBase, nIndexDrive, IDE_CMD_SECURITY_SET_PASSWORD, password)) {
        cromwellError();
        while(1);
    }
    cromwellSuccess();
    printk("           Make a note of the password above.\n");
    printk("           Press Button A to continue");

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
    return true;
}

bool UnlockHDD(int nIndexDrive) {
    u8 password[20];
    unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    
    if (!Confirm("Confirm unlocking", "Yes", "No", 0)) return false;
    
    if (CalculateDrivePassword(nIndexDrive,password)) {
        printk("           Unable to calculate drive password - eeprom corrupt?  Aborting\n");
        return false;
    }
    if (DriveSecurityChange(uIoBase, nIndexDrive, IDE_CMD_SECURITY_DISABLE, password)) {
        printk("           Failed!");
    }
    printk("\n\n\n\n\n           \2This drive is now unlocked.\n\n");
    printk("           \2Press Button A to continue");

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
    return true;
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
        
    FATXFormatCacheDrives(nIndexDrive);

    HDDMenuHeader("Cache drives formatted");
    HDDMenuFooter();
}

void FormatDriveC(void *driveId){
    int nIndexDrive = *(int *)driveId;

    if(ConfirmDialog("                  Confirm format C: drive?", 1))
        return;                                 //Cancel operation.
        
    FATXFormatDriveC(nIndexDrive);
    
    HDDMenuHeader("C: drive formatted");
    HDDMenuFooter();
}

void FormatDriveE(void *driveId){
    int nIndexDrive = *(int *)driveId;

    if(ConfirmDialog("                  Confirm format E: drive?", 1))
        return;                                 //Cancel operation.
        
    FATXFormatDriveE(nIndexDrive);    

    HDDMenuHeader("E: drive formatted");
    HDDMenuFooter();
}

void HDDMenuFooter(void) {
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n\n           Press Button 'A' to continue.");
    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}

void HDDMenuHeader(char *title) {
    printk("\n\n\n\n\n           ");
    VIDEO_ATTR=0xffffef37;
    printk("\2%s\2\n\n\n\n           ", title);
    VIDEO_ATTR=0xffc8c8c8;
}

void DisplayHDDInfo(void *driveId) {
    int nIndexDrive = *(int *)driveId;
    u8 MBRBuffer[512];
    u8 i;
    XboxPartitionTable * mbr = (XboxPartitionTable *)MBRBuffer;

    printk("\n           Hard Disk Drive(%s)", nIndexDrive ? "slave":"master");

    printk("\n\n\1           Model : %s", tsaHarddiskInfo[nIndexDrive].m_szIdentityModelNumber);
    printk("\n\1           Serial : %s", tsaHarddiskInfo[nIndexDrive].m_szSerial);
    printk("\n\1           Firmware : %s", tsaHarddiskInfo[nIndexDrive].m_szFirmware);
    printk("\n\1           Capacity : %u GB", (tsaHarddiskInfo[nIndexDrive].m_dwCountSectorsTotal / (2*1024*1024)));     //In GB
    printk("\n\1           Sectors : %u ", tsaHarddiskInfo[nIndexDrive].m_dwCountSectorsTotal);
    printk("\n\1           # conductors : %u ", tsaHarddiskInfo[nIndexDrive].m_bCableConductors);
    printk("\n\1           FATX Formatted? : %s ", tsaHarddiskInfo[nIndexDrive].m_enumDriveType==EDT_XBOXFS ? "Yes" : "No");
    printk("\n\1           Xbox MBR on HDD? : %s", tsaHarddiskInfo[nIndexDrive].m_fHasMbr ? "Yes" : "No");
    if(tsaHarddiskInfo[nIndexDrive].m_fHasMbr == 1){
        if(BootIdeReadSector(nIndexDrive, &MBRBuffer[0], 0x00, 0, 512)) {
            VIDEO_ATTR=0xffff0000;
            printk("\n\1                Unable to read MBR sector...\n");
        }
        else{
            for(i = 0; i < 14; i++){
                if(mbr->TableEntries[i].Name[0] != ' ' && mbr->TableEntries[i].LBAStart != 0){          //Valid partition entry only
                    printk("\n\1                 %s", mbr->TableEntries[i].Name);
                    printk("\n\1                     Active: %s", mbr->TableEntries[i].Flags == PE_PARTFLAGS_IN_USE ? "Yes" : "No");
                    printk("    Start at: %u    Size: %u", mbr->TableEntries[i].LBAStart, mbr->TableEntries[i].LBASize);
                }
            }
        }
    }
    HDDMenuFooter();
}

void FormatDriveFG(void *driveId) {
    u8 nDriveIndex = (*(u8 *)driveId) & 0x0f;
    u8 formatOption = (*(u8 *)driveId) & 0xf0;
    u32 fsize,gstart,gsize;

    u32 nExtendSectors = tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal - SECTOR_EXTEND;

    switch(formatOption){
        case F_GEQUAL:                                          //Split amount of sectors evenly on 2 partitions
            if(nExtendSectors % 2){                             //Odd number of sectors
                fsize = (nExtendSectors + 1) >> 1;              //F: will be 1 sector bigger than G:
                gsize = (nExtendSectors - 1) >> 1;              //Sorry G:
            }
            else{
                fsize = nExtendSectors >> 1;
                gsize = nExtendSectors >> 1;
            }
            if(fsize >= LBASIZE_1024GB)
                fsize = LBASIZE_1024GB - 1;
            if(gsize >= LBASIZE_1024GB)
                gsize = LBASIZE_1024GB - 1;
            gstart = SECTOR_EXTEND + fsize;
            break;
        case FMAX_G:            //F = LBASIZE_1024GB - 1 and G: takes the rest
            fsize = LBASIZE_1024GB - 1;
            gsize = nExtendSectors - fsize;
            gstart = SECTOR_EXTEND + fsize;
            break;
        case F137_G:            //F = LBASIZE_137GB and G takes the rest
            fsize = LBASIZE_137GB;
            gsize = nExtendSectors - fsize;
            if(gsize >= LBASIZE_1024GB)
                gsize = LBASIZE_1024GB - 1;
            gstart = SECTOR_EXTEND + fsize;
            break;
        case F_NOG:             //F < LBASIZE_1024GB - 1.
            fsize = nExtendSectors;
            gstart = SECTOR_EXTEND;
            gsize = 0;
            break;
        default:
            return;
            break;
    }
    if(!ConfirmDialog("                  Confirm format F: drive?", 1)){
        if(FATXFormatExtendedDrive(nDriveIndex, 5, SECTOR_EXTEND, fsize))                  //F: drive is partition 6 in table
            HDDMenuHeader("F: drive formatted");
        else
            HDDMenuHeader("F: format ERROR");

        HDDMenuFooter();
    }
    if(!ConfirmDialog("                  Confirm format G: drive?", 1)){
        if(FATXFormatExtendedDrive(nDriveIndex, 6, gstart, fsize))                         //G: drive is partition 7 in table
            HDDMenuHeader("G: drive formatted");
        else
            HDDMenuHeader("G: format ERROR");

        HDDMenuFooter();
    }
}
