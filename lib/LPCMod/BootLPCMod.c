#include "boot.h"
#include "VideoInitialization.h"
#include "BootLPCMod.h"
#include "lpcmod_v1.h"
#include "LEDMenuActions.h"


//Sets default values to most important settings.
void initialLPCModOSBoot(_LPCmodSettings *LPCmodSettings){
	LPCmodSettings->OSsettings.migrateSetttings = 0;
	LPCmodSettings->OSsettings.activeBank = 0;
	LPCmodSettings->OSsettings.Quickboot = 0;
	LPCmodSettings->OSsettings.selectedMenuItem = 0;
	LPCmodSettings->OSsettings.fanSpeed = DEFAULT_FANSPEED;
	LPCmodSettings->OSsettings.bootTimeout = BOOT_TIMEWAIT;
	LPCmodSettings->OSsettings.LEDColor = LED_GREEN;	//Set for next boot
	LPCmodSettings->OSsettings.enableNetwork = 0;
	LPCmodSettings->OSsettings.useDHCP = 0;
	LPCmodSettings->OSsettings.biosName0[0] = 0;
	LPCmodSettings->OSsettings.biosName1[0] = 0;
	LPCmodSettings->OSsettings.biosName2[0] = 0;
	LPCmodSettings->OSsettings.biosName3[0] = 0;
	LPCmodSettings->OSsettings.biosName4[0] = 0;
	LPCmodSettings->OSsettings.biosName5[0] = 0;
	LPCmodSettings->OSsettings.biosName6[0] = 0;
	LPCmodSettings->OSsettings.biosName7[0] = 0;

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
	LPCmodSettings->LCDsettings.customString0[0] = 0;
	LPCmodSettings->LCDsettings.customString1[0] = 0;
	LPCmodSettings->LCDsettings.customString2[0] = 0;
	LPCmodSettings->LCDsettings.customString3[0] = 0;
}

//Probes CPLD for chip revision and return a single byte ID.
u8 LPCMod_HW_rev(void){
	return(ReadFromIO(SYSCON_REG));
}
