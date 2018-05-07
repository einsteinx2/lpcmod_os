/*
 * DebuggerLogger.h
 *
 *  Created on: Apr 22, 2018
 *      Author: cromwelldev
 */

#ifndef LIB_LPCMOD_DEBUGLOGGER_H_
#define LIB_LPCMOD_DEBUGLOGGER_H_

void debugLoggerInit(void);
unsigned char logRotate(void);
void forceFlushLog(void);
void printTextLogger(const char* const debugFlag, unsigned char logLevel, const char* const functionName, const char* const buffer, ...)
    __attribute__ ((format (printf, 4, 5)));
void lwipXBlastPrint(const char* const category, unsigned char lwipDbgLevel, const char* const functionName, const char* const message, ...)
    __attribute__ ((format (printf, 4, 5)));


#endif /* LIB_LPCMOD_DEBUGLOGGER_H_ */
