/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "HDDMenuActions.h"

void AssertLockUnlock(void *driveId){
    int nIndexDrive = *(int *)driveId;
    if((tsaHarddiskInfo[nIndexDrive].m_securitySettings &0x0004)==0x0004) {       //Drive is already locked
        UnlockHDD(nIndexDrive);
    }
    else {
        LockHDD(nIndexDrive);
    }
}

void LockHDD(int nIndexDrive) {
    u8 password[20];
    unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    int i;

    if (!Confirm("Confirm locking", "Yes", "No", 0)) return;    
    
    if (CalculateDrivePassword(nIndexDrive,password)) {
        printk("           Unable to calculate drive password - eeprom corrupt?");
        return;
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
}

void UnlockHDD(int nIndexDrive) {
    u8 password[20];
    unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    
    if (!Confirm("Confirm unlocking", "Yes", "No", 0)) return;    
    
    if (CalculateDrivePassword(nIndexDrive,password)) {
        printk("           Unable to calculate drive password - eeprom corrupt?  Aborting\n");
        return;
    }
    if (DriveSecurityChange(uIoBase, nIndexDrive, IDE_CMD_SECURITY_DISABLE, password)) {
        printk("           Failed!");
    }
    printk("\n\n\n\n\n           \2This drive is now unlocked.\n\n");
    printk("           \2Press Button A to continue");

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
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
