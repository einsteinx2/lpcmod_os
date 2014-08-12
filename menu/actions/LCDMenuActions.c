/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"
#include "LCDMenuActions.h"

void LCDToggleEN5V(void * itemStr){
	LPCmodSettings.LCDsettings.enable5V = ~LPCmodSettings.LCDsettings.enable5V;
	sprintf(itemStr,"Enable LCD : %s", LPCmodSettings.LCDsettings.enable5V? "Yes" : "No");
}
