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
#include "LEDMenuActions.h"

void LEDGood(void *whatever) {
	LEDHeader("Good", "gxgx");
	goodLED();
	LEDFooter();
	inputLED();
}

void LEDError(void *whatever) {
	LEDHeader("Error", "rxxx");
	errorLED();
	LEDFooter();
	inputLED();
}

void LEDBusy(void *whatever) {
	LEDHeader("Busy", "rgog");
	busyLED();
	LEDFooter();
	inputLED();
}

void LEDImportant(void *whatever) {
	LEDHeader("Important", "roro");
	importantLED();
	LEDFooter();
	inputLED();
}

void LEDInput(void *whatever) {
	LEDHeader("Input", "oxox");
	inputLED();
	LEDFooter();
	inputLED();
}

void LEDDownloading(void *whatever) {
	LEDHeader("Downloading", "ogrr");
	downloadingLED();
	LEDFooter();
	inputLED();
}

void LEDUber(void *whatever) {
	LEDHeader("Uber load", "rxrx");
	uberLED();
	LEDFooter();
	inputLED();
}

void LEDHigh(void *whatever) {
	LEDHeader("High load", "rrrr");
	highLED();
	LEDFooter();
	inputLED();
}

void LEDMid(void *whatever) {
	LEDHeader("Mid load", "ooox");
	midLED();
	LEDFooter();
	inputLED();
}

void LEDLow(void *whatever) {
	LEDHeader("Low load", "gggg");
	lowLED();
	LEDFooter();
	inputLED();
}

void LEDHeader(char *name, char *pattern) {
	printk("\n\n\n\n\n           ");
	printk("This is what the '%s' pattern looks like throughout Gentoox\n\n           ", name);
	printk("It's sequence is '%s'.\n\n           ", pattern);
	printk("Press Button 'A' to continue.");
}
	
void LEDFooter(void) {
	while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}
