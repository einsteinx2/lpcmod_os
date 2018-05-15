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

void DebugVFS_init(void)
{
    close(ReadFileHandle);
    cycler = 0;
}

int getEntryName(unsigned char index, const char * *const  out)
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

FILEX open(const char* path, FileOpenMode mode)
{
    if(0 == strcmp(path, ReadFileName) && (mode & FileOpenMode_Read) && !(mode & FileOpenMode_Write))
    {
        return ReadFileHandle;
    }

    if(strcmp(path, ReadFileName) && !(mode & FileOpenMode_Read) && (mode & FileOpenMode_Write))
    {
        return WriteFileHandle;
    }

    return 0;
}

int close(FILEX handle)
{
    if(ReadFileHandle == handle)
    {
        fileCursor = 0;
    }

    return 0;
}

int read(FILEX handle, unsigned char* out, unsigned int size)
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

int write(FILEX handle, const unsigned char* in, unsigned int size)
{
    if(WriteFileHandle == handle)
    {
        return size;
    }
    return -1;
}


int eof(FILEX handle)
{
    if(ReadFileHandle == handle)
    {
      handle = ReadFileHandle - 1;
      return ReadFileMaxSize == fileCursor;
    }

    return 0;
}

FileInfo stat(const char* path)
{
    FileInfo returnStruct;
    memset(&returnStruct, 0x00, sizeof(FileInfo));
    if(0 == strcmp(path, ReadFileName))
    {
        returnStruct.attributes = FileAttr_ReadOnly;
        returnStruct.nameLength = strlen(ReadFileName);
        strcpy(returnStruct.name, ReadFileName);
        returnStruct.size = ReadFileMaxSize;
        /* No date & time setting */
    }

    return returnStruct;
}

int rename(const char* path, const char* newName)
{
    return -1;
}

int mkdir(const char* path)
{
    return -1;
}

int remove(const char* path)
{
    return -1;
}

int chdir(const char* path)
{
    return -1;
}

DIREX opendir(const char* path)
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

FileInfo readdir(DIREX handle)
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

int closedir(DIREX handle)
{
    if(RootFolderHandle == handle)
    {
        cycler = 0;
    }
    return 0;
}

