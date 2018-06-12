/*
 * IdeDriveInfo.h
 *
 *  Created on: Apr 7, 2018
 *      Author: cromwelldev
 */

#ifndef DRIVERS_IDEDRIVEINFO_H_
#define DRIVERS_IDEDRIVEINFO_H_

unsigned long long BootIdeGetSectorCount(int nDriveIndex);
int BootIdeGetSectorSize(int nDriveIndex);
int BootIdeDeviceConnected(int nDriveIndex);
int BootIdeDeviceIsATAPI(int nDriveIndex);

#endif /* DRIVERS_IDEDRIVEINFO_H_ */
