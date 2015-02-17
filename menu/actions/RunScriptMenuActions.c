/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "RunScriptMenuActions.h"
#include "ToolsMenuActions.h"
#include "boot.h"
#include "BootIde.h"
#include "BootFATX.h"
#include "video.h"
#include "lpcmod_v1.h"
#include "memory_layout.h"
#include "lib/scriptEngine/xblastScriptEngine.h"

void loadRunScript(void *fname){
    int res;
    FATXFILEINFO fileinfo;
    FATXPartition *partition;

    partition = OpenFATXPartition (0, SECTOR_SYSTEM, SYSTEM_SIZE);
    
    res = LoadFATXFile(partition, fname, &fileinfo);
    CloseFATXPartition (partition);
    if (!res) {
        printk ("\n\n\n\n\n           Loading script failed");
        dots ();
        cromwellError ();
        while (1)
            ;
    }
    fileinfo.fileSize = trimScript(&(fileinfo.buffer), fileinfo.fileSize);
    runScript(fileinfo.buffer, fileinfo.fileSize, 0, NULL);   //No param for now
    free(fileinfo.buffer);

    ToolFooter();

    return;
}


void saveScriptToFlash(void *fname){
    int res;
    FATXFILEINFO fileinfo;
    FATXPartition *partition;

    partition = OpenFATXPartition (0, SECTOR_SYSTEM, SYSTEM_SIZE);

    res = LoadFATXFile(partition, fname, &fileinfo);
    CloseFATXPartition (partition);
    if (!res) {
        printk ("\n\n\n\n\n           Loading script failed");
        dots ();
        cromwellError ();
        while (1)
            ;
    }
    fileinfo.fileSize = trimScript(&(fileinfo.buffer), fileinfo.fileSize);
    LPCmodSettings.firstScript.ScripMagicNumber = 0xFAF1;
    LPCmodSettings.firstScript.nextEntryPosition = fileinfo.fileSize + sizeof(_LPCmodSettings) + 1;
    scriptSavingPtr = malloc(fileinfo.fileSize);
    memcpy(scriptSavingPtr, fileinfo.buffer, fileinfo.fileSize);
    free(fileinfo.buffer);

    ToolFooter();

    return;
}

void loadScriptFromFlash(void * ignored){
    u8 * buffer;
    int size;

    size = fetchBootScriptFromFlash(&buffer);
    runScript(buffer, size, 0, NULL);   //No param for now
    free(buffer);

    ToolFooter();

    return;
}
