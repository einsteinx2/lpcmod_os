/*
 *
 *
 */

#ifndef _BootLPCMod_H_
#define _BootLPCMod_H_

#include "VideoInitialization.h"

#define HDD4780_DEFAULT_NBLINES    4
#define HDD4780_DEFAULT_LINELGTH    20
#define DEFAULT_FANSPEED    20

void initialLPCModOSBoot(_LPCmodSettings *LPCmodSettings);

u16 LPCMod_HW_rev(void);

void LPCMod_ReadIO(struct _GenPurposeIOs *GPIOstruct);

void LPCMod_LCDBankString(char * string, u8 bankID);

#endif // _BootLPCMod_H_
