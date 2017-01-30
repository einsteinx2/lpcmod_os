/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"
#include "i2c.h"
#include "video.h"
#include "BootEEPROM.h"
#include "FlashUi.h"
#include "InfoMenuActions.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/time/timeManagement.h"
#include "lib/cromwell/cromString.h"
#include "xblast/settings/xblastSettingsChangeTracker.h"
#include "xblast/settings/xblastSettingsImportExport.h"
#include "MenuActions.h"
#include "string.h"
#include "lib/LPCMod/xblastDebug.h"
#include <stddef.h>

void ShowTemperature(void *whatever)
{
    int c, cx;
    int f, fx;
    UiHeader("Temperature");
    I2CGetTemperature(&c, &cx);
    f = ((9.0 / 5.0) * c) + 32.0;
    fx = ((9.0 / 5.0) * cx) + 32.0;
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n           CPU temperature: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%d°C / %d°F", c, f);
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n           Board temperature: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%d°C / %d°F", cx, fx);
    UIFooter();
}

void ShowVideo(void *whatever)
{
    UiHeader("Video");
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n           Encoder: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", VideoEncoderName());
    VIDEO_ATTR=0xffc8c8c8;
    printk("Cable: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", AvCableName());
    UIFooter();
}

void ShowEeprom(void *whatever)
{
    UiHeader("EEPROM");
    BootEepromPrintInfo();
    UIFooter();
}

void ShowFlashChip(void *whatever)
{
    UiHeader("Flash device");
    BootShowFlashDevice();
    UIFooter();
}
