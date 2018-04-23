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
void formatNewDrives(void);

void LPCMod_WriteIO(unsigned char port, unsigned char value);
void LPCMod_FastWriteIO(unsigned char port, unsigned char value);
void LPCMod_WriteGenPurposeIOs(void);

void quickboot(unsigned char bank);

void switchOSBank(FlashBank bank);
void switchBootBank(FlashBank bank);

void WriteToIO(unsigned short address, unsigned char data);
unsigned char ReadFromIO(unsigned short address);

/**
 * USB_DEVICE - macro used to describe a specific usb device
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific device.
 */
#define USB_DEVICE_XBLAST(vend,prod, extra) \
    .match_flags = USB_DEVICE_ID_MATCH_DEVICE, .idVendor = (vend), .idProduct = (prod), .driver_info = extra

#endif // _BootLPCMod_H_
