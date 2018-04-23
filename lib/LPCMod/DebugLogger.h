/*
 * DebuggerLogger.h
 *
 *  Created on: Apr 22, 2018
 *      Author: cromwelldev
 */

#ifndef LIB_LPCMOD_DEBUGLOGGER_H_
#define LIB_LPCMOD_DEBUGLOGGER_H_

void debugLoggerInit(void);
void logRotate(void);
void printTextLogger(const char* debugFlag, const char* functionName, char* buffer, ...);


#endif /* LIB_LPCMOD_DEBUGLOGGER_H_ */
