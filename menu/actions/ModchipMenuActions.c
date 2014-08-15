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
	(LPCmodSettings.OSsettings.Quickboot) = (LPCmodSettings.OSsettings.Quickboot)? 0 : 1;
	sprintf(itemStr,"Quickboot : %s",LPCmodSettings.OSsettings.Quickboot? "Yes" : "No");
}

void resetSettings(void *whatever){
	initialLPCModOSBoot(&LPCmodSettings);
	QuickReboot();
}

void toggleTSOPControl(void * itemPtr){
	TEXTMENUITEM * tempItemPtr = (TEXTMENUITEM *)&itemPtr;
	if(LPCmodSettings.OSsettings.TSOPcontrol & 0x01){			//If already active
		LPCmodSettings.OSsettings.TSOPcontrol &= 0xFE;	//Make sure to toggle only bit0.
	}
	else{
		LPCmodSettings.OSsettings.TSOPcontrol |= 0x01;	//Make sure to toggle only bit0.
	}
	sprintf(tempItemPtr->szCaption,"Control Xbox TSOP : %s", (LPCmodSettings.OSsettings.TSOPcontrol) & 0x01? "Yes" : "No");
	sprintf(tempItemPtr->nextMenuItem->szCaption, "Xbox TSOP split : %s",
		(LPCmodSettings.OSsettings.TSOPcontrol) & 0x01?	((LPCmodSettings.OSsettings.TSOPcontrol) & 0x02? "4-way" : "2-way") : "No");
}

void toggleTSOPSplit(void * itemStr){
	if((LPCmodSettings.OSsettings.TSOPcontrol & 0x02) || !(LPCmodSettings.OSsettings.TSOPcontrol & 0x01)){	//If TSOPControl bit1 is set or bit0 is not
		LPCmodSettings.OSsettings.TSOPcontrol &= 0xFD;	//Make sure to toggle only bit1.
	}
	else {
		LPCmodSettings.OSsettings.TSOPcontrol |= 0x02;	//Make sure to toggle only bit1.
	}
	sprintf(itemStr, "Xbox TSOP split : %s",
			(LPCmodSettings.OSsettings.TSOPcontrol) & 0x01?	((LPCmodSettings.OSsettings.TSOPcontrol) & 0x02? "4-way" : "2-way") : "No");
}
