/*
 * fatx.h
 *
 *  Created on: Feb 21, 2017
 *      Author: cromwelldev
 */

#ifndef FATX_H_
#define FATX_H_

#define XBOX_EXTEND_STARTLBA   (0x00EE8AB0UL)
#define XBOX_STORE_STARTLBA    (0x0055F400UL)
#define XBOX_SYSTEM_STARTLBA    (0x00465400UL)
#define XBOX_MBRTABLE_STARTLBA    (0x00000000UL)
#define XBOX_CACHE1_STARTLBA    (0x00000400UL)
#define XBOX_CACHE2_STARTLBA    (0x00177400UL)
#define XBOX_CACHE3_STARTLBA    (0x002EE400UL)

#define MBRTABLE_LBASIZE   (XBOX_CACHE1_STARTLBA - XBOX_MBRTABLE_STARTLBA)
#define STORE_LBASIZE    (XBOX_EXTEND_STARTLBA - XBOX_STORE_STARTLBA)         //0x9896B0
#define SYSTEM_LBASIZE    (XBOX_STORE_STARTLBA  - XBOX_SYSTEM_STARTLBA)
#define CACHE1_LBASIZE    (XBOX_CACHE2_STARTLBA - XBOX_CACHE1_STARTLBA)
#define CACHE2_LBASIZE    (XBOX_CACHE3_STARTLBA - XBOX_CACHE2_STARTLBA)
#define CACHE3_LBASIZE    (XBOX_SYSTEM_STARTLBA - XBOX_CACHE3_STARTLBA)

#define LBASIZE_512GB   1073741824UL                      //Switch to 64K clusters beyond that
#define LBASIZE_1024GB  2147483645UL                      //Max LBA size supported by Xbox
#define LBASIZE_256GB   536870912UL                       //Switch to 32K clusters beyond that
#define LBASIZE_137GB   (0x0FFFFFFFUL - XBOX_EXTEND_STARTLBA)     //LBA28 limited F: drive size.

#define FATX16_MAXLBA   2096800UL                         //Max number of sectors possible of a FATX16 partition. Higher than that is FATX32.

#define FATX_DRIVE_MAGIC "BRFR"

/*Taken from XBPartitionner*/
// This flag (part of PARTITION_ENTRY.pe_flags) tells you whether/not a
// partition is being used (whether/not drive G is active, for example)
#define FATX_PE_PARTFLAGS_IN_USE 0x80000000


// Size of FATX partition header (boot sector/superblock)
#define FATX_PARTITION_HEADERSIZE 0x1000

// FATX partition magic
#define FATX_PARTITION_MAGIC 0x58544146         //"FATX" in ASCII.

// FATX chain table block size
#define FATX_CHAINTABLE_BLOCKSIZE 4096

// ID of the root FAT cluster
#define FATX_ROOT_FAT_CLUSTER 1

// Size of FATX directory entries
#define FATX_DIRECTORYENTRY_SIZE 0x40

// File attribute: read only
#define FATX_FILEATTR_READONLY 0x01
// File attribute: hidden
#define FATX_FILEATTR_HIDDEN 0x02
// File attribute: system
#define FATX_FILEATTR_SYSTEM 0x04
// File attribute: archive
#define FATX_FILEATTR_ARCHIVE 0x20
// Directory entry flag indicating entry is a sub-directory
#define FATX_FILEATTR_DIRECTORY 0x10

// max filename size
#define FATX_FILENAME_MAX 42

typedef struct
{
        unsigned char Name[16];
        unsigned int Flags;
        unsigned int LBAStart;
        unsigned int LBASize;
        unsigned int Reserved;
} XboxPartitionTableEntry;

typedef struct
{
        unsigned char   Magic[16];
        char    Res0[32];
        XboxPartitionTableEntry TableEntries[14];
} XboxPartitionTable;

//Taken from ReactOS' vfat.h source and Xbox-Linux archives.
typedef struct {
    unsigned char FilenameLength;
    unsigned char Attrib;
    char Filename[42];
    unsigned int FirstCluster;
    unsigned int FileSize;
    unsigned short UpdateTime;
    unsigned short UpdateDate;
    unsigned short CreationTime;
    unsigned short CreationDate;
    unsigned short AccessTime;
    unsigned short AccessDate;
}__attribute__((packed)) FATXDIRINFO;                   //For a total of 64 bytes.

const unsigned char NbFATXPartPerHDD = 7;

#define ISFATX_FS(fs) (fs == FS_FATX16 || fs == FS_FATX32)
#define NOTFATX_FS(fs) (fs != FS_FATX16 && fs != FS_FATX32)

#endif /* FATX_H_ */
