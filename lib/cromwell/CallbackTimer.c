/*
 * CallbackTimer.c
 *
 *  Created on: May 7, 2018
 *      Author: cromwelldev
 */

#include "CallbackTimer.h"
#include "lib/time/timeManagement.h"
#include "stdlib.h"
#include "lib/LPCMod/xblastDebug.h"
#include <stddef.h>

typedef struct TimerInstance_t
{
    int id;
    unsigned int interval_ms;
    unsigned int lastExec_ms;
    callbackTimerHandler handler;
    struct TimerInstance_t* nextTimer;
}TimerInstance_t;

static TimerInstance_t* firstInstance;
static unsigned char initDone = 0;

void callbackTimer_init()
{
    firstInstance = NULL;
    initDone = 1;
}

void callbackTimer_execute()
{
    TimerInstance_t* currentTimer = firstInstance;

    while(initDone && NULL != currentTimer)
    {
        if(getElapsedTimeSince(currentTimer->lastExec_ms) >= currentTimer->interval_ms)
        {
            XBlastLogger(DEBUG_CALLBACKTIMER, DBG_LVL_DEBUG | DBG_FLG_SPI, "execute handler id:%u", currentTimer->id);
            (*currentTimer->handler)();
            currentTimer->lastExec_ms = getMS();
        }

        currentTimer = currentTimer->nextTimer;
    }
}

int newCallbackTimer(callbackTimerHandler handler, int interval_ms)
{
    TimerInstance_t* lastTimer = firstInstance;
    TimerInstance_t* newTimer;
    int timerId = 0;

    if(0 == initDone)
    {
        return -1;
    }

    newTimer = calloc(1, sizeof(TimerInstance_t));
    if(NULL == newTimer)
    {
        return -1;
    }

    if(NULL != lastTimer)
    {
        while(NULL != lastTimer->nextTimer)
        {
            lastTimer = lastTimer->nextTimer;
        }
        timerId = lastTimer->id + 1;
        lastTimer->nextTimer = newTimer;
    }
    else
    {
        firstInstance = newTimer;
    }
    XBlastLogger(DEBUG_CALLBACKTIMER, DBG_LVL_INFO, "Adding callback timer id:%u", timerId);

    newTimer->handler = handler;
    newTimer->id = timerId;
    newTimer->interval_ms = interval_ms;
    newTimer->lastExec_ms = getMS();

    return 0;
}

void stopCallbackTimer(int id)
{
    TimerInstance_t* targetTimer = firstInstance;
    TimerInstance_t* previousTimer = firstInstance;

    while(initDone && NULL != targetTimer)
    {
        if(targetTimer->id == id)
        {
            previousTimer->nextTimer = targetTimer->nextTimer;
            XBlastLogger(DEBUG_CALLBACKTIMER, DBG_LVL_INFO, "Removing callback timer id:%u", targetTimer->id);
            free(targetTimer);
            break;
        }

        previousTimer = targetTimer;
        targetTimer = targetTimer->nextTimer;
    }
}
