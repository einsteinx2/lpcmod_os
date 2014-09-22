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
bool LockHDD(int nIndexDrive, bool verbose);
bool UnlockHDD(int nIndexDrive, bool verbose);

void DisplayHDDPassword(void *driveId);

void FormatCacheDrives(void *driveId);
void FormatDriveC(void *driveId);
void FormatDriveE(void *driveId);

void HDDMenuHeader(char *title);
void HDDMenuFooter(void);

void DisplayHDDInfo(void *driveId);

void FormatDriveFG(void *driveId);
