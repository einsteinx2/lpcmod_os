/*
 * PartitionTable.c
 *
 *  Created on: Mar 27, 2018
 *      Author: cromwelldev
 */

#include "PartTable.h"
#include "ff.h"
#include "string.h"

const unsigned char NbFATXPartPerHDD = 7;

FATFS FatXFs[_VOLUMES];      /* File system object for logical drive */

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
    memset(FatXFs, 0x00, sizeof(FATFS) * _VOLUMES);
    fatx_init();

    if(tsaHarddiskInfo[0].m_fDriveExists && 0 == tsaHarddiskInfo[0].m_fAtapi)
    {
        if(FR_OK == fatx_getbrfr(0))
        {
            mountBasic(0);
        }
    }
}

void mountBasic(unsigned char driveNumber)
{

}
