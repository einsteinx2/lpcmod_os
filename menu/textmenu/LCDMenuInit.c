#include "MenuInits.h"
#include "lpcmod_v1.h"
#include "config.h"
#include "boot.h"
#include "LCDMenuActions.h"
#include "string.h"
#include "stdio.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include "xblast/HardwareIdentifier.h"

TEXTMENU* LCDMenuInit(void)
{

    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "LCD settings");

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Enable LCD : ");
    strcpy(itemPtr->szParameter, LPCmodSettings.LCDsettings.enable5V? "Yes" : "No");
    itemPtr->functionPtr = LCDToggleEN5V;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr = LCDToggleEN5V;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr = LCDToggleEN5V;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "LCD type : ");
    switch(LPCmodSettings.LCDsettings.lcdType)
    {
        case LCDTYPE_HD44780 :
            strcpy(itemPtr->szParameter,"HD44780");
            break;
        case LCDTYPE_KS0073 :
            strcpy(itemPtr->szParameter,"KS0073");
            break;
        default:
            strcpy(itemPtr->szParameter,"Unknown");
            break;
    }
    itemPtr->functionPtr = LCDChangeLCDType;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr = LCDChangeLCDType;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr = LCDChangeLCDType;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Number of lines : ");
    sprintf(itemPtr->szParameter, "%u", LPCmodSettings.LCDsettings.nbLines);
    itemPtr->functionLeftPtr = LCDDecreaseNbLines;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr = LCDIncreaseNbLines;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Line's length : ");
    sprintf(itemPtr->szParameter, "%u", LPCmodSettings.LCDsettings.lineLength);
    itemPtr->functionLeftPtr = LCDDecreaseLineLength;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr = LCDIncreaseLineLength;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Backlight : ");
    sprintf(itemPtr->szParameter, "%d%%", LPCmodSettings.LCDsettings.backlight);
    itemPtr->functionPtr = NULL;
    itemPtr->functionDataPtr= NULL;
    itemPtr->functionLeftPtr = LCDDecrementBacklight;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr = LCDIncrementBacklight;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    itemPtr->functionLTPtr = LCDDecrementBacklight;
    itemPtr->functionLTDataPtr = itemPtr->szParameter;
    itemPtr->functionRTPtr = LCDIncrementBacklight;
    itemPtr->functionRTDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    //No need to display contrast settings if SmartXX OPX or Xecuter 3 is detected, they do not support it.
    if(isLCDContrastSupport())
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Contrast : ");
        sprintf(itemPtr->szParameter, "%d%%", LPCmodSettings.LCDsettings.contrast);
        itemPtr->functionPtr = NULL;
        itemPtr->functionDataPtr= NULL;
        itemPtr->functionLeftPtr = LCDDecrementContrast;
        itemPtr->functionLeftDataPtr = itemPtr->szParameter;
        itemPtr->functionRightPtr = LCDIncrementContrast;
        itemPtr->functionRightDataPtr = itemPtr->szParameter;
        itemPtr->functionLTPtr = LCDDecrementContrast;
        itemPtr->functionLTDataPtr = itemPtr->szParameter;
        itemPtr->functionRTPtr = LCDIncrementContrast;
        itemPtr->functionRTDataPtr = itemPtr->szParameter;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Display message at boot : ");
    strcpy(itemPtr->szParameter, LPCmodSettings.LCDsettings.displayMsgBoot ? "Yes" : "No");
    itemPtr->functionPtr = LCDToggleDisplayBootMsg;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr = LCDToggleDisplayBootMsg;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr = LCDToggleDisplayBootMsg;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Display booting BIOS name : ");
    strcpy(itemPtr->szParameter, LPCmodSettings.LCDsettings.displayBIOSNameBoot ? "Yes" : "No");
    itemPtr->functionPtr = LCDToggledisplayBIOSNameBoot;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr = LCDToggledisplayBIOSNameBoot;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr = LCDToggledisplayBIOSNameBoot;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Display custom text : ");
    strcpy(itemPtr->szParameter, LPCmodSettings.LCDsettings.customTextBoot ? "Yes" : "No");
    itemPtr->functionPtr = LCDToggledisplayCustomTextBoot;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr = LCDToggledisplayCustomTextBoot;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr = LCDToggledisplayCustomTextBoot;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Custom line 1");
    itemPtr->functionPtr = editCustomString0;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Custom line 2");
    itemPtr->functionPtr = editCustomString1;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Custom line 3");
    itemPtr->functionPtr = editCustomString2;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Custom line 4");
    itemPtr->functionPtr = editCustomString3;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}
