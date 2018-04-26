/*
 * DebuggerLogger.c
 *
 *  Created on: Apr 22, 2018
 *      Author: cromwelldev
 */

#include "DebugLogger.h"
#include "FatFSAccessor.h"
#include "lib/time/timeManagement.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "lwip/debug.h" /* lwip */
#include "BootLPCMod.h"

#define MaxBuffSize 1024

#define LogRotationDepth 5

#define logFilename "xblast.log"
static const char* const ActiveLogFileLocation = "MASTER_X:" PathSep logFilename;

static unsigned char initDone = 0;
static FIL activeLogHandle;

static void writeString(const char* const string, unsigned char writeToLogFile);
static unsigned char checkDebugFlag(const char* const szDebugFlag);
static const char* const getLogLevelString(unsigned char logLevel);

void debugLoggerInit(void)
{
    if(0 == logRotate())
    {
        initDone = 1;
    }
}

unsigned char logRotate(void)
{
    signed char i = LogRotationDepth;
    char fullPath[50];
    char newName[50];
    FRESULT result;

    //TODO: disable interrupt?

    if(NULL != activeLogHandle.obj.fs)
    {
        f_close(&activeLogHandle);
    }

    sprintf(newName, "%s.%u", ActiveLogFileLocation, i);
    XBlastLogger(DEBUG_LOGGER, DBG_LVL_DEBUG, "Erase file: %s.", newName);
    f_unlink(newName);

    while(0 <= --i)
    {
        sprintf(fullPath, "%s.%u", ActiveLogFileLocation, i);
        XBlastLogger(DEBUG_LOGGER, DBG_LVL_DEBUG, "Rename:%s -> %s.", fullPath, newName);
        f_rename(fullPath, newName);
        strcpy(newName, fullPath);
    }

    XBlastLogger(DEBUG_LOGGER, DBG_LVL_DEBUG, "Rename:%s -> %s.", ActiveLogFileLocation, fullPath);
    f_rename(ActiveLogFileLocation, fullPath);

    result = f_open(&activeLogHandle, ActiveLogFileLocation, FA_OPEN_APPEND | FA_WRITE);
    XBlastLogger(DEBUG_LOGGER, DBG_LVL_DEBUG, "Open:%s.", ActiveLogFileLocation);
    if(FR_OK != result)
    {
        XBlastLogger(DEBUG_LOGGER, DBG_LVL_ERROR, "Couldn't open log file.");
        return 1;
    }

    return 0;
}

void printTextLogger(const char* const debugFlag, unsigned char logLevel, const char* const functionName, const char* const buffer, ...)
{
    FRESULT result;
    va_list vl;
    char tempBuf[MaxBuffSize];
    unsigned char writeToLogfile;

    if(NULL != activeLogHandle.obj.fs)
    {
        writeToLogfile = checkDebugFlag(debugFlag);
        va_start(vl,buffer);
        sprintf(tempBuf, "[%s][%s][%s] ", getLogLevelString(logLevel), debugFlag, functionName);
        writeString(tempBuf, writeToLogfile);
        vsprintf(tempBuf,buffer,vl);
        writeString(tempBuf, writeToLogfile);
        va_end(vl);

        if('\n' != tempBuf[strlen(tempBuf) - 1])
        {
            writeString("\n", writeToLogfile);
        }

        if(initDone && writeToLogfile)
        {
            result = f_sync(&activeLogHandle);
            XBlastLogger(DEBUG_LOGGER, DBG_LVL_DEBUG, "Sync log to drive. result:%u", result);
        }
    }
}


//TODO: replace with macro to eliminate va_args handling?
void lwipXBlastPrint(const char* const category, unsigned char lwipDbgLevel, const char* const functionName, const char* const message, ...)
{
    char* string;
    unsigned char convertedLogLevel = DBG_LVL_FATAL;

    /* Remove DBG_ON flag */
    lwipDbgLevel &= ~(unsigned char)(LWIP_DBG_ON);

    if(lwipDbgLevel & LWIP_DBG_HALT)
    {
        convertedLogLevel = DBG_LVL_FATAL;
    }
    else if(lwipDbgLevel & LWIP_DBG_TRACE)
    {
        convertedLogLevel = DBG_LVL_TRACE;
    }
    else if((lwipDbgLevel & LWIP_DBG_STATE) || (lwipDbgLevel & LWIP_DBG_FRESH))
    {
        convertedLogLevel = DBG_LVL_DEBUG;
    }
    else
    {
        switch(lwipDbgLevel & LWIP_DBG_MASK_LEVEL)
        {
        case LWIP_DBG_LEVEL_ALL:
            convertedLogLevel = DBG_LVL_INFO;
            break;
        case LWIP_DBG_LEVEL_WARNING:
            convertedLogLevel = DBG_LVL_WARN;
            break;
        case LWIP_DBG_LEVEL_SERIOUS:
            convertedLogLevel = DBG_LVL_ERROR;
            break;
        case LWIP_DBG_LEVEL_SEVERE:
            convertedLogLevel = DBG_LVL_FATAL;
            break;
        }
    }
    va_list vargs;
    va_start(vargs, message);
    printTextLogger(category, convertedLogLevel, functionName, message, vargs);
    va_end(vargs);
}

static void writeString(const char* const string, unsigned char writeToLogFile)
{
    FRESULT result;
    /* write to SPI debugger, if applicable */
#ifdef SPITRACE
    printTextSPI(string);
#endif
    /* Write to log file*/
    if(initDone && writeToLogFile)
    {
        XBlastLogger(DEBUG_LOGGER, DBG_LVL_DEBUG, "WriteToLog:\"%s\"", string);
        result = f_puts(string, &activeLogHandle);
        XBlastLogger(DEBUG_LOGGER, DBG_LVL_DEBUG, "puts result:%u", result);
    }
#ifdef SPITRACE
    else
    {
        //If you miss characters, add delay function here (wait_us()). A couple microseconds should give enough time for the Arduino to catchup.
        wait_us_blocking(50);
    }
#endif
}

static unsigned char checkDebugFlag(const char* const szDebugFlag)
{
    /* List of Debug categories that must not trigger Logfile writes */
#define ForbiddenListLength 3
    char* forbiddenList[ForbiddenListLength] =
    {
        STRINGIFY(DEBUG_CORE_FATFS),
        STRINGIFY(DEBUG_IDE_DRIVER),
        STRINGIFY(DEBUG_LOGGER)
    };
    unsigned char i;

    for(i = 0; i < ForbiddenListLength; i++)
    {
        if(0 == strcmp(szDebugFlag, forbiddenList[i]))
        {
            return 0;
        }
    }

    return 1;
}

static const char* const getLogLevelString(unsigned char logLevel)
{
    switch(logLevel)
    {
    case DBG_LVL_FATAL:
        return "FATAL";
    case DBG_LVL_ERROR:
        return "ERROR";
    case DBG_LVL_WARN:
        return "WARN";
    case DBG_LVL_INFO:
        return "INFO";
    case DBG_LVL_DEBUG:
        return "DEBUG";
    case DBG_LVL_TRACE:
        return "TRACE";
    }

    return "\0";
}
