#ifndef _KEYBOARDMENU_H_
#define _KEYBOARDMENU_H_

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define KEY_BCKSPC	0x3
#define KEY_OK		0x1
#define KEY_CANCEL	0x2
#define KEY_SHIFT	0x4
#define KEY_CAPS	0x5

char keymap[5][10] = {{'1','2','3','4','5','6','7','8','9','0'},
						  {'A','B','C','D','E','F','G','H','I','J'},
						  {'K','L','M','N','O','P','Q','R','S','T'},
						  {'U','V','W','X','Y','Z', 0,  0,  0,  0 },
						  {KEY_SHIFT,KEY_CAPS,0,0,' ',0,KEY_BCKSPC,0,KEY_CANCEL,KEY_OK}};
char shiftkeymap[5][10] = {{'!','"','#','$','%','&','^','*','(',')'},
						   {'a','b','c','d','e','f','g','h','i','j'},
						   {'k','l','m','n','o','p','q','r','s','t'},
						   {'u','v','w','x','y','z', 0,  0,  0,  0 },
						   {KEY_SHIFT,KEY_CAPS,0,0,' ',0,KEY_BCKSPC,0,KEY_CANCEL,KEY_OK}};



//This draws and handles input for the OnScreen Keyboard
void KeyboardMenu(void * itemStr);
void KeyboardMenuDraw(char * itemStr);

#endif
