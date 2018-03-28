/*
 * PartTable.h
 *
 *  Created on: Mar 27, 2018
 *      Author: cromwelldev
 */

#ifndef FS_FF12B_SRC_FATFSACCESSOR_H_
#define FS_FF12B_SRC_FATFSACCESSOR_H_

#define NbDrivesSupported 2
#define HDD_Master  0
#define HDD_Slave   0

#define NbFATXPartPerHDD 7
#define Part_C    1
#define Part_E    0
#define Part_F    5
#define Part_G    6
#define Part_X    2
#define Part_Y    3
#define Part_Z    4



void FatFS_init(void);
int mountBasic(unsigned char driveNumber);

#endif /* FS_FF12B_SRC_FATFSACCESSOR_H_ */
