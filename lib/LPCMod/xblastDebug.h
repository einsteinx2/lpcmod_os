/*
 * xblastDebug.h
 *
 *  Created on: Aug 16, 2016
 *      Author: bennyboy
 */

#ifndef LIB_LPCMOD_XBLASTDEBUG_H_
#define LIB_LPCMOD_XBLASTDEBUG_H_

#ifdef SPITRACE
#define DEBUG_ALWAYS_SHOW       0
#define DEBUG_BOOT_LOG          0
#define DEBUG_FLASH_DRIVER      0
#define DEBUG_FLASH_LOWLEVEL    0
#define DEBUG_FLASH_UI          0
#define DEBUG_EXCEPTIONS        0
#define DEBUG_IDE_DRIVER        0
#define DEBUG_VIDEO_DRIVER      0
#define DEBUG_EEPROM_DRIVER     0
#define DEBUG_LWIP              0
#define DEBUG_HW_ID             0
#define DEBUG_GENERAL_UI        0
#define DEBUG_MISC              0

extern void printTextSPI(const char * functionName, char * buffer, ...);
#define debugSPIPrint(activate,...) do { if(activate) printTextSPI(__func__, ##__VA_ARGS__); }while(0)
#define lwipSPIPrint(...) do { printTextSPI(__func__, ##__VA_ARGS__); }while(0)
#else
#ifdef __FLASH_SIMULATOR__
#include <stdio.h>
#define debugSPIPrint(...) printf(__VA_ARGS__)
#else
#define debugSPIPrint(...)
#endif
#endif

#ifdef SPI_INT_TRACE
#define debugSPIPrintInt(...) printTextSPI(__func__, ##__VA_ARGS__)
#else
#define debugSPIPrintInt(...)
#endif

#endif /* LIB_LPCMOD_XBLASTDEBUG_H_ */
