/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _BootLPCMod_H_
#define _BootLPCMod_H_

#include "xblast/settings/xblastSettingsDefs.h"
#include "xblastDebug.h"
#include "lpcmod_v1.h"
#include "config.h"
#include <stdbool.h>


//Globals to save value of LPC register
unsigned char xF70ELPCRegister;
unsigned char x00FFLPCRegister;

bool TSOPRecoveryMode;

FlashBank currentFlashBank;
unsigned char A19controlModBoot;

struct _GenPurposeIOs{
    bool GPO3;
    bool GPO2;
    bool GPO1;
    bool GPO0;

    bool GPI1;
    bool GPI0;

    bool A19BufEn;

    bool EN_5V;
}__attribute__((packed))GenPurposeIOs;  //byte-long struct.

unsigned short LPCMod_HW_rev(void);
void LPCMod_ReadIO(struct _GenPurposeIOs *GPIOstruct);
int LPCMod_ReadJPGFromHDD(const char *jpgFilename);
void LPCMod_WriteIO(unsigned char port, unsigned char value);
void LPCMod_FastWriteIO(unsigned char port, unsigned char value);
void LPCMod_WriteGenPurposeIOs(void);

void switchOSBank(FlashBank bank);
void switchBootBank(FlashBank bank);

void WriteToIO(unsigned short address, unsigned char data);
unsigned char ReadFromIO(unsigned short address);

#endif // _BootLPCMod_H_
