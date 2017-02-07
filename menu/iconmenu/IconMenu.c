/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
 * Redesigned icon menu, allowing icons to be added/removed at runtime.
 * 02/10/04 - David Pye dmp@davidmpye.dyndns.org
 * You should not need to edit this file in normal circumstances - icons are
 * added/removed via boot/IconMenuInit.c
 */

#include "boot.h"
#include "video.h"
#include "memory_layout.h"
#include "rc4.h"
#include "sha1.h"
#include "BootFATX.h"
#include "cpu.h"
#include "BootIde.h"
#include "MenuActions.h"
#include "config.h"
#include "IconMenu.h"
#include "lib/LPCMod/BootLCD.h"
#include "lib/time/timeManagement.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include "xblast/settings/xblastSettings.h"
#include "xblast/settings/xblastSettingsChangeTracker.h"
#include "lib/cromwell/cromString.h"
#include "lib/cromwell/cromSystem.h"
#include "xblast/HardwareIdentifier.h"
#include "string.h"

#define TRANSPARENTNESS 0x30
#define SELECTED 0xff

int iconTimeRemain = 0;
unsigned int temp = 1;

static void generateIconMenuStructure(void);

void AddIcon(ICON *newIcon)
{
    ICON *iconPtr = firstIcon;
    ICON *currentIcon = NULL;

    while (iconPtr != NULL)
    {
        currentIcon = iconPtr;
        iconPtr = iconPtr->nextIcon;
    }
    
    if (currentIcon == NULL)
    {
        //This is the first icon in the chain
        firstIcon = newIcon;
    }
    else //Append to the end of the chain
    {
        currentIcon->nextIcon = newIcon;
    }
    iconPtr = newIcon;
    iconPtr->nextIcon = NULL;
    iconPtr->previousIcon = currentIcon; 
}

static void IconMenuDraw(int nXOffset, int nYOffset)
{
    ICON *iconPtr;            //Icon the code is currently working on.
    ICON *iconSeeker;        //Pointer to the icon we want to be selected at boot.
    ICON* firstVisibleIcon = selectedIcon;    //Could also have been named rightIcon
    ICON* lastVisibleIcon = selectedIcon;      //and leftIcon since only 3 icons are shown on the iconMenu.
    int iconcount;
    unsigned char opaqueness;
    int tempX, tempY;

    unsigned char uncommittedChanges = LPCMod_CountNumberOfChangesInSettings(false, NULL);
    uncommittedChanges += generateEEPROMChangeList(false, NULL); //do not generate strings
    if(LPCMod_checkForBootScriptChanges())
    {
        uncommittedChanges += 1;
    }

    if(LPCMod_checkForBackupEEPROMChange())
    {
        uncommittedChanges += 1;
    }

    generateIconMenuStructure();

    //Seeking icon with desired bankID value must be done when both firstVisibleIcon and selectedIcon are NULL.
    //This way, seeking desired icon will only occur at initial draw.
/*
    if (firstVisibleIcon==NULL) firstVisibleIcon = firstIcon;
    if (selectedIcon==NULL) selectedIcon = firstIcon;
    iconPtr = firstVisibleIcon;
*/
    // At boot only.
    if(selectedIcon == NULL)
    {
        switch(LPCmodSettings.OSsettings.activeBank)
        {
            default:
                firstVisibleIcon = firstIcon;
                selectedIcon = firstVisibleIcon;
                break;
            case BNK512:
                firstVisibleIcon = firstIcon;
                selectedIcon = icon512BankIcon;
                break;
            case BNK256:
                firstVisibleIcon = firstIcon;
                selectedIcon = icon256BankIcon;
                break;
            case BNKFULLTSOP:
                if(LPCmodSettings.OSsettings.TSOPhide == false)
                {
                    firstVisibleIcon = icon256BankIcon;
                    selectedIcon = iconFullTSOPBankIcon;
                }
                break;
            case BNKTSOPSPLIT0:
                if(LPCmodSettings.OSsettings.TSOPhide == false)
                {
                    firstVisibleIcon = icon256BankIcon;
                    selectedIcon = iconSplitTSOPBank0Icon;
                }
                break;
            case BNKTSOPSPLIT1:
                if(LPCmodSettings.OSsettings.TSOPhide == false)
                {
                    firstVisibleIcon = iconSplitTSOPBank0Icon;
                    selectedIcon = iconSplitTSOPBank1Icon;
                }
                break;
        }
        if(firstVisibleIcon->nextIcon != NULL)
        {
            lastVisibleIcon = firstVisibleIcon->nextIcon;
        }
        if(lastVisibleIcon->nextIcon != NULL)
        {
            lastVisibleIcon = lastVisibleIcon->nextIcon;
        }
    }
    else
    {
        // redraw
        iconPtr = firstIcon;
        iconSeeker = NULL;

        // Check selected icon is still in icon list.
        while(iconPtr != NULL)
        {
            if(iconPtr == selectedIcon)
            {
                iconSeeker = iconPtr;
                break;
            }
            iconPtr = iconPtr->nextIcon;
        }

        // Selected icon not in list. Default to first icon.
        if(iconSeeker == NULL)
        {
            selectedIcon = firstIcon;
        }

        firstVisibleIcon = selectedIcon;
        lastVisibleIcon = selectedIcon;

        // There's at least 1 icon on the right
        if(selectedIcon->nextIcon != NULL)
        {
            lastVisibleIcon = selectedIcon->nextIcon;
            //No icon on the left and at least 1 extra than preivously on the right
            if(selectedIcon->previousIcon == NULL && lastVisibleIcon->nextIcon != NULL)
            {
                // Selected=first and last visible is 3rd displayed icon
                lastVisibleIcon = lastVisibleIcon->nextIcon;
            }
            else if(selectedIcon->previousIcon != NULL)
            {
                //Selected!=first. Selected will be positionned in middle.
                firstVisibleIcon = selectedIcon->previousIcon;
            }
        }
        else //No icon on the right
        {
            //One icon on left of selected
            if(firstVisibleIcon->previousIcon != NULL)
            {
                firstVisibleIcon = firstVisibleIcon->previousIcon;
            }
            // 2 icons on the right of selected
            if(firstVisibleIcon->previousIcon != NULL)
            {
                firstVisibleIcon = firstVisibleIcon->previousIcon;
            }
        }
    }

    iconPtr = firstVisibleIcon;

    //There are max 3 (three) 'bays' for displaying icons in - we only draw the 3.
    for (iconcount = 0; iconcount < 3; iconcount++)
    {
        if (iconPtr == NULL)
        {
            //No more icons to draw
            return;
        }

        tempX = nXOffset+140*(iconcount+1);

        if (iconPtr == selectedIcon)
        {
            //Selected icon has less transparency
            //and has a caption drawn underneath it
            opaqueness = SELECTED;
            VIDEO_ATTR = 0xffffff;
            centerPrintK((tempX + ICON_WIDTH / 2), nYOffset+20, "%s", iconPtr->szCaption);
        }
        else
        {
            opaqueness = TRANSPARENTNESS;
        }
        
        BootVideoJpegBlitBlend(
            (unsigned char *)(FB_START+((vmode.width * (nYOffset-74))+tempX) * 4),
            vmode.width, // dest bytes per line
            &jpegBackdrop, // source jpeg object
            (unsigned char *)(jpegBackdrop.pData+(iconPtr->iconSlot * 3)),
            0xff00ff|(((unsigned int)opaqueness)<<24),
            (unsigned char *)(jpegBackdrop.pBackdrop + ((1024 * (nYOffset-74)) + tempX) * 3),
            ICON_WIDTH, ICON_HEIGHT
        );
        lastVisibleIcon = iconPtr;
        iconPtr = iconPtr->nextIcon;
    }

    // If there is an icon off screen to the left, draw this icon.
    if(firstVisibleIcon->previousIcon != NULL)
    {
        //opaqueness = TRANSPARENTNESS;
        opaqueness = SELECTED;
        BootVideoJpegBlitBlend(
            (unsigned char *)(FB_START+((vmode.width * (nYOffset-74))+nXOffset+(50)) * 4),
            vmode.width, // dest bytes per line
            &jpegBackdrop, // source jpeg object
            (unsigned char *)(jpegBackdrop.pData),
            0xff00ff|(((unsigned int)opaqueness)<<24),
            (unsigned char *)(jpegBackdrop.pBackdrop + ((1024 * (nYOffset-74)) + nXOffset+(50)) * 3),
            ICON_WIDTH, ICON_HEIGHT
        );
    }

    // If there is an icon off screen to the right, draw this icon.
    if(lastVisibleIcon->nextIcon != NULL)
    {
        //opaqueness = TRANSPARENTNESS;
        opaqueness = SELECTED;
        BootVideoJpegBlitBlend(
            (unsigned char *)(FB_START+((vmode.width * (nYOffset-74))+nXOffset+(510)) * 4),
            vmode.width, // dest bytes per line
            &jpegBackdrop, // source jpeg object
            (unsigned char *)(jpegBackdrop.pData+(ICON_WIDTH * 3)),
            0xff00ff|(((unsigned int)opaqueness)<<24),
            (unsigned char *)(jpegBackdrop.pBackdrop + ((1024 * (nYOffset-74)) + nXOffset+(510)) * 3),
            ICON_WIDTH, ICON_HEIGHT
        );
    }

    tempX = VIDEO_CURSOR_POSX;
    tempY = VIDEO_CURSOR_POSY;
    VIDEO_CURSOR_POSX=((172+((vmode.width-640)/2))<<2);
    VIDEO_CURSOR_POSY=vmode.height - 250;

    if(temp != 0)
    {
        centerScreenPrintk(VIDEO_CURSOR_POSY, "\2Please select from menu \2 (%i)", iconTimeRemain);
    }
    else
    {
        centerScreenPrintk(VIDEO_CURSOR_POSY, "\2Please select from menu \2");
    }

    if(uncommittedChanges > 0)
    {
        //There are settings that have changed.
        VIDEO_CURSOR_POSY = vmode.height - 30;
        VIDEO_CURSOR_POSX = vmode.width - 480;
        VIDEO_ATTR=0x88c8c8c8;
        printk("Uncommitted changes: %u", uncommittedChanges);
    }

    VIDEO_CURSOR_POSX = tempX;
    VIDEO_CURSOR_POSY = tempY;
}

bool IconMenu(void)
{
    unsigned int COUNT_start;
    int oldIconTimeRemain = 0;
    char bankString[20];

    extern int nTempCursorMbrX, nTempCursorMbrY; 
    int nTempCursorResumeX, nTempCursorResumeY ;
    int nTempCursorX, nTempCursorY;
    int nModeDependentOffset=(vmode.width-640)/2;  
    unsigned int varBootTimeWait = LPCmodSettings.OSsettings.bootTimeout * 1000;        //Just to have a default value.
    char timeoutString[21];                            //To display timeout countdown on xLCD
    timeoutString[20] = 0;
    
    nTempCursorResumeX=nTempCursorMbrX;
    nTempCursorResumeY=nTempCursorMbrY;

    nTempCursorX=VIDEO_CURSOR_POSX;
    nTempCursorY=vmode.height-80;
    

    // We save the complete framebuffer to memory (we restore at exit)
    //videosavepage = malloc(FB_SIZE);
    memcpy(videosavepage,(void*)FB_START,FB_SIZE);
    
    VIDEO_CURSOR_POSX=((252+nModeDependentOffset)<<2);
    VIDEO_CURSOR_POSY=nTempCursorY-100;

    if(LPCmodSettings.OSsettings.bootTimeout == 0 || isXBE() || LPCmodSettings.OSsettings.activeBank == BNKOS ||
            //No countdown if activeBank is set to a TSOP bank and TSOP boot icon are hidden.
       (LPCmodSettings.OSsettings.TSOPhide && (LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT0 ||
                                               LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT1 ||
                                               LPCmodSettings.OSsettings.activeBank == BNKFULLTSOP)))
    {
        varBootTimeWait = 0;  //Disable boot timeout
        temp = 0;
    }


//#ifndef SILENT_MODE
    //In silent mode, don't draw the menu the first time.
    //If we get a left/right xpad event, it will be registered,
    //and the menu will 'appear'. Otherwise, it will proceed quietly
    //and boot the default boot item
    VIDEO_ATTR=0xffc8c8c8;
    //printk("Select from Menu\n");
    VIDEO_ATTR=0xffffffff;
//#endif

    IconMenuDraw(nModeDependentOffset, nTempCursorY);

    //Initial LCD string print.
    if(xLCD.enable == 1)
    {
        if(LPCmodSettings.LCDsettings.customTextBoot == 0)
        {
            xLCD.PrintLine[1](CENTERSTRING, selectedIcon->szCaption);
            xLCD.ClearLine(2);
            xLCD.ClearLine(3);
        }
    }
    COUNT_start = getMS();
    //Main menu event loop.
    while(cromwellLoop())
    {
        int changed=0;
        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1)
        {
            if (selectedIcon->nextIcon!=NULL)
            {
                selectedIcon = selectedIcon->nextIcon;
                memcpy((void*)FB_START,videosavepage,FB_SIZE);
                changed=1;
            }
            temp = 0;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1)
        {
            if (selectedIcon->previousIcon!=NULL)
            {
                memcpy((void*)FB_START,videosavepage,FB_SIZE);
                selectedIcon = selectedIcon->previousIcon;
                changed=1;
            }
            temp = 0;
        }
        //If anybody has toggled the xpad left/right, disable the timeout.
        if(temp != 0)
        {
            temp = getMS() - COUNT_start;
            oldIconTimeRemain = iconTimeRemain;
            iconTimeRemain = (varBootTimeWait - temp)/1000;
            if(oldIconTimeRemain != iconTimeRemain)
            {
                changed = 1;
                memcpy((void*)FB_START,videosavepage,FB_SIZE);
            }
        }
        
        if ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1) || risefall_xpad_STATE(XPAD_STATE_START) == 1 || 
            (temp > varBootTimeWait && varBootTimeWait > 0))
        {
            memcpy((void*)FB_START,videosavepage,FB_SIZE);
            VIDEO_CURSOR_POSX=nTempCursorResumeX;
            VIDEO_CURSOR_POSY=nTempCursorResumeY;
            

            // Display custom boot message on LCD
            if(xLCD.enable == 1)
            {
                if(selectedIcon != advancedMenuIcon)
                {
                    if(LPCmodSettings.LCDsettings.displayMsgBoot == 0)    //Other icons boots banks so LCD rules apply here.
                    {
                        xLCD.Command(DISP_CLEAR);
                    }
                    else if(LPCmodSettings.LCDsettings.customTextBoot == 0)             //Do not use custom strings.
                    {                                                                   //Do not display bank name
                        if(LPCmodSettings.LCDsettings.displayBIOSNameBoot == 0)         //Only top line will be left on LCD.
                        {
                            xLCD.ClearLine(1);
                        }
                        else                                                             //Display booting bank,
                        {
                            LPCMod_LCDBankString(bankString, selectedIcon->bankID);
                            xLCD.PrintLine[1](CENTERSTRING, bankString);
                        }
                        xLCD.ClearLine(2); //Clear countdown line on screen since countdown is over and we're about to boot.
                    }
                }
            }
            //Icon selected - invoke function pointer.
            if (selectedIcon->functionPtr!=NULL)
            {
                selectedIcon->functionPtr(selectedIcon->functionDataPtr);
            }

            //If we come back to this menu, make sure we are redrawn, and that we replace the saved video page
            changed = 1;
            //TODO: fix background change color when returning to IconMenu.
            memcpy((void*)FB_START,videosavepage,FB_SIZE);
        }

        if(changed)
        {
        	if(changed == 1)
        	{
        		BootVideoClearScreen(&jpegBackdrop, nTempCursorY, VIDEO_CURSOR_POSY+1);
        	}

            IconMenuDraw(nModeDependentOffset, nTempCursorY);

            //LCD string print.
            if(xLCD.enable == 1)
            {
                if(LPCmodSettings.LCDsettings.customTextBoot == 0)
                {
                    LPCMod_LCDBankString(bankString, selectedIcon->bankID);
                    xLCD.PrintLine[1](CENTERSTRING, bankString);
                    if(LPCmodSettings.LCDsettings.nbLines >= 4)
                    {
                        if(temp != 0)
                        {
                            sprintf(timeoutString, "Auto boot in %ds", iconTimeRemain);
                            xLCD.PrintLine[2](CENTERSTRING, timeoutString);
                        }
                        else
                        {
                            xLCD.ClearLine(2);
                        }
                        xLCD.ClearLine(3);
                    }
                }
            }
            changed = 0;
        }
    }

    return 1;   //Always return 1 to stay in while loop in BootResetAction.
}

static void generateIconMenuStructure(void)
{
    firstIcon = NULL;

    icon512BankIcon->nextIcon = NULL;
    icon256BankIcon->nextIcon = NULL;
    iconFullTSOPBankIcon->nextIcon = NULL;
    iconSplitTSOPBank0Icon->nextIcon = NULL;
    iconSplitTSOPBank1Icon->nextIcon = NULL;
    advancedMenuIcon->nextIcon = NULL;

    if(TSOPRecoveryMode == false) //Do not try to boot anything if in TSOP recovery.
    {
        if(isXBlastCompatible())
        {
            // 512 bank
            if(strlen(LPCmodSettings.OSsettings.biosName512Bank) > 0)
            {
                icon512BankIcon->szCaption = LPCmodSettings.OSsettings.biosName512Bank;
            }
            else
            {
                icon512BankIcon->szCaption = "Boot 512KB bank";
            }
            AddIcon(icon512BankIcon);

            if(strlen(LPCmodSettings.OSsettings.biosName256Bank) > 0)
            {
                icon256BankIcon->szCaption = LPCmodSettings.OSsettings.biosName256Bank;
            }
            else
            {
                icon256BankIcon->szCaption = "Boot 256KB bank";
            }
            AddIcon(icon256BankIcon);
        }

        if(LPCmodSettings.OSsettings.TSOPhide == false)
        {
            if(isPureXBlast() && LPCmodSettings.OSsettings.TSOPcontrol)
            {
                if(strlen(LPCmodSettings.OSsettings.biosNameTSOPFullSplit0) > 0)
                {
                    iconSplitTSOPBank0Icon->szCaption = LPCmodSettings.OSsettings.biosNameTSOPFullSplit0;
                }
                else
                {
                    iconSplitTSOPBank0Icon->szCaption = "Boot OnBoard Bank0";
                }
                AddIcon(iconSplitTSOPBank0Icon);

                if(strlen(LPCmodSettings.OSsettings.biosNameTSOPSplit1) > 0)
                {
                    iconSplitTSOPBank1Icon->szCaption = LPCmodSettings.OSsettings.biosNameTSOPSplit1;
                }
                else
                {
                    iconSplitTSOPBank1Icon->szCaption = "Boot OnBoard Bank1";
                }
                AddIcon(iconSplitTSOPBank1Icon);
            }
            else             //No split.
            {
                if(strlen(LPCmodSettings.OSsettings.biosNameTSOPFullSplit0) > 0)
                {
                    iconFullTSOPBankIcon->szCaption = LPCmodSettings.OSsettings.biosNameTSOPFullSplit0;
                }
                else
                {
                    iconFullTSOPBankIcon->szCaption = "Boot OnBoard BIOS";
                }
                AddIcon(iconFullTSOPBankIcon);
            }
        }
    }

#ifdef ADVANCED_MENU
    AddIcon(advancedMenuIcon);
#endif
}
