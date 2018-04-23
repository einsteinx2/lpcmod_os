/*
 * FatFSTestHelper.c
 *
 *  Created on: Mar 31, 2018
 *      Author: cromwelldev
 */

#include "FatFSTestHelper.h"
#include "diskio.h"

unsigned long long BootIdeGetSectorCount(unsigned char drive)
{
    QWORD secCount = 0;
    disk_ioctl(drive, GET_SECTOR_COUNT, &secCount);

    return secCount;
}

int BootIdeDeviceConnected(int nDriveIndex)
{
    return nDriveIndex ? 0: 1;
}

int BootIdeDeviceIsATAPI(int nDriveIndex)
{
    return 0;
}
