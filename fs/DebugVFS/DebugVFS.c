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
    return -1;
}

FILEX debugvfsopen(const char* path, FileOpenMode mode)
{
    char checkPath[25];
    sprintf(checkPath, PathSep"%s"PathSep"%s", PartName, ReadFileName);
    if(0 == strcmp(path, checkPath) && (mode & FileOpenMode_Read) && !(mode & FileOpenMode_Write))
    {
        return ReadFileHandle;
    }

    if(0 != strcmp(path, checkPath) && !(mode & FileOpenMode_Read) && (mode & FileOpenMode_Write))
    {
        return WriteFileHandle;
    }

    return 0;
}

int debugvfsclose(FILEX handle)
{
    if(ReadFileHandle == handle)
    {
        fileCursor = 0;
    }

    return 0;
}

int debugvfsread(FILEX handle, unsigned char* out, unsigned int size)
{
    unsigned int readSize = size;
    if(ReadFileHandle == handle)
    {
        handle = ReadFileHandle - 1;
        if(ReadFileMaxSize < readSize + fileCursor)
        {
            readSize = ReadFileMaxSize - fileCursor;
        }
        memset(out, 0x55, readSize);
        fileCursor += readSize;

        return readSize;
    }
    return -1;
}

int debugvfswrite(FILEX handle, const unsigned char* in, unsigned int size)
{
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
      handle = ReadFileHandle - 1;
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

