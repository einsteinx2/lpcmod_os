/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MenuInits.h"
#include "boot.h"
#include "NetworkMenuActions.h"
#include "string.h"
#include "stdio.h"
#include "xblast/settings/xblastSettingsDefs.h"

TEXTMENU* NetworkMenuInit(void)
{
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;

    menuPtr = (TEXTMENU*)calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Network Settings Menu");

    itemPtr = (TEXTMENUITEM*)calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Use DHCP : ");
    strcpy(itemPtr->szParameter, LPCmodSettings.OSsettings.useDHCP ? "Yes" : "No");
    itemPtr->functionPtr = toggleUseDHCP;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr = toggleUseDHCP;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr = toggleUseDHCP;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Static IP : ");
    sprintf(itemPtr->szParameter, "%u.%u.%u.%u",
             LPCmodSettings.OSsettings.staticIP[0],
             LPCmodSettings.OSsettings.staticIP[1],
             LPCmodSettings.OSsettings.staticIP[2],
             LPCmodSettings.OSsettings.staticIP[3]);
    itemPtr->functionPtr = editStaticIP;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Static Mask : ");
    sprintf(itemPtr->szParameter, "%u.%u.%u.%u",
             LPCmodSettings.OSsettings.staticMask[0],
             LPCmodSettings.OSsettings.staticMask[1],
             LPCmodSettings.OSsettings.staticMask[2],
             LPCmodSettings.OSsettings.staticMask[3]);
    itemPtr->functionPtr = editStaticMask;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Static Gateway : ");
    sprintf(itemPtr->szParameter, "%u.%u.%u.%u",
             LPCmodSettings.OSsettings.staticGateway[0],
             LPCmodSettings.OSsettings.staticGateway[1],
             LPCmodSettings.OSsettings.staticGateway[2],
             LPCmodSettings.OSsettings.staticGateway[3]);
    itemPtr->functionPtr = editStaticGateway;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Static DNS1 : ");
    sprintf(itemPtr->szParameter, "%u.%u.%u.%u",
             LPCmodSettings.OSsettings.staticDNS1[0],
             LPCmodSettings.OSsettings.staticDNS1[1],
             LPCmodSettings.OSsettings.staticDNS1[2],
             LPCmodSettings.OSsettings.staticDNS1[3]);
    itemPtr->functionPtr = editStaticDNS1;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Static DNS2 : ");
    sprintf(itemPtr->szParameter, "%u.%u.%u.%u",
             LPCmodSettings.OSsettings.staticDNS2[0],
             LPCmodSettings.OSsettings.staticDNS2[1],
             LPCmodSettings.OSsettings.staticDNS2[2],
             LPCmodSettings.OSsettings.staticDNS2[3]);
    itemPtr->functionPtr = editStaticDNS2;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}
