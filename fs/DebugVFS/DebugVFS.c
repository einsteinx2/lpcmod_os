/*
 * DebugVFS.c
 *
 *  Created on: May 14, 2018
 *      Author: cromwelldev
 */

#include "DebugVFS.h"
#include "FatFSAccessor.h"
#include "stdio.h"
#include "string.h"
#include "lib/LPCMod/xblastDebug.h"

static const char *const PartName = "DebugVFS";
#define RootFolderHandle 1
static unsigned char cycler;
static const char *const ReadFileName = "DebugFile";
#define ReadFileMaxSize 8 * 1024 * 1024 /* 8MB */

#define ReadFileHandle 1
#define WriteFileHandle 2
static unsigned int fileCursor;

void debugvfs_init(void)
{
    debugvfsclose(ReadFileHandle);
    cycler = 0;
    XBlastLogger(DEBUG_DVFS, DBG_LVL_INFO, "DebugVFS init done.");
}

int debugvfsgetEntryName(unsigned char index, const char * *const  out)
{
    if(index == 0)
    {
        *out = PartName;
    }
    else
    {
        *out = NULL;
    }
    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "returning:\"%s\"", *out == NULL ? "<NULL>" : *out);
    return -1;
}

FILEX debugvfsopen(const char* path, FileOpenMode mode)
{
    char checkPath[25];
    sprintf(checkPath, PathSep"%s"PathSep"%s", PartName, ReadFileName);

    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "%s, mode:%u", path, mode);
    if(0 == strcmp(path, checkPath) && (mode & FileOpenMode_Read) && !(mode & FileOpenMode_Write))
    {
        XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "Handing out ReadFileHandle");
        return ReadFileHandle;
    }

    if(0 != strcmp(path, checkPath) && !(mode & FileOpenMode_Read) && (mode & FileOpenMode_Write))
    {
        XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "Handing out WriteFileHandle");
        return WriteFileHandle;
    }

    return 0;
}

int debugvfsclose(FILEX handle)
{
    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "Closing handle:%u", handle);
    if(ReadFileHandle == handle)
    {
        fileCursor = 0;
    }

    return 0;
}

int debugvfsread(FILEX handle, unsigned char* out, unsigned int size)
{
    unsigned int readSize = size;
    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "handle:%u  size:%u", handle, readSize);
    if(ReadFileHandle == handle)
    {
        if(ReadFileMaxSize < readSize + fileCursor)
        {
            readSize = ReadFileMaxSize - fileCursor;
        }

        XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "fileCursor:%u  readSize:%u", fileCursor, readSize);
        memset(out, 0x55, readSize);
        fileCursor += readSize;

        return readSize;
    }
    return -1;
}

int debugvfswrite(FILEX handle, const unsigned char* in, unsigned int size)
{
    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "handle:%u  size:%u", handle, size);
    if(WriteFileHandle == handle)
    {
        return size;
    }
    return -1;
}


int debugvfseof(FILEX handle)
{
    if(ReadFileHandle == handle)
    {
        return ReadFileMaxSize == fileCursor;
    }

    return 0;
}

FileInfo debugvfsstat(const char* path)
{
    FileInfo returnStruct;
    char checkPath[25];
    sprintf(checkPath, PathSep"%s"PathSep"%s", PartName, ReadFileName);
    memset(&returnStruct, 0x00, sizeof(FileInfo));
    if(0 == strcmp(path, checkPath))
    {
        returnStruct.attributes = FileAttr_ReadOnly;
        returnStruct.nameLength = strlen(ReadFileName);
        strcpy(returnStruct.name, ReadFileName);
        returnStruct.size = ReadFileMaxSize;
        /* No date & time setting */
    }

    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "name:%s (%u)", returnStruct.name, returnStruct.nameLength);
    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "flags:%u size:%u", returnStruct.attributes, returnStruct.size);
    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "date:%u time:%u", returnStruct.modDate, returnStruct.modTime);

    return returnStruct;
}

int debugvfsrename(const char* path, const char* newName)
{
    return -1;
}

int debugvfsmkdir(const char* path)
{
    return -1;
}

int debugvfsremove(const char* path)
{
    return -1;
}

int debugvfschdir(const char* path)
{
    return -1;
}

DIREX debugvfsopendir(const char* path)
{
    char tempPath[10];
    sprintf(tempPath, PathSep"%s", PartName);
    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "Open dir:\"%s\"", path);

    if(0 == strcmp(path, tempPath))
    {
        cycler = 0;
        return RootFolderHandle;
    }

    return 0;
}

FileInfo debugvfsreaddir(DIREX handle)
{

    FileInfo returnStruct;

    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "handle:%u", handle);

    memset(&returnStruct, 0x00, sizeof(FileInfo));

    if(RootFolderHandle == handle)
    {
        if(cycler++ == 0)
        {
            returnStruct.attributes = FileAttr_ReadOnly;
            returnStruct.nameLength = strlen(ReadFileName);
            strcpy(returnStruct.name, ReadFileName);
            returnStruct.size = ReadFileMaxSize;
        }
    }
    XBlastLogger(DEBUG_DVFS, DBG_LVL_DEBUG, "dir %s(%u) size:%u attr:%u", returnStruct.name, returnStruct.nameLength, returnStruct.size, returnStruct.attributes);


    return returnStruct;
}

int debugvfsclosedir(DIREX handle)
{
    if(RootFolderHandle == handle)
    {
        cycler = 0;
    }
    return 0;
}

