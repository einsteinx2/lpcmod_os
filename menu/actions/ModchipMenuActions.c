/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ModchipMenuActions.h"
#include "TextMenu.h"
#include "lpcmod_v1.h"
#include "boot.h"
#include "LEDMenuActions.h"
#include "lib/LPCMod/BootLCD.h"


void decrementActiveBank(void * itemStr) {
    switch(LPCmodSettings.OSsettings.activeBank){
    case BNK512:
        if((LPCmodSettings.OSsettings.TSOPcontrol) & 0x01){        //TSOP is split
            LPCmodSettings.OSsettings.activeBank = BNKTSOPSPLIT1;
            sprintf(itemStr,"TSOP bank1");
        }
        else {
            LPCmodSettings.OSsettings.activeBank = BNKFULLTSOP;
            sprintf(itemStr,"TSOP");
        }
        break;
    case BNK256:
        LPCmodSettings.OSsettings.activeBank = BNK512;
        sprintf(itemStr,"512KB");
        break;
    case BNKTSOPSPLIT0:
    case BNKFULLTSOP:
        LPCmodSettings.OSsettings.activeBank = BNK256;
        sprintf(itemStr,"256KB");
        break;
    case BNKTSOPSPLIT1:
        sprintf(itemStr,"TSOP bank0");
        LPCmodSettings.OSsettings.activeBank = BNKTSOPSPLIT0;
        break;
    }
    return;
}



void incrementActiveBank(void * itemStr) {
    switch(LPCmodSettings.OSsettings.activeBank){
    case BNK512:
        LPCmodSettings.OSsettings.activeBank = BNK256;
        sprintf(itemStr,"256KB");
        break;
    case BNK256:

        if(LPCmodSettings.OSsettings.TSOPcontrol & 0x01){
            sprintf(itemStr,"TSOP bank0");
            LPCmodSettings.OSsettings.activeBank = BNKTSOPSPLIT0;
        }
        else {
            sprintf(itemStr,"TSOP");
            LPCmodSettings.OSsettings.activeBank = BNKFULLTSOP;
        }
        break;
    case BNKTSOPSPLIT1:
    case BNKFULLTSOP:
        LPCmodSettings.OSsettings.activeBank = BNK512;
        sprintf(itemStr,"512KB");
        break;
    case BNKTSOPSPLIT0:
        LPCmodSettings.OSsettings.activeBank = BNKTSOPSPLIT1;
        sprintf(itemStr,"TSOP bank1");
        break;
    }
    return;
}

void decrementAltBank(void * itemStr) {
    switch(LPCmodSettings.OSsettings.altBank){
        case BNK512:
            if((LPCmodSettings.OSsettings.TSOPcontrol) & 0x01){        //TSOP is split
                LPCmodSettings.OSsettings.altBank = BNKTSOPSPLIT1;
                sprintf(itemStr,"TSOP bank1");
            }
            else {
                LPCmodSettings.OSsettings.altBank = BNKFULLTSOP;
                sprintf(itemStr,"TSOP");
            }
            break;
        case BNK256:
            LPCmodSettings.OSsettings.altBank = BNK512;
            sprintf(itemStr,"512KB");
            break;
        case BNKTSOPSPLIT0:
        case BNKFULLTSOP:
            LPCmodSettings.OSsettings.altBank = BNK256;
            sprintf(itemStr,"256KB");
            break;
        case BNKTSOPSPLIT1:
            sprintf(itemStr,"TSOP bank0");
            LPCmodSettings.OSsettings.altBank = BNKTSOPSPLIT0;
            break;
        }
        return;
    }



void incrementAltBank(void * itemStr) {
    switch(LPCmodSettings.OSsettings.altBank){
        case BNK512:
            LPCmodSettings.OSsettings.altBank = BNK256;
            sprintf(itemStr,"256KB");
            break;
        case BNK256:

            if(LPCmodSettings.OSsettings.TSOPcontrol & 0x01){
                sprintf(itemStr,"TSOP bank0");
                LPCmodSettings.OSsettings.altBank = BNKTSOPSPLIT0;
            }
            else {
                sprintf(itemStr,"TSOP");
                LPCmodSettings.OSsettings.altBank = BNKFULLTSOP;
            }
            break;
        case BNKTSOPSPLIT1:
        case BNKFULLTSOP:
            LPCmodSettings.OSsettings.altBank = BNK512;
            sprintf(itemStr,"512KB");
            break;
        case BNKTSOPSPLIT0:
            LPCmodSettings.OSsettings.altBank = BNKTSOPSPLIT1;
            sprintf(itemStr,"TSOP bank1");
            break;
        }
        return;
    }


void decrementbootTimeout(void * itemStr){
    if(LPCmodSettings.OSsettings.bootTimeout > 0)    //Logic
        LPCmodSettings.OSsettings.bootTimeout -= 1;
    sprintf(itemStr, "%ds", LPCmodSettings.OSsettings.bootTimeout);
    return;
}
void incrementbootTimeout(void * itemStr){
    if(LPCmodSettings.OSsettings.bootTimeout < 240)    //I've got to set a limit there.
        LPCmodSettings.OSsettings.bootTimeout += 1;
    sprintf(itemStr, "%ds", LPCmodSettings.OSsettings.bootTimeout);
    return;
}

void toggleQuickboot(void * itemStr){
    (LPCmodSettings.OSsettings.Quickboot) = (LPCmodSettings.OSsettings.Quickboot)? 0 : 1;
    sprintf(itemStr,"%s",LPCmodSettings.OSsettings.Quickboot? "Yes" : "No");
}

void resetSettings(void *whatever){
    if(ConfirmDialog("        Confirm reset XBlast OS settings?", 1))
        return;
    //initialLPCModOSBoot(&LPCmodSettings);
    memset(&LPCmodSettings, 0xFF, sizeof(_LPCmodSettings));
    SlowReboot();
}

void editBIOSName(void *bankID){
    switch(*(u8 *)bankID){
        case BNK512:
            OnScreenKeyboard(LPCmodSettings.OSsettings.biosName0, BIOSNAMEMAXLENGTH, 3);
            break;
        case BNK256:
            OnScreenKeyboard(LPCmodSettings.OSsettings.biosName1, BIOSNAMEMAXLENGTH, 3);
            break;
        case BNKTSOPSPLIT0:
        case BNKFULLTSOP:
            OnScreenKeyboard(LPCmodSettings.OSsettings.biosName2, BIOSNAMEMAXLENGTH, 3);
            break;
        default:        //BNKTSOPSPLIT1
            OnScreenKeyboard(LPCmodSettings.OSsettings.biosName3, BIOSNAMEMAXLENGTH, 3);
            break;
    }
    if(LPCmodSettings.LCDsettings.customTextBoot)
        xLCD.PrintLine3(JUSTIFYLEFT,LPCmodSettings.LCDsettings.customString3);    
}


//The two functions below requires that the menu items be in that order:
//Quickboot bank->Alternative Bank->TSOP Control->TSOP Split
void toggleTSOPControl(void * itemPtr){
    TEXTMENUITEM * tempItemPtr = (TEXTMENUITEM *)itemPtr;
    if(LPCmodSettings.OSsettings.TSOPcontrol & 0x01){            //If already active
        LPCmodSettings.OSsettings.TSOPcontrol &= 0xFE;    //Make sure to toggle only bit0 and turn OFF.
        if(LPCmodSettings.OSsettings.altBank == BNKTSOPSPLIT0 ||
           LPCmodSettings.OSsettings.altBank == BNKTSOPSPLIT1){    //If altBank setting was set to TSOP bank 1,2 or 3.
            LPCmodSettings.OSsettings.altBank = BNKFULLTSOP;    //Single TSOP bank so make sure altBank is properly set.
            sprintf(tempItemPtr->previousMenuItem->szParameter,"TSOP");
        }
        if(LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT0 ||
           LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT1){    //If activeBank setting was set to TSOP bank 1,2 or 3.
            LPCmodSettings.OSsettings.activeBank = BNKFULLTSOP;    //Single TSOP bank so make sure activeBank is properly set.
            sprintf(tempItemPtr->previousMenuItem->previousMenuItem->szParameter,"TSOP");
        }
    }
    else{
        LPCmodSettings.OSsettings.TSOPcontrol |= 0x01;    //Make sure to toggle only bit0.
    }
    sprintf(tempItemPtr->szParameter,"%s", (LPCmodSettings.OSsettings.TSOPcontrol) & 0x01? "Yes" : "No");
}
