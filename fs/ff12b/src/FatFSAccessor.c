/*
 * PartitionTable.c
 *
 *  Created on: Mar 27, 2018
 *      Author: cromwelldev
 */

//#include "ff.h"
#include "FatFSAccessor.h"
#ifdef _PCSIM
#include <string.h>
#include <stdio.h>
#define DEBUG_FATX_FS 0
#define debugSPIPrint(activate,...)
#include "../../../pc_tools/fatfs_test/src/FatFSTestHelper.h"
#else
#include "string.h"
#include "BootIde.h"
#include "lib/LPCMod/xblastDebug.h"
#endif
#include <stdarg.h>

/*---------------------------------------------------------------*/
/* Private static variables */
/*---------------------------------------------------------------*/
static FATFS FatXFs[NbDrivesSupported][NbFATXPartPerHDD];      /* File system object for logical drive */

#if _USE_FASTSEEK
static DWORD SeekTbl[16];          /* Link map table for fast seek feature */
#endif

#define MaxOpenFileCount (_FS_LOCK / 2)
static FIL FileHandleArray[MaxOpenFileCount];

#define MaxOpenDirCount (_FS_LOCK / 2)
static DIR DirectoryHandleArray[MaxOpenDirCount];

#define MaxPathLength 255
static char cwd[MaxPathLength + sizeof('\0')];

static const char* const PartitionNameList[_VOLUMES] = { _VOLUME_STRS };


/*---------------------------------------------------------------*/

const char* const * const PartitionNameStrings[NbDrivesSupported] =
{
 (const char* const*)&PartitionNameList[0],
 (const char* const*)&PartitionNameList[_VOLUMES / 2]
};

PARTITION VolToPart[_VOLUMES] =
{
 {0, 0}, /* XBOX SHELL, E:\ */
 {0, 1}, /* XBOX DATA, C:\ */
 {0, 2}, /* XBOX GAME SWAP 1, X:\ */
 {0, 3}, /* XBOX GAME SWAP 2, Y:\ */
 {0, 4}, /* XBOX GAME SWAP 3, Z:\ */
 {0, 5}, /* XBOX F, F:\ */
 {0, 6}, /* XBOX G, G:\ */
 {1, 0}, /* XBOX SHELL, E:\ */
 {1, 1}, /* XBOX DATA, C:\ */
 {1, 2}, /* XBOX GAME SWAP 1, X:\ */
 {1, 3}, /* XBOX GAME SWAP 2, Y:\ */
 {1, 4}, /* XBOX GAME SWAP 3, Z:\ */
 {1, 5}, /* XBOX F, F:\ */
 {1, 6}  /* XBOX G, G:\ */
};

#define FILE_VALID(x) if((MaxOpenFileCount <= x) || (0 == FileHandleArray[x].obj.fs)) return -1;
#define DIRE_VALID(x, y) memset(&y, 0x00, sizeof(FileInfo)); \
                        if((MaxOpenDirCount <= x) || (0 == DirectoryHandleArray[x].obj.fs)) return y;

#define FILE_HANDLE_VALID(x) if(0 == handle || (MaxOpenFileCount < handle)) return -1; x -= 1;
#define DIRE_HANDLE_VALID(x) if(0 == handle || (MaxOpenDirCount < handle)) return -1; x -= 1;

extern
int get_ldnumber (      /* Returns logical drive number (-1:invalid drive) */
    const TCHAR** path  /* Pointer to pointer to the path name */

);

static void convertToFileInfo(FileInfo* out, const FILINFO* in)
{
    out->nameLength = in->namelength;
    memcpy(out->name, in->fnamex, 42 > out->nameLength ? 42 : out->nameLength);
    out->attributes = in->fattrib;
    out->size = in->fsize;
    out->modDate= in->fdate;
    out->modTime = in->ftime;
}

void FatFS_init(void)
{
    unsigned char i;
    memset(FatXFs, 0x00, sizeof(FATFS) * NbFATXPartPerHDD * NbDrivesSupported);
#if _USE_FASTSEEK
    memset(SeekTbl, 0x00, sizeof(DWORD) * 16);
#endif
    memset(FileHandleArray, 0x00, sizeof(FIL) * MaxOpenFileCount);
    memset(DirectoryHandleArray, 0x00, sizeof(DIR) * MaxOpenDirCount);
    memset(cwd, '\0', sizeof(char) * (MaxPathLength + sizeof('\0')));
    fatx_init();

    for(i = 0; i < NbDrivesSupported; i++)
    {
        if(BootIdeDeviceConnected(i) && 0 == BootIdeDeviceIsATAPI(i))
        {
            mountAll(i);
        }
    }
}

/* Will mount C, E, F, G, X, Y, Z if available*/
int mountAll(unsigned char driveNumber)
{
    unsigned char i;
    XboxPartitionTable tempTable;

    if(driveNumber >= NbDrivesSupported)
    {
        return -1;
    }
    debugSPIPrint(DEBUG_FATX_FS, "Attempting to mount base partitions for drive %u\n", driveNumber);

    if(FR_OK == fatx_getbrfr(driveNumber))
    {
        if(FR_OK == fatx_getmbr(driveNumber, &tempTable))
        {
            //TODO: constant for number of standard partitions.
            for(i = 0; i < NbFATXPartPerHDD; i++)
            {
                debugSPIPrint(DEBUG_FATX_FS, "Drive: %u, PartIndex: %u, mountStatus: %u\n", driveNumber, i);
                if(0 == FatXFs[driveNumber][i].fs_typex)
                {
                    debugSPIPrint(DEBUG_FATX_FS, "PartFlag: 0x%08X\n", tempTable.TableEntries[i].Flags);
                    if(tempTable.TableEntries[i].Flags & FATX_PE_PARTFLAGS_IN_USE)
                    {
                        fatxmount(driveNumber, i);
                    }
                }
            }
        }
        else
        {
            debugSPIPrint(DEBUG_FATX_FS, "Error! No MBR.\n");
        }
    }
    else
    {
        debugSPIPrint(DEBUG_FATX_FS, "Error! No BRFR.\n");
    }

    return 0;
}

int fatxmount(unsigned char driveNumber, unsigned char partitionNumber)
{
    int result;
    char mountStr[20];

    sprintf(mountStr, "%s%s", PartitionNameStrings[driveNumber][partitionNumber], ":\\");

    debugSPIPrint(DEBUG_FATX_FS, "Mounting \"%s\" partition\n", PartitionNameStrings[driveNumber][partitionNumber]);
    //TODO: Constant for mount immediately flag.
    result = f_mount(&FatXFs[driveNumber][partitionNumber], mountStr, 1);
    if(FR_OK == result)
    {
        debugSPIPrint(DEBUG_FATX_FS, "Mount \"%s\" partition success!\n", PartitionNameStrings[driveNumber][partitionNumber]);
    }
    else
    {
        debugSPIPrint(DEBUG_FATX_FS, "Error! Mount \"%s\" partition. Code: %u!\n", PartitionNameStrings[driveNumber][partitionNumber], result);
    }

    return result;
}

int isMounted(unsigned char driveNumber, unsigned char partitionNumber)
{
    if(NbDrivesSupported <= driveNumber)
    {
        return -1;
    }

    if(NbFATXPartPerHDD <= partitionNumber)
    {
        return -1;
    }

    return FatXFs[driveNumber][partitionNumber].fs_typex;
}

int fdisk(unsigned char driveNumber, XboxDiskLayout xboxDiskLayout)
{
    XboxPartitionTable workingMbr;
    unsigned long long diskSizeLba;
    unsigned int fDriveLbaSize;
    //XXX: Assume 512Bytes sectors for now.

    if(NbDrivesSupported <= driveNumber)
    {
        return -1;
    }

    diskSizeLba = BootIdeGetSectorCount(driveNumber);

    if(((XBOX_EXTEND_STARTLBA - 1) > diskSizeLba) || (XBOX_EXTEND_STARTLBA >= diskSizeLba && XboxDiskLayout_Base != xboxDiskLayout))
    {
        return -1;
    }

    if(fatx_getmbr(driveNumber, &workingMbr))
    {
        return -1;
    }

    switch(xboxDiskLayout)
    {
    case XboxDiskLayout_Base:
        memcpy(&workingMbr, &BackupPartTable, sizeof(XboxPartitionTable));
        break;
    case XboxDiskLayout_FOnly:
        if(diskSizeLba - XBOX_EXTEND_STARTLBA < FATX_MIN_PART_SIZE_LBA)
        {
            return -1;
        }

        workingMbr.TableEntries[5].LBAStart = XBOX_EXTEND_STARTLBA;
        workingMbr.TableEntries[5].LBASize = diskSizeLba - XBOX_EXTEND_STARTLBA;
        workingMbr.TableEntries[5].Flags = 0;
        /* Do not set Part in use flag here, in case mkfs doesn't go through. mkfs is supposed to set it. */
        workingMbr.TableEntries[6].LBAStart = 0;
        workingMbr.TableEntries[6].LBASize = 0;
        workingMbr.TableEntries[6].Flags = 0;
        break;
    case XboxDiskLayout_F120GRest:
        workingMbr.TableEntries[5].LBAStart = XBOX_EXTEND_STARTLBA;
        workingMbr.TableEntries[5].LBASize = LBASIZE_137GB;
        workingMbr.TableEntries[5].Flags = 0;
        workingMbr.TableEntries[6].LBAStart = LBA28_BOUNDARY;
        workingMbr.TableEntries[6].LBASize = diskSizeLba - LBA28_BOUNDARY;
        workingMbr.TableEntries[6].Flags = 0;
        break;
    case XboxDiskLayout_FGSplit:
        diskSizeLba -= XBOX_EXTEND_STARTLBA;
        diskSizeLba &=  ~((unsigned long long)(FATX_CHAINTABLE_BLOCKSIZE / 8 - 1));
        fDriveLbaSize = diskSizeLba / 2;

        if(LBASIZE_1024GB >= fDriveLbaSize)
        {
            fDriveLbaSize = LBASIZE_1024GB;
        }
        else
        {
            if(LBASIZE_1024GB >= fDriveLbaSize)
            {
                /* Check with 64KB clusters */
                fDriveLbaSize &= ~((unsigned long long)(FATX_MAX_CLUSTERSIZE_INSECTORS - 1));
            }

            if(LBASIZE_512GB >= fDriveLbaSize)
            {
                /* Check with 32KB clusters */
                fDriveLbaSize &= ~((unsigned long long)(FATX_MID_CLUSTERSIZE_INSECTORS - 1));
            }

            if(LBASIZE_256GB >= fDriveLbaSize)
            {
                /* Check with 16KB clusters */
                fDriveLbaSize &= ~((unsigned long long)(FATX_MIN_CLUSTERSIZE_INSECTORS - 1));
            }
        }
        //TODO: Calculate optimal partition size for F drive depending on cluster size
        workingMbr.TableEntries[5].LBAStart = XBOX_EXTEND_STARTLBA;
        workingMbr.TableEntries[5].LBASize = fDriveLbaSize;
        workingMbr.TableEntries[5].Flags = 0;
        workingMbr.TableEntries[6].LBAStart = XBOX_EXTEND_STARTLBA + fDriveLbaSize;
        workingMbr.TableEntries[6].LBASize = diskSizeLba - (XBOX_EXTEND_STARTLBA + fDriveLbaSize);
        workingMbr.TableEntries[6].Flags = 0;
        break;
    default:
        return -1;
        break;
    }

    return fatx_fdisk(driveNumber, &workingMbr);
}

int fatxmkfs(unsigned char driveNumber, unsigned char partNumber)
{
    XboxPartitionTable workingMbr;
    unsigned char workBuf[FATX_CHAINTABLE_BLOCKSIZE];
    char partName[22];
    sprintf(partName, "%s:\\", PartitionNameStrings[driveNumber][partNumber]);

    if(FR_OK != fatx_getmbr(driveNumber, &workingMbr))
    {
        return -1;
    }

    if(FR_OK != f_mkfs(partName, FM_FATXANY, FATX_MIN_CLUSTERSIZE_INSECTORS, workBuf, FATX_CHAINTABLE_BLOCKSIZE))
    {
        return -1;
    }

    return 0;
}


FILEX fatxopen(const char* path, FileOpenMode mode)
{
    unsigned char i;

    /* Find unused File descriptor in array */
    for (i = 0; i < MaxOpenFileCount; i++)
    {
        if(0 == FileHandleArray[i].obj.fs)
        {
            break;
        }
    }

    if(MaxOpenFileCount == i)
    {
        return 0;
    }

    if(FR_OK != f_open(&FileHandleArray[i], path, mode))
    {
        return 0;
    }

    return i + 1;
}

int fatxclose(FILEX handle)
{
    FILE_HANDLE_VALID(handle)

    if(0 != FileHandleArray[handle].obj.fs)
    {
        return f_close(&FileHandleArray[handle]);
    }
    return -1;
}

int fatxread(FILEX handle, unsigned char* out, unsigned int size)
{
    UINT bytesRead;

    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    if(FR_OK == f_read(&FileHandleArray[handle], out, size, &bytesRead))
    {
        return bytesRead;
    }

    return -1;
}

int fatxwrite(FILEX handle, const unsigned char* in, unsigned int size)
{
    UINT bytesWrote;

    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    if(FR_OK == f_write(&FileHandleArray[handle], in, size, &bytesWrote))
    {
        return bytesWrote;
    }

    return -1;
}

int fatxseek(FILEX handle, unsigned int offset)
{
    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    if(FR_OK != f_lseek(&FileHandleArray[handle], offset))
    {
        return -1;
    }

    return 0;
}

int fatxsync(FILEX handle)
{
    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    if(FR_OK != f_sync(&FileHandleArray[handle]))
    {
        return -1;
    }

    return 0;
}

FileInfo fatxstat(const char* path)
{
    FileInfo returnStruct;
    FILINFO getter;
    memset(&returnStruct, 0x00, sizeof(FileInfo));
    if(FR_OK == f_stat(path, &getter))
    {
        convertToFileInfo(&returnStruct, &getter);
    }

    return returnStruct;
}


int fatxmkdir(const char* path)
{
    if(FR_OK != f_mkdir(path))
    {
        return -1;
    }

    return 0;
}

DIRE fatxopendir(const char* path)
{
    unsigned char i;

    /* Find unused Directory descriptor in array */
    for (i = 0; i < MaxOpenDirCount; i++)
    {
        if(0 == DirectoryHandleArray[i].obj.fs)
        {
            break;
        }
    }

    if(MaxOpenDirCount == i)
    {
        return 0;
    }

    if(FR_OK != f_opendir(&DirectoryHandleArray[i], path))
    {
        return 0;
    }

    return i + 1;
}

FileInfo fatxreaddir(DIRE handle)
{
    FileInfo returnStruct;
    FILINFO getter;

    if(0 == handle || (MaxOpenDirCount < handle))
    {
        /* Invalidate handle */
        handle = 255;
    }
    else
    {
        handle -= 1;
    }

    DIRE_VALID(handle, returnStruct)

    if(FR_OK == f_readdir(&DirectoryHandleArray[handle], &getter))
    {
        convertToFileInfo(&returnStruct, &getter);
    }

    return returnStruct;
}

DIRE fatxfindfirst(FileInfo* out, const char* path, const char* pattern)
{
    unsigned char i;
    FILINFO getter;
    memset(out, 0x00, sizeof(FileInfo));

    /* Find unused Directory descriptor in array */
    for (i = 0; i < MaxOpenDirCount; i++)
    {
        if(0 == DirectoryHandleArray[i].obj.fs)
        {
            break;
        }
    }

    if(MaxOpenDirCount == i)
    {
        return 0;
    }

    if(FR_OK == f_findfirst(&DirectoryHandleArray[i], &getter, path, pattern))
    {
        convertToFileInfo(out, &getter);
    }

    return i + 1;
}

int fatxfindnext(DIRE handle, FileInfo* out)
{
    FILINFO getter;

    DIRE_HANDLE_VALID(handle)
    memset(out, 0x00, sizeof(FileInfo));

    if(0 == DirectoryHandleArray[handle].obj.fs)
    {
        return -1;
    }

    if(FR_OK == f_findnext(&DirectoryHandleArray[handle], &getter))
    {
        convertToFileInfo(out, &getter);

        return 0;
    }

    return -1;
}

int fatxrewinddir(DIRE handle)
{
    DIRE_HANDLE_VALID(handle)
    if(0 == DirectoryHandleArray[handle].obj.fs)
    {
        return -1;
    }

    if(FR_OK != f_rewinddir(&DirectoryHandleArray[handle]))
    {
        return -1;
    }

    return 0;
}

int fatxclosedir(DIRE handle)
{
    DIRE_HANDLE_VALID(handle)
    if(0 == DirectoryHandleArray[handle].obj.fs)
    {
        return -1;
    }

    if(FR_OK != f_closedir(&DirectoryHandleArray[handle]))
    {
        return -1;
    }

    return 0;
}


int fatxdelete(const char* path)
{
    if(FR_OK != f_unlink(path))
    {
        return -1;
    }

    return 0;
}

int fatxrename(const char* path, const char* newName)
{
    if(strlen(newName) > 42)
    {
        return -1;
    }

    if(FR_OK != f_rename(path, newName))
    {
        return -1;
    }

    return 0;
}


int fatxchdir(const char* path)
{
    if(MaxPathLength < strlen(path))
    {
        return -1;
    }

    if(FR_OK == f_chdir(path))
    {
        sprintf(cwd, "%s", path);
        return 0;
    }

    return -1;
}

int fatxchdrive(const char* path)
{
    /* Not useful for the moment. */
    return -1;
}

const char* fatxgetcwd(void)
{
    return cwd;
}


int fatxgetfree(const char* path)
{
    DWORD clusters;

    int vol = get_ldnumber(&path);
    FATFS* fatfs = &FatXFs[0][vol];
    if(0 > vol)
    {
        return -1;
    }

    if(FR_OK == f_getfree(path, &clusters, &fatfs))
    {
        return clusters;
    }

    return -1;
}

int fatxgetclustersize(unsigned char driveNumber, unsigned char partNumber)
{
    if(NbDrivesSupported <= driveNumber || NbFATXPartPerHDD <= partNumber)
    {
        return -1;
    }

    return FatXFs[driveNumber][partNumber].csize;
}


int fatxputc(FILEX handle, char c)
{
    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    return f_putc(c, &FileHandleArray[handle]);
}

int fatxputs(FILEX handle, const char* sz)
{
    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    return f_puts(sz, &FileHandleArray[handle]);
}

int fatxprintf(FILEX handle, const char* sz, ...)
{
    int wrote;
    va_list args;

    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    va_start( args, sz );
    wrote = f_printf(&FileHandleArray[handle], sz, args);
    va_end( args );


    return wrote;
}

int fatxgets(FILEX handle, char* out, unsigned int len)
{
    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    f_gets(out, len, &FileHandleArray[handle]);

    return 0;
}

long long int fatxtell(FILEX handle)
{
    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    return f_tell(&FileHandleArray[handle]);
}

int fatxeof(FILEX handle)
{
    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    if(f_eof(&FileHandleArray[handle]))
    {
        /* At end of file */
        return 0;
    }

    return 1;
}

long long int fatxsize(FILEX handle)
{
    FILE_HANDLE_VALID(handle)
    FILE_VALID(handle)

    return f_size(&FileHandleArray[handle]);
}

