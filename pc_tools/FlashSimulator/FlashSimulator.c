/*
 * FlashSimulator.c
 *
 *  Created on: Dec 7, 2016
 *      Author: cromwelldev
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "FlashLowLevel.h"
#include "FlashDriver.h"

#define FlashSize 1024 * 1024

void populateSettingsStructWithDefault(_LPCmodSettings *LPCmodSettings);

int main(int argc, const char* argv[])
{
    unsigned char controlBuffer[FlashSize];
    unsigned int controlBufUsedSize = FlashSize;
    unsigned char readBackBuffer[FlashSize];
    const OBJECT_FLASH * objectFlash = NULL;
    unsigned int counter = 0;
    _LPCmodSettings readBackSettings;

    //TODO: getopt() for parameters.
    char inputfilename[] = "\"";
    int ar;
    bool hasInputFile = false;

    while ((ar = getopt(argc, argv, "hi:")) != -1)
    {
        switch (ar)
        {
         case 'h':
            printf("something\n");
            break;
         case 'i':
            strcpy(inputfilename, optarg);
            printf("Filename :%s\n", inputfilename);
            hasInputFile = true;
            break;
        }
    }

    memset(&readBackSettings, 0xff, sizeof(_LPCmodSettings));
    populateSettingsStructWithDefault(&LPCmodSettings);

    Flash_Init();

    memset(readBackBuffer, 0x11, FlashSize);

    if(hasInputFile == false)
    {
        for(unsigned int i = 0; i < FlashSize; i++)
        {
            controlBuffer[i] = (unsigned char)i;
        }
    }
    else
    {
        memset(controlBuffer, 0xff, FlashSize);
        FILE * f = fopen(inputfilename, "rb");

        if(f == NULL)
        {
            printf("file open error...\n");
            printf("Entered path is : %s\n", inputfilename);
            return 1;
        }

        fseek(f, 0L, SEEK_END);
        controlBufUsedSize = ftell(f);
        fseek(f, 0L, SEEK_SET);

        fread(controlBuffer, sizeof(unsigned char), controlBufUsedSize, f);
        fclose(f);
    }

    FlashProgress flashProgress = Flash_getProgress();

    while(1)
    {
        Flash_executeFlashFSM();

        flashProgress = Flash_getProgress();

        //if(flashProgress.currentFlashOp == FlashOp_Idle)
        {
            switch(counter)
            {
            case 0:
            {
                FlashProgress temp = Flash_ReadDeviceInfo(&objectFlash);
                if(temp.flashErrorCode == FlashErrorcodes_NoError && flashProgress.currentFlashOp == FlashOp_Idle)
                {
                    flashProgress = Flash_SimpleBIOSBankFlash(controlBuffer, FlashSize / 4, 0);
                    if(flashProgress.currentFlashOp == FlashOp_PendingOp)
                    {
                        counter++;
                    }
                }
                break;
            }
            case 1:
                if(flashProgress.currentFlashOp == FlashOp_Completed || flashProgress.currentFlashOp == FlashOp_Error)
                {
                    counter++;
                    Flash_freeFlashFSM();
                }
                break;
            case 2:
                if(flashProgress.currentFlashOp == FlashOp_Idle)
                {
                    flashProgress = Flash_ReadBIOSBank(FlashBank_OSBank);
                    if(flashProgress.currentFlashOp == FlashOp_PendingOp)
                    {
                        counter++;
                    }
                }
                break;
            case 3:
                if(flashProgress.currentFlashOp == FlashOp_Completed || flashProgress.currentFlashOp == FlashOp_Error)
                {
                    counter++;
                    const unsigned char* biosBuf;
                    unsigned int biosSize = getBiosBuffer(&biosBuf);
                    unsigned int compare = memcmp(controlBuffer, biosBuf, biosSize);

                    if(compare)
                    {
                        printf("read back don't match!!\n");
                    }
                    Flash_freeFlashFSM();
                }
                break;
            case 4:
                if(flashProgress.currentFlashOp == FlashOp_Idle)
                {
                    flashProgress = Flash_SaveXBlastOSSettings();
                    if(flashProgress.currentFlashOp == FlashOp_PendingOp)
                    {
                        counter++;
                    }
                }
                break;
            case 5:
                if(flashProgress.currentFlashOp == FlashOp_Completed || flashProgress.currentFlashOp == FlashOp_Error)
                {
                    counter++;
                    Flash_freeFlashFSM();
                }
                break;
            case 6:
                if(flashProgress.currentFlashOp == FlashOp_Idle)
                {
                    flashProgress = Flash_ReadXBlastOSSettingsRequest();
                    if(flashProgress.currentFlashOp == FlashOp_PendingOp)
                    {
                        counter++;
                    }
                }
                break;
            case 7:
                if(flashProgress.currentFlashOp == FlashOp_Completed || flashProgress.currentFlashOp == FlashOp_Error)
                {
                    counter = 4;
                    Flash_LoadXBlastOSSettings(&readBackSettings);
                    if(memcmp(&readBackSettings, &LPCmodSettings, sizeof(_LPCmodSettings)) == 0)
                    {
                        printf("Settings readback identical\n");
                        LPCmodSettings.OSsettings.Quickboot++;
                    }
                    else
                    {
                        printf("ERROR!!! Settings readback mismatch!!!\n");
                    }
                    Flash_freeFlashFSM();
                }
                break;
            }
        }

        if(flashProgress.currentFlashOp == FlashOp_Completed)
        {

        }

        if(flashProgress.currentFlashOp == FlashOp_Error)
        {

        }
    }


    return 0;
}



#define HDD4780_DEFAULT_NBLINES    4
#define HDD4780_DEFAULT_LINELGTH    20
//Sets default values to most important settings.
void populateSettingsStructWithDefault(_LPCmodSettings *LPCmodSettings){
    unsigned char i;

    LPCmodSettings->settingsVersion = CurrentSettingsVersionNumber;
    LPCmodSettings->OSsettings.activeBank = 84;
    LPCmodSettings->OSsettings.altBank = 87;
    LPCmodSettings->OSsettings.Quickboot = 0;
    LPCmodSettings->OSsettings.selectedMenuItem = 84;
    LPCmodSettings->OSsettings.fanSpeed = 20;
    LPCmodSettings->OSsettings.bootTimeout = 16;
    LPCmodSettings->OSsettings.LEDColor = 1;    //Set for next boot
    LPCmodSettings->OSsettings.TSOPcontrol = 0;
    LPCmodSettings->OSsettings.TSOPhide = 0;
    LPCmodSettings->OSsettings.runBankScript = 0;
    LPCmodSettings->OSsettings.runBootScript = 0;
    LPCmodSettings->OSsettings.backgroundColorPreset = 1;
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
        LPCmodSettings->OSsettings.biosName512Bank[i] = 0;
        LPCmodSettings->OSsettings.biosName256Bank[i] = 0;
        LPCmodSettings->OSsettings.biosNameTSOPFullSplit0[i] = 0;
        LPCmodSettings->OSsettings.biosNameTSOPSplit1[i] = 0;

        LPCmodSettings->LCDsettings.customString0[i] = 0;
        LPCmodSettings->LCDsettings.customString1[i] = 0;
        LPCmodSettings->LCDsettings.customString2[i] = 0;
        LPCmodSettings->LCDsettings.customString3[i] = 0;
    }

    LPCmodSettings->flashScript.scriptSize = 0;
}
