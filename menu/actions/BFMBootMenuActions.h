#ifndef _BFMBOOTMENUACTIONS_H_
#define _BFMBOOTMENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define METOOBFM_MAGIC1 0x314d4642

void bootBFMBios(void *fname);
void decodeAndSetupBFMBios(unsigned char *fileBuf, unsigned int fileSize);

#endif
