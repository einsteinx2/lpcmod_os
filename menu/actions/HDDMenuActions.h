/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "boot.h"
#include "BootIde.h"
#include "video.h"
#include "BootFATX.h"

#define F_GEQUAL 0x10
#define FMAX_G 0x20
#define F137_G 0x40
#define F_NOG 0x80

void AssertLockUnlock(void *driveId);
void AssertLockUnlockFromNetwork(void *itemPtr);
bool LockHDD(int nIndexDrive, bool verbos, unsigned char *eepromPtr);
int UnlockHDD(int nIndexDrive, bool verbose, unsigned char *eepromPtr, bool internalEEPROM);
bool masterPasswordUnlockSequence(int nIndexDrive);

void DisplayHDDPassword(void *driveId);

void FormatCacheDrives(void *driveId);
void FormatDriveC(void *driveId);
void FormatDriveE(void *driveId);

void HDDMenuHeader(char *title);

void DisplayHDDInfo(void *driveId);

void FormatDriveFG(void *driveId);

void AssertSMARTEnableDisable(void *itemPtr);
void CheckSMARTRETURNSTATUS(void * drive);
