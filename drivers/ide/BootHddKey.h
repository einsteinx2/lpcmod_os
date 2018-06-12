/*
 * BootHddKey.h
 *
 *  Created on: Sep 15, 2016
 *      Author: bennyboy
 */

#ifndef DRIVERS_IDE_BOOTHDDKEY_H_
#define DRIVERS_IDE_BOOTHDDKEY_H_

int CalculateDrivePassword(int driveId, unsigned char *key, unsigned char *eepromPtr);
void HMAC_hdd_calculation(int version,unsigned char *HMAC_result, ... );
unsigned int BootHddKeyGenerateEepromKeyData(unsigned char *pbEeprom_data, unsigned char *pbResult);

#endif /* DRIVERS_IDE_BOOTHDDKEY_H_ */
