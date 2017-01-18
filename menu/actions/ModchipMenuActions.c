/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ModchipMenuActions.h"
#include "ResetMenuActions.h"
#include "IconMenu.h"
#include "lpcmod_v1.h"
#include "boot.h"
#include "LEDMenuActions.h"
#include "lib/LPCMod/BootLCD.h"
#include "string.h"
#include "xblast/settings/xblastSettings.h"
#include "menu/misc/OnScreenKeyboard.h"
#include "menu/misc/ConfirmDialog.h"

static void interlinkMenuItems(TEXTMENUITEM* first, TEXTMENUITEM* second);

void decrementActiveBank(void* itemStr)
{
    switch(LPCmodSettings.OSsettings.activeBank)
    {
    case BNK512:
        if(LPCmodSettings.OSsettings.TSOPcontrol)        //TSOP is split
        {
            LPCmodSettings.OSsettings.activeBank = BNKTSOPSPLIT1;
        }
        else
        {
            LPCmodSettings.OSsettings.activeBank = BNKFULLTSOP;
        }
        break;
    case BNK256:
        LPCmodSettings.OSsettings.activeBank = BNK512;
        break;
    case BNKTSOPSPLIT0:
    case BNKFULLTSOP:
        LPCmodSettings.OSsettings.activeBank = BNK256;
        break;
    case BNKTSOPSPLIT1:
        LPCmodSettings.OSsettings.activeBank = BNKTSOPSPLIT0;
        break;
    }

    sprintf(itemStr, "%s", getSpecialSettingString(SpecialSettingsPtrArrayIndexName_ActiveBank, LPCmodSettings.OSsettings.activeBank));
    return;
}



void incrementActiveBank(void* itemStr)
{
    switch(LPCmodSettings.OSsettings.activeBank)
    {
    case BNK512:
        LPCmodSettings.OSsettings.activeBank = BNK256;
        break;
    case BNK256:

        if(LPCmodSettings.OSsettings.TSOPcontrol)
        {
            LPCmodSettings.OSsettings.activeBank = BNKTSOPSPLIT0;
        }
        else
        {
            LPCmodSettings.OSsettings.activeBank = BNKFULLTSOP;
        }
        break;
    case BNKTSOPSPLIT1:
    case BNKFULLTSOP:
        LPCmodSettings.OSsettings.activeBank = BNK512;
        break;
    case BNKTSOPSPLIT0:
        LPCmodSettings.OSsettings.activeBank = BNKTSOPSPLIT1;
        break;
    }

    sprintf(itemStr, "%s", getSpecialSettingString(SpecialSettingsPtrArrayIndexName_ActiveBank, LPCmodSettings.OSsettings.activeBank));
    return;
}

void decrementAltBank(void* itemStr)
{
    switch(LPCmodSettings.OSsettings.altBank)
    {
    case BNKOS:
        if(LPCmodSettings.OSsettings.TSOPcontrol)        //TSOP is split
        {
            LPCmodSettings.OSsettings.altBank = BNKTSOPSPLIT1;
        }
        else
        {
            LPCmodSettings.OSsettings.altBank = BNKFULLTSOP;
        }
        break;
    case BNK512:
        LPCmodSettings.OSsettings.altBank = BNKOS;
        break;
    case BNK256:
        LPCmodSettings.OSsettings.altBank = BNK512;
        break;
    case BNKTSOPSPLIT0:
    case BNKFULLTSOP:
        LPCmodSettings.OSsettings.altBank = BNK256;
        break;
    case BNKTSOPSPLIT1:
        LPCmodSettings.OSsettings.altBank = BNKTSOPSPLIT0;
        break;
    }

    sprintf(itemStr, "%s", getSpecialSettingString(SpecialSettingsPtrArrayIndexName_AltBank, LPCmodSettings.OSsettings.altBank));
    return;
}



void incrementAltBank(void* itemStr)
{
    switch(LPCmodSettings.OSsettings.altBank)
    {
    case BNKOS:
        LPCmodSettings.OSsettings.altBank = BNK512;
        break;
    case BNK512:
        LPCmodSettings.OSsettings.altBank = BNK256;
        break;
    case BNK256:
        if(LPCmodSettings.OSsettings.TSOPcontrol)
        {
            LPCmodSettings.OSsettings.altBank = BNKTSOPSPLIT0;
        }
        else
        {
            LPCmodSettings.OSsettings.altBank = BNKFULLTSOP;
        }
        break;
    case BNKTSOPSPLIT1:
    case BNKFULLTSOP:
        LPCmodSettings.OSsettings.altBank = BNKOS;
        break;
    case BNKTSOPSPLIT0:
        LPCmodSettings.OSsettings.altBank = BNKTSOPSPLIT1;
        break;
    }

    sprintf(itemStr, "%s", getSpecialSettingString(SpecialSettingsPtrArrayIndexName_AltBank, LPCmodSettings.OSsettings.altBank));
    return;
}


void decrementbootTimeout(void* itemStr)
{
    if(LPCmodSettings.OSsettings.bootTimeout > 0)    //Logic
    {
        LPCmodSettings.OSsettings.bootTimeout -= 1;
    }

    sprintf(itemStr, "%ds", LPCmodSettings.OSsettings.bootTimeout);
    return;
}
void incrementbootTimeout(void* itemStr)
{
    if(LPCmodSettings.OSsettings.bootTimeout < 240)    //I've got to set a limit there.
    {
        LPCmodSettings.OSsettings.bootTimeout += 1;
    }
    sprintf(itemStr, "%ds", LPCmodSettings.OSsettings.bootTimeout);
    return;
}

void toggleQuickboot(void* itemStr)
{
    (LPCmodSettings.OSsettings.Quickboot) = (LPCmodSettings.OSsettings.Quickboot)? 0 : 1;
    sprintf(itemStr,"%s",LPCmodSettings.OSsettings.Quickboot? "Yes" : "No");
}

void resetSettings(void* ignored)
{
    if(ConfirmDialog("Confirm reset XBlast OS settings?", 1))
    {
        return;
    }
    //initialLPCModOSBoot(&LPCmodSettings);
    populateSettingsStructWithDefault(&LPCmodSettings);
    SlowReboot(NULL);
}

void editBIOSName(void* bankID)
{
    switch(*(unsigned char *)bankID)
    {
        case FlashBank_512Bank:
            OnScreenKeyboard(LPCmodSettings.OSsettings.biosName512Bank, BIOSNAMEMAXLENGTH, 3, FULL_KEYBOARD);
            break;
        case FlashBank_256Bank:
            OnScreenKeyboard(LPCmodSettings.OSsettings.biosName256Bank, BIOSNAMEMAXLENGTH, 3, FULL_KEYBOARD);
            break;
        case FlashBank_SplitTSOP0Bank:
        case FlashBank_FullTSOPBank:
            OnScreenKeyboard(LPCmodSettings.OSsettings.biosNameTSOPFullSplit0, BIOSNAMEMAXLENGTH, 3, FULL_KEYBOARD);
            break;
        case FlashBank_SplitTSOP1Bank:
            OnScreenKeyboard(LPCmodSettings.OSsettings.biosNameTSOPSplit1, BIOSNAMEMAXLENGTH, 3, FULL_KEYBOARD);
            break;
    }
    if(LPCmodSettings.LCDsettings.customTextBoot)
    {
        xLCD.PrintLine[3](JUSTIFYLEFT,LPCmodSettings.LCDsettings.customString3);
    }
}


//The function below requires that the menu items be in that order:
//Quickboot bank->Alternative Bank->TSOP Control
void toggleTSOPcontrol(void* customStruct)
{
    BankSelectCommonParams* params = (BankSelectCommonParams *)customStruct;

    if(LPCmodSettings.OSsettings.TSOPcontrol)            //If already active
    {
        LPCmodSettings.OSsettings.TSOPcontrol = 0x00;    //turn OFF.
        if(LPCmodSettings.OSsettings.altBank == BNKTSOPSPLIT0 ||
           LPCmodSettings.OSsettings.altBank == BNKTSOPSPLIT1)    //If altBank setting was set to TSOP bank 0 or 1.
        {
            LPCmodSettings.OSsettings.altBank = BNKFULLTSOP;    //Single TSOP bank so make sure altBank is properly set.
        }
        if(LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT0 ||
           LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT1)    //If activeBank setting was set to TSOP bank 0 or 1.
        {
            LPCmodSettings.OSsettings.activeBank = BNKFULLTSOP;    //Single TSOP bank so make sure activeBank is properly set.
        }
    }
    else
    {
        LPCmodSettings.OSsettings.TSOPcontrol = 0x01;    //Make sure to toggle only bit1.
        if(LPCmodSettings.OSsettings.altBank == BNKFULLTSOP)    //If altBank setting was set to TSOP FULL.
        {
            LPCmodSettings.OSsettings.altBank = BNKTSOPSPLIT0;
        }
        if(LPCmodSettings.OSsettings.activeBank == BNKFULLTSOP)    //If activeBank setting was set to TSOP FULL.
        {
            LPCmodSettings.OSsettings.activeBank = BNKTSOPSPLIT0;
        }
    }

    reorderTSOPNameMenuEntries(params);
    sprintf(params->powerButString, getSpecialSettingString(SpecialSettingsPtrArrayIndexName_ActiveBank, LPCmodSettings.OSsettings.activeBank));
    sprintf(params->ejectButString, getSpecialSettingString(SpecialSettingsPtrArrayIndexName_AltBank, LPCmodSettings.OSsettings.altBank));
    sprintf(params->tsopControlString, "%s", (LPCmodSettings.OSsettings.TSOPcontrol)? "Yes" : "No");
}

void toggleTSOPhide(void* itemStr)
{
    LPCmodSettings.OSsettings.TSOPhide = LPCmodSettings.OSsettings.TSOPhide ? 0 : 1;
    sprintf(itemStr, "%s", LPCmodSettings.OSsettings.TSOPhide ? "Yes" : "No");
}

void reorderTSOPNameMenuEntries(BankSelectCommonParams* params)
{
    if(LPCmodSettings.OSsettings.TSOPcontrol)
    {
        interlinkMenuItems(params->bank256ItemPtr, params->tsopBank0ItemPtr);
        interlinkMenuItems(params->tsopBank0ItemPtr, params->tsopBank1ItemPtr);
        interlinkMenuItems(params->tsopBank1ItemPtr, params->resetAllItemPtr);
    }
    else
    {
        interlinkMenuItems(params->bank256ItemPtr, params->tsopFullItemPtr);
        interlinkMenuItems(params->tsopFullItemPtr, params->resetAllItemPtr);
    }
}

static void interlinkMenuItems(TEXTMENUITEM* first, TEXTMENUITEM* second)
{
    first->nextMenuItem = second;
    second->previousMenuItem = first;
}
