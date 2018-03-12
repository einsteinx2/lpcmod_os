/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#define _FILE_OFFSET_BITS 64
#define __USE_LARGEFILE

#include "diskio.h"		/* FatFs lower layer API */
#include <stdio.h>
#include <errno.h>


typedef struct {
    DSTATUS status;
    WORD sz_sector;
    QWORD n_sectors;
    HANDLE h_drive;
} STAT;

static volatile STAT diskImage;

int assign_drives(const char* diskImageName)
{
    errno = 0;
    diskImage.h_drive = fopen(diskImageName, "r+b");
    if(diskImage.h_drive)
    {
        disk_status(0);
        return 0;
    }
    else
    {
        printf("Image Open Err = %d\n", errno);
    }

    return 1;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    diskImage.status = 0;
    diskImage.sz_sector = SECTOR_SIZE;
    fseeko(diskImage.h_drive, 0L, SEEK_END);
    QWORD size = ftello(diskImage.h_drive);
    diskImage.n_sectors = size / (unsigned int)SECTOR_SIZE;
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DRESULT disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    disk_status(pdrv);
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{

    if(0 == fseeko(diskImage.h_drive, sector * SECTOR_SIZE, SEEK_SET))
    {
        if(count == fread(buff, SECTOR_SIZE, count, diskImage.h_drive))
        {
            return RES_OK;
        }
        return RES_ERROR;
    }


	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{

    if(0 == fseeko(diskImage.h_drive, sector * SECTOR_SIZE, SEEK_SET))
    {
        errno = 0;
        if(count == fwrite(buff, SECTOR_SIZE, count, diskImage.h_drive))
        {
            return RES_OK;
        }
        else
        {
            printf("Image Write Err = %d\n", errno);
        }
        return RES_ERROR;
    }


	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res = RES_PARERR;
    switch (cmd) {
    case CTRL_SYNC:         /* Nothing to do */
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT:  /* Get number of sectors on the drive */
        *(DWORD*)buff = diskImage.n_sectors;
        res = RES_OK;
        break;

    case GET_SECTOR_SIZE:   /* Get size of sector for generic read/write */
        *(WORD*)buff = diskImage.sz_sector;
        res = RES_OK;
        break;

    case GET_BLOCK_SIZE:    /* Get internal block size in unit of sector */
        *(DWORD*)buff = SZ_BLOCK;
        res = RES_OK;
        break;

    }

    return res;

}

