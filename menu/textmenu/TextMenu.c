/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "TextMenu.h"
#include "lib/LPCMod/BootLCD.h"
int breakOutOfMenu=0;
u32 temp, oldTemp; 
int timeRemain = 0;
int oldTimeRemain = 0;
int visibleCount = 0;

void TextMenuDraw(TEXTMENU *menu, TEXTMENUITEM *firstVisibleMenuItem, TEXTMENUITEM *selectedItem);

void TextMenuAddItem(TEXTMENU *menu, TEXTMENUITEM *newMenuItem) {
	TEXTMENUITEM *menuItem = menu->firstMenuItem;
	TEXTMENUITEM *currentMenuItem=NULL;
	
	while (menuItem != NULL) {
		currentMenuItem = menuItem;
		menuItem = menuItem->nextMenuItem;
	}
	
	if (currentMenuItem==NULL) { 
		//This is the first icon in the chain
		menu->firstMenuItem = newMenuItem;
	}
	//Append to the end of the chain
	else currentMenuItem->nextMenuItem = newMenuItem;
	newMenuItem->nextMenuItem = NULL;
	newMenuItem->previousMenuItem = currentMenuItem; 
}

void TextMenuDraw(TEXTMENU* menu, TEXTMENUITEM *firstVisibleMenuItem, TEXTMENUITEM *selectedItem) {
	TEXTMENUITEM *item=NULL;
	int menucount;
	int i;
	
	VIDEO_CURSOR_POSX=75;
	VIDEO_CURSOR_POSY=125;
	
	//Draw the menu title.
	VIDEO_ATTR=0xff00ff;
	if(menu->longTitle) {
		int Length=strlen(menu->szCaption);
		int CharsProcessed = 0;
		char c;
		int CharsSinceNewline = 0;
		printk("\2          \2");
		while (CharsProcessed<Length) {
			c = menu->szCaption[CharsProcessed];
			CharsProcessed++;
			CharsSinceNewline++;
			if(CharsSinceNewline >= 21) {
				printk("\2\n\n\2");
				VIDEO_CURSOR_POSX=75;
				printk("\2          \2");
				CharsSinceNewline = 1;
			}
			printk("\2%c\2",c);
		}
	} else {
		printk("\2          %s\2", menu->szCaption);
	}
	
	if(temp != 0) {
		// If we have a timeout running...
		printk("  (%i)\2", timeRemain);
	}

	VIDEO_CURSOR_POSY+=30;
	
	//Draw the menu items
	VIDEO_CURSOR_POSX=150;
	
	//If we were moving up, the 
	
	item=firstVisibleMenuItem;

	visibleCount = menu->visibleCount;

	if(visibleCount == 0) {
		visibleCount = 8;
	}

	for (menucount=0; menucount<visibleCount; menucount++) {
		if (item==NULL) {
			//No more menu items to draw
			return;
		}
		//Selected item in yellow
		if (item == selectedItem){ 
			VIDEO_ATTR=0xffef37;
		}
		else VIDEO_ATTR=0xffffff;
		//Font size 2=big.
		printk("\n\2               %s",item->szCaption);
		if(item->szParameter[0] != 0)
			printk("\2%s\n", item->szParameter);
		else
			printk("\n");
		item=item->nextMenuItem;
		//LCD string print.
		
	}
	VIDEO_ATTR=0xffffff;
	if(xLCD.enable == 1){
		bool colon=false;
		char titleLine[xLCD.LineSize + 1];
		xLCD.PrintLine2(JUSTIFYLEFT, menu->szCaption);
		titleLine[xLCD.LineSize] = 0;					//End of line character.
		memset(titleLine,0x20,xLCD.LineSize);			//Fill with "Space" characters.
		for(i = 0; i < strlen(selectedItem->szCaption); i++){
			if(selectedItem->szCaption[i] != ':'){
				if( i < xLCD.LineSize)
					titleLine[i] = selectedItem->szCaption[i];					//Copy characters as long as we're under 20 characters or no ':' was encountered.
			}
			else{
				if( i < xLCD.LineSize)
					titleLine[i] = selectedItem->szCaption[i];					//Print out the ':' character anyway.
				colon = true;
				break;												//Leave the for-loop as no other character will be printed on this line.
			}
		}
		xLCD.PrintLine3(JUSTIFYLEFT, titleLine);
		if(colon) {
			memset(titleLine,0x20,xLCD.LineSize);			//Fill with "Space" characters.
			u8 nbChars = strlen(selectedItem->szParameter);		//Number of character in string
			for (i = 0; i < nbChars; i++){				//Justify text to the right of the screen.
				titleLine[xLCD.LineSize - nbChars + i] = selectedItem->szParameter[i];
			}
			xLCD.PrintLine4(JUSTIFYLEFT, titleLine);
			colon = false;
		}
		else{
			xLCD.ClearLine(3);
		}

	}
}

void TextMenu(TEXTMENU *menu, TEXTMENUITEM *selectedItem) {
	temp = menu->timeout;
	u32 COUNT_start;
	COUNT_start = IoInputDword(0x8008);

	TEXTMENUITEM *itemPtr, *selectedMenuItem, *firstVisibleMenuItem;
	BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
	
	if (selectedItem!=NULL) selectedMenuItem = selectedItem;
	else selectedMenuItem = menu->firstMenuItem;
	
	firstVisibleMenuItem = menu->firstMenuItem;
	TextMenuDraw(menu, firstVisibleMenuItem, selectedMenuItem);
	
	//Main menu event loop.
	while(1)
	{
		int changed=0;
		wait_ms(10);

		if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1)
		{
			oldTemp = temp;
			temp = 0;
			if(oldTemp != 0) {
					BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
			}

			if (selectedMenuItem->previousMenuItem!=NULL) {
				if (firstVisibleMenuItem == selectedMenuItem) {
					firstVisibleMenuItem = selectedMenuItem->previousMenuItem;
					BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
				}
				selectedMenuItem = selectedMenuItem->previousMenuItem;
				TextMenuDraw(menu, firstVisibleMenuItem, selectedMenuItem);
			}
		} 
		else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1) {
			oldTemp = temp;
			temp = 0;
			if(oldTemp != 0) {
					BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
			}

			int i=0;
			if (selectedMenuItem->nextMenuItem!=NULL) {
				TEXTMENUITEM *lastVisibleMenuItem = firstVisibleMenuItem;
				//8 menu items per page.
				for (i=0; i<visibleCount-1; i++) {
					if (lastVisibleMenuItem->nextMenuItem==NULL) break;
					lastVisibleMenuItem = lastVisibleMenuItem->nextMenuItem;
				}
				if (selectedMenuItem == lastVisibleMenuItem) {
					firstVisibleMenuItem = firstVisibleMenuItem->nextMenuItem;
					BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
				}
				selectedMenuItem = selectedMenuItem->nextMenuItem;
				TextMenuDraw(menu, firstVisibleMenuItem, selectedMenuItem);
			}
		}
		else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1 || risefall_xpad_STATE(XPAD_STATE_START) == 1 || (u32)temp>(0x369E99*MENU_TIMEWAIT)) {
			temp = 0;
			BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
			VIDEO_ATTR=0xffffff;
			//Menu item selected - invoke function pointer.
			if (selectedMenuItem->functionPtr!=NULL) selectedMenuItem->functionPtr(selectedMenuItem->functionDataPtr);
			//Clear the screen again	
			BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
			VIDEO_ATTR=0xffffff;
			//Did the function that was run set the 'Quit the menu' flag?
			if (breakOutOfMenu) {
				breakOutOfMenu=0;
				return;
			}
			//We need to redraw ourselves
			TextMenuDraw(menu, firstVisibleMenuItem, selectedMenuItem);
		}
		else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1) {
			oldTemp = temp;
			temp = 0;
			if(oldTemp != 0) {
					BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
			}

			VIDEO_ATTR=0xffffff;
			//Menu item selected - invoke function pointer.
			if (selectedMenuItem->functionLeftPtr!=NULL) {
				selectedMenuItem->functionLeftPtr(selectedMenuItem->functionLeftDataPtr);
				//Clear the screen again	
				BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
				VIDEO_ATTR=0xffffff;
				//Did the function that was run set the 'Quit the menu' flag?
				if (breakOutOfMenu) {
					breakOutOfMenu=0;
					return;
				}
			}
			//We need to redraw ourselves
			TextMenuDraw(menu, firstVisibleMenuItem, selectedMenuItem);
		}
		else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1) {
			oldTemp = temp;
			temp = 0;
			if(oldTemp != 0) {
					BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
			}

			VIDEO_ATTR=0xffffff;
			//Menu item selected - invoke function pointer.
			if (selectedMenuItem->functionRightPtr!=NULL) {
				selectedMenuItem->functionRightPtr(selectedMenuItem->functionRightDataPtr);
				//Clear the screen again	
				BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
				VIDEO_ATTR=0xffffff;
				//Did the function that was run set the 'Quit the menu' flag?
				if (breakOutOfMenu) {
					breakOutOfMenu=0;
					return;
				}
			}
			//We need to redraw ourselves
			TextMenuDraw(menu, firstVisibleMenuItem, selectedMenuItem);
		}
/*		else if (risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) == 1) {
			oldTemp = temp;
			temp = 0;
			if(oldTemp != 0) {
					BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
			}

			VIDEO_ATTR=0xffffff;
			//Menu item selected - invoke function pointer.
			if (selectedMenuItem->functionRightPtr!=NULL) {
				selectedMenuItem->functionRightPtr(selectedMenuItem->functionRightDataPtr);
				//Clear the screen again	
				BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
				VIDEO_ATTR=0xffffff;
				//Did the function that was run set the 'Quit the menu' flag?
				if (breakOutOfMenu) {
					breakOutOfMenu=0;
					return;
				}
			}
			//We need to redraw ourselves
			TextMenuDraw(menu, firstVisibleMenuItem, selectedMenuItem);
		}
		else if (risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) == 1) {
			oldTemp = temp;
			temp = 0;
			if(oldTemp != 0) {
					BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
			}

			VIDEO_ATTR=0xffffff;
			//Menu item selected - invoke function pointer.
			if (selectedMenuItem->functionLeftPtr!=NULL) {
				selectedMenuItem->functionLeftPtr(selectedMenuItem->functionLeftDataPtr);
				//Clear the screen again	
				BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
				VIDEO_ATTR=0xffffff;
				//Did the function that was run set the 'Quit the menu' flag?
				if (breakOutOfMenu) {
					breakOutOfMenu=0;
					return;
				}
			}
			//We need to redraw ourselves
			TextMenuDraw(menu, firstVisibleMenuItem, selectedMenuItem);
		}
*/
		else if (risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) == 1 || risefall_xpad_STATE(XPAD_STATE_BACK) == 1) {
			BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
			VIDEO_ATTR=0xffffff;
			temp = 0;
			bypassConfirmDialog[0] = 0;	//reset for confirmDialog
			return;
		}

		if (temp != 0) {
			temp = IoInputDword(0x8008) - COUNT_start;
			oldTimeRemain = timeRemain;
			timeRemain = MENU_TIMEWAIT - temp/0x369E99;
			if (oldTimeRemain != timeRemain) {
				BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
				TextMenuDraw(menu, firstVisibleMenuItem, selectedMenuItem);
			}
		}
	}
}

