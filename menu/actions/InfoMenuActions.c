/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"
#include "i2c.h"
#include "video.h"
#include "BootEEPROM.h"
#include "FlashUi.h"
#include "InfoMenuActions.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/time/timeManagement.h"
#include "lib/cromwell/cromString.h"
#include "xblast/settings/xblastSettingsChangeTracker.h"
#include "xblast/settings/xblastSettingsImportExport.h"
#include "MenuActions.h"
#include "string.h"
#include "lib/LPCMod/xblastDebug.h"
#include <stddef.h>

void ShowTemperature(void *whatever)
{
    int c, cx;
    int f, fx;
    UiHeader("Temperature");
    I2CGetTemperature(&c, &cx);
    f = ((9.0 / 5.0) * c) + 32.0;
    fx = ((9.0 / 5.0) * cx) + 32.0;
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n           CPU temperature: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%d°C / %d°F", c, f);
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n           Board temperature: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%d°C / %d°F", cx, fx);
    UIFooter();
}

void ShowVideo(void *whatever)
{
    UiHeader("Video");
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n           Encoder: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", VideoEncoderName());
    VIDEO_ATTR=0xffc8c8c8;
    printk("Cable: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", AvCableName());
    UIFooter();
}

void ShowEeprom(void *whatever)
{
    UiHeader("EEPROM");
    BootEepromPrintInfo();
    UIFooter();
}

void ShowFlashChip(void *whatever)
{
    UiHeader("Flash device");
    BootShowFlashDevice();
    UIFooter();
}

#if 0
void ShowUncommittedChanges(void *whatever)
{
#define MAXITEMSINONESCREEN 20
#define SELECTEDITEMTEXTMENUCOLOR 0xffef37
#define NORMALTEXTMENUCOLOR 0xffc8c8c8
    bool noExit = true, redrawList = true;
    char printString[40];
    unsigned char UncommittedChanges = 0, NbOfItemsToList = 0;
    bool IPChange;
    unsigned char i, j, k;
    unsigned char numberOfEEPROMChanges, selectedEntryItem = 0, firstVisibleEntryItem = 0;
    _settingsPtrStruct originalSettingsPtrStruct;
    setCFGFileTransferPtr(&LPCmodSettingsOrigFromFlash, &originalSettingsPtrStruct);
    while(noExit)
    {
        if(redrawList == true)
        {
            UncommittedChanges = LPCMod_CountNumberOfChangesInSettings(false, NULL);
            //cleanChangeListStruct(); //TODO: Useful?
            NbOfItemsToList = UncommittedChanges;
            redrawList = false;
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            printk("\n\n");
            VIDEO_ATTR=0xffffef37 ;
            sprintf(printString, "Uncommitted change%s information:", UncommittedChanges > 1 ? "s" : "");
            UiHeader(printString);
            VIDEO_ATTR=NORMALTEXTMENUCOLOR;


            if(UncommittedChanges == 0)
            {
                printk("\n           No uncommitted change.");
            }
            else
            {
                if(selectedEntryItem > MAXITEMSINONESCREEN)
                {
                    firstVisibleEntryItem = selectedEntryItem - MAXITEMSINONESCREEN;
                }
                else
                {
                    firstVisibleEntryItem = 0;
                }

                if(UncommittedChanges > MAXITEMSINONESCREEN)
                {
                    NbOfItemsToList = MAXITEMSINONESCREEN;
                }

                numberOfEEPROMChanges = generateEEPROMChangeList(false, NULL);

                if(numberOfEEPROMChanges > 0)
                {
                    for(i = firstVisibleEntryItem; i < numberOfEEPROMChanges && i < MAXITEMSINONESCREEN; i++)
                    {

                    }
                    NbOfItemsToList -= (numberOfEEPROMChanges > MAXITEMSINONESCREEN ? MAXITEMSINONESCREEN : numberOfEEPROMChanges) - firstVisibleEntryItem;
                }

                unsigned char bootScriptChange = LPCMod_checkForBootScriptChanges() ? 1 : 0;
                if(bootScriptChange && NbOfItemsToList > 0)
                {
                    if(firstVisibleEntryItem <= numberOfEEPROMChanges)
                    {
                        if(selectedEntryItem == numberOfEEPROMChanges)
                        {
                            VIDEO_ATTR=SELECTEDITEMTEXTMENUCOLOR;
                        }
                        else{
                            VIDEO_ATTR=NORMALTEXTMENUCOLOR;
                        }
                        printk("\n           Boot script in flash modified");
                        NbOfItemsToList -= 1;
                    }
                }

                //Normal OS paramters listing.
                k = 0;
                int firstItem = ((int)firstVisibleEntryItem - numberOfEEPROMChanges) > 0 ? (firstVisibleEntryItem - numberOfEEPROMChanges) : 0;
                for(i = firstItem; (i < NBTXTPARAMS) && (k < NbOfItemsToList); i++)
                {
                    if(i == selectedEntryItem - firstItem)
                    {
                        VIDEO_ATTR=SELECTEDITEMTEXTMENUCOLOR;
                    }
                    else
                    {
                        VIDEO_ATTR=NORMALTEXTMENUCOLOR;
                    }

                    if(i < IPTEXTPARAMGROUP)
                    {
                        if(*originalSettingsPtrStruct.settingsPtrArray[i] != *settingsPtrStruct.settingsPtrArray[i])
                        {
                            if(i < NBBOOLEANPARAMS)
                            {
                                printk("\n           %s \"%s\" -> \"%s\"", xblastcfgstrings[i], *originalSettingsPtrStruct.settingsPtrArray[i] ? "Yes" : "No", *settingsPtrStruct.settingsPtrArray[i] ? "Yes" : "No");
                            }
                            else
                            {
                                printk("\n           %s \"%u\" -> \"%u\"", xblastcfgstrings[i], *originalSettingsPtrStruct.settingsPtrArray[i], *settingsPtrStruct.settingsPtrArray[i]);
                            }

                            k += 1;
                        }
                    }
                    else if(i < TEXTPARAMGROUP)
                    {
                        for(j = 0; j < 4; j++)
                        {
                            if(originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][j] != settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][j])
                            {
                                printk("\n           %s \"%u.%u.%u.%u\" -> \"%u.%u.%u.%u\"", xblastcfgstrings[i],
                                            originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][0],
                                            originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][1],
                                            originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][2],
                                            originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][3],
                                            settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][0],
                                            settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][1],
                                            settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][2],
                                            settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][3]);

                                k += 1;
                                break;
                            }
                        }
                    }
                    else if(i < SPECIALPARAMGROUP)
                    {
                        if(strcmp(originalSettingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP], settingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP]))
                        {
                            printk("\n           %s \"%s\" -> \"%s\"", xblastcfgstrings[i], originalSettingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP], settingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP]);

                            k += 1;
                        }
                    }
                    else
                    {
                        if(*originalSettingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP] != *settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP])
                        {
                            printk("\n           %s \"%u\" -> \"%u\"", xblastcfgstrings[i], *originalSettingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP], *settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP]);

                            k += 1;
                        }
                    }
                }
            }

            debugSPIPrint("UncommittedChanges=%u firstVisibleEntryItem=%u NbOfItemsToList=%u\n", UncommittedChanges, firstVisibleEntryItem, NbOfItemsToList);
            debugSPIPrint("selectedEntryItem=%u\n", selectedEntryItem);
            VIDEO_ATTR=0xffc8c8c8;
            printk("\n\n           Press Button 'B' or 'Back' to return.");
        }

        if((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) == 1) || (risefall_xpad_STATE(XPAD_STATE_BACK) == 1))
        {
            noExit = false;
        }

        if((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1) || (risefall_xpad_STATE(XPAD_STATE_START) == 1))
        {
            redrawList = true;
            //Print confirm dialog to revert change
        }

        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1)
        {
            if(selectedEntryItem > 0)
            {
                redrawList = true;
                selectedEntryItem--;
                //Move SelectedItem up
            }
        }

        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1)
        {
            if(UncommittedChanges > 0 && selectedEntryItem < UncommittedChanges - 1)
            {
                redrawList = true;
                selectedEntryItem++;
                //Move SelectedItem down
            }
        }
        wait_ms(10);
    }
}
#endif
