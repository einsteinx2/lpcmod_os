/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MenuInits.h"
#include "ToolsMenuActions.h"
#include "lpcmod_v1.h"
#include "string.h"
#include "stdio.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include "xblast/settings/xblastSettingsImportExport.h"
#include "xblast/HardwareIdentifier.h"
#include "FatFSAccessor.h"

TEXTMENU *ToolsMenuInit(void)
{
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    int i=0;
    FileInfo cfgFileInfo;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Tools");

    if(isXBE() == false || isXBlastOnLPC())
    {
        //Save EEPROM data to flash
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption,"Save EEPROM to modchip");
        itemPtr->functionPtr = saveEEPromToFlash;
        TextMenuAddItem(menuPtr, itemPtr);
        saveEEPROMPtr = itemPtr;

        //Restore EEPROM data from flash
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Restore EEPROM from modchip");
        itemPtr->functionPtr = restoreEEPromFromFlash;
        itemPtr->functionDataPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
        restoreEEPROMPtr = itemPtr;

#ifdef DEV_FEATURES
        //Erase EEPROM data from flash
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Erase EEPROM from modchip");
        itemPtr->functionPtr = eraseEEPromFromFlash;
        itemPtr->functionDataPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
        eraseEEPROMPtr = itemPtr;
#endif
    }

    //Dangerous stuff is going on in there.
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Edit EEPROM content");
    itemPtr->functionPtr = warningDisplayEepromEditMenu;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);
    editEEPROMPtr = itemPtr;

    //Wipe EEPROM section that holds non-vital data.
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Reset system settings");
    itemPtr->functionPtr = wipeEEPromUserSettings;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    //Do not show this entry if 1.6/1.6b
    if(getMotherboardRevision() != XboxMotherboardRevision_1_6)
    {
        //128MB MEMORY TEST
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "128MB RAM test");
        itemPtr->functionPtr=showMemTest;
        itemPtr->functionDataPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    if(isTSOPSplitCapable() &&  //Don't show this when Xbox motherboard is not 1.0/1.1.
       LPCmodSettings.OSsettings.TSOPcontrol)          //Don't show if TSOP split is not enabled.
    {
        //TSOP split manual control
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "TSOP recover force bank : ");
        if(A19controlModBoot == BNKTSOPSPLIT0)
            strcpy(itemPtr->szParameter, "Bank0");
        else if(A19controlModBoot == BNKTSOPSPLIT1)
            strcpy(itemPtr->szParameter, "Bank1");
        else
            strcpy(itemPtr->szParameter, "No");
        itemPtr->functionPtr = nextA19controlModBootValue;
        itemPtr->functionDataPtr= itemPtr->szParameter;
        itemPtr->functionLeftPtr=prevA19controlModBootValue;
        itemPtr->functionLeftDataPtr = itemPtr->szParameter;
        itemPtr->functionRightPtr=nextA19controlModBootValue;
        itemPtr->functionRightDataPtr = itemPtr->szParameter;
        TextMenuAddItem(menuPtr, itemPtr);
    }
/*
    //TSOP recovery entries. Do not show if already in TSOP recovery
    if((isXBE() == false || fHasHardware == SYSCON_ID_V1)
       && !TSOPRecoveryMode) {
        //TSOP recovery
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "TSOP Recovery");
        itemPtr->functionPtr=TSOPRecoveryReboot;
        itemPtr->functionDataPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }
*/
    if(isMounted(HDD_Master, Part_C))
    {
        cfgFileInfo = fatxstat(getSettingsFileLocation());
        //Save xblast.cfg
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption, "Save %s", getSettingsFileLocation() + strlen(PathSep"MASTER_"));
        itemPtr->functionPtr = saveXBlastcfg;
        itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
        *(unsigned char*)itemPtr->functionDataPtr = cfgFileInfo.size ? 1 : 0;
        itemPtr->dataPtrAlloc = 1;
        TextMenuAddItem(menuPtr, itemPtr);

        if(cfgFileInfo.size)
        {
            //Load xblast.cfg
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption, "Load %s", getSettingsFileLocation() + strlen(PathSep"MASTER_"));
            itemPtr->functionPtr = loadXBlastcfg;
            itemPtr->functionDataPtr = NULL;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "XBlast scripts");
    itemPtr->functionPtr = DrawChildTextMenu;
    itemPtr->functionDataPtr = XBlastScriptMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);

#ifdef DEV_FEATURES
    {
        //Developers tools
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Developer tools");
        itemPtr->functionPtr = DrawChildTextMenu;
        itemPtr->functionDataPtr = DeveloperMenuInit();
        TextMenuAddItem(menuPtr, itemPtr);
    }
#endif

    if(EepromSanityCheck(&LPCmodSettings.bakeeprom) == EEPROM_EncryptInvalid)
    {
        saveEEPROMPtr->nextMenuItem = editEEPROMPtr;
        editEEPROMPtr->previousMenuItem = saveEEPROMPtr;
    }

    return menuPtr;
}
