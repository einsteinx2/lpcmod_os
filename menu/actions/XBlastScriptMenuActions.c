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
#include "BootIde.h"
#include "video.h"
#include "lpcmod_v1.h"
#include "memory_layout.h"
#include "xblast/scriptEngine/xblastScriptEngine.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include "lib/cromwell/cromString.h"
#include "Gentoox.h"
#include "MenuActions.h"
#include "string.h"

bool loadScriptFromHDD(char * filename, FATXFILEINFO *fileinfo)
{
    int res;
    FATXPartition *partition;

    partition = OpenFATXPartition (0, SECTOR_SYSTEM, SYSTEM_SIZE);

    res = LoadFATXFile(partition, filename, fileinfo);
    CloseFATXPartition (partition);
    if (!res)
    {
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

void loadRunScript(void *fname)
{
    FATXFILEINFO fileinfo;
    if(loadScriptFromHDD(fname, &fileinfo))
    {
        if(fileinfo.fileSize > 0)
        {
            UiHeader("Running script...");
            printk("\n           Press both triggers, Start and White buttons at the same time to force quit.");
            runScript(fileinfo.buffer, fileinfo.fileSize, 0, NULL);   //No param for now
        }
        else
        {
            UiHeader("Cannot run script.");
            printk("\n           Error reading script from HDD.");
        }
        free(fileinfo.buffer);
    }

    UIFooter();

    return;
}


void saveScriptToFlash(void *fname)
{
    FATXFILEINFO fileinfo;
    unsigned int compareSize = LPCmodSettings.flashScript.scriptSize;

    if(loadScriptFromHDD(fname, &fileinfo))
    {
        if(fileinfo.fileSize <= ScriptSavedInFlashMaxSizeInBytes)
        {
            if(LPCmodSettings.flashScript.scriptSize > fileinfo.fileSize)
            {
                compareSize = fileinfo.fileSize;
            }

            if(LPCmodSettings.flashScript.scriptSize != fileinfo.fileSize ||
               memcmp(LPCmodSettings.flashScript.scriptData, fileinfo.buffer, compareSize) == true)
            {
                LPCmodSettings.flashScript.scriptSize = fileinfo.fileSize;
                memcpy(LPCmodSettings.flashScript.scriptData, fileinfo.buffer, fileinfo.fileSize);
                UiHeader("Saved boot script to flash.");

                printk("\n           Script occupies %u bytes of %u bytes available.", LPCmodSettings.flashScript.scriptSize , ScriptSavedInFlashMaxSizeInBytes);
            }
            else
            {
                UiHeader("Boot script already saved.");

                printk("\n           Selected script file is already present in flash.");
            }
        }
        else
        {
            UiHeader("Cannot save boot script.");
            printk("\n           Script size is too big for flash space left available.");
            printk("\n           Script requires %u bytes and only %u bytes are available.", fileinfo.fileSize , ScriptSavedInFlashMaxSizeInBytes);
        }
        free(fileinfo.buffer);
    }
    UIFooter();

    return;
}

void loadScriptFromFlash(void * ignored)
{
    if(LPCmodSettings.flashScript.scriptSize > 0)
    {
        UiHeader("Running script...");
        printk("\n           Press both triggers, Start and White buttons at the same time to force quit.");
        runScript(LPCmodSettings.flashScript.scriptData, LPCmodSettings.flashScript.scriptSize, 0, NULL);   //No param for now
    }
    else
    {
        UiHeader("Cannot run script.");
        printk("\n           Error reading script from flash.");
    }

    UIFooter();

    return;
}

void toggleRunBootScript(void * itemStr)
{

    if(LPCmodSettings.OSsettings.runBootScript)
    {
           LPCmodSettings.OSsettings.runBootScript = 0;
    }
    else
    {
        if(LPCmodSettings.flashScript.scriptSize > 0)
        {
            LPCmodSettings.OSsettings.runBootScript = 1;
        }
        else
        {
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            UiHeader("Cannot activate boot script.");
            printk("\n           No script found in flash.");
            UIFooter();
        }
    }
    sprintf(itemStr,"%s",LPCmodSettings.OSsettings.runBootScript? "Yes" : "No");
}

void toggleRunBankScript(void * itemStr)
{
    FATXFILEINFO fileinfo;

    if(LPCmodSettings.OSsettings.runBankScript)
    {
        LPCmodSettings.OSsettings.runBankScript = 0;
    }
    else
    {
    	BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
        if(loadScriptFromHDD("\\XBlast\\scripts\\bank.script", &fileinfo))
        {
            free(fileinfo.buffer);
            LPCmodSettings.OSsettings.runBankScript = 1;
        }
        else
        {
            UiHeader("Cannot activate bank script.");
            printk("\n           \"C:\\XBlast\\scripts\\bank.script\" not found on HDD.");
            UIFooter();
        }
    }
    sprintf(itemStr,"%s",LPCmodSettings.OSsettings.runBankScript? "Yes" : "No");
}

void deleteFlashScriptFromFlash(void * ignored)
{
    LPCmodSettings.flashScript.scriptSize = 0;
}
