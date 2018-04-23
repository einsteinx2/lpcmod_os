/*
 * DebuggerLogger.c
 *
 *  Created on: Apr 22, 2018
 *      Author: cromwelldev
 */

#include "DebugLogger.h"
#include "FatFSAccessor.h"

#define MaxBuffSize 1024

#define LogRotationDepth 5

#define logFilename "xblast.log"
static const char* const ActiveLogFileLocation = "MASTER_Z:" PathSep logFilename;

static unsigned char initDone = 0;
static FIL activeLogHandle = 0;

static void writeString(const char* string, unsigned char writeToLogFile);
static unsigned char checkDebugFlag(const char* szDebugFlag);

void debugLoggerInit(void)
{
    FRESULT result;
    logRotate();


    initDone = 1;
}

void logRotate(void)
{
    unsigned char i = LogRotationDepth;
    char fullPath[50];
    char newName[50];

    if(NULL != activeLogHandle.obj.fs)
    {
        f_close(&activeLogHandle);
    }

    sprintf(newName, "%s.%u", ActiveLogFileLocation, i);
    f_unlink(newName);

    while(i--)
    {
        sprintf(fullPath, "%s.%u", ActiveLogFileLocation, i);
        f_rename(fullPath, newName);
        strcpy(newName, fullPath);
    }

    f_rename(ActiveLogFileLocation, fullPath);
    result = f_open(&activeLogHandle, ActiveLogFileLocation, FA_OPEN_APPEND);
}

void printTextLogger(const char* debugFlag, const char* functionName, char* buffer, ...)
{
    va_list vl;
    char tempBuf[MaxBuffSize];
    unsigned char writeToLogfile;


    if(NULL != activeLogHandle.obj.fs)
    {
        writeToLogfile = checkDebugFlag(debugFlag)
        va_start(vl,buffer);
        sprintf(tempBuf, "[%s] ", functionName);
        writeString(tempBuf);
        vsprintf(tempBuf,buffer,vl);
        writeString(tempBuf);
        va_end(vl);

        writeString("\r\n");

        f_sync(&activeLogHandle);
    }
}

static void writeString(const char* string, unsigned char writeToLogFile)
{
    /* Write to log file*/
    if(writeToLogFile)
    {
        f_puts(string, &activeLogHandle);
    }

    /* write to SPI debugger, if applicable */
}

static unsigned char checkDebugFlag(const char* szDebugFlag)
{
#define STRINGIFY0(v) #v
    /* List of Debug categories that must not trigger Logfile writes */
    char* forbiddenList[] =
    {
     STRINGIFY0(DEBUG_CORE_FATFS)
    };
}
