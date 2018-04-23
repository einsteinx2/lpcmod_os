#include "MenuInits.h"
#include "lpcmod_v1.h"
#include "config.h"
#include "boot.h"
#include "ModchipMenuActions.h"
#include "string.h"
#include "stdio.h"
#include "xblast/HardwareIdentifier.h"
#include "xblast/settings/xblastSettings.h"

TEXTMENU *ModchipMenuInit(void)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;

    menuPtr = (TEXTMENU*)calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "XBlast settings");

    if(isXBlastOnLPC())
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Idle timeout : ");
        sprintf(itemPtr->szParameter, "%ds", LPCmodSettings.OSsettings.bootTimeout);
        itemPtr->functionLeftPtr = decrementbootTimeout;
        itemPtr->functionLeftDataPtr = itemPtr->szParameter;
        itemPtr->functionRightPtr = incrementbootTimeout;
        itemPtr->functionRightDataPtr = itemPtr->szParameter;
        itemPtr->functionLTPtr = decrementbootTimeout;
        itemPtr->functionLTDataPtr = itemPtr->szParameter;
        itemPtr->functionRTPtr = incrementbootTimeout;
        itemPtr->functionRTDataPtr = itemPtr->szParameter;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Quickboot : ");
        strcpy(itemPtr->szParameter, LPCmodSettings.OSsettings.Quickboot? "Yes" : "No");
        itemPtr->functionPtr = toggleQuickboot;
        itemPtr->functionDataPtr = itemPtr->szParameter;
        itemPtr->functionLeftPtr = toggleQuickboot;
        itemPtr->functionLeftDataPtr = itemPtr->szParameter;
        itemPtr->functionRightPtr = toggleQuickboot;
        itemPtr->functionRightDataPtr = itemPtr->szParameter;
        TextMenuAddItem(menuPtr, itemPtr);


        BankSelectCommonParams* customParams = malloc(sizeof(BankSelectCommonParams));

        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Power button boot : ");
        strcpy(itemPtr->szParameter, getSpecialSettingDisplayString(SpecialSettingsPtrArrayIndexName_ActiveBank, LPCmodSettings.OSsettings.activeBank));
        customParams->powerButString = itemPtr->szParameter;
        itemPtr->functionPtr = incrementActiveBank;
        itemPtr->functionDataPtr = customParams->powerButString;
        itemPtr->dataPtrAlloc = true;   //Signal only once as allocated mem is shared on 2/3 entries.
        itemPtr->functionLeftPtr = decrementActiveBank;
        itemPtr->functionLeftDataPtr = customParams->powerButString;
        itemPtr->functionRightPtr = incrementActiveBank;
        itemPtr->functionRightDataPtr = customParams->powerButString;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Eject button boot : ");
        strcpy(itemPtr->szParameter, getSpecialSettingDisplayString(SpecialSettingsPtrArrayIndexName_AltBank, LPCmodSettings.OSsettings.altBank));
        customParams->ejectButString = itemPtr->szParameter;
        itemPtr->functionPtr = incrementAltBank;
        itemPtr->functionDataPtr = customParams->ejectButString;
        itemPtr->functionLeftPtr = decrementAltBank;
        itemPtr->functionLeftDataPtr = customParams->ejectButString;
        itemPtr->functionRightPtr = incrementAltBank;
        itemPtr->functionRightDataPtr = customParams->ejectButString;
        TextMenuAddItem(menuPtr, itemPtr);

        if(isPureXBlast() && isTSOPSplitCapable())
        {
            //Don't show this when Xbox motherboard is not 1.0/1.1.
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            strcpy(itemPtr->szCaption, "Control Xbox TSOP : ");
            strcpy(itemPtr->szParameter, (LPCmodSettings.OSsettings.TSOPcontrol) ? "Yes" : "No");
            customParams->tsopControlString = itemPtr->szParameter;
            itemPtr->functionPtr = toggleTSOPcontrol;
            itemPtr->functionDataPtr = customParams;
            itemPtr->functionLeftPtr = toggleTSOPcontrol;
            itemPtr->functionLeftDataPtr = customParams;
            itemPtr->functionRightPtr = toggleTSOPcontrol;
            itemPtr->functionRightDataPtr = customParams;
            TextMenuAddItem(menuPtr, itemPtr);

        }
    
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Hide TSOP boot icon : ");
        strcpy(itemPtr->szParameter, (LPCmodSettings.OSsettings.TSOPhide) ? "Yes" : "No");
        itemPtr->functionPtr = toggleTSOPhide;
        itemPtr->functionDataPtr = itemPtr->szParameter;
        itemPtr->functionLeftPtr = toggleTSOPhide;
        itemPtr->functionLeftDataPtr = itemPtr->szParameter;
        itemPtr->functionRightPtr = toggleTSOPhide;
        itemPtr->functionRightDataPtr = itemPtr->szParameter;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Bank0(512KB) BIOS name");
        itemPtr->functionPtr = editBIOSName;
        itemPtr->functionDataPtr = malloc(sizeof(FlashBank));
        *(char*)itemPtr->functionDataPtr = FlashBank_512Bank;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Bank1(256KB) BIOS name");
        customParams->bank256ItemPtr = itemPtr;
        itemPtr->functionPtr = editBIOSName;
        itemPtr->functionDataPtr = malloc(sizeof(FlashBank));
        *(char*)itemPtr->functionDataPtr = FlashBank_256Bank;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        customParams->tsopBank0ItemPtr = itemPtr;
        strcpy(itemPtr->szCaption, "TSOP bank0 name");
        itemPtr->functionPtr = editBIOSName;
        itemPtr->functionDataPtr = malloc(sizeof(FlashBank));
        *(char*)itemPtr->functionDataPtr = FlashBank_SplitTSOP0Bank;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        customParams->tsopBank1ItemPtr = itemPtr;
        strcpy(itemPtr->szCaption, "TSOP bank1 name");
        itemPtr->functionPtr = editBIOSName;
        itemPtr->functionDataPtr = malloc(sizeof(FlashBank));
        *(char*)itemPtr->functionDataPtr = FlashBank_SplitTSOP1Bank;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        customParams->tsopFullItemPtr = itemPtr;
        strcpy(itemPtr->szCaption, "TSOP BIOS name");
        itemPtr->functionPtr = editBIOSName;
        itemPtr->functionDataPtr= malloc(sizeof(FlashBank));
        *(char*)itemPtr->functionDataPtr = FlashBank_FullTSOPBank;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        customParams->resetAllItemPtr = itemPtr;
        strcpy(itemPtr->szCaption, "Reset all settings");
        itemPtr->functionPtr = resetSettings;
        TextMenuAddItem(menuPtr, itemPtr);

        reorderTSOPNameMenuEntries(customParams);
    }
    else
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Reboot from Modchip");
        itemPtr->noSelect = NOSELECTERROR;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "to edit these settings.");
        itemPtr->noSelect = NOSELECTERROR;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    return menuPtr;
}
