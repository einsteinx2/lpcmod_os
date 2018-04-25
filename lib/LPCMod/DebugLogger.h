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
void printTextLogger(const char* debugFlag, unsigned char logLevel, const char* functionName, char* buffer, ...);
void lwipXBlastPrint(const char* category, unsigned char lwipDbgLevel, const char* functionName, ...);


#endif /* LIB_LPCMOD_DEBUGLOGGER_H_ */
