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


void decrementActiveBank(void * activeBank) {
	switch(*(u8 *)activeBank){
	case BNK512:
		*(u8 *)activeBank = BNKTSOP;
		break;
	case BNK256:
		*(u8 *)activeBank = BNK512;
		break;
	case BNKTSOP:
		*(u8 *)activeBank = BNK256;
		break;
	}
	return;
}



void incrementActiveBank(void * activeBank) {
	switch(*(u8 *)activeBank){
	case BNK512:
		*(u8 *)activeBank = BNK256;
		break;
	case BNK256:
		*(u8 *)activeBank = BNKTSOP;
		break;
	case BNKTSOP:
		*(u8 *)activeBank = BNK512;
		break;
	}
	return;
}


void decrementbootTimeout(void * bootTimeout){
	if(*(u8 *)bootTimeout > 0)	//Logic
		*(u8 *)bootTimeout -= 1;
	return;
}
void incrementbootTimeout(void * bootTimeout){
	if(*(u8 *)bootTimeout < 240)	//I've got to set a limit there.
			*(u8 *)bootTimeout += 1;
	return;
}

void toggleQuickboot(void * Quickboot){
	*(bool *)Quickboot += 1;
}
