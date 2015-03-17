/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"
#include "video.h"
#include "xbox.h"
#include "BootEEPROM.h"
#include "BootFlash.h"
#include "InfoMenuActions.h"


void ShowTemperature(void *whatever) {
    int c, cx;
    int f, fx;
    InfoHeader("Temperature");
    I2CGetTemperature(&c, &cx);
    f = ((9.0 / 5.0) * c) + 32.0;
    fx = ((9.0 / 5.0) * cx) + 32.0;
    VIDEO_ATTR=0xffc8c8c8;
    printk("CPU temperature: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%d°C / %d°F\n           ", c, f);
    VIDEO_ATTR=0xffc8c8c8;
    printk("GPU temperature: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%d°C / %d°F\n", cx, fx);
    UIFooter();
}

void ShowVideo(void *whatever) {
    InfoHeader("Video");
    VIDEO_ATTR=0xffc8c8c8;
    printk("Encoder: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", VideoEncoderName());
    VIDEO_ATTR=0xffc8c8c8;
    printk("Cable: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", AvCableName());
    UIFooter();
}

void ShowEeprom(void *whatever) {
    InfoHeader("EEPROM");
    BootEepromPrintInfo();
    UIFooter();
}

void ShowFlashChip(void *whatever) {
    InfoHeader("Flash device");
    BootShowFlashDevice();
    UIFooter();
}

void ShowCPUInfo(void *whatever){
    u32 temp;
    InfoHeader("CPU info");
    printk("CPU Frequency: ");
    VIDEO_ATTR=0xffc8c800;
    temp = getCPUFreq();
    printk("%X MHz\n", temp);
    UIFooter();
}

void InfoHeader(char *title) {
    printk("\n\n\n\n\n");
    VIDEO_ATTR=0xffffef37;
    printk("\2%s information:\2\n\n\n\n\n\n\n\n           ", title);
    VIDEO_ATTR=0xffc8c8c8;
}
