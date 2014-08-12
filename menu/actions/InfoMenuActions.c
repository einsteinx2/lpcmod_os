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
#include "InfoMenuActions.h"


void ShowTemperature(void *whatever) {
	int c, cx;
	int f, fx;
	InfoHeader("Temperature");
	if (I2CGetTemperature(&c, &cx)) {
		f = ((9.0 / 5.0) * c) + 32.0;
		fx = ((9.0 / 5.0) * cx) + 32.0;
		VIDEO_ATTR=0xffc8c8c8;
		printk("CPU temperature: ");
		VIDEO_ATTR=0xffc8c800;
		printk("%d°C / %d°F\n           ", c, f);
		VIDEO_ATTR=0xffc8c8c8;
		printk("Board temperature: ");
		VIDEO_ATTR=0xffc8c800;
		printk("%d°C / %d°F", cx, fx);
	} else {
		VIDEO_ATTR=0xffff0000;
      printk("Error getting temperatures!  ");
		VIDEO_ATTR=0xffc8c8c8;
		printk("This does not work on Xbox v1.6+.");
	}
	InfoFooter();
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
	InfoFooter();
}

void ShowEeprom(void *whatever) {
	InfoHeader("EEPROM");
	BootEepromPrintInfo();
	InfoFooter();
}

void InfoHeader(char *title) {
	printk("\n\n\n\n\n           ");
	VIDEO_ATTR=0xffffef37;
	printk("\2%s information:\2\n\n\n\n\n\n\n\n           ", title);
	VIDEO_ATTR=0xffc8c8c8;
}

void InfoFooter(void) {
	VIDEO_ATTR=0xffc8c8c8;
	printk("\n\n           Press Button 'A' to continue.");
	while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}
