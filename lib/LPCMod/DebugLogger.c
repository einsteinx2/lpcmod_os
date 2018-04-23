/*
 * DebuggerLogger.c
 *
 *  Created on: Apr 22, 2018
 *      Author: cromwelldev
 */

#include "DebugLogger.h"
#include "FatFSAccessor.h"

#define MaxBuffSize 1024

static const char* const ActiveLogFileLocation = "MASTER_Z:"PathSep"xblast.log";

void debugLoggerInit(void)
{

}

void logRotate(void)
{

}

void printTextLogger(const char* functionName, char* buffer, ...)
{
    va_list vl;
    char tempBuf[MaxBuffSize];

    FILEX handle = fatxopen(ActiveLogFileLocation, FileOpenMode_OpenAppend);

    if(handle)
    {
        va_start(vl,buffer);

        va_end(vl);
        fatxputs("\r\n", handle);

        fatxclose(handle);
    }
}
