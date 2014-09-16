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
    strcpy(itemPtr->szCaption,"Enable LCD : ");
    sprintf(itemPtr->szParameter, "%s", LPCmodSettings.LCDsettings.enable5V? "Yes" : "No");
    itemPtr->functionPtr= LCDToggleEN5V;
    itemPtr->functionDataPtr= itemPtr->szParameter;
    itemPtr->functionLeftPtr=LCDToggleEN5V;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=LCDToggleEN5V;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"LCD type : ");
    switch(LPCmodSettings.LCDsettings.lcdType) {
        case HD44780 :
            strcpy(itemPtr->szParameter,"HD44780");
            break;
        default:
            strcpy(itemPtr->szParameter,"Unknown");
            break;
    }
    itemPtr->functionPtr= NULL;
    itemPtr->functionDataPtr= NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Backlight : ");
    sprintf(itemPtr->szParameter, "%d%%", LPCmodSettings.LCDsettings.backlight);
    itemPtr->functionPtr= NULL;
    itemPtr->functionDataPtr= NULL;
    itemPtr->functionLeftPtr=LCDDecrementBacklight;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=LCDIncrementBacklight;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Contrast : ");
    sprintf(itemPtr->szParameter, "%d%%", LPCmodSettings.LCDsettings.contrast);
    itemPtr->functionPtr= NULL;
    itemPtr->functionDataPtr= NULL;
    itemPtr->functionLeftPtr=LCDDecrementContrast;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=LCDIncrementContrast;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Display message at boot : ");
    sprintf(itemPtr->szParameter, "%s", LPCmodSettings.LCDsettings.displayMsgBoot? "Yes" : "No");
    itemPtr->functionPtr= LCDToggleDisplayBootMsg;
    itemPtr->functionDataPtr= itemPtr->szParameter;
    itemPtr->functionLeftPtr=LCDToggleDisplayBootMsg;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=LCDToggleDisplayBootMsg;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Display booting BIOS name : ");
    sprintf(itemPtr->szParameter, "%s", LPCmodSettings.LCDsettings.displayBIOSNameBoot? "Yes" : "No");
    itemPtr->functionPtr= LCDToggledisplayBIOSNameBoot;
    itemPtr->functionDataPtr= itemPtr->szParameter;
    itemPtr->functionLeftPtr=LCDToggledisplayBIOSNameBoot;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=LCDToggledisplayBIOSNameBoot;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Display custom text : ");
    sprintf(itemPtr->szParameter, "%s", LPCmodSettings.LCDsettings.customTextBoot? "Yes" : "No");
    itemPtr->functionPtr= LCDToggledisplayCustomTextBoot;
    itemPtr->functionDataPtr= itemPtr->szParameter;
    itemPtr->functionLeftPtr=LCDToggledisplayCustomTextBoot;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=LCDToggledisplayCustomTextBoot;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Custom line 1");
    itemPtr->functionPtr= editCustomString0;
    itemPtr->functionDataPtr= NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Custom line 2");
    itemPtr->functionPtr= editCustomString1;
    itemPtr->functionDataPtr= NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Custom line 3");
    itemPtr->functionPtr= editCustomString2;
    itemPtr->functionDataPtr= NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Custom line 4");
    itemPtr->functionPtr= editCustomString3;
    itemPtr->functionDataPtr= NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}
