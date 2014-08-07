#include "boot.h"
#include "VideoInitialization.h"
#include "BootLPCMod.h"

void initialLPCModOSBoot(_LPCmodSettings *LPCmodSettings){
	LPCmodSettings->OSsettings.migrateSetttings = 0;
	LPCmodSettings->OSsettings.activeBank = 0;
	LPCmodSettings->OSsettings.Quickboot = 0;
	LPCmodSettings->OSsettings.selectedMenuItem = 0;
	LPCmodSettings->OSsettings.fanSpeed = DEFAULT_FANSPEED;
	LPCmodSettings->OSsettings.bootTimeout = BOOT_TIMEWAIT;
	LPCmodSettings->OSsettings.enableNetwork = 0;
	LPCmodSettings->OSsettings.useDHCP = 0;

	LPCmodSettings->LCDsettings.migrateLCD = 0;
	LPCmodSettings->LCDsettings.enable5V = 0;
	LPCmodSettings->LCDsettings.lcdType = 0;
	LPCmodSettings->LCDsettings.nbLines = HDD4780_DEFAULT_NBLINES;
	LPCmodSettings->LCDsettings.lineLength = HDD4780_DEFAULT_LINELGTH;
	LPCmodSettings->LCDsettings.backlight = 0;
	LPCmodSettings->LCDsettings.contrast = 0;
	LPCmodSettings->LCDsettings.displayMsgBoot = 0;
	LPCmodSettings->LCDsettings.customTextBoot = 0;
	LPCmodSettings->LCDsettings.displayBIOSNameBoot = 0;
}
