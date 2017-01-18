/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MenuInits.h"
#include "include/boot.h"
#include "LEDMenuActions.h"
#include "xblast/settings/xblastSettings.h"
#include "string.h"

TEXTMENU* LEDMenuInit(void)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    
    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "LED menu");

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, getSpecialSettingString(SpecialSettingsPtrArrayIndexName_LEDColor, LED_GREEN));
    itemPtr->functionPtr = LEDGreen;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, getSpecialSettingString(SpecialSettingsPtrArrayIndexName_LEDColor, LED_RED));
    itemPtr->functionPtr = LEDRed;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, getSpecialSettingString(SpecialSettingsPtrArrayIndexName_LEDColor, LED_ORANGE));
    itemPtr->functionPtr = LEDOrange;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, getSpecialSettingString(SpecialSettingsPtrArrayIndexName_LEDColor, LED_CYCLE));
    itemPtr->functionPtr = LEDCycle;
    TextMenuAddItem(menuPtr, itemPtr);
    
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, getSpecialSettingString(SpecialSettingsPtrArrayIndexName_LEDColor, LED_OFF));
    itemPtr->functionPtr = LEDOff;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}
