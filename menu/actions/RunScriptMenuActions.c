/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "RunScriptMenuActions.h"
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
    runScript(fileinfo.buffer, fileinfo.fileSize, 0, NULL);   //No param for now
    free(fileinfo.buffer);

    return;
}
