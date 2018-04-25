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
#define DBG_LVL_OFF DBG_LVL_FATAL + 1


#define CURRENT_DBG_LVL DBG_LVL_INFO

#ifdef DEBUGLOGGER
/* Max verbosity allowed per category */
/* Message will be shown if CURRENT_DBG_LVL is below the threshold set for the categories below */
#define DEBUG_ALWAYS_SHOW       DBG_LVL_INFO
#define DEBUG_BOOT_LOG          DBG_LVL_INFO
#define DEBUG_FLASH_DRIVER      DBG_LVL_FATAL
#define DEBUG_FLASH_LOWLEVEL    DBG_LVL_FATAL
#define DEBUG_FLASH_UI          DBG_LVL_INFO
#define DEBUG_EXCEPTIONS        DBG_LVL_FATAL
#define DEBUG_IDE_DRIVER        DBG_LVL_INFO   /* Not printed in text log to avoid logger loops */
#define DEBUG_IDE_LOCK          DBG_LVL_INFO
#define DEBUG_VIDEO_DRIVER      DBG_LVL_WARN
#define DEBUG_EEPROM_DRIVER     DBG_LVL_INFO
#define DEBUG_LWIP              DBG_LVL_INFO
#define DEBUG_HW_ID             DBG_LVL_INFO
#define DEBUG_GENERAL_UI        DBG_LVL_WARN
#define DEBUG_SCRIPT            DBG_LVL_INFO
#define DEBUG_MISC              DBG_LVL_INFO
#define DEBUG_SETTINGS          DBG_LVL_INFO

#define DEBUG_USB               0   //TODO: fix
#define DEBUG_USB_INFO          0   //TODO: fix
#define DEBUG_USB_ERR           0   //TODO: fix
#define DEBUG_USB_WARN          0   //TODO: fix

#define DEBUG_FATX_FS           DBG_LVL_DEBUG
#define DEBUG_CORE_FATFS        DBG_LVL_WARN    /* Not printed in text log to avoid logger loops */
#define DEBUG_LOGGER            DBG_LVL_ERROR   /* Will essentially double the string prints on SPI. Not printed in text log to avoid logger loops */

#if DEBUG_USB >= 1
#define DEBUG 1
#define DEBUG_MODE 1
#endif

extern void lwipXBlastPrint(unsigned char lwipDbgLevel, const char* activate, const char* functionName, ...);
#define XBlastLogger(level, activate, string, ...) do { if(CURRENT_DBG_LVL <= level) printTextLogger(level, #activate, __func__, string, ##__VA_ARGS__); }while(0)
//TODO: modify usbSPIPrint to take Debug log levels into account
#define usbSPIPrint(activate, category, string, ...) do { printTextLogger(activate, category, __func__, string, ##__VA_ARGS__); }while(0)
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
