/*
 * CallbackTimer.c
 *
 *  Created on: May 7, 2018
 *      Author: cromwelldev
 */

#include "CallbackTimer.h"
#include "lib/time/timeManagement.h"
#include "stdlib.h"
#include <stddef.h>

typedef struct TimerInstance_t
{
    int id;
    unsigned int interval_us;
    unsigned int lastExec_us;
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
        if(getElapseMicroSecondsSince(currentTimer->lastExec_us) >= currentTimer->interval_us)
        {
            (*currentTimer->handler)();
            currentTimer->lastExec_us = getUS();
        }

        currentTimer = currentTimer->nextTimer;
    }
}

int newCallbackTimer(callbackTimerHandler handler, int interval_us)
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

    newTimer->handler = handler;
    newTimer->id = timerId;
    newTimer->interval_us = interval_us;
    newTimer->lastExec_us = getUS();

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
            free(targetTimer);
            break;
        }

        previousTimer = targetTimer;
        targetTimer = targetTimer->nextTimer;
    }
}
