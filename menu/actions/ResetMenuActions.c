/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ResetMenuActions.h"
#include "BootEEPROM.h"
#include "FlashUi.h"
#include "boot.h"
#include "i2c.h"
#include "lib/LPCMod/DebugLogger.h"

void SlowReboot(void* ignored)
{
    if(SaveXBlastOSSettings())
    {
        assertWriteEEPROM();
        BootStopUSB();
        forceFlushLog();
        I2CRebootSlow();
    }
}

void QuickReboot(void* ignored)
{
    if(SaveXBlastOSSettings())
    {
        assertWriteEEPROM();
        BootStopUSB();
        forceFlushLog();
        I2CRebootQuick();
    }
}

void PowerOff(void* ignored)
{
    if(SaveXBlastOSSettings())
    {
        assertWriteEEPROM();
        forceFlushLog();
        I2CPowerOff();
    }
}

void SlowRebootNoSave(void* ignored)
{
    BootStopUSB();
    forceFlushLog();
    I2CRebootSlow();
}

void QuickRebootNoSave(void* ignored)
{
    BootStopUSB();
    forceFlushLog();
    I2CRebootQuick();
}

void PowerOffNoSave(void* ignored)
{
    forceFlushLog();
    I2CPowerOff();
}
