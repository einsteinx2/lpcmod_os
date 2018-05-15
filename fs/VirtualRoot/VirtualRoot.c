/*
 * VirtualRoot.c
 *
 *  Created on: Apr 27, 2018
 *      Author: cromwelldev
 */

#include "VirtualRoot.h"
#include "FatFSAccessor.h"
#include "lib/LPCMod/xblastDebug.h"
#include "string.h"
#include "stdio.h"
#include "DebugVFS.h"
#include <limits.h>

/* To enable DebugVFS */
#define DEBUGVFS 1


typedef struct
{
    int         (*getEntryName) (unsigned char index, const char * *const  out);
    FILEX       (*open)         (const char* path, FileOpenMode mode);
    int         (*read)         (FILEX handle, unsigned char* out, unsigned int size);
    int         (*write)        (FILEX handle, const unsigned char* in, unsigned int size);
    int         (*close)        (FILEX handle);

    int         (*eof)          (FILEX handle);
    FileInfo    (*stat)         (const char* path);

    int         (*rename)       (const char* path, const char* newName);
    int         (*mkdir)        (const char* path);
    int         (*remove)       (const char* path);
    int         (*chdir)        (const char* path);

    DIREX       (*opendir)      (const char* path);
    FileInfo    (*readdir)      (DIREX handle);
    int         (*closedir)     (DIREX handle);
} ChildFS_t;

/*-----------------------------------------*/

static const ChildFS_t* currentChildAccessor;
typedef enum
{
    AccessIndexStart = 0,
    FatFSAccessIndex = AccessIndexStart,
#if DEBUGVFS
    DebugVFSAccessIndex,
#endif
    AccessIndexSize
}AccessIndexEnum;


static const ChildFS_t ChildAccessors[AccessIndexSize] =
{
 {
     .getEntryName = fatxgetActivePartName,
     .open = fatxopen,
     .read = fatxread,
     .write = fatxwrite,
     .close = fatxclose,
     .eof = fatxeof,
     .stat = fatxstat,
     .rename = fatxrename,
     .mkdir = fatxmkdir,
     .remove = fatxdelete,
     .chdir = fatxchdir,
     .opendir = fatxopendir,
     .readdir = fatxreaddir,
     .closedir = fatxclosedir,
 }
#if DEBUGVFS
 ,{
     .getEntryName = debugvfsgetEntryName,
     .open = debugvfsopen,
     .read = debugvfsread,
     .write = debugvfswrite,
     .close = debugvfsclose,
     .eof = debugvfseof,
     .stat = debugvfsstat,
     .rename = debugvfsrename,
     .mkdir = debugvfsmkdir,
     .remove = debugvfsremove,
     .chdir = debugvfschdir,
     .opendir = debugvfsopendir,
     .readdir = debugvfsreaddir,
     .closedir = debugvfsclosedir,
 }
#endif
};


#define MaxPathLength 255
static char cwd[MaxPathLength + sizeof('\0')];
const static DIREX VirtualRootDirHandle = INT_MAX;

static unsigned char virtualRootCycler;
static unsigned char tempAccessorIndex;

static int pathProcess_Absolute(const char* const path);
static int pathProcess_GoingForward(const char* const path);
static int pathProcess_GoingBack(void);

static void combinePath(char* out, const char* add);

static inline void setcwd(const char* const path);

/*-----------------------------------------*/

void VirtualRootInit(void)
{
    FatFS_init();
    memset(cwd, '\0', sizeof(char) * (MaxPathLength + sizeof('\0')));
    setcwd(PathSep);

    currentChildAccessor = NULL;

    virtualRootCycler = 0;
    tempAccessorIndex = AccessIndexStart;

    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "VirtualRoot init done.");
}

FILEX vroot_open(const char* path, FileOpenMode mode)
{
    char workPath[300];
    combinePath(workPath, path);
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "path:\"%s\"  mode:%u", workPath, mode);
    if(NULL != currentChildAccessor)
    {
        return currentChildAccessor->open(workPath, mode);
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!!  No file op in vroot.");

    return 0;
}

int vroot_read(FILEX handle, unsigned char* out, unsigned int size)
{
    if(NULL != currentChildAccessor)
    {
        return currentChildAccessor->read(handle, out, size);
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!!  No file op in vroot.");

    return -1;
}

int vroot_write(FILEX handle, const unsigned char* in, unsigned int size)
{
    if(NULL != currentChildAccessor)
    {
        return currentChildAccessor->write(handle, in, size);
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!!  No file op in vroot.");

    return -1;
}

void vroot_close(FILEX handle)
{
    if(NULL != currentChildAccessor)
    {
        currentChildAccessor->close(handle);
        return;
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!!  No file op in vroot.");

    return;
}

int vroot_eof(FILEX handle)
{
    if(NULL != currentChildAccessor)
    {
        return currentChildAccessor->eof(handle);
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!!  No file op in vroot.");

    return -1;
}

FileInfo vroot_stat(const char* path)
{
    FileInfo returnStruct;
    char workPath[300];
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "path:\"%s\"", path);

    if(NULL != currentChildAccessor)
    {
        combinePath(workPath, path);
        return currentChildAccessor->stat(workPath);
    }

    returnStruct.name[0] = '\0';
    returnStruct.nameLength = 0;
    returnStruct.attributes = FileAttr_Directory | FileAttr_ReadOnly | FileAttr_SysFile;
    returnStruct.size = 0;
    returnStruct.modDate = 0;
    returnStruct.modTime = 0;


    strcpy(returnStruct.name, path);
    returnStruct.nameLength = strlen(returnStruct.name);
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "found:\"%s\"", returnStruct.name);

    return returnStruct;
}

int vroot_rename(const char* path, const char* newName)
{
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "old:\"%s\" -> \"%s\"", path, newName);
    if(NULL != currentChildAccessor)
    {
        //TODO: Make sure newName has proper path */
        return currentChildAccessor->rename(path, newName);
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!!  No file op in vroot.");

    return -1;
}

int vroot_mkdir(const char* path)
{
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "path:\"%s\"", path);
    if(NULL != currentChildAccessor)
    {
        return currentChildAccessor->mkdir(path);
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!!  No file op in vroot.");

    return -1;
}

int vroot_remove(const char* path)
{
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "path:\"%s\"", path);
    if(NULL != currentChildAccessor)
    {
        return currentChildAccessor->remove(path);
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!!  No file op in vroot.");

    return -1;
}

int vroot_cd(const char* path)
{
    int result;
    char* sepPos;
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "path:\"%s\"", path);
    if(MaxPathLength < strlen(path))
    {
        XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "!!!Error, too long.");
        return -1;
    }

    if(cPathSep == *path)
    {
        /* Is an absolute path */
        return pathProcess_Absolute(path);
    }


    if(0 == strcmp(path, ".."))
    {
        return pathProcess_GoingBack();
    }
    else
    {
        /* Going forward */
        return pathProcess_GoingForward(path);
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!! Don't know how to process.");

    return -1;
}

const char* vroot_getcwd(void)
{
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "%s", cwd);
    return cwd;
}

DIREX vroot_opendir(const char* path)
{
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "path:\"%s\"", path);
    if(NULL != currentChildAccessor)
    {
        return currentChildAccessor->opendir(path);
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "Returning vroot handle.");

    return VirtualRootDirHandle;
}

FileInfo vroot_readdir(DIREX handle)
{
    FileInfo returnStruct;
    const char* string = NULL;

    if(VirtualRootDirHandle != handle && NULL != currentChildAccessor)
    {
        return currentChildAccessor->readdir(handle);
    }

    returnStruct.name[0] = '\0';
    returnStruct.nameLength = 0;
    returnStruct.attributes = FileAttr_Directory | FileAttr_ReadOnly | FileAttr_SysFile;
    returnStruct.size = 0;
    returnStruct.modDate = 0;
    returnStruct.modTime = 0;

    if(VirtualRootDirHandle == handle)
    {
        XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "readdir in vroot.");

        if(AccessIndexSize > tempAccessorIndex)
        {
            while(AccessIndexSize <= tempAccessorIndex)
            {
                virtualRootCycler = ChildAccessors[tempAccessorIndex].getEntryName(virtualRootCycler, &string);
                if(-1 != virtualRootCycler)
                {
                    break;
                }
                tempAccessorIndex++;
                virtualRootCycler = 0;
            }

            if(NULL != string)
            {
                sprintf(returnStruct.name, "%s", string);
                XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "found:\"%s\"", returnStruct.name);
                returnStruct.nameLength = strlen(returnStruct.name);
            }
        }
        else
        {
            virtualRootCycler = -1;
        }
    }

    return returnStruct;
}

void vroot_closedir(DIREX handle)
{
    if(NULL != currentChildAccessor)
    {
        currentChildAccessor->closedir(handle);
        return;
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "Close vroot handle.");

    virtualRootCycler = 0;
    tempAccessorIndex = AccessIndexStart;

    return;
}


/*-----------------------------------------------*/

static int pathProcess_Absolute(const char* const path)
{
    int result;
    unsigned char childAccessorIterator;
    const char* sepPos = path;
    if('\0' == path[1])
    {
        XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "Back to vroot.");
        setcwd(PathSep);
        currentChildAccessor = NULL;

        return 0;
    }

    for(childAccessorIterator = AccessIndexStart; childAccessorIterator < AccessIndexSize; childAccessorIterator++)
    {
        result = ChildAccessors[childAccessorIterator].chdir(sepPos);
        XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "chdir:\"%s\"   result:%u.", sepPos, result);
        if(0 == result)
        {
            currentChildAccessor = &ChildAccessors[childAccessorIterator];
            setcwd(path);
            return 0;
        }
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!! Invalid path.");

    return -1;
}

static int pathProcess_GoingForward(const char* const path)
{
    char workPath[300];
    unsigned char childAccessorIterator;

    /* Not an absolute path */
    if(cPathSep != *path)
    {
        combinePath(workPath, path);
        XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "New path:\"%s\".", workPath);
        for(childAccessorIterator = AccessIndexStart; childAccessorIterator < AccessIndexSize; childAccessorIterator++)
        {
            if(0 == ChildAccessors[childAccessorIterator].chdir(workPath))
            {
                currentChildAccessor = &ChildAccessors[childAccessorIterator];
                setcwd(workPath);

                return 0;
            }
        }
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!! Invalid path.");

    return -1;

}

static int pathProcess_GoingBack(void)
{
    char workPath[300];
    int pathLen, result;
    char* sepPos;

    strcpy(workPath, cwd);
    pathLen = strlen(workPath);
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "workPath:\"%s\"  len:%u", workPath, pathLen);

    sepPos = strrchr(workPath + sizeof(cPathSep), cPathSep);
    if(NULL == sepPos)
    {
        XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "!!!Error, requested root. Already at root.");
        /* Already at root */
        return -1;
    }

    *sepPos = '\0';
    sepPos = strrchr(workPath + sizeof(cPathSep), cPathSep);

    if(NULL == sepPos)
    {
        XBlastLogger(DEBUG_VROOT, DBG_LVL_WARN, "Invalid path. Returning to root.");
        currentChildAccessor = NULL;
        setcwd(PathSep);
        return 0;
    }

    sepPos[1] = '\0';
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "new path:\"%s\"", workPath);

    if(NULL != currentChildAccessor)
    {
        result = currentChildAccessor->chdir(workPath);

        if(0 == result)
        {
            XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "Sucess. Path exist in FatFS");
            setcwd(workPath);
        }
        else
        {
            currentChildAccessor = NULL;
            setcwd(PathSep);
        }
    }
    XBlastLogger(DEBUG_VROOT, DBG_LVL_ERROR, "Error!!! Invalid path.");

    return -1;
}

static void combinePath(char* out, const char* add)
{
    int cwdLen = strlen(cwd);
    if(cwdLen)
    {
        if(cPathSep == cwd[cwdLen - 1])
        {
            sprintf(out, "%s%s", cwd, add);
            return;
        }
    }
    sprintf(out, "%s"PathSep"%s", cwd, add);
}

static inline void setcwd(const char* const path)
{
    XBlastLogger(DEBUG_VROOT, DBG_LVL_DEBUG, "new cwd:\"%s\"", path);
    strcpy(cwd, path);
}

