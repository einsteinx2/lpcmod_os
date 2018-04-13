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
#include "FatFSAccessor.h"
#include "lib/LPCMod/xblastDebug.h"

static const char* const biosDirectoryLocation = "MASTER_C:\\XBlast\\scripts\\";

const char* const getScriptDirectoryLocation(void)
{
    return biosDirectoryLocation;
}


static FILEX openScript(const char* filename, unsigned int* outSize)
{
    FILEX handle;
    handle = fatxopen(filename, FileOpenMode_OpenExistingOnly | FileOpenMode_Read);

    if(0 == handle)
    {
        debugSPIPrint(DEBUG_SCRIPT, "No script file.\n");
        return 0;
    }

    *outSize = fatxsize(handle);
    return handle;
}

int testScriptFromHDD(char * filename)
{
    FILEX handle;
    unsigned int size, newSize;
    unsigned char* fileBuf;

    handle = openScript(filename, &size);

    if(0 == handle || 0 == size)
    {
        debugSPIPrint(DEBUG_SCRIPT, "Error.\n");
        return -1;
    }

    fileBuf = malloc(size * sizeof(unsigned char));

    if(NULL == fileBuf)
    {
        debugSPIPrint(DEBUG_BOOT_LOG, "malloc failed.\n");
        return -1;
    }

    if(fatxread(handle, fileBuf, size) != size)
    {
        free(fileBuf);
        debugSPIPrint(DEBUG_BOOT_LOG, "Read incomplete.\n");
        return -1;
    }
    newSize = trimScript(&fileBuf, size);

    fatxclose(handle);
    free(fileBuf);

    return 0 < newSize && newSize <= size;
}

void loadRunScriptNoParams(void* fname)
{
    loadRunScriptWithParams(fname, 0, NULL);
}

void loadRunScriptWithParams(const char *fname, int paramCount, int * param)
{
    FILEX handle;
    unsigned int size, newSize;
    unsigned char* fileBuf;

    handle = openScript(fname, &size);

    if(0 == handle || 0 == size)
    {
        debugSPIPrint(DEBUG_SCRIPT, "Error.\n");
        return;
    }

    fileBuf = malloc(size * sizeof(unsigned char));

    if(NULL == fileBuf)
    {
        debugSPIPrint(DEBUG_BOOT_LOG, "malloc failed.\n");
        return;
    }

    if(fatxread(handle, fileBuf, size) != size)
    {
        debugSPIPrint(DEBUG_BOOT_LOG, "Read incomplete.\n");
        newSize = 0;
    }
    else
    {
        newSize = trimScript(&fileBuf, size);
    }

    fatxclose(handle);

    if(0 < newSize && newSize <= size)
    {
        UiHeader("Running script...");
        printk("\n           Press both triggers, Start and White buttons at the same time to force quit.");
        runScript(fileBuf, newSize, paramCount, param);
    }
    else
    {
        UiHeader("Cannot run script.");
        printk("\n           Error reading script from HDD.");
    }

    free(fileBuf);
    UIFooter();

    return;
}


void saveScriptToFlash(void *fname)
{
    unsigned int compareSize = LPCmodSettings.flashScript.scriptSize;
    unsigned int size, newSize;
    unsigned char* fileBuf;
    FILEX handle = openScript(fname, &size);

    if(0 == handle || 0 == size)
    {
        debugSPIPrint(DEBUG_SCRIPT, "Error.\n");
        return;
    }

    fileBuf = malloc(size * sizeof(unsigned char));

    if(NULL == fileBuf)
    {
        debugSPIPrint(DEBUG_BOOT_LOG, "malloc failed.\n");
        return;
    }

    if(fatxread(handle, fileBuf, size) != size)
    {
        debugSPIPrint(DEBUG_BOOT_LOG, "Read incomplete.\n");
        return;
    }
    else
    {
        newSize = trimScript(&fileBuf, size);
    }

    fatxclose(handle);

    if(0 < newSize && newSize <= size)
    {
        if(newSize <= ScriptSavedInFlashMaxSizeInBytes)
        {
            if(LPCmodSettings.flashScript.scriptSize > newSize)
            {
                compareSize = newSize;
            }

            if(LPCmodSettings.flashScript.scriptSize != newSize ||
               memcmp(LPCmodSettings.flashScript.scriptData, fileBuf, compareSize) == true)
            {
                LPCmodSettings.flashScript.scriptSize = newSize;
                memcpy(LPCmodSettings.flashScript.scriptData, fileBuf, newSize);
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
            printk("\n           Script requires %u bytes and only %u bytes are available.", newSize , ScriptSavedInFlashMaxSizeInBytes);
        }
    }

    free(fileBuf);

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

    if(LPCmodSettings.OSsettings.runBankScript)
    {
        LPCmodSettings.OSsettings.runBankScript = 0;
    }
    else
    {
    	BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
        if(testScriptFromHDD("MASTER_C:\\XBlast\\scripts\\bank.script"))
        {
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
