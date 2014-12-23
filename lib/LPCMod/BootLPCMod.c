#include "boot.h"
#include "VideoInitialization.h"
#include "BootLPCMod.h"
#include "lpcmod_v1.h"
#include "LEDMenuActions.h"
#include "BootFATX.h"

char *xblastcfgstrings[NBTXTPARAMS] = {
        //Contains either numerical or boolean values.
	"quickboot=",
	"fanspeed=",
	"boottimeout=",
	"tsopcontrol=",
	"enablenetwork=",
	"usedhcp=",
	"enable5v=",
	"nblines=",
	"linelength=",
	"backlight=",
	"contrast=",
	"displaybootmsg=",
	"customtextboot=",
	"displaybiosnameboot=",

	//Contains IP text strings.
        "staticip=",
        "staticgateway=",
        "staticmask=",
        "staticdns1=",
        "staticdns2=",

        //Contains text strings.
        "512kbname=",
        "256kbname=",
        "tsop0name=",
        "tsop1name=",
	"customstring0=",
	"customstring1=",
	"customstring2=",
	"customstring3=",

	//Special case.
        "activebank=",
        "altbank=",
	"ledcolor=",
        "lcdtype="
};

unsigned char *settingsPtrArray[IPTEXTPARAMGROUP] = {
        &(LPCmodSettings.OSsettings.Quickboot),
        &(LPCmodSettings.OSsettings.fanSpeed),
        &(LPCmodSettings.OSsettings.bootTimeout),
        &(LPCmodSettings.OSsettings.TSOPcontrol),
        &(LPCmodSettings.OSsettings.enableNetwork),
        &(LPCmodSettings.OSsettings.useDHCP),
        &(LPCmodSettings.LCDsettings.enable5V),
        &(LPCmodSettings.LCDsettings.nbLines),
        &(LPCmodSettings.LCDsettings.lineLength),
        &(LPCmodSettings.LCDsettings.backlight),
        &(LPCmodSettings.LCDsettings.contrast),
        &(LPCmodSettings.LCDsettings.displayMsgBoot),
        &(LPCmodSettings.LCDsettings.customTextBoot),
        &(LPCmodSettings.LCDsettings.displayBIOSNameBoot)
};

unsigned char *IPsettingsPtrArray[TEXTPARAMGROUP-IPTEXTPARAMGROUP] = {
        LPCmodSettings.OSsettings.staticIP,
        LPCmodSettings.OSsettings.staticGateway,
        LPCmodSettings.OSsettings.staticMask,
        LPCmodSettings.OSsettings.staticDNS1,
        LPCmodSettings.OSsettings.staticDNS2
};

char *textSettingsPtrArray[SPECIALPARAMGROUP - TEXTPARAMGROUP] = {
        LPCmodSettings.OSsettings.biosName0,
        LPCmodSettings.OSsettings.biosName1,
        LPCmodSettings.OSsettings.biosName2,
        LPCmodSettings.OSsettings.biosName3,
        LPCmodSettings.LCDsettings.customString0,
        LPCmodSettings.LCDsettings.customString1,
        LPCmodSettings.LCDsettings.customString2,
        LPCmodSettings.LCDsettings.customString3
};

unsigned char *specialCasePtrArray[4] = {
        &(LPCmodSettings.OSsettings.activeBank),
        &(LPCmodSettings.OSsettings.altBank),
        &(LPCmodSettings.OSsettings.LEDColor),
        &(LPCmodSettings.LCDsettings.lcdType)
};


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

void LPCMod_ReadIO(struct _GenPurposeIOs *GPIOstruct){
    struct _GenPurposeIOs *localGPIOstruct;
    u8 temp = ReadFromIO(XBLAST_IO);
    //If no valid pointer is specified, take Global struct.
    if(GPIOstruct == NULL)
        localGPIOstruct = &GenPurposeIOs;
    else
        localGPIOstruct = GPIOstruct;

    localGPIOstruct->GPO3 = (temp & 0x80) >> 7;
    localGPIOstruct->GPO2 = (temp & 0x40) >> 6;
    localGPIOstruct->GPO1 = (temp & 0x20) >> 5;
    localGPIOstruct->GPO0 = (temp & 0x10) >> 4;
    localGPIOstruct->GPI1 = (temp & 0x08) >> 3;
    localGPIOstruct->GPI0 = (temp & 0x04) >> 2;
    localGPIOstruct->A19BufEn = (temp & 0x02) >> 1;
    localGPIOstruct->EN_5V = (temp & 0x01);
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
                if(LPCmodSettings.OSsettings.TSOPcontrol)
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

int LPCMod_ReadCFGFromHDD(_LPCmodSettings *LPCmodSettings){
    FATXFILEINFO fileinfo;
    FATXPartition *partition;
    int res = false;
    int dcluster;
    const char *cfgFileName = "\\XBlast\\xblast.cfg";
    const char *path="\\XBlast\\";
    char compareBuf[100];                     //100 character long seems acceptable
    u8 i;
    bool settingLoaded[NBTXTPARAMS];
    int stringStartPtr = 0, stringStopPtr = 0, valueStartPtr = 0;
    bool CRdetected;
    u8 textStringCopyLength;


    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);
    if(partition != NULL){
        dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "XBlast");
        if((dcluster != -1) && (dcluster != 1)) {
            res = FATXFindFile(partition, (char *)cfgFileName, FATX_ROOT_FAT_CLUSTER, &fileinfo);
        }
        if(res){
            
            //Initially, no setting has been loaded from txt.
            for(i = 0; i < NBTXTPARAMS; i++)
            {
                settingLoaded[i] = false;
            }
            //partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);
            if(LoadFATXFile(partition, (char *)cfgFileName, &fileinfo)){
                while(stringStopPtr < fileinfo.fileSize){      //We stay in file
                    while(fileinfo.buffer[stringStopPtr] != '\n' && stringStopPtr < fileinfo.fileSize){        //While we don't hit a new line and still in file
                        stringStopPtr++;        //Move on to next character in file.
                    }
                    if(fileinfo.buffer[stringStartPtr] == '#' ){       //This is a comment or empty line
                    
                        stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
                        continue;
                    }
                    
                    //stringStartPtr is now a beginning of the line and stringStopPtr is at the end of it.
                    valueStartPtr = 0;
                    CRdetected = fileinfo.buffer[stringStopPtr - 1] == '\r' ? 1 : 0;    //Dos formatted line?
                    //Copy line in compareBuf.
                    strncpy(compareBuf, &fileinfo.buffer[stringStartPtr], stringStopPtr - CRdetected - stringStartPtr);
                    //Manually append terminating character at the end of the string
                    compareBuf[stringStopPtr - CRdetected - stringStartPtr] = '\0';
                    //if(compareBuf[0] != '\0')
                    //printk("\n       %s", compareBuf); //debug, print the whole file line by line.
                    while(compareBuf[valueStartPtr] != '=' && valueStartPtr < 100 && compareBuf[valueStartPtr] != '\n' && compareBuf[valueStartPtr] != '\0'){     //Search for '=' character.
                        valueStartPtr++;
                    }
                    
                    stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
                    if(valueStartPtr < MINPARAMLENGTH || valueStartPtr >= 30){    //If line is not properly constructed
                        continue;
                    }
                    else{
                        for(i = 0; i < NBTXTPARAMS; i++)
                        {
                            if(!strncmp(compareBuf, xblastcfgstrings[i], valueStartPtr) && !settingLoaded[i]){   //Match
                                settingLoaded[i] = true;        //Recurring entries not allowed.
                                valueStartPtr++;
                                if(i < IPTEXTPARAMGROUP){       //Numerical value parse
                                    if(compareBuf[valueStartPtr] >='0' && compareBuf[valueStartPtr] <='9'){
                                        *settingsPtrArray[i] = (u8)atoi(&compareBuf[valueStartPtr]);
                                    }
                                    else if(!strncmp(&compareBuf[valueStartPtr], "Y", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "y", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "T", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "t", 1)){
                                        *settingsPtrArray[i] = 1;
                                    }
                                    else if(!strncmp(&compareBuf[valueStartPtr], "N", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "n", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "F", 1) ||
                                            !strncmp(&compareBuf[valueStartPtr], "f", 1)){
                                        *settingsPtrArray[i] = 0;
                                    }
                                }
                                else if(i < TEXTPARAMGROUP){       //IP string value parse
                                    assertCorrectIPString(IPsettingsPtrArray[i - IPTEXTPARAMGROUP], &compareBuf[valueStartPtr]);
                                }
                                else if(i < SPECIALPARAMGROUP){    //Text value parse
                                    if((stringStopPtr - valueStartPtr) >= 20)
                                        textStringCopyLength = 20;
                                    else
                                        textStringCopyLength = stringStopPtr - valueStartPtr;
                                    strncpy(textSettingsPtrArray[i - TEXTPARAMGROUP], &compareBuf[valueStartPtr], textStringCopyLength);
                                    textSettingsPtrArray[i - TEXTPARAMGROUP][20] = '\0';
                                }
                                else{
                                    switch(i){
                                        case 27:
                                        case 28:
                                            if(!strcmp(&compareBuf[valueStartPtr], "BNK512"))
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = BNK512;
                                            else if(!strcmp(&compareBuf[valueStartPtr], "BNK256"))
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = BNK256;
                                            else if(!strcmp(&compareBuf[valueStartPtr], "BNKTSOPSPLIT0"))
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKTSOPSPLIT0;
                                            else if(!strcmp(&compareBuf[valueStartPtr], "BNKTSOPSPLIT1"))
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKTSOPSPLIT1;
                                            else if(!strcmp(&compareBuf[valueStartPtr], "BNKFULLTSOP"))
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = BNKFULLTSOP;
                                            break;
                                        case 29:
                                            if(!strncmp(&compareBuf[valueStartPtr], "Of", 2) || !strncmp(&compareBuf[valueStartPtr], "of", 2))    //LED_OFF
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_OFF;
                                            else if(!strncmp(&compareBuf[valueStartPtr], "G", 1) || !strncmp(&compareBuf[valueStartPtr], "g", 1))    //LED_GREEN
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_GREEN;
                                            if(!strncmp(&compareBuf[valueStartPtr], "R", 1) || !strncmp(&compareBuf[valueStartPtr], "r", 1))    //LED_RED
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_RED;
                                            if(!strncmp(&compareBuf[valueStartPtr], "Or", 2) || !strncmp(&compareBuf[valueStartPtr], "or", 2))    //LED_ORANGE
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_ORANGE;
                                            if(!strncmp(&compareBuf[valueStartPtr], "C", 1) || !strncmp(&compareBuf[valueStartPtr], "c", 1))    //LED_CYCLE
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = LED_CYCLE;
                                            break;
                                        case 30:
                                            if(!strcmp(&compareBuf[valueStartPtr], "HD44780"))
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = HD44780 ;
                                            else if(!strcmp(&compareBuf[valueStartPtr], "KS0073"))
                                                *specialCasePtrArray[i - SPECIALPARAMGROUP] = KS0073 ;
                                            break;
                                    } //switch(i)
                                } //!if(i < IPTEXTPARAMGROUP){
                            } //if(!strncmp(compareBuf, xblastcfgstrings[i], valueStartPtr) && !settingLoaded[i])
                        } //for(i = 0; i < NBTXTPARAMS; i++)
                    } //!if(valueStartPtr >= 30)
                } //while(stringStopPtr < fileinfo.fileSize)
                free(fileinfo.buffer);
            } //if(LoadFATXFilefixed (partition, (char *)cfgFileName, &fileinfo, fileBuf))
            else{
                return 4; //Cannot open file.
            }
        } //if(res)
        else{
            return 3; //Cannot file file in cluster chain map.
        }
        CloseFATXPartition(partition);
        //while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);  //Debug to see string prints
    }
    else{
        return 2; //Cannot open partition.
    }
    return 0;
}

int LPCMod_SaveCFGToHDD(void){
FATXFILEINFO fileinfo;
    FATXPartition *partition;
    int res = false;
    char * filebuf;
    char tempString[22];
    u32 cursorpos, totalbytes = 0;
    int dcluster, i;
    const char *path="\\XBlast\\";
    const char *cfgFileName = "\\XBlast\\xblast.cfg";


    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);

    if(partition != NULL){
        dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "XBlast");
        if((dcluster != -1) && (dcluster != 1)) {
            res = FATXFindFile(partition, (char *)cfgFileName, dcluster, &fileinfo);
        }
        if(res){                //File already exist
            if(ConfirmDialog("              Overwrite C:\\XBlast\\xblast.cfg?", 1)){
                CloseFATXPartition(partition);
                ToolHeader("Saving to C:\\XBlast\\xblast.cfg aborted.");
                cromwellWarning();
                ToolFooter();
                initialSetLED(LPCmodSettings.OSsettings.LEDColor);
                return 1;
            }
            filebuf = (char *)malloc(FATX16CLUSTERSIZE);
            memset(filebuf, 0x00, FATX16CLUSTERSIZE);
            for(i = 0; i < NBTXTPARAMS; i++){
                cursorpos = 0;
                strcpy(&filebuf[cursorpos], xblastcfgstrings[i]);
                cursorpos += strlen(xblastcfgstrings[i]);
                //New line inserted in.
                cursorpos += strlen(tempString);
                strncpy(&filebuf[cursorpos],tempString, strlen(tempString));    //Skip terminating character.
                //filebuf[cursorpos] = 0x0A;
                //cursorpos += 1;
                totalbytes += cursorpos;
            }
            filebuf[cursorpos] = 0;     //Terminating character at the end of file.


            free(filebuf);
        }
        ToolHeader("Saved settings to C:\\XBlast\\xblast.cfg");

        CloseFATXPartition(partition);
    }
    else{
        ToolHeader("Error opening partition. Drive formatted?");
    }

    ToolFooter();
    return 0;
}
