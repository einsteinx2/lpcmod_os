/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "XBlastScriptMenuActions.h"
#include "ToolsMenuActions.h"
#include "boot.h"
#include "BootIde.h"
#include "BootFATX.h"
#include "video.h"
#include "lpcmod_v1.h"
#include "memory_layout.h"
#include "lib/scriptEngine/xblastScriptEngine.h"

bool loadScriptFromHDD(char * filename, FATXFILEINFO *fileinfo){
    int res;
    FATXPartition *partition;

    partition = OpenFATXPartition (0, SECTOR_SYSTEM, SYSTEM_SIZE);

    res = LoadFATXFile(partition, filename, fileinfo);
    CloseFATXPartition (partition);
    if (!res) {
        printk ("\n\n\n\n\n           Loading script failed");
        dots ();
        cromwellError ();
        fileinfo->fileSize = 0;
        fileinfo->buffer = NULL;
        return false;
    }
    fileinfo->fileSize = trimScript(&(fileinfo->buffer), fileinfo->fileSize);
    return true;
}

void loadRunScript(void *fname){
    FATXFILEINFO fileinfo;
    if(loadScriptFromHDD(fname, &fileinfo)){
        runScript(fileinfo.buffer, fileinfo.fileSize, 0, NULL);   //No param for now
        free(fileinfo.buffer);
    }

    UIFooter();

    return;
}


void saveScriptToFlash(void *fname){
    FATXFILEINFO fileinfo;

    if(loadScriptFromHDD(fname, &fileinfo)){
        LPCmodSettings.firstScript.ScripMagicNumber = 0xFAF1;
        LPCmodSettings.firstScript.nextEntryPosition = fileinfo.fileSize + sizeof(_LPCmodSettings) + 1;
        scriptSavingPtr = malloc(fileinfo.fileSize);
        memcpy(scriptSavingPtr, fileinfo.buffer, fileinfo.fileSize);
        free(fileinfo.buffer);
    }

    UIFooter();

    return;
}

void loadScriptFromFlash(void * ignored){
    u8 * buffer;
    int size;

    size = fetchBootScriptFromFlash(&buffer);
    runScript(buffer, size, 0, NULL);   //No param for now
    free(buffer);

    UIFooter();

    return;
}

void toggleRunBootScript(void * itemStr){
    (LPCmodSettings.OSsettings.runBootScript) = (LPCmodSettings.OSsettings.runBootScript)? 0 : 1;
    sprintf(itemStr,"%s",LPCmodSettings.OSsettings.runBootScript? "Yes" : "No");
}
