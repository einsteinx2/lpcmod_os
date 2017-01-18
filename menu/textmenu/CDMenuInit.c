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
#include "BootIde.h"
#include "CDMenuActions.h"
#include "string.h"

TEXTMENU* CDMenuInit(void)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    int i = 0;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "CD Menu");

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption, "Eject CD");
    itemPtr->functionPtr = CDEject;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption, "Inject CD");
    itemPtr->functionPtr = CDInject;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);
        
    return menuPtr;
}
