/*
 * FatFSTestHelper.h
 *
 *  Created on: Mar 31, 2018
 *      Author: cromwelldev
 */

#ifndef FATFSTESTHELPER_H_
#define FATFSTESTHELPER_H_

unsigned long long BootIdeGetSectorCount(unsigned char drive);
int BootIdeDeviceConnected(int nDriveIndex);
int BootIdeDeviceIsATAPI(int nDriveIndex);

#endif /* FATFSTESTHELPER_H_ */
