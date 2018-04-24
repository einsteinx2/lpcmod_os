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
void printTextLogger(unsigned char logLevel, const char* debugFlag, const char* functionName, char* buffer, ...);
void lwipXBlastPrint(unsigned char lwipDbgLevel, const char* activate, const char* functionName, ...);


#endif /* LIB_LPCMOD_DEBUGLOGGER_H_ */
