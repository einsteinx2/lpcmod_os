/*
 * system.c
 *
 *  Created on: Jan 30, 2017
 *      Author: cromwelldev
 */

#include "FlashUi.h"
#include "FlashDriver.h"
#include "lib/time/timeManagement.h"
#include "lib/LPCMod/LCDRingBuffer.h"
#include "WebServerOps.h"
#include "lib/LPCMod/BootLCD.h"
#include "CallbackTimer.h"

unsigned char cromwellLoop(void)
{
    if(currentNetworkState != NetworkState_Idle)
    {
        run_lwip();
    }

    if(xLCD.enable)
    {
        updateLCDRingBuffer();
    }

    callbackTimer_execute();

    //debugSPIPrint loop
    //LCD loop

    return 1;
}
