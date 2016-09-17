/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

void FlashBiosFromHDD512(void *fname);
void FlashBiosFromHDD256(void *fname);
void FlashBiosFromHDDOS(void *fname);

void FlashBiosFromHDD(void *fname);
void FlashBiosFromCD(void *cdromId);

void enableNetflash(void *flashType);
void enableWebupdate(void *whatever);

void switchBank(char bank);
void switchOSBank(unsigned char bank);
void switchBootBank(unsigned char bank);

void FlashFooter(void);
