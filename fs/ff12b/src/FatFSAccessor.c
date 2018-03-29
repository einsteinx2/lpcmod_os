/*
 * PartitionTable.c
 *
 *  Created on: Mar 27, 2018
 *      Author: cromwelldev
 */

#include "ff.h"
#include "string.h"
#include "BootIde.h"
#include "FatFSAccessor.h"
#include "lib/LPCMod/xblastDebug.h"

/*---------------------------------------------------------------*/
/* Private static variables */
/*---------------------------------------------------------------*/
static FATFS FatXFs[NbDrivesSupported][NbFATXPartPerHDD];      /* File system object for logical drive */

#if _USE_FASTSEEK
static DWORD SeekTbl[16];          /* Link map table for fast seek feature */
#endif

#define MaxOpenFileCount 2
static FIL FileHandleArray[MaxOpenFileCount];

#define MaxOpenDirCount 2
static DIR DirectoryHandleArray[MaxOpenDirCount];

#define MaxPathLength 255
static char cwd[MaxPathLength];

/*---------------------------------------------------------------*/

PARTITION VolToPart[] =
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

void FatFS_init(void)
{
    unsigned char i;
    memset(FatXFs, 0x00, sizeof(FATFS) * NbFATXPartPerHDD * NbDrivesSupported);
    memset(SeekTbl, 0x00, sizeof(DWORD) * 16);
    memset(FileHandleArray, 0x00, sizeof(FIL) * MaxOpenFileCount);
    memset(DirectoryHandleArray, 0x00, sizeof(DIR) * MaxOpenDirCount);
    memset(cwd, '\0', sizeof(char) * MaxPathLength);
    fatx_init();

    for(i = 0; i < NbDrivesSupported; i++)
    {
        if(tsaHarddiskInfo[i].m_fDriveExists && 0 == tsaHarddiskInfo[i].m_fAtapi)
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
    const char* const partNames[] = { _VOLUME_STRS };
    FRESULT result;

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
                        debugSPIPrint(DEBUG_FATX_FS, "Mounting \"%s\" partition\n", partNames[driveNumber][i]);
                        //TODO: Constant for mount immediately flag.
                        result = f_mount(&FatXFs[driveNumber][i], &partNames[driveNumber][i] , 1);
                        if(FR_OK == result)
                        {
                            debugSPIPrint(DEBUG_FATX_FS, "Mount \"%s\" partition success!\n", partNames[driveNumber][i]);
                        }
                        else
                        {
                            debugSPIPrint(DEBUG_FATX_FS, "Error! Mount \"%s\" partition. Code: %u!\n", partNames[driveNumber][i], result);
                        }
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
    int result;
    XboxPartitionTable workingMbr;
    unsigned long long diskSize;
    //XXX: Assume 512Bytes sectors for now.

    if(NbDrivesSupported <= driveNumber)
    {
        return -1;
    }

    diskSize = BootIdeGetSectorCount(driveNumber);

    if(((XBOX_EXTEND_STARTLBA - 1) > diskSize) || (XBOX_EXTEND_STARTLBA >= diskSize && XboxDiskLayout_Base != xboxDiskLayout))
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
        if(diskSize - XBOX_EXTEND_STARTLBA < FATX_MIN_PART_SIZE_LBA)
        {
            return -1;
        }

        workingMbr.TableEntries[5].LBAStart = XBOX_EXTEND_STARTLBA;
        workingMbr.TableEntries[5].LBASize = diskSize - XBOX_EXTEND_STARTLBA;
        /* Do not set Part in use flag here, in case mkfs doesn't go through. mkfs is supposed to set it. */
        break;
    case XboxDiskLayout_F120GRest:
        break;
    case XboxDiskLayout_FGSplit:
        break;
    default:
        result = -1;
        break;
    }

    return fatx_fdisk(driveNumber, &workingMbr);
}

int fatxmkfs(unsigned char driveNumber, unsigned char partNumber)
{
    return 0;
}


FILE fopen(const char* path, FileOpenMode mode)
{
    return 0;
}

int fclose(FILE handle)
{
    return 0;
}

int fread(FILE handle, unsigned char* out, unsigned int size)
{
    return 0;
}

int fwrite(FILE handle, const unsigned char* in, unsigned int size)
{
    return 0;
}

int fseek(FILE handle, unsigned int offset)
{
    return 0;
}

int fsync(FILE handle)
{
    return 0;
}

FileInfo fstat(const char* path)
{
    FileInfo returnStruct;
    return returnStruct;
}


int mkdir(const char* path)
{
    return 0;
}

DIRE fopendir(const char* path)
{
    return 0;
}

FileInfo freaddir(DIRE handle)
{
    FileInfo returnStruct;
    return returnStruct;
}

DIRE findfirst(FileInfo* out, const char* path, const char* pattern)
{
    return 0;
}

int findnext(DIRE handle, FileInfo* out)
{
    return 0;
}

int frewinddir(DIRE handle)
{
    return 0;
}

int fclosedir(DIRE handle)
{
    return 0;
}


int fdelete(const char* path)
{
    return 0;
}

int frename(const char* path, const char* newName)
{
    return 0;
}


int fchdir(const char* path)
{
    return 0;
}

int fchdrive(const char* path)
{
    return 0;
}

const char* getcwd(void)
{
    return NULL;
}


int fgetfree(const char* path)
{
    return 0;
}

int getclustersize(unsigned char driveNumber, unsigned char partNumber)
{
    return 0;
}


int fputc(FILE handle, char c)
{
    return 0;
}

int fputs(FILE handle, const char* sz)
{
    return 0;
}

int fprintf(FILE handle, const char* sz, ...)
{
    return 0;
}

int fgets(FILE handle, unsigned int len)
{
    return 0;
}

int ftell(FILE handle)
{
    return 0;
}

int feof(FILE handle)
{
    return 0;
}

int fsize(FILE handle)
{
    return 0;
}
