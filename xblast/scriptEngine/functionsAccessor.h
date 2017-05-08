/*
 * functionsAccessor.h
 *
 *  Created on: May 6, 2017
 *      Author: bennyboy
 */

#ifndef FUNCTIONSACCESSOR_H_
#define FUNCTIONSACCESSOR_H_

#include "lpcmod_v1.h"
#include <stdbool.h>

bool gpiFunction(unsigned char port);
bool gpoFunction(unsigned char port, unsigned char value);
bool waitFunction(int ms);
bool bootFunction(FlashBank bank);
bool fanFunction(unsigned char value);
bool ledFunction(char * value);
bool lcdPrintFunction(unsigned char line, char * text, unsigned char stringLength);
bool lcdClearLineFunction(unsigned char line);
bool lcdResetFunction(void);
bool lcdBacklightFunction(unsigned char value);
bool lcdPowerFunction(unsigned char value);
unsigned char SPIRead(void);
bool SPIWrite(unsigned char data);
unsigned char XPADRead(int xpadVariable);

bool emergencyEscape(void);

#endif /* XBLAST_SCRIPTENGINE_FUNCTIONSACCESSOR_H_ */
