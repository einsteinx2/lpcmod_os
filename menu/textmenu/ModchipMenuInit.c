#include "lpcmod_v1.h"
#include "include/config.h"
#include "TextMenu.h"
#include "include/boot.h"
#include "ModchipMenuActions.h"

TEXTMENU *ModchipMenuInit(void) {

	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;

	menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
	memset(menuPtr,0x00,sizeof(TEXTMENU));
	strcpy(menuPtr->szCaption, "LPCMod settings");

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Idle timeout : %ds", LPCmodSettings.OSsettings.bootTimeout);
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Quickboot : %s",LPCmodSettings.OSsettings.Quickboot? "Yes" : "No");
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	if(LPCmodSettings.OSsettings.activeBank == BNK512)
		sprintf(itemPtr->szCaption,"Quickboot bank : 512KB");
	else if (LPCmodSettings.OSsettings.activeBank == BNK256)
		sprintf(itemPtr->szCaption,"Quickboot bank : 256KB");
	else if (LPCmodSettings.OSsettings.activeBank == BNKTSOP)
		sprintf(itemPtr->szCaption,"Quickboot bank : TSOP");
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	itemPtr->functionLeftPtr=decrementActiveBank;
	itemPtr->functionLeftDataPtr = &LPCmodSettings.OSsettings.activeBank;
	itemPtr->functionRightPtr=incrementActiveBank;
	itemPtr->functionRightDataPtr = &LPCmodSettings.OSsettings.activeBank;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Reset all settings");
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	return menuPtr;
}
