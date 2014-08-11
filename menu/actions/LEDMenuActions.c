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

void LEDGreen(void *whatever) {
	setLED("gggg");
}

void LEDRed(void *whatever) {
	setLED("rrrr");
}

void LEDOrange(void *whatever) {
	setLED("oooo");
}

void LEDCycle(void *whatever) {
	setLED("rgog");
}

void LEDOff(void *whatever) {
	setLED("xxxx");
}

void LEDFirstBoot(void *whatever) {
	setLED("roro");
}
