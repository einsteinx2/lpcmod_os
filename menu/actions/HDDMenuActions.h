/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdbool.h>

#define F_GEQUAL 0x10
#define FMAX_G 0x20
#define F137_G 0x40
#define F_NOG 0x80

typedef struct
{
    unsigned char driveIndex;
    char* string1;
    char* string2;
}LockUnlockCommonParams;

void AssertLockUnlock(void* customStructPtr);
void AssertLockUnlockFromNetwork(void* customStructPtr);
bool LockHDD(int nIndexDrive, bool verbos, unsigned char* eepromPtr);
int UnlockHDD(int nIndexDrive, bool verbose, unsigned char* eepromPtr, bool internalEEPROM);
bool masterPasswordUnlockSequence(int nIndexDrive);

void DisplayHDDPassword(void* customString);

void FormatCacheDrives(void* driveId);
void FormatDriveC(void* driveId);
void FormatDriveE(void* driveId);

void DisplayHDDInfo(void* driveId);

void FormatDriveFG(void* driveId);

void AssertSMARTEnableDisable(void* customString);
void CheckSMARTRETURNSTATUS(void* customString);
