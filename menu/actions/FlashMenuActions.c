/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "FlashMenuActions.h"

#include "include/boot.h"
#include "BootIde.h"
#include "TextMenu.h"
#include "FlashMenuActions.h"

#include "boot.h"
#include "memory_layout.h"
#include "BootFATX.h"

extern void cromwellError(void);
extern void dots(void);

void FlashBiosFromHDD(void *fname) {
#ifdef FLASH
	int res;
	int offset;

	FATXPartition *partition;

	partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);

	FATXFILEINFO fileinfo;
	res = LoadFATXFilefixed(partition, fname, &fileinfo, (char*)0x100000);
	if (!res) {
		printk("\n\n\n\n\n           Loading BIOS failed");
		dots();
		cromwellError();
		while(1);
	}

	offset = 0;

	res = BootReflashAndReset((char*)0x100000,offset,fileinfo.fileSize);

	printk("\n\n\n\n\n           Flash failed");
   CloseFATXPartition(partition);
	dots();
	cromwellError();
	while(1);
#endif
}

void FlashBiosFromCD(void *cdromId) {
#ifdef FLASH
	extern unsigned char *videosavepage;
	memcpy((void*)FB_START,videosavepage,FB_SIZE);
	BootLoadFlashCD(*(int *)cdromId);
#endif
}

void enableNetflash(void *whatever) {
#ifdef FLASH
	extern unsigned char *videosavepage;
	memcpy((void*)FB_START,videosavepage,FB_SIZE);
	VIDEO_ATTR=0xffef37;
	printk("\n\n\n\n\n\n");
	VIDEO_ATTR=0xffc8c8c8;
	initialiseNetwork();
	netFlash();
#endif
}

void enableWebupdate(void *whatever) {
#ifdef FLASH
	extern unsigned char *videosavepage;
	memcpy((void*)FB_START,videosavepage,FB_SIZE);
	VIDEO_ATTR=0xffef37;
	printk("\n\n\n\n\n\n");
	VIDEO_ATTR=0xffc8c8c8;
	initialiseNetwork();
	webUpdate();
#endif
}

