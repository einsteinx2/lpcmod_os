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
#include "boot.h"
#include "memory_layout.h"
#include "BootFATX.h"


TEXTMENU* HDDFlashMenuInit(void) {
	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;
	FATXFILEINFO fileinfo;
	FATXPartition *partition;

	int i=0;
	char *fnames[256];
	int n=0;
	int bioses=0;
	int res;
	int dcluster;
	char *path="\\BIOS\\";
	char *fullPath = (char*)malloc(20);
	char *fullPathptr = fullPath;

	memset(fullPath, 0, 20);

	// Generate the menu title.
	strcpy(fullPath, "'C:");
	fullPathptr += 3;
	strcpy(fullPathptr, path);
	fullPathptr += strlen(path);
	strcpy(fullPathptr, "'");
	fullPathptr = NULL;

	partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);

	menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
	memset(menuPtr,0,sizeof(TEXTMENU));

	strcpy(menuPtr->szCaption, fullPath);

	if(partition != NULL) {
		dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "BIOS");
		if((dcluster != -1) && (dcluster != 1)) {
			n = FATXListDir(partition, dcluster, &fnames[0], 256, path);
			for (i=0; i<n; i++) {
				// Check the file.
				res = FATXFindFile(partition, fnames[i], FATX_ROOT_FAT_CLUSTER, &fileinfo);
		
				if((res) && (fileinfo.fileSize%(256*1024) == 0)) {
					// If it's a (readable) file - i.e. not a directory.
					// AND it's filesize is divisible by 256k.
					itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
					memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
					sprintf(itemPtr->szCaption,fnames[i]+strlen(path));
					itemPtr->functionPtr = FlashBiosFromHDD;
					itemPtr->functionDataPtr = fnames[i];
					TextMenuAddItem(menuPtr, itemPtr);
					bioses++;
				}
			}
			if(n < 1) {
				// If there were no directories and no files.
				itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
				memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
				sprintf(itemPtr->szCaption,"No files in C:\\BIOS.");
				itemPtr->functionPtr = NULL;
				TextMenuAddItem(menuPtr, itemPtr);
			} else if(bioses==0) {
				// If there were directories, but no files.
				itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
				memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
				sprintf(itemPtr->szCaption,"No BIOS files in C:\\BIOS.");
				itemPtr->functionPtr = NULL;
				TextMenuAddItem(menuPtr, itemPtr);
			}
		} else {
			// If C:\BIOS doesnt exist.
			itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
			memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
			sprintf(itemPtr->szCaption,"C:\\BIOS does not exist.");
			itemPtr->functionPtr = NULL;
			TextMenuAddItem(menuPtr, itemPtr);
		}
	} else {
		// If the partition couldn't be opened at all.
		itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
		memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
		sprintf(itemPtr->szCaption,"Error reading C:\\ partition.");
		itemPtr->functionPtr = NULL;
		TextMenuAddItem(menuPtr, itemPtr);
	}
	return menuPtr;
}
