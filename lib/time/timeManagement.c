/*
 * timeManagement.c
 *
 *  Created on: Aug 7, 2016
 *      Author: bennyboy
 */

#include "timeManagement.h"
#include "boot.h"

static unsigned int currentHeldTime_ms = 0;

static inline unsigned int getPITCount(void)
{
	return BIOS_TICK_COUNT;
}

static inline unsigned int getAPICCount(void)
{
	return IoInputDword(0x8008);
}

void wait_us(unsigned int ticks) {
/*
	  32 Bit range = 1200 sec ! => 20 min
	1. sec = 0x369E99
	1 ms =  3579,545

*/
    unsigned int COUNT_start;
    unsigned int temp;
    unsigned int COUNT_TO;
    unsigned int HH;

    // Maximum Input range
    if (ticks>(1200*1000))
	{
    	ticks = 1200*1000;
	}

    COUNT_TO = (unsigned int) ((float)(ticks*3.579545));
    COUNT_start = getAPICCount();

    while(1) {
        HH = getAPICCount();
        temp = HH-COUNT_start;

        // We reached the counter
        if (temp>COUNT_TO)
		{
        	break;
		}
    };


}

void wait_ms(unsigned int ticks) {
/*
	  32 Bit range = 1200 sec ! => 20 min
	1. sec = 0x369E99
	1 ms =  3579,545

*/
    unsigned int COUNT_start;
    unsigned int temp;
    unsigned int COUNT_TO;
    unsigned int HH;

    // Maximum Input range
    if (ticks>(1200*1000))
	{
    	ticks = 1200*1000;
	}

    COUNT_TO = (unsigned int) ((float)(ticks*3579.545));
    COUNT_start = getAPICCount();

    while(1)
    {
        HH = getAPICCount();
        temp = HH-COUNT_start;

        // We reached the counter
        if (temp>COUNT_TO)
		{
        	break;
		}
    };
}

void updateTime(void)
{
	currentHeldTime_ms = getPITCount();
}

unsigned int getMS(void)
{
	return currentHeldTime_ms;
}

unsigned int getElapsedTimeSince(unsigned int startValue_ms)
{
	return currentHeldTime_ms - startValue_ms;
}

//To accomodate lwip
unsigned int sys_now(void)
{
	return (unsigned int)(getAPICCount() / 3579.545);
}

unsigned int getRandSeed(void)
{
    return getAPICCount();
}
