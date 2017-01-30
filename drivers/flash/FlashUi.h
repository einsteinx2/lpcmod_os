/*
 * FlashUi.h
 *
 *  Created on: Dec 12, 2016
 *      Author: cromwelldev
 */

#ifndef FLASHUI_H_
#define FLASHUI_H_

#include <stdbool.h>

void FlashFileFromBuffer(unsigned char *fileBuf, unsigned int fileSize, bool askConfirm);
void BootShowFlashDevice(void);
bool SaveXBlastOSSettings(void);


void setBiosJob(const unsigned char *data, unsigned int size, bool askConfirm);
bool executeFlashDriver(void);

bool FlashPrintResult(void);

// Temp only
void blockExecuteFlashJob(void);

#endif /* FLASHUI_H_ */
