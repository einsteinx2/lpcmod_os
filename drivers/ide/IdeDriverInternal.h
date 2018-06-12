/*
 * IdeDriverInternal.h
 *
 *  Created on: Jun 8, 2018
 *      Author: cromwelldev
 */

#ifndef DRIVERS_IDE_IDEDRIVERINTERNAL_H_
#define DRIVERS_IDE_IDEDRIVERINTERNAL_H_

#include "IdeDriver.h"

int Internal_ATAPIDataRead(int nDriveIndex, void * pbBuffer, unsigned int block, int n_bytes);

#endif /* DRIVERS_IDE_IDEDRIVERINTERNAL_H_ */
