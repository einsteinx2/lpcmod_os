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
        if(fileinfo.fileSize > 0){
            ToolHeader("Running script...");
            printk("\n           Press both triggers, Start and White buttons at the same time to force quit.");
            runScript(fileinfo.buffer, fileinfo.fileSize, 0, NULL);   //No param for now
        }
        else{
            ToolHeader("Cannot run script.");
            printk("\n           Error reading script from HDD.");
        }
        free(fileinfo.buffer);
    }

    UIFooter();

    return;
}


void saveScriptToFlash(void *fname){
    FATXFILEINFO fileinfo;

    if(loadScriptFromHDD(fname, &fileinfo)){
        if(fileinfo.fileSize <= (0x3FE00 - 0x3F000 - sizeof(_LPCmodSettings))){  //Should give 2812 bytes
            LPCmodSettings.firstScript.ScripMagicNumber = 0xFAF1;
            LPCmodSettings.firstScript.nextEntryPosition = fileinfo.fileSize + sizeof(_LPCmodSettings) + 1;
            scriptSavingPtr = malloc(fileinfo.fileSize);
            memcpy(scriptSavingPtr, fileinfo.buffer, fileinfo.fileSize);
            ToolHeader("Saved boot script to flash.");
            printk("\n           Script occupies %u bytes of %u bytes available.", fileinfo.fileSize , 0x3FE00 - 0x3F000 - sizeof(_LPCmodSettings));
        }
        else{
            ToolHeader("Cannot save boot script.");
            printk("\n           Script size is too big for flash space left available.");
            printk("\n           Script requires %u bytes and only %u bytes are available.", fileinfo.fileSize , 0x3FE00 - 0x3F000 - sizeof(_LPCmodSettings));
        }
        free(fileinfo.buffer);
    }

    UIFooter();

    return;
}

void loadScriptFromFlash(void * ignored){
    u8 * buffer;
    int size;

    size = fetchBootScriptFromFlash(&buffer);
    if(size > 0){
        ToolHeader("Running script...");
        printk("\n           Press both triggers, Start and White buttons at the same time to force quit.");
        runScript(buffer, size, 0, NULL);   //No param for now
        free(buffer);
    }
    else{
        ToolHeader("Cannot run script.");
        printk("\n           Error reading script from flash.");
    }

    UIFooter();

    return;
}

void toggleRunBootScript(void * itemStr){

    if(LPCmodSettings.OSsettings.runBootScript){
           LPCmodSettings.OSsettings.runBootScript = 0;
   }
    else{
        if(LPCmodSettings.firstScript.ScripMagicNumber == 0xFAF1){
            LPCmodSettings.OSsettings.runBootScript = 1;
        }
        else{
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            ToolHeader("Cannot activate boot script.");
            printk("\n           No script found in flash.");
            UIFooter();
        }
    }
    sprintf(itemStr,"%s",LPCmodSettings.OSsettings.runBootScript? "Yes" : "No");
}

void toggleRunBankScript(void * itemStr){
    FATXFILEINFO fileinfo;

    if(LPCmodSettings.OSsettings.runBankScript){
        LPCmodSettings.OSsettings.runBankScript = 0;
    }
    else{
    	BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
        if(loadScriptFromHDD("\\XBlast\\scripts\\bank.script", &fileinfo)){
            free(fileinfo.buffer);
            LPCmodSettings.OSsettings.runBankScript = 1;
        }
        else{
            ToolHeader("Cannot activate bank script.");
            printk("\n           \"C:\\XBlast\\scripts\\bank.script\" not found on HDD.");
            UIFooter();
        }
    }
    sprintf(itemStr,"%s",LPCmodSettings.OSsettings.runBankScript? "Yes" : "No");
}
