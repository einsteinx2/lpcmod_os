#include "boot.h"
#include "VideoInitialization.h"
#include "BootLPCMod.h"
#include "lpcmod_v1.h"
#include "LEDMenuActions.h"


//Sets default values to most important settings.
void initialLPCModOSBoot(_LPCmodSettings *LPCmodSettings){
    u8 i;

    LPCmodSettings->OSsettings.migrateSetttings = 0;
    LPCmodSettings->OSsettings.activeBank = BNK512;
    LPCmodSettings->OSsettings.altBank = BNK256;
    LPCmodSettings->OSsettings.Quickboot = 0;
    LPCmodSettings->OSsettings.selectedMenuItem = 0;
    LPCmodSettings->OSsettings.fanSpeed = DEFAULT_FANSPEED;
    LPCmodSettings->OSsettings.bootTimeout = BOOT_TIMEWAIT;
    LPCmodSettings->OSsettings.LEDColor = LED_GREEN;    //Set for next boot
    LPCmodSettings->OSsettings.TSOPcontrol = 0;
    LPCmodSettings->OSsettings.enableNetwork = 0;
    LPCmodSettings->OSsettings.useDHCP = 0;


    LPCmodSettings->LCDsettings.migrateLCD = 0;
    LPCmodSettings->LCDsettings.enable5V = 0;
    LPCmodSettings->LCDsettings.lcdType = 0;
    LPCmodSettings->LCDsettings.nbLines = HDD4780_DEFAULT_NBLINES;
    LPCmodSettings->LCDsettings.lineLength = HDD4780_DEFAULT_LINELGTH;
    LPCmodSettings->LCDsettings.backlight = 50;
    LPCmodSettings->LCDsettings.contrast = 20;
    LPCmodSettings->LCDsettings.displayMsgBoot = 0;
    LPCmodSettings->LCDsettings.customTextBoot = 0;
    LPCmodSettings->LCDsettings.displayBIOSNameBoot = 0;
    
    
    for(i = 0; i < HDD4780_DEFAULT_LINELGTH + 1; i++){
        LPCmodSettings->OSsettings.biosName0[i] = 0;
        LPCmodSettings->OSsettings.biosName1[i] = 0;
        LPCmodSettings->OSsettings.biosName2[i] = 0;
        LPCmodSettings->OSsettings.biosName3[i] = 0;
        LPCmodSettings->OSsettings.biosName4[i] = 0;
        LPCmodSettings->OSsettings.biosName5[i] = 0;
        LPCmodSettings->OSsettings.biosName6[i] = 0;
        LPCmodSettings->OSsettings.biosName7[i] = 0;
    
        LPCmodSettings->LCDsettings.customString0[i] = 0;
        LPCmodSettings->LCDsettings.customString1[i] = 0;
        LPCmodSettings->LCDsettings.customString2[i] = 0;
        LPCmodSettings->LCDsettings.customString3[i] = 0;
    }
}

//Probes CPLD for chip revision and return a single byte ID.
u16 LPCMod_HW_rev(void){
    return ReadFromIO(SYSCON_REG);
}

void LPCMod_LCDBankString(char * string, u8 bankID){
    switch(bankID){
        case BNK512:
            if(LPCmodSettings.OSsettings.biosName0[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosName0);
            }
            else{
                sprintf(string, "%s", "512KB bank");
            }
            break;
        case BNK256:
            if(LPCmodSettings.OSsettings.biosName1[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosName1);
            }
            else{
                sprintf(string, "%s", "256KB bank");
            }
            break;
        case BNKTSOP:
            if(LPCmodSettings.OSsettings.biosName2[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosName2);
            }
            else{
                if(LPCmodSettings.OSsettings.TSOPcontrol & 0x01)
                    sprintf(string, "%s", "OnBoard Bank0");
                else
                    sprintf(string, "%s", "OnBoard BIOS");
            }
            break;
        case BNKTSOP1:
            if(LPCmodSettings.OSsettings.biosName3[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosName3);
            }
            else{
                sprintf(string, "%s", "OnBoard Bank1");
            }
            break;
        case BNKTSOP2:
            if(LPCmodSettings.OSsettings.biosName3[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosName4);
            }
            else{
                sprintf(string, "%s", "OnBoard Bank2");
            }
            break;
        case BNKTSOP3:
            if(LPCmodSettings.OSsettings.biosName4[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosName5);
            }
            else{
                sprintf(string, "%s", "OnBoard Bank3");
            }
            break;
         default:
         	sprintf(string, "%s", "Settings");
         	break;
    }


}
