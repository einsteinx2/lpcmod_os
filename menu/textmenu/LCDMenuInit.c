#include "lpcmod_v1.h"
#include "include/config.h"
#include "TextMenu.h"
#include "include/boot.h"
#include "LCDMenuActions.h"

TEXTMENU *LCDMenuInit(void) {

	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;

	menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
	memset(menuPtr,0x00,sizeof(TEXTMENU));
	strcpy(menuPtr->szCaption, "LCD settings");

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Enable LCD : %s", LPCmodSettings.LCDsettings.enable5V? "Yes" : "No");
	itemPtr->functionPtr= LCDToggleEN5V;
	itemPtr->functionDataPtr= itemPtr->szCaption;
	itemPtr->functionLeftPtr=LCDToggleEN5V;
	itemPtr->functionLeftDataPtr = itemPtr->szCaption;
	itemPtr->functionRightPtr=LCDToggleEN5V;
	itemPtr->functionRightDataPtr = itemPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	switch(LPCmodSettings.LCDsettings.lcdType) {
		case HD44780 :
			sprintf(itemPtr->szCaption,"LCD type : HD44780");
			break;
		default:
			sprintf(itemPtr->szCaption,"LCD type : Unknown");
			break;
	}
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Backlight : %d%%", LPCmodSettings.LCDsettings.backlight);
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	itemPtr->functionLeftPtr=LCDDecrementBacklight;
	itemPtr->functionLeftDataPtr = itemPtr->szCaption;
	itemPtr->functionRightPtr=LCDIncrementBacklight;
	itemPtr->functionRightDataPtr = itemPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Contrast: %d%%", LPCmodSettings.LCDsettings.contrast);
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	itemPtr->functionLeftPtr=LCDDecrementContrast;
	itemPtr->functionLeftDataPtr = itemPtr->szCaption;
	itemPtr->functionRightPtr=LCDIncrementContrast;
	itemPtr->functionRightDataPtr = itemPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Display message at boot : %s", LPCmodSettings.LCDsettings.displayMsgBoot? "Yes" : "No");
	itemPtr->functionPtr= LCDToggleDisplayBootMsg;
	itemPtr->functionDataPtr= itemPtr->szCaption;
	itemPtr->functionLeftPtr=LCDToggleDisplayBootMsg;
	itemPtr->functionLeftDataPtr = itemPtr->szCaption;
	itemPtr->functionRightPtr=LCDToggleDisplayBootMsg;
	itemPtr->functionRightDataPtr = itemPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Display booting BIOS name : %s", LPCmodSettings.LCDsettings.displayBIOSNameBoot? "Yes" : "No");
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Display custom text at boot : %s", LPCmodSettings.LCDsettings.customTextBoot? "Yes" : "No");
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Custom line 1");
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Custom line 2");
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Custom line 3");
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption,"Custom line 4");
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr= NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	return menuPtr;
}
