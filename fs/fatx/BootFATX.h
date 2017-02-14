#ifndef _BootFATX_H_
#define _BootFATX_H_

// Definitions for FATX on-disk structures
// (c) 2001 Andrew de Quincey

#include <stdbool.h>


#define STORE_SIZE    (0x131F00000ULL)
#define SYSTEM_SIZE    (0x1f400000UL)
#define CACHE1_SIZE    (0x2ee80000UL)
#define CACHE2_SIZE    (0x2ee80000UL)
#define CACHE3_SIZE    (0x2ee80000UL)

#define SECTOR_EXTEND   (0x00EE8AB0UL)
#define SECTOR_STORE    (0x0055F400UL)
#define SECTOR_SYSTEM    (0x00465400UL)
#define SECTOR_CONFIG    (0x00000000UL)
#define SECTOR_CACHE1    (0x00000400UL)
#define SECTOR_CACHE2    (0x00177400UL)
#define SECTOR_CACHE3    (0x002EE400UL)

#define SECTORD_CONFIG	 (SECTOR_CACHE1 - SECTOR_CONFIG)
#define SECTORS_STORE    (SECTOR_EXTEND - SECTOR_STORE)         //0x9896B0
#define SECTORS_SYSTEM    (SECTOR_STORE  - SECTOR_SYSTEM)
#define SECTORS_CACHE1    (SECTOR_CACHE2 - SECTOR_CACHE1)
#define SECTORS_CACHE2    (SECTOR_CACHE3 - SECTOR_CACHE2)
#define SECTORS_CACHE3    (SECTOR_SYSTEM - SECTOR_CACHE3)

#define LBASIZE_512GB   1073741824UL                      //Switch to 64K clusters beyond that
#define LBASIZE_1024GB  2147483645UL                      //Max LBA size supported by Xbox
#define LBASIZE_256GB   536870912UL                       //Switch to 32K clusters beyond that
#define LBASIZE_137GB   (0x0FFFFFFFUL - SECTOR_EXTEND)     //LBA28 limited F: drive size.

#define FATX16_MAXLBA   2096800UL                         //Max number of sectors possible of a FATX16 partition. Higher than that is FATX32.

/*Taken from XBPartitionner*/
// This flag (part of PARTITION_ENTRY.pe_flags) tells you whether/not a
// partition is being used (whether/not drive G is active, for example)
#define PE_PARTFLAGS_IN_USE 0x80000000


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

//Default number of retry if Write to disc fails.
#define DEFAULT_WRITE_RETRY     3

#define FATX16CLUSTERSIZE 16384

// This structure describes a FATX partition
typedef struct {

  int nDriveIndex;
 
  // The starting byte of the partition
  unsigned long long partitionStart;

  // The size of the partition in bytes
  unsigned long long partitionSize;

  // The cluster size of the partition
  unsigned long clusterSize;

  // Number of clusters in the partition
  unsigned long clusterCount;

  // Size of entries in the cluster chain map
  unsigned long chainMapEntrySize;

  // The cluster chain map table (which may be in words OR dwords)
  union {
    unsigned short *words;
    unsigned long *dwords;
  } clusterChainMap;
  
  // Address of cluster 1
  unsigned long long cluster1Address;
  
} FATXPartition;

typedef struct {                                        //Also known as FATX SuperBlock.
    unsigned int magic;
    unsigned int volumeID;
    unsigned int clusterSize;
    unsigned short nbFAT;
    unsigned int unknown;
    unsigned char  unused[0xfee];
}__attribute__((packed)) PARTITIONHEADER;               //For a total of 4096(0x1000) bytes.

typedef struct {
    char filename[FATX_FILENAME_MAX];
    int clusterId;
    unsigned long fileSize;
    unsigned long fileRead;
    unsigned char *buffer;
} FATXFILEINFO;


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

//TODO: One of the 2 functions below will need to be removed.
//LoadFATXFilefixed requires you to specify pointer (unsigned char* Position) for allocated memory.
//LoadFATXFile allocates memory and point to it using the "buffer" unsigned char pointer in fileinfo struct.
//int LoadFATXFilefixed(FATXPartition *partition,char *filename, FATXFILEINFO *fileinfo,unsigned char* Position);
//A decision has been made. Left it commented for "legacy" purposes.
int LoadFATXFile(FATXPartition *partition,char *filename, FATXFILEINFO *fileinfo);
int FATXListDir(FATXPartition *partition, int clusterId, char **res, int reslen, char *prefix);
int FATXFindDir(FATXPartition *partition, int clusterId, char *dir);
void PrintFATXPartitionTable(int nDriveIndex);
int FATXSignature(int nDriveIndex,unsigned int block);
FATXPartition *OpenFATXPartition(int nDriveIndex,unsigned int partitionOffset,
                        unsigned long long partitionSize);
int FATXRawRead (int drive, int sector, unsigned long long byte_offset, int byte_len, char *buf);
void DumpFATXTree(FATXPartition *partition);
void _DumpFATXTree(FATXPartition* partition, int clusterId, int nesting);
void LoadFATXCluster(FATXPartition* partition, int clusterId, unsigned char* clusterData);
unsigned long getNextClusterInChain(FATXPartition* partition, int clusterId);
void CloseFATXPartition(FATXPartition* partition);
int FATXFindFile(FATXPartition* partition,char* filename,int clusterId, FATXFILEINFO *fileinfo);
int _FATXFindFile(FATXPartition* partition,char* filename,int clusterId, FATXFILEINFO *fileinfo);
int FATXLoadFromDisk(FATXPartition* partition, FATXFILEINFO *fileinfo);
void FATXCreateDirectoryEntry(unsigned char * buffer, char *entryName, unsigned int entryNumber, unsigned int cluster);
int FATXCheckFATXMagic(unsigned char driveId);
//bool FATXCheckBRFR(unsigned char drive);
void FATXSetBRFR(unsigned char drive);
bool FATXCheckMBR(unsigned char driveId);
void FATXSetMBR(unsigned char driveId, XboxPartitionTable *p_table);
void FATXSetInitMBR(unsigned char driveId);
void FATXFormatCacheDrives(int nIndexDrive, bool verbose);
void FATXFormatDriveC(int nIndexDrive, bool verbose);
void FATXFormatDriveE(int nIndexDrive, bool verbose);
void FATXFormatExtendedDrive(unsigned char driveId, unsigned char partition, unsigned int lbaStart, unsigned int lbaSize);

#endif //    _BootFATX_H_
