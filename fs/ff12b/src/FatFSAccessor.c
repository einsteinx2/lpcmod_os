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

FATFS FatXFs[NbDrivesSupported][NbFATXPartPerHDD];      /* File system object for logical drive */

#if _USE_FASTSEEK
DWORD SeekTbl[16];          /* Link map table for fast seek feature */
#endif

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

int isMounted(unsigned char driveId, unsigned char partitionNumber)
{
    if(NbDrivesSupported <= driveId)
    {
        return -1;
    }

    if(NbFATXPartPerHDD <= partitionNumber)
    {
        return -1;
    }

    return FatXFs[driveId][partitionNumber].fs_typex;
}
