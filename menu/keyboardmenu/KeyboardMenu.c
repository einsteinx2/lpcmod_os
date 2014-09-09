/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
 * Onscreen keyboard interface.
 * 14/08/20 - Benjamin Fiset-Deschenes
 * You should not need to edit this file in normal circumstances - Call to
 * the keyboard should be made through KeyboardMenuInit.c
 */

#include "boot.h"
#include "video.h"
#include "xbox.h"
//#include "MenuActions.h"
#include "config.h"
#include "KeyboardMenu.h"
#include "lib/LPCMod/BootLCD.h"



void KeyboardMenuDraw(char * itemStr) {
    ICON *iconPtr;
    ICON *iconSeeker;
    int iconcount;
    u8 opaqueness;
    int tempX, tempY;

        if (iconPtr==selectedIcon) {
            //Selected icon has less transparency
            //and has a caption drawn underneath it
            opaqueness = SELECTED;
            VIDEO_CURSOR_POSX=nXOffset+140*(iconcount+1)*4;
            VIDEO_CURSOR_POSY=nYOffset+20;
            VIDEO_ATTR=0xffffff;
            printk("%s\n",iconPtr->szCaption);
            //LCD string print.
            if(xLCD->enable){
                xLCD->PrintLine2(JUSTIFYLEFT, iconPtr->szCaption);
                xLCD->ClearLine(2);
                xLCD->ClearLine(3);
            }
        }
        else opaqueness = TRANSPARENTNESS;
        
        BootVideoJpegBlitBlend(
            (u8 *)(FB_START+((vmode.width * (nYOffset-74))+nXOffset+(140*(iconcount+1))) * 4),
            vmode.width, // dest bLPCmodSettingsytes per line
            &jpegBackdrop, // source jpeg object
            (u8 *)(jpegBackdrop.pData+(iconPtr->iconSlot * jpegBackdrop.bpp)),
            0xff00ff|(((u32)opaqueness)<<24),
            (u8 *)(jpegBackdrop.pBackdrop + ((jpegBackdrop.width * (nYOffset-74)) + nXOffset+(140*(iconcount+1))) * jpegBackdrop.bpp),
            ICON_WIDTH, ICON_HEIGHT
        );
        lastVisibleIcon = iconPtr;
        iconPtr = iconPtr->nextIcon;

    // If there is an icon off screen to the left, draw this icon.
    if(firstVisibleIcon->previousIcon != NULL) {
        //opaqueness = TRANSPARENTNESS;
        opaqueness = SELECTED;
        BootVideoJpegBlitBlend(
            (u8 *)(FB_START+((vmode.width * (nYOffset-74))+nXOffset+(50)) * 4),
            vmode.width, // dest bytes per line
            &jpegBackdrop, // source jpeg object
            (u8 *)(jpegBackdrop.pData+(ICON_WIDTH*5 * jpegBackdrop.bpp)),
            0xff00ff|(((u32)opaqueness)<<24),
            (u8 *)(jpegBackdrop.pBackdrop + ((jpegBackdrop.width * (nYOffset-74)) + nXOffset+(50)) * jpegBackdrop.bpp),
            ICON_WIDTH, ICON_HEIGHT
        );
    }

    // If there is an icon off screen to the right, draw this icon.
    if(lastVisibleIcon->nextIcon != NULL) {
        //opaqueness = TRANSPARENTNESS;
        opaqueness = SELECTED;
        BootVideoJpegBlitBlend(
            (u8 *)(FB_START+((vmode.width * (nYOffset-74))+nXOffset+(510)) * 4),
            vmode.width, // dest bytes per line
            &jpegBackdrop, // source jpeg object
            (u8 *)(jpegBackdrop.pData+(ICON_WIDTH*6 * jpegBackdrop.bpp)),
            0xff00ff|(((u32)opaqueness)<<24),
            (u8 *)(jpegBackdrop.pBackdrop + ((jpegBackdrop.width * (nYOffset-74)) + nXOffset+(510)) * jpegBackdrop.bpp),
            ICON_WIDTH, ICON_HEIGHT
        );
    }

    tempX = VIDEO_CURSOR_POSX;
    tempY = VIDEO_CURSOR_POSY;
    VIDEO_CURSOR_POSX=((172+((vmode.width-640)/2))<<2);
    VIDEO_CURSOR_POSY=vmode.height - 250;

    if(temp != 0) {
        printk("\2Select from Menu \2 (%i)", iconTimeRemain);    
    } else {
        VIDEO_CURSOR_POSX += 52;
        printk("\2Select from Menu \2", iconTimeRemain);    
    }

    VIDEO_CURSOR_POSX = tempX;
    VIDEO_CURSOR_POSY = tempY;
}

void KeyboardMenu(void) {
        
    u32 COUNT_start;
    int oldIconTimeRemain = 0;
    ICON *iconPtr=NULL;

    extern int nTempCursorMbrX, nTempCursorMbrY; 
    int nTempCursorResumeX, nTempCursorResumeY ;
    int nTempCursorX, nTempCursorY;
    int nModeDependentOffset=(vmode.width-640)/2;  
    
    nTempCursorResumeX=nTempCursorMbrX;
    nTempCursorResumeY=nTempCursorMbrY;

    nTempCursorX=VIDEO_CURSOR_POSX;
    nTempCursorY=vmode.height-80;
    
    // We save the complete framebuffer to memory (we restore at exit)
    videosavepage = malloc(FB_SIZE);
    memcpy(videosavepage,(void*)FB_START,FB_SIZE);
    
    VIDEO_CURSOR_POSX=((252+nModeDependentOffset)<<2);
    VIDEO_CURSOR_POSY=nTempCursorY-100;

//#ifndef SILENT_MODE
    //In silent mode, don't draw the menu the first time.
    //If we get a left/right xpad event, it will be registered,
    //and the menu will 'appear'. Otherwise, it will proceed quietly
    //and boot the default boot item
    VIDEO_ATTR=0xffc8c8c8;
    //printk("Select from Menu\n");
    VIDEO_ATTR=0xffffffff;
    IconMenuDraw(nModeDependentOffset, nTempCursorY);
//#endif
    COUNT_start = IoInputDword(0x8008);
    //Main menu event loop.
    while(1)
    {
        int changed=0;
        wait_ms(10);    
        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1)
        {
            if (selectedIcon->nextIcon!=NULL) {
                //A bit ugly, but need to find the last visible icon, and see if 
                //we are moving further right from it.
                lastVisibleIcon=firstVisibleIcon;
                int i=0;
                for (i=0; i<2; i++) {
                    if (lastVisibleIcon->nextIcon==NULL) {
                        break;
                    }
                    lastVisibleIcon = lastVisibleIcon->nextIcon;
                }
                if (selectedIcon == lastVisibleIcon) {
                    //We are moving further right, so slide all the icons along. 
                    if(lastVisibleIcon->nextIcon != NULL) {
                        firstVisibleIcon = firstVisibleIcon->nextIcon;    
                    }
                    //As all the icons have moved, we need to refresh the entire page.
                    memcpy((void*)FB_START,videosavepage,FB_SIZE);
                }
                memcpy((void*)FB_START,videosavepage,FB_SIZE);
                selectedIcon = selectedIcon->nextIcon;
                changed=1;
            }
            temp=0;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1)
        {
            if (selectedIcon->previousIcon!=NULL) {
                if (selectedIcon == firstVisibleIcon) {
                    //We are moving further left, so slide all the icons along. 
                    if(firstVisibleIcon->previousIcon != NULL) {
                        firstVisibleIcon = firstVisibleIcon->previousIcon;
                    }
                    //As all the icons have moved, we need to refresh the entire page.
                    memcpy((void*)FB_START,videosavepage,FB_SIZE);
                }
                memcpy((void*)FB_START,videosavepage,FB_SIZE);
                selectedIcon = selectedIcon->previousIcon;
                changed=1;
            }
            temp=0;
        }

        
        if ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1) || risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_START) == 1 || 
                (u32)(temp>(0x369E99*varBootTimeWait))) {
            memcpy((void*)FB_START,videosavepage,FB_SIZE);
            VIDEO_CURSOR_POSX=nTempCursorResumeX;
            VIDEO_CURSOR_POSY=nTempCursorResumeY;
            
            if (temp>(0x369E99*varBootTimeWait)) timedOut=1;
            //Icon selected - invoke function pointer.
            if (selectedIcon->functionPtr!=NULL) selectedIcon->functionPtr(selectedIcon->functionDataPtr);
            //If we come back to this menu, make sure we are redrawn, and that we replace the saved video page
            changed=1;
            memcpy((void*)FB_START,videosavepage,FB_SIZE);
        }

        if (changed) {
            BootVideoClearScreen(&jpegBackdrop, nTempCursorY, VIDEO_CURSOR_POSY+1);
            IconMenuDraw(nModeDependentOffset, nTempCursorY);
            changed=0;
        }
        
    }
}

