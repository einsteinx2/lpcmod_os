#ifndef _BootFATX_H_
#define _BootFATX_H_

#include <sys/types.h>

// Definitions for FATX on-disk structures
// (c) 2001 Andrew de Quincey


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


// Size of FATX partition header
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
  u_int64_t partitionStart;

  // The size of the partition in bytes
  u_int64_t partitionSize;

  // The cluster size of the partition
  u_int32_t clusterSize;

  // Number of clusters in the partition
  u_int32_t clusterCount;

  // Size of entries in the cluster chain map
  u_int32_t chainMapEntrySize;

  // The cluster chain map table (which may be in words OR dwords)
  union {
    u_int16_t *words;
    u_int32_t *dwords;
  } clusterChainMap;
  
  // Address of cluster 1
  u_int64_t cluster1Address;
  
} FATXPartition;

typedef struct {                                        //Also known as FATX SuperBlock.
    u32 magic;
    u32 volumeID;
    u32 clusterSize;
    u16 nbFAT;
    u32 unknown;
    u8  unused[0xfee];
}__attribute__((packed)) PARTITIONHEADER;               //For a total of 4096(0x1000) bytes.

typedef struct {
    char filename[FATX_FILENAME_MAX];
    int clusterId;
    u_int32_t fileSize;
    u_int32_t fileRead;
    u8 *buffer;
} FATXFILEINFO;


//Taken from ReactOS' vfat.h source and Xbox-Linux archives.
typedef struct {
    u8 FilenameLength;
    u8 Attrib;
    char Filename[42];
    u32 FirstCluster;
    u32 FileSize;
    u16 UpdateTime;
    u16 UpdateDate;
    u16 CreationTime;
    u16 CreationDate;
    u16 AccessTime;
    u16 AccessDate;
}__attribute__((packed)) FATXDIRINFO;                   //For a total of 64 bytes.

typedef struct
{
        u8 Name[16];
        u32 Flags;
        u32 LBAStart;
        u32 LBASize;
        u32 Reserved;
} XboxPartitionTableEntry;

typedef struct
{
        u8   Magic[16];
        char    Res0[32];
        XboxPartitionTableEntry TableEntries[14];
} XboxPartitionTable;

int LoadFATXFilefixed(FATXPartition *partition,char *filename, FATXFILEINFO *fileinfo,u8* Position);
int LoadFATXFile(FATXPartition *partition,char *filename, FATXFILEINFO *fileinfo);
void PrintFAXPartitionTable(int nDriveIndex);
int FATXSignature(int nDriveIndex,unsigned int block);
FATXPartition *OpenFATXPartition(int nDriveIndex,unsigned int partitionOffset,
                        u_int64_t partitionSize);
int FATXRawRead (int drive, int sector, unsigned long long byte_offset, int byte_len, char *buf);
void DumpFATXTree(FATXPartition *partition);
void _DumpFATXTree(FATXPartition* partition, int clusterId, int nesting);
void LoadFATXCluster(FATXPartition* partition, int clusterId, unsigned char* clusterData);
u_int32_t getNextClusterInChain(FATXPartition* partition, int clusterId);
void CloseFATXPartition(FATXPartition* partition);
int FATXFindFile(FATXPartition* partition,char* filename,int clusterId, FATXFILEINFO *fileinfo);
int _FATXFindFile(FATXPartition* partition,char* filename,int clusterId, FATXFILEINFO *fileinfo);
int FATXLoadFromDisk(FATXPartition* partition, FATXFILEINFO *fileinfo);
void FATXCreateDirectoryEntry(u8 * buffer, char *entryName, u32 entryNumber, u32 cluster);
//bool FATXCheckBRFR(u8 drive);
void FATXSetBRFR(u8 drive);
bool FATXCheckMBR(u8 driveId);
void FATXSetMBR(u8 driveId, XboxPartitionTable *p_table);
void FATXSetInitMBR(u8 driveId);
void FATXFormatCacheDrives(int nIndexDrive, bool verbose);
void FATXFormatDriveC(int nIndexDrive, bool verbose);
void FATXFormatDriveE(int nIndexDrive, bool verbose);
void FATXFormatExtendedDrive(u8 driveId, u8 partition, u32 lbaStart, u32 lbaSize);

#endif //    _BootFATX_H_
