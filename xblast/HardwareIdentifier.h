/*
 * HardwareIdentifier.h
 *
 *  Created on: Jan 3, 2017
 *      Author: cromwelldev
 */

#ifndef XBLAST_HARDWAREIDENTIFIER_H_
#define XBLAST_HARDWAREIDENTIFIER_H_

#include <stdbool.h>

//TODO: remove once set static in c file
extern unsigned short fHasHardware;
extern unsigned char fSpecialEdition;

//Xbox motherboard revision enum.
typedef enum {
    XboxMotherboardRevision_DEVKIT   = 0x00, //Includes a bunch of revisions
    XboxMotherboardRevision_DEBUGKIT = 0x01, //2 known version ID
    XboxMotherboardRevision_1_0      = 0x02, //1.0
    XboxMotherboardRevision_1_1      = 0x03, //1.1
    XboxMotherboardRevision_1_2      = 0x04, //1.2/1.3
    XboxMotherboardRevision_1_4      = 0x05, //1.4/1.5
    XboxMotherboardRevision_1_6      = 0x06, //1.6/1.6b
    XboxMotherboardRevision_UNKNOWN  = 0x07  //dafuk?
} XboxMotherboardRevision;

void identifyModchipHardware(void);
void identifyXboxHardware(void);

bool isXBlastOnTSOP(void);
bool isXBlastOnLPC(void);
bool isXBlastCompatible(void);
bool isPureXBlast(void);
bool isLCDSupported(void);
bool isXecuter3(void);
bool isXBE(void);
bool isLCDContrastSupport(void);
bool isFrostySupport(void);
const char * getModchipName(void);

unsigned short getCPUSPeedInMHz(void);
XboxMotherboardRevision getMotherboardRevision(void);
const char* getMotherboardRevisionString(void);
bool isTSOPSplitCapable(void);

#endif /* XBLAST_HARDWAREIDENTIFIER_H_ */
