/*
 * system.c
 *
 *  Created on: Jan 30, 2017
 *      Author: cromwelldev
 */

#include "FlashDriver.h"
#include "lib/time/timeManagement.h"
#include "WebServerOps.h"

unsigned char cromwellLoop(void)
{
    Flash_executeFlashFSM();
    updateTime();
    run_lwip();

    //debugSPIPrint loop
    //LCD loop

    return 1;
}
