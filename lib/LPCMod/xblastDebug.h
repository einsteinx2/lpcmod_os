/*
 * xblastDebug.h
 *
 *  Created on: Aug 16, 2016
 *      Author: bennyboy
 */

#ifndef LIB_LPCMOD_XBLASTDEBUG_H_
#define LIB_LPCMOD_XBLASTDEBUG_H_

#include "DebugLogger.h"

#define STRINGIFY(x) #x
#define DBG_LVL_FATAL     5
#define DBG_LVL_ERROR     4
#define DBG_LVL_WARN      3
#define DBG_LVL_INFO      2
#define DBG_LVL_DEBUG     1
#define DBG_LVL_TRACE     0   /* Not implement for now */

#define CURRENT_DBG_LVL DBG_LVL_WARN

#ifdef DEBUGLOGGER
#define DEBUG_ALWAYS_SHOW       1
#define DEBUG_BOOT_LOG          0
#define DEBUG_FLASH_DRIVER      0
#define DEBUG_FLASH_LOWLEVEL    0
#define DEBUG_FLASH_UI          0
#define DEBUG_EXCEPTIONS        0
#define DEBUG_IDE_DRIVER        0   /* Not printed in text log to avoid logger loops */
#define DEBUG_IDE_LOCK          0
#define DEBUG_VIDEO_DRIVER      0
#define DEBUG_EEPROM_DRIVER     0
#define DEBUG_LWIP              1
#define DEBUG_HW_ID             0
#define DEBUG_GENERAL_UI        0
#define DEBUG_SCRIPT            0
#define DEBUG_MISC              0
#define DEBUG_SETTINGS          0

#define DEBUG_USB               0
#define DEBUG_USB_INFO          0
#define DEBUG_USB_ERR           0
#define DEBUG_USB_WARN          0

#define DEBUG_FATX_FS           0
#define DEBUG_CORE_FATFS        0   /* Not printed in text log to avoid logger loops */
#define DEBUG_LOGGER            0   /* Will essentially double the string prints on SPI. Not printed in text log to avoid logger loops */

#if DEBUG_USB >= 1
#define DEBUG 1
#define DEBUG_MODE 1
#endif

extern void lwipXBlastPrint(unsigned char lwipDbgLevel, const char* activate, const char* functionName, ...);
#define XblastLogger(level, activate,...) do { if(CURRENT_DBG_LVL <= level) printTextLogger(level, #activate, __func__, ##__VA_ARGS__); }while(0)
//TODO: modify usbSPIPrint to take Debug log levels into account
#define usbSPIPrint(activate, ...) do { printTextLogger(activate, __func__, ##__VA_ARGS__); }while(0)
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
