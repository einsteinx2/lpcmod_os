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
#include "BootEEPROM.h"
#include "LEDMenuActions.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include <stddef.h>

void LEDGreen(void *whatever) {
    setLED("gggg");
    LPCmodSettings.OSsettings.LEDColor = LED_GREEN;
}

void LEDRed(void *whatever) {
    setLED("rrrr");
    LPCmodSettings.OSsettings.LEDColor = LED_RED;
}

void LEDOrange(void *whatever) {
    setLED("oooo");
    LPCmodSettings.OSsettings.LEDColor = LED_ORANGE;
}

void LEDCycle(void *whatever) {
    setLED("rgog");
    LPCmodSettings.OSsettings.LEDColor = LED_CYCLE;
}

void LEDOff(void *whatever) {
    setLED("xxxx");
    LPCmodSettings.OSsettings.LEDColor = LED_OFF;
}

void LEDFirstBoot(void *whatever) {
    setLED("roro");
    LPCmodSettings.OSsettings.LEDColor = LED_GREEN;        //Just in case
}

void initialSetLED(unsigned char ledChoice) {
    if(ledChoice == LED_RED)
            LEDRed(NULL);
    else if(ledChoice == LED_ORANGE)
            LEDOrange(NULL);
    else if(ledChoice == LED_CYCLE)
            LEDCycle(NULL);
    else if(ledChoice == LED_OFF)
            LEDOff(NULL);
    else
            LEDGreen(NULL);
}
