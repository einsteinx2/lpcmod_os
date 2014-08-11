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


void decrementActiveBank(void * itemStr) {
	switch(LPCmodSettings.OSsettings.activeBank){
	case BNK512:
		LPCmodSettings.OSsettings.activeBank = BNKTSOP;
		sprintf(itemStr,"Quickboot bank : TSOP");
		break;
	case BNK256:
		LPCmodSettings.OSsettings.activeBank = BNK512;
		sprintf(itemStr,"Quickboot bank : 512KB");
		break;
	case BNKTSOP:
		LPCmodSettings.OSsettings.activeBank = BNK256;
		sprintf(itemStr,"Quickboot bank : 256KB");
		break;
	}
	return;
}



void incrementActiveBank(void * itemStr) {
	switch(LPCmodSettings.OSsettings.activeBank){
	case BNK512:
		LPCmodSettings.OSsettings.activeBank = BNK256;
		sprintf(itemStr,"Quickboot bank : 256KB");
		break;
	case BNK256:
		LPCmodSettings.OSsettings.activeBank = BNKTSOP;
		sprintf(itemStr,"Quickboot bank : TSOP");
		break;
	case BNKTSOP:
		LPCmodSettings.OSsettings.activeBank = BNK512;
		sprintf(itemStr,"Quickboot bank : 512KB");
		break;
	}
	return;
}


void decrementbootTimeout(void * itemStr){
	if(LPCmodSettings.OSsettings.bootTimeout > 0)	//Logic
		LPCmodSettings.OSsettings.bootTimeout -= 1;
	sprintf(itemStr, "Idle timeout : %ds", LPCmodSettings.OSsettings.bootTimeout);
	return;
}
void incrementbootTimeout(void * itemStr){
	if(LPCmodSettings.OSsettings.bootTimeout < 240)	//I've got to set a limit there.
		LPCmodSettings.OSsettings.bootTimeout += 1;
	sprintf(itemStr, "Idle timeout : %ds", LPCmodSettings.OSsettings.bootTimeout);
	return;
}

void toggleQuickboot(void * itemStr){
	(bool)(LPCmodSettings.OSsettings.Quickboot) += 1;
	sprintf(itemStr,"Quickboot : %s",LPCmodSettings.OSsettings.Quickboot? "Yes" : "No");
}
