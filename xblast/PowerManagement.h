/*
 * PowerManagement.h
 *
 *  Created on: Dec 11, 2016
 *      Author: cromwelldev
 */

#ifndef POWERMANAGEMENT_H_
#define POWERMANAGEMENT_H_

#include "FlashDriver.h"

bool fSeenPowerButtonPress;
bool fSeenEjectButtonPress;

void assertBankScriptExecBankBoot(void * data);
void assertBankScriptExecTSOPBoot(void * data);
void BootOriginalBios(FlashBank bank) ;
void BootModBios(FlashBank bank);

bool canPowerDown(void);

#endif /* POWERMANAGEMENT_H_ */
