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
	strcpy(menuPtr->szCaption, "XBlast settings");

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption,"Idle timeout : ");
	sprintf(itemPtr->szParameter, "%ds", LPCmodSettings.OSsettings.bootTimeout);
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	itemPtr->functionLeftPtr=decrementbootTimeout;
	itemPtr->functionLeftDataPtr = itemPtr->szParameter;
	itemPtr->functionRightPtr=incrementbootTimeout;
	itemPtr->functionRightDataPtr = itemPtr->szParameter;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption,"Quickboot : ");
	sprintf(itemPtr->szParameter, "%s", LPCmodSettings.OSsettings.Quickboot? "Yes" : "No");
	itemPtr->functionPtr= toggleQuickboot;
	itemPtr->functionDataPtr= itemPtr->szParameter;
	itemPtr->functionLeftPtr=toggleQuickboot;
	itemPtr->functionLeftDataPtr = itemPtr->szParameter;
	itemPtr->functionRightPtr=toggleQuickboot;
	itemPtr->functionRightDataPtr = itemPtr->szParameter;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption,"Quickboot bank : ");
	if(LPCmodSettings.OSsettings.activeBank == BNK512)
		strcpy(itemPtr->szParameter,"512KB");
	else if (LPCmodSettings.OSsettings.activeBank == BNK256)
		strcpy(itemPtr->szParameter,"256KB");
	else if (LPCmodSettings.OSsettings.activeBank == BNKTSOP)
		if(LPCmodSettings.OSsettings.TSOPcontrol & 0x01)
			strcpy(itemPtr->szParameter,"TSOP bank0");			//Show this string if TSOP is split.
		else
			strcpy(itemPtr->szParameter,"TSOP");				//TSOP is not split.
	else if (LPCmodSettings.OSsettings.activeBank == BNKTSOP1)
		strcpy(itemPtr->szParameter,"TSOP bank1");
	else if (LPCmodSettings.OSsettings.activeBank == BNKTSOP2)
		strcpy(itemPtr->szParameter,"TSOP bank2");
	else if (LPCmodSettings.OSsettings.activeBank == BNKTSOP3)
		strcpy(itemPtr->szParameter,"TSOP bank3");
	itemPtr->functionPtr=incrementActiveBank;
	itemPtr->functionDataPtr = itemPtr->szParameter;
	itemPtr->functionLeftPtr=decrementActiveBank;
	itemPtr->functionLeftDataPtr = itemPtr->szParameter;
	itemPtr->functionRightPtr=incrementActiveBank;
	itemPtr->functionRightDataPtr = itemPtr->szParameter;
	TextMenuAddItem(menuPtr, itemPtr);

if(mbVersion == REV1_1 || mbVersion == REV1_0){		//Don't show this when Xbox motherboard is not 1.0/1.1.
/*
 * The 2 following menu entries must be in line.
 *
 *
 */
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption,"Control Xbox TSOP : ");
	sprintf(itemPtr->szParameter, "%s", (LPCmodSettings.OSsettings.TSOPcontrol) & 0x01? "Yes" : "No");
	itemPtr->functionPtr= toggleTSOPControl;
	itemPtr->functionDataPtr= itemPtr;
	itemPtr->functionLeftPtr=toggleTSOPControl;
	itemPtr->functionLeftDataPtr = itemPtr;
	itemPtr->functionRightPtr=toggleTSOPControl;
	itemPtr->functionRightDataPtr = itemPtr;

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption,"Xbox TSOP split : ");
	sprintf(itemPtr->szParameter, "%s",		//Print "No" if Control Xbox TSOP is set to "No"
			(LPCmodSettings.OSsettings.TSOPcontrol) & 0x01?	((LPCmodSettings.OSsettings.TSOPcontrol) & 0x02? "4-way" : "2-way") : "No");
	itemPtr->functionPtr= toggleTSOPSplit;
	itemPtr->functionDataPtr= itemPtr->szParameter;
	itemPtr->functionLeftPtr=toggleTSOPSplit;
	itemPtr->functionLeftDataPtr = itemPtr->szParameter;
	itemPtr->functionRightPtr=toggleTSOPSplit;
	itemPtr->functionRightDataPtr = itemPtr->szParameter;
}

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Reset all settings");
	itemPtr->szParameter[0] = 0;
	itemPtr->functionPtr= resetSettings;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	return menuPtr;
}
