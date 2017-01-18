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

void SlowReboot(void *ignored)
{
    if(SaveXBlastOSSettings())
    {
        assertWriteEEPROM();
        BootStopUSB();
        I2CRebootSlow();
    }
}

void QuickReboot(void *ignored)
{
    if(SaveXBlastOSSettings())
    {
        assertWriteEEPROM();
        BootStopUSB();
        I2CRebootQuick();
    }
}

void PowerOff(void *ignored)
{
    if(SaveXBlastOSSettings())
    {
        assertWriteEEPROM();
        I2CPowerOff();
    }
}
