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
    LPCmodSettings->OSsettings.enableNetwork = 1;
    LPCmodSettings->OSsettings.useDHCP = 1;
    LPCmodSettings->OSsettings.staticIP[0] = 192;
    LPCmodSettings->OSsettings.staticIP[1] = 168;
    LPCmodSettings->OSsettings.staticIP[2] = 0;
    LPCmodSettings->OSsettings.staticIP[3] = 250;
    LPCmodSettings->OSsettings.staticGateway[0] = 192;
    LPCmodSettings->OSsettings.staticGateway[1] = 168;
    LPCmodSettings->OSsettings.staticGateway[2] = 0;
    LPCmodSettings->OSsettings.staticGateway[3] = 1;
    LPCmodSettings->OSsettings.staticMask[0] = 255;
    LPCmodSettings->OSsettings.staticMask[1] = 255;
    LPCmodSettings->OSsettings.staticMask[2] = 255;
    LPCmodSettings->OSsettings.staticMask[3] = 0;
    LPCmodSettings->OSsettings.staticDNS1[0] = 192;
    LPCmodSettings->OSsettings.staticDNS1[1] = 168;
    LPCmodSettings->OSsettings.staticDNS1[2] = 0;
    LPCmodSettings->OSsettings.staticDNS1[3] = 1;
    LPCmodSettings->OSsettings.staticDNS2[0] = 192;
    LPCmodSettings->OSsettings.staticDNS2[1] = 168;
    LPCmodSettings->OSsettings.staticDNS2[2] = 0;
    LPCmodSettings->OSsettings.staticDNS2[3] = 1;


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
        case BNKTSOPSPLIT0:
        case BNKFULLTSOP:
            if(LPCmodSettings.OSsettings.biosName2[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosName2);
            }
            else{
                if(LPCmodSettings.OSsettings.TSOPcontrol & 0x02)
                    sprintf(string, "%s", "OnBoard Bank0");
                else
                    sprintf(string, "%s", "OnBoard BIOS");
            }
            break;
        case BNKTSOPSPLIT1:
            if(LPCmodSettings.OSsettings.biosName3[0] != 0){
                sprintf(string, "%s", LPCmodSettings.OSsettings.biosName3);
            }
            else{
                sprintf(string, "%s", "OnBoard Bank1");
            }
            break;
         default:
         	sprintf(string, "%s", "Settings");
         	break;
    }


}
