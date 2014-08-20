/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* This is where you should customise your calls to the onscreen keyboard.
 * The code in KeyboardMenu.c should normally be left alone.
 */
#include "include/config.h"
#include "BootIde.h"
#include "KeyboardMenu.h"
//#include "KeyboardActions.h"		//your time will come
#include "lpcmod_v1.h"


void KeyboardMenuInit(void *itemStr) {
	KeyboardMenuDraw((char *)itemStr);			//Function that lets you navigate the keyboard.
												//Prints the edited string and place cursor at the end of it
}
