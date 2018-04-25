/*
 * HardwareIdentifier.c
 *
 *  Created on: Jan 3, 2017
 *      Author: cromwelldev
 */

#include "HardwareIdentifier.h"
#include "boot.h"
#include "lpcmod_v1.h"
#include "FlashDriver.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "encoder.h"
#include "i2c.h"
#include "cpu.h"

//Global to hide code when running in XBE without modchip detected.
//TODO: make static to hide
unsigned short fHasHardware;
unsigned char fSpecialEdition;
static unsigned short cpuSpeed;
static XboxMotherboardRevision mbVersion;
static const char *xbox_mb_rev[8] =
{
    "DevKit",
    "DebugKit",
    "1.0",
    "1.1",
    "1.2/1.3",
    "1.4/1.5",
    "1.6/1.6b",
    "Unknown"
};

void identifyModchipHardware(void)
{
    const OBJECT_FLASH* bootFlash = NULL;
    fSpecialEdition = 0;

    fHasHardware = LPCMod_HW_rev();         //Will output 0xff if no supported modchip detected.

    if(fHasHardware == 0xff)
    {
        fHasHardware = SYSCON_ID_UNKNOWN;
    }
    XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "Modchip hardware ID is: 0x%04X", fHasHardware);

    //Check which flash chip is detected by system.
    Flash_ReadDeviceInfo(&bootFlash);

    switch(fHasHardware)
    {
    case SYSCON_ID_V1:
        XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "%s detected on LPC bus.", getModchipName());
        goto nextStepXBLASTV1;
    case SYSCON_ID_V1_PRE_EDITION:
        fSpecialEdition = SYSCON_ID_V1_PRE_EDITION;
        XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "%s detected on LPC bus.", getModchipName());
        //Since Pre-Edition is the same device functionality-wise, we just force fHasHardware back to plain V1 to offer same features in OS.
        fHasHardware = SYSCON_ID_V1;
nextStepXBLASTV1:

        if(bootFlash->flashType.m_bManufacturerId == 0xbf && bootFlash->flashType.m_bDeviceId == 0x5b)
        {     //If we detected a SST49LF080A
            XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "%s flash chip detected. We booted from LPC indeed.", getModchipName());
            //Make sure we'll be reading from OS Bank
            switchOSBank(FlashBank_OSBank);
        }
        else
        {  //SST49LF080A flash chip was NOT detected.
            XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "%s flash chip NOT detected. Assuming we booted from TSOP", getModchipName());
            fHasHardware = SYSCON_ID_V1_TSOP;
            WriteToIO(XODUS_CONTROL, RELEASED0); //Make sure D0/A15 is not grounded.
        }

        LPCMod_ReadIO(NULL);
        XBlastLogger(DEBUG_HW_ID, DBG_LVL_DEBUG, "Read XBlast Lite V1 IO status.");
        break;
    case SYSCON_ID_XT:
        XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "%s detected on LPC bus.", getModchipName());

       if(bootFlash->flashType.m_bManufacturerId == 0xbf && bootFlash->flashType.m_bDeviceId == 0x5b)
       {     //If we detected a SST49LF080A
           XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "%s flash chip detected. We booted from LPC indeed.", getModchipName());
           //Make sure we'll be reading from OS Bank
           switchOSBank(FlashBank_OSBank);
       }
       else
       {  //SST49LF080A flash chip was NOT detected.
           XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "%s flash chip NOT detected. Assuming we booted from TSOP", getModchipName());
           fHasHardware = SYSCON_ID_XT_TSOP;
           WriteToIO(XODUS_CONTROL, RELEASED0); //Make sure D0/A15 is not grounded.
       }
       break;
    default:
        XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "No XBlast OS compatible hardware found.");
        unsigned int x3probe = I2CTransmitByteGetReturn(0x51, 0x0);  //Xecuter 3 will send out 0xff
        XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "Probing for X3 EEprom. Result: 0x%08X", x3probe);
        if(x3probe != 0xff && x3probe != ERR_I2C_ERROR_BUS && x3probe != ERR_I2C_ERROR_TIMEOUT) //Another (hacky) way to detect is to probe SMBus at addresses
        {
            fHasHardware = SYSCON_ID_X3;                    //normally unused by the Xbox. By my own experimentation, address
            XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "Assuming %s chip detected.", getModchipName());  //0x51 isn't used when X3 is NOT plugged. Then probing the SMBus
        }                                                   //offset 0 of address 0x51 will return either 0xff or 0x80000002.
                                                            //Any other value will be assumed coming from the (encrypted?)
                                                            //X3 eeprom and thus instructing the program that a X3 is detected.
                                                            //More tests will be needed to verify and confirm this theory.
                                                            //Tests have been done on NTSC-U 1.0 and 1.6(a) Xboxes so far.
        else
        {
            fHasHardware = 0;               //Unknown device, set to 0 to indicate no known hardware.
        }

        currentFlashBank = FlashBank_OSBank;//Make sure the system knows we're on the right bank.
        TSOPRecoveryMode = 0;               //Whatever happens, it's not possible to recover TSOP on other modchips.
        break;
    }
}

void identifyXboxHardware(void)
{
    // We look how much memory we have ..
    BootDetectMemorySize();
    XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "Detected RAM size : %uMB.", xbox_ram);

    cpuSpeed = getCPUFreq();

    mbVersion = I2CGetXboxMBRev();
    XBlastLogger(DEBUG_HW_ID, DBG_LVL_INFO, "Xbox motherboad rev: %s.", xbox_mb_rev[mbVersion]);
}

bool isXBlastOnTSOP(void)
{
    return fHasHardware == SYSCON_ID_V1_TSOP ||
           fHasHardware == SYSCON_ID_XT_TSOP;
}

bool isXBlastOnLPC(void)
{
    return fHasHardware == SYSCON_ID_V1 ||
           fHasHardware == SYSCON_ID_XT;
}

bool isXBlastCompatible(void)
{
    return isXBlastOnLPC() ||
           isXBlastOnTSOP();
}

bool isPureXBlast(void)
{
    return fHasHardware == SYSCON_ID_V1 ||
           fHasHardware == SYSCON_ID_V1_TSOP;
}

bool isLCDSupported(void)
{
    return isPureXBlast() ||
           fHasHardware == SYSCON_ID_XX1 ||
           fHasHardware == SYSCON_ID_XX2 ||
           fHasHardware == SYSCON_ID_XXOPX ||
           fHasHardware == SYSCON_ID_XX3 ||
           fHasHardware == SYSCON_ID_X3;
}

bool isXecuter3(void)
{
     return (fHasHardware == SYSCON_ID_X3);
}

bool isXBE(void)
{
    return (cromwell_config == XROMWELL);
}

bool isLCDContrastSupport(void)
{
    return isLCDSupported() && isXecuter3() == false && fHasHardware != SYSCON_ID_XXOPX;
}

bool isFrostySupport(void)
{
    return (video_encoder == ENCODER_CONEXANT);
}

const char * getModchipName(void)
{
    switch(fHasHardware)
    {
    case SYSCON_ID_V1_TSOP:
    case SYSCON_ID_V1:
        if(fSpecialEdition == SYSCON_ID_V1_PRE_EDITION)
        {
            return "XBlast Lite V1 Pre-Edition";
        }
        return "XBlast Lite V1";
    case SYSCON_ID_XT_TSOP:
    case SYSCON_ID_XT:
        return "Aladdin XBlast";
    case SYSCON_ID_XX1:
    case SYSCON_ID_XX2:
        return "SmartXX V1/V2";
    case SYSCON_ID_XXOPX:
        return "SmartXX LT OPX";
    case SYSCON_ID_XX3:
        return "SmartXX V3";
    case SYSCON_ID_X3:
        return "Xecuter 3";
    }
    return "Unsupported modchip";
}

unsigned short getCPUSPeedInMHz(void)
{
    return cpuSpeed;
}

XboxMotherboardRevision getMotherboardRevision(void)
{
    return mbVersion;
}

const char* getMotherboardRevisionString(void)
{
    return xbox_mb_rev[getMotherboardRevision()];
}

bool isTSOPSplitCapable(void)
{
#if defined(DEV_FEATURES) || defined(CUSTOM_TSOP)
    return isPureXBlast();
#else
    return isPureXBlast() && (mbVersion == XboxMotherboardRevision_1_1 ||
                              mbVersion == XboxMotherboardRevision_1_0);
#endif
}
