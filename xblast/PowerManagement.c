/*
 * PowerManagement.c
 *
 *  Created on: Dec 11, 2016
 *      Author: cromwelldev
 */

#include "PowerManagement.h"
#include "FlashUi.h"
#include "i2c.h"
#include "lpcmod_v1.h"
#include "boot.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "BootFATX.h"
#include "xblast/scriptEngine/xblastScriptEngine.h"
#include "xblast/HardwareIdentifier.h"

void assertBankScriptExecBankBoot(void * data)
{
    FATXFILEINFO fileinfo;
    int bank = *(unsigned char *)data;

    if(LPCmodSettings.OSsettings.runBankScript)
    {
        extern bool loadScriptFromHDD(char * filename, FATXFILEINFO *fileinfo);
        if(loadScriptFromHDD("\\XBlast\\scripts\\bank.script", &fileinfo))
        {
            runScript(fileinfo.buffer, fileinfo.fileSize, 1, (int*)&bank);
            free(fileinfo.buffer);
        }
    }

    BootModBios(bank);
}

void assertBankScriptExecTSOPBoot(void * data)
{
    FATXFILEINFO fileinfo;
    int bank = *(unsigned char *)data;

    if(LPCmodSettings.OSsettings.runBankScript)
    {
        extern bool loadScriptFromHDD(char * filename, FATXFILEINFO *fileinfo);
        if(loadScriptFromHDD("\\XBlast\\scripts\\bank.script", &fileinfo))
        {
            runScript(fileinfo.buffer, fileinfo.fileSize, 1, (int*)&bank);
            free(fileinfo.buffer);
        }
    }

    BootOriginalBios(bank);
}

// Booting Original Bios
void BootOriginalBios(FlashBank bank)
{
    if(SaveXBlastOSSettings())
    {
        assertWriteEEPROM();
        BootStopUSB();

        if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT || fHasHardware == SYSCON_ID_V1_TSOP || fHasHardware == SYSCON_ID_XT_TSOP)
        {
                //WriteToIO(XODUS_CONTROL, RELEASED0);    //Release D0
            if(getMotherboardRevision() == XboxMotherboardRevision_1_6 || getMotherboardRevision() == XboxMotherboardRevision_UNKNOWN)
            {
                switchBootBank(KILL_MOD);    // switch to original bios. Mute modchip.
            }
            else
            {
                switchBootBank(bank);    // switch to original bios but modchip listen to LPC commands.
            }

            I2CTransmitWord(0x10, 0x1b00 + ( I2CTransmitByteGetReturn(0x10, 0x1b) & 0xfb )); // clear noani-bit
        }
        else
        {
            I2CTransmitWord(0x10, 0x1b00 + ( I2CTransmitByteGetReturn(0x10, 0x1b) | 0x04 )); // set noani-bit
        }

        I2CRebootQuick();
    }
}

// Booting bank Modbios
void BootModBios(FlashBank bank)
{
    if(SaveXBlastOSSettings())
    {
        assertWriteEEPROM();
        switchBootBank(bank);

        BootStopUSB();

        if(isXBE() == false || fHasHardware == SYSCON_ID_V1_TSOP || fHasHardware == SYSCON_ID_XT_TSOP)
        {
            I2CTransmitWord(0x10, 0x1b00 + ( I2CTransmitByteGetReturn(0x10, 0x1b) & 0xfb )); // clear noani-bit
        }
        else
        {
            I2CTransmitWord(0x10, 0x1b00 + ( I2CTransmitByteGetReturn(0x10, 0x1b) | 0x04 )); // set noani-bit
        }

        I2CRebootQuick();
    }
}

bool canPowerDown(void)
{
    FlashProgress progress = Flash_getProgress();
    return (progress.currentFlashOp == FlashOp_Idle);
}
