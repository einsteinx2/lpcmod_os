/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "HDDMenuActions.h"

void LockHDD(void *driveId) {
	int nIndexDrive = *(int *)driveId;
	u8 password[20];
	unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
	int i;

	if (!Confirm("Confirm locking", "Yes", "No", 0)) return;	
	
	if (CalculateDrivePassword(nIndexDrive,password)) {
		printk("           Unable to calculate drive password - eeprom corrupt?");
		return;
	}
	printk("\n\n\n\n\n           Gentoox Loader locks drives with a master password of\n\n           \"\2GENTOOX\2\"\n\n\n           Please remember this ");
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

void UnlockHDD(void *driveId) {
	int nIndexDrive = *(int *)driveId;
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
