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

void AssertLockUnlock(void *driveId);
void LockHDD(int nIndexDrive);
void UnlockHDD(int nIndexDrive);

void DisplayHDDPassword(void *driveId);

void FormatCacheDrives(void *driveId);
void FormatDriveC(void *driveId);
void FormatDriveE(void *driveId);

void HDDMenuHeader(char *title);
void HDDMenuFooter(void);
