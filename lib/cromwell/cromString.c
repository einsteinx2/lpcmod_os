/*
 * cromString.c
 *
 *  Created on: Aug 8, 2016
 *      Author: cromwelldev
 */

#include "stdio.h"
#include "video.h"
#include <stddef.h>

int printk(const char *szFormat, ...) {  // printk displays to video
    char szBuffer[512*2];
    unsigned int wLength=0;
    va_list argList;
    va_start(argList, szFormat);
    wLength=(unsigned int) vsprintf(szBuffer, szFormat, argList);

    va_end(argList);

    szBuffer[sizeof(szBuffer)-1]=0;
        if (wLength>(sizeof(szBuffer)-1)) wLength = sizeof(szBuffer)-1;
    szBuffer[wLength]='\0';

    BootVideoChunkedPrint(szBuffer);
    return wLength;
}
