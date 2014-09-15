// Functions for processing FATX partitions
// (c) 2001 Andrew de Quincey

#include "boot.h"
#include "BootFATX.h"
#include <sys/types.h>


#undef FATX_DEBUG

//#define FATX_INFO


XboxPartitionTable BackupPartTbl =
{
    { '*', '*', '*', '*', 'P', 'A', 'R', 'T', 'I', 'N', 'F', 'O', '*', '*', '*', '*' },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    {
        { { 'X', 'B', 'O', 'X', ' ', 'S', 'H', 'E', 'L', 'L', ' ', ' ', ' ', ' ', ' ', ' '}, PE_PARTFLAGS_IN_USE, SECTOR_STORE, SECTORS_STORE, 0 },
        { { 'X', 'B', 'O', 'X', ' ', 'D', 'A', 'T', 'A', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, PE_PARTFLAGS_IN_USE, SECTOR_SYSTEM, SECTORS_SYSTEM, 0 },
        { { 'X', 'B', 'O', 'X', ' ', 'G', 'A', 'M', 'E', ' ', 'S', 'W', 'A', 'P', ' ', '1'}, PE_PARTFLAGS_IN_USE, SECTOR_CACHE1, SECTORS_CACHE1, 0 },
        { { 'X', 'B', 'O', 'X', ' ', 'G', 'A', 'M', 'E', ' ', 'S', 'W', 'A', 'P', ' ', '2'}, PE_PARTFLAGS_IN_USE, SECTOR_CACHE2, SECTORS_CACHE2, 0 },
        { { 'X', 'B', 'O', 'X', ' ', 'G', 'A', 'M', 'E', ' ', 'S', 'W', 'A', 'P', ' ', '3'}, PE_PARTFLAGS_IN_USE, SECTOR_CACHE3, SECTORS_CACHE3, 0 },
        { { 'X', 'B', 'O', 'X', ' ', 'F', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, 0, SECTOR_EXTEND, 0, 0 },
        { { 'X', 'B', 'O', 'X', ' ', 'G', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, 0, SECTOR_EXTEND + 0, 0, 0 },
        { { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, 0, 0, 0, 0 },
        { { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, 0, 0, 0, 0 },
        { { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, 0, 0, 0, 0 },
        { { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, 0, 0, 0, 0 },
        { { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, 0, 0, 0, 0 },
        { { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, 0, 0, 0, 0 },
        { { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, 0, 0, 0, 0 },
    }
};

int checkForLastDirectoryEntry(unsigned char* entry) {

    // if the filename length byte is 0 or 0xff,
    // this is the last entry
    if ((entry[0] == 0xff) || (entry[0] == 0)) {
        return 1;
    }

    // wasn't last entry
    return 0;
}

int FATXListDir(FATXPartition *partition, int clusterId, char **res, int reslen, char *prefix){
    unsigned char* curEntry;
    unsigned char clusterData[partition->clusterSize];
    int i = 0;
    int c = 0;
    u_int32_t filenameSize;
    u_int32_t entryClusterId;
    char foundFilename[50];

    while(clusterId != -1) {
        // load cluster data
        LoadFATXCluster(partition, clusterId, clusterData);

        // loop through it, outputing entries
        for(i=0; i< partition->clusterSize / FATX_DIRECTORYENTRY_SIZE; i++) {
            // work out the currentEntry
            curEntry = clusterData + (i * FATX_DIRECTORYENTRY_SIZE);

            // first of all, check that it isn't an end of directory marker
            if (checkForLastDirectoryEntry(curEntry)) {
                return c;
            }

            // get the filename size
            filenameSize = curEntry[0];

            // check if file is deleted
            if (filenameSize == 0xE5) {
                continue;
            }

            // check size is OK
            if ((filenameSize < 1) || (filenameSize > FATX_FILENAME_MAX)) {
#ifdef FATX_INFO
                printk("Invalid filename size: %i\n", filenameSize);
#endif
                continue;
            }

            res[c] = malloc (filenameSize + 1 + strlen (prefix));
            strcpy (res[c], prefix);
            memcpy(res[c]+strlen (prefix), curEntry+2, filenameSize);
            res[c][filenameSize + strlen (prefix)] = '\0';

            c++;
            if (c >= reslen)
                return c;

        }
        // Find next cluster
        clusterId = getNextClusterInChain(partition, clusterId);
    }

    return c;
}

int FATXFindDir(FATXPartition *partition, int clusterId, char *dir){
    unsigned char* curEntry;
    unsigned char clusterData[partition->clusterSize];
    int i = 0;
    u_int32_t filenameSize;
    u_int32_t flags;
    u_int32_t entryClusterId;
    char seekFilename[50];
    char foundFilename[50];

    while(clusterId != -1) {
        // load cluster data
        LoadFATXCluster(partition, clusterId, clusterData);

        // loop through it, outputing entries
        for(i=0; i< partition->clusterSize / FATX_DIRECTORYENTRY_SIZE; i++) {
            // work out the currentEntry
            curEntry = clusterData + (i * FATX_DIRECTORYENTRY_SIZE);

            // first of all, check that it isn't an end of directory marker
            if (checkForLastDirectoryEntry(curEntry)) {
                return -1;
            }

            // get the filename size
            filenameSize = curEntry[0];

            // check if file is deleted
            if (filenameSize == 0xE5) {
                continue;
            }

            // check size is OK
            if ((filenameSize < 1) || (filenameSize > FATX_FILENAME_MAX)) {
#ifdef FATX_INFO
                printk("Invalid filename size: %i\n", filenameSize);
#endif
                continue;
            }

            // extract the filename
            memset(foundFilename, 0, 50);
            memcpy(foundFilename, curEntry+2, filenameSize);
            foundFilename[filenameSize] = 0;

            // get rest of data
            flags = curEntry[1];
            entryClusterId = *((u_int32_t*) (curEntry + 0x2c));

            // is it what we're looking for...  We use _strncasecmp since fatx
            // isnt case sensitive.
            if (strlen(dir)==strlen(foundFilename) && _strncasecmp(foundFilename, dir,strlen(dir)) == 0) {
                if (flags & FATX_FILEATTR_DIRECTORY) {
                    return entryClusterId;
                } else {
                    return -1;
                }
            }
        }
        // Find next cluster
        clusterId = getNextClusterInChain(partition, clusterId);
    }
    return 0;           //Keep compiler happy.
}

int LoadFATXFilefixed(FATXPartition *partition,char *filename, FATXFILEINFO *fileinfo,u8* Position) {

    if(partition == NULL) {
        VIDEO_ATTR=0xffe8e8e8;
    } else {
        if(FATXFindFile(partition,filename,FATX_ROOT_FAT_CLUSTER,fileinfo)) {
#ifdef FATX_DEBUG
            printk("ClusterID : %d\n",fileinfo->clusterId);
            printk("fileSize  : %d\n",fileinfo->fileSize);
#endif
            fileinfo->buffer = Position;
            memset(fileinfo->buffer,0xff,fileinfo->fileSize);
            
            if(FATXLoadFromDisk(partition, fileinfo)) {
                return true;
            } else {
#ifdef FATX_INFO
                printk("LoadFATXFile : error loading %s\n",filename);
#endif
                return false;
            }
        } else {
#ifdef FATX_INFO
            printk("LoadFATXFile : file %s not found\n",filename);
#endif
            return false;
        }
    }
    return false;
}

int LoadFATXFile(FATXPartition *partition,char *filename, FATXFILEINFO *fileinfo) {

    if(partition == NULL) {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("LoadFATXFile : no open FATX partition\n");
#endif
    } else {
        if(FATXFindFile(partition,filename,FATX_ROOT_FAT_CLUSTER,fileinfo)) {
#ifdef FATX_DEBUG
            printk("ClusterID : %d\n",fileinfo->clusterId);
            printk("fileSize  : %d\n",fileinfo->fileSize);
#endif
            fileinfo->buffer = malloc(fileinfo->fileSize);
            memset(fileinfo->buffer,0,fileinfo->fileSize);
            if(FATXLoadFromDisk(partition, fileinfo)) {
                return true;
            } else {
#ifdef FATX_INFO
                printk("LoadFATXFile : error loading %s\n",filename);
#endif
                return false;
            }
        } else {
#ifdef FATX_INFO
            printk("LoadFATXFile : file %s not found\n",filename);
#endif
            return false;
        }
    }
    return false;
}

void PrintFAXPartitionTable(int nDriveIndex) {
    FATXPartition *partition = NULL;
    FATXFILEINFO fileinfo;

    VIDEO_ATTR=0xffe8e8e8;
    printk("FATX Partition Table:\n");
    memset(&fileinfo,0,sizeof(FATXFILEINFO));

    if(FATXSignature(nDriveIndex,SECTOR_SYSTEM)) {
        VIDEO_ATTR=0xffe8e8e8;
        printk("Partition SYSTEM\n");
        partition = OpenFATXPartition(nDriveIndex,SECTOR_SYSTEM,SYSTEM_SIZE);
        if(partition == NULL) {
            VIDEO_ATTR=0xffe8e8e8;
            printk("PrintFAXPartitionTable : error on opening STORE\n");
        } else {
            DumpFATXTree(partition);
        }
    }

    VIDEO_ATTR=0xffc8c8c8;
}

int FATXSignature(int nDriveIndex,unsigned int block) {
    u8 ba[512];

    if(BootIdeReadSector(0, &ba[0], block, 0, 512)) {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("FATXSignature : Unable to read FATX sector\n");
#endif
        return false;
    } else {
        if( (ba[0]=='F') && (ba[1]=='A') && (ba[2]=='T') && (ba[3]=='X') ) {
            return true;
        } else {
            return false;
        }
    }
}

FATXPartition *OpenFATXPartition(int nDriveIndex,
        unsigned int partitionOffset,
        u_int64_t partitionSize) {
    unsigned char partitionInfo[FATX_PARTITION_HEADERSIZE];
    FATXPartition *partition;
    int readSize;
    unsigned int chainTableSize;

#ifdef FATX_DEBUG
    printk("OpenFATXPartition : Read partition header\n");
#endif
    // load the partition header
    readSize = FATXRawRead(nDriveIndex, partitionOffset, 0,
            FATX_PARTITION_HEADERSIZE, (char *)&partitionInfo);

      if (readSize != FATX_PARTITION_HEADERSIZE) {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("OpenFATXPartition : Out of data while reading partition header\n");
#endif
        return NULL;
    }

    // check the magic
    if (*((u_int32_t*) &partitionInfo) != FATX_PARTITION_MAGIC) {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("OpenFATXPartition : No FATX partition found at requested offset\n");
#endif
        return NULL;
    }

#ifdef FATX_DEBUG
    printk("OpenFATXPartition : Allocating Partition struct\n");
#endif
    // make up new structure
    partition = (FATXPartition*) malloc(sizeof(FATXPartition));
    if (partition == NULL) {
#ifdef FATX_INFO
        printk("OpenFATXPartition : Out of memory\n");
#endif
        return NULL;
    }
#ifdef FATX_DEBUG
    printk("OpenFATXPartition : Allocating Partition struct done\n");
#endif
    memset(partition,0,sizeof(FATXPartition));

    // setup the easy bits
    partition->nDriveIndex = nDriveIndex;
    partition->partitionStart = partitionOffset;
    partition->partitionSize = partitionSize;
    partition->clusterSize = 0x4000;
    partition->clusterCount = partition->partitionSize / 0x4000;
    partition->chainMapEntrySize = (partition->clusterCount >= 0xfff4) ? 4 : 2;

    // Now, work out the size of the cluster chain map table
    chainTableSize = partition->clusterCount * partition->chainMapEntrySize;
    if (chainTableSize % FATX_CHAINTABLE_BLOCKSIZE) {
        // round up to nearest FATX_CHAINTABLE_BLOCKSIZE bytes
        chainTableSize = ((chainTableSize / FATX_CHAINTABLE_BLOCKSIZE) + 1)
                * FATX_CHAINTABLE_BLOCKSIZE;
    }

#ifdef FATX_DEBUG
    printk("OpenFATXPartition : Allocating chaintable struct\n");
#endif
      // Load the cluster chain map table
    partition->clusterChainMap.words = (u_int16_t*) malloc(chainTableSize);
        if (partition->clusterChainMap.words == NULL) {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("OpenFATXPartition : Out of memory\n");
#endif
        return NULL;
    }

#ifdef FATX_DEBUG
    printk("Part stats : CL Count    %d \n", partition->clusterCount);
    printk("Part stats : CL Size    %d \n", partition->clusterSize);
    printk("Part stats : CM Size    %d \n", partition->chainMapEntrySize);
    printk("Part stats : Table Size    %d \n", chainTableSize);
    printk("Part stats : Part Size    %d \n", partition->partitionSize);
#endif

    readSize = FATXRawRead(nDriveIndex, partitionOffset, FATX_PARTITION_HEADERSIZE,
            chainTableSize, (char *)partition->clusterChainMap.words);

        if (readSize != chainTableSize) {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("Out of data while reading cluster chain map table\n");
#endif
    }
    partition->cluster1Address = ( ( FATX_PARTITION_HEADERSIZE + chainTableSize) );

    return partition;
}

void DumpFATXTree(FATXPartition *partition) {
    // OK, start off the recursion at the root FAT
    _DumpFATXTree(partition, FATX_ROOT_FAT_CLUSTER, 0);
}

void _DumpFATXTree(FATXPartition* partition, int clusterId, int nesting) {

    int endOfDirectory;
    unsigned char* curEntry;
    unsigned char clusterData[partition->clusterSize];
    int i,j;
    char writeBuf[512];
    char filename[50];
    u_int32_t filenameSize;
    u_int32_t fileSize;
    u_int32_t entryClusterId;
    unsigned char flags;
    char flagsStr[5];

    // OK, output all the directory entries
    endOfDirectory = 0;
    while(clusterId != -1) {
        LoadFATXCluster(partition, clusterId, clusterData);

        // loop through it, outputing entries
        for(i=0; i< partition->clusterSize / FATX_DIRECTORYENTRY_SIZE; i++) {

            // work out the currentEntry
            curEntry = clusterData + (i * FATX_DIRECTORYENTRY_SIZE);

            // first of all, check that it isn't an end of directory marker
            if (checkForLastDirectoryEntry(curEntry)) {
                endOfDirectory = 1;
                break;
            }

            // get the filename size
            filenameSize = curEntry[0];

            // check if file is deleted
            if (filenameSize == 0xE5) {
                continue;
            }

            // check size is OK
            if ((filenameSize < 1) || (filenameSize > FATX_FILENAME_MAX)) {
                VIDEO_ATTR=0xffe8e8e8;
                printk("_DumpFATXTree : Invalid filename size: %i\n", filenameSize);
            }

            // extract the filename
            memset(filename, 0, 50);
            memcpy(filename, curEntry+2, filenameSize);
            filename[filenameSize] = 0;

            // get rest of data
            flags = curEntry[1];
            entryClusterId = *((u_int32_t*) (curEntry + 0x2c));
            fileSize = *((u_int32_t*) (curEntry + 0x30));

            // wipe fileSize
            if (flags & FATX_FILEATTR_DIRECTORY) {
                fileSize = 0;
            }

            // zap flagsStr
            strcpy(flagsStr, "    ");

            // work out other flags
            if (flags & FATX_FILEATTR_READONLY) {
                          flagsStr[0] = 'R';
            }
            if (flags & FATX_FILEATTR_HIDDEN) {
                flagsStr[1] = 'H';
            }
            if (flags & FATX_FILEATTR_SYSTEM) {
                flagsStr[2] = 'S';
            }
            if (flags & FATX_FILEATTR_ARCHIVE) {
                flagsStr[3] = 'A';
            }

            // check we don't have any unknown flags
/*
            if (flags & 0xc8) {
                VIDEO_ATTR=0xffe8e8e8;
                printk("WARNING: file %s has unknown flags %x\n", filename, flags);
            }
*/
            
            // Output it
            for(j=0; j< nesting; j++) {
                writeBuf[j] = ' ';
            }

            VIDEO_ATTR=0xffe8e8e8;
            printk("/%s  [%s] (SZ:%i CL%x))\n",filename, flagsStr,
                    fileSize, entryClusterId);

            // If it is a sub-directory, recurse
            /*
            if (flags & FATX_FILEATTR_DIRECTORY) {
                _DumpFATXTree(partition, entryClusterId, nesting+1);
            }
            */
            // have we hit the end of the directory yet?
        }        
        if (endOfDirectory) {
            break;
        }    
        clusterId = getNextClusterInChain(partition, clusterId);
    }
}

int FATXLoadFromDisk(FATXPartition* partition, FATXFILEINFO *fileinfo) {

    unsigned char clusterData[partition->clusterSize];
    int fileSize = fileinfo->fileSize;
    int written;
    int clusterId = fileinfo->clusterId;
    u8 *ptr;

    fileinfo->fileRead = 0;
    ptr = fileinfo->buffer;

    // loop, outputting clusters
    while(clusterId != -1) {
        // Load the cluster data
        LoadFATXCluster(partition, clusterId, clusterData);

        // Now, output it
        written = (fileSize <= partition->clusterSize) ? fileSize : partition->clusterSize;
        memcpy(ptr,clusterData,written);
        fileSize -= written;
        fileinfo->fileRead+=written;
        ptr+=written;

        // Find next cluster
        clusterId = getNextClusterInChain(partition, clusterId);
    }

    // check we actually found enough data
    if (fileSize != 0) {
#ifdef FATX_INFO
       printk("Hit end of cluster chain before file size was zero\n");
#endif
        //free(fileinfo->buffer);
        //fileinfo->buffer = NULL;
        return false;
    }
    return true;
}

int FATXFindFile(FATXPartition* partition,
                    char* filename,
                    int clusterId, FATXFILEINFO *fileinfo) {

    int i = 0;
#ifdef FATX_DEBUG
    VIDEO_ATTR=0xffc8c8c8;
    printk("FATXFindFile : %s\n",filename);
#endif

      // convert any '\' to '/' characters
      while(filename[i] != 0) {
            if (filename[i] == '\\') {
                  filename[i] = '/';
            }
            i++;
      }
      
    // skip over any leading / characters
      i=0;
      while((filename[i] != 0) && (filename[i] == '/')) {
            i++;
      }

    return _FATXFindFile(partition,&filename[i],clusterId,fileinfo);
}

int _FATXFindFile(FATXPartition* partition,
                    char* filename,
                    int clusterId, FATXFILEINFO *fileinfo) {
    unsigned char* curEntry;
    unsigned char clusterData[partition->clusterSize];
    int i = 0;
    int endOfDirectory;
    u_int32_t filenameSize;
    u_int32_t flags;
    u_int32_t entryClusterId;
    u_int32_t fileSize;
    char seekFilename[50];
    char foundFilename[50];
    char* slashPos;
    int lookForDirectory = 0;
    int lookForFile = 0;


    // work out the filename we're looking for
    slashPos = strrchr0(filename, '/');
    if (slashPos == NULL) {
    // looking for file
        lookForFile = 1;

        // check seek filename size
        if (strlen(filename) > FATX_FILENAME_MAX) {
#ifdef FATX_INFO
            printk("Bad filename supplied (one leafname is too big)\n");
#endif
            return false;
        }

        // copy the filename to look for
        strcpy(seekFilename, filename);
    } else {
        // looking for directory
        lookForDirectory = 1;

        // check seek filename size
        if ((slashPos - filename) > FATX_FILENAME_MAX) {
#ifdef FATX_INFO
            printk("Bad filename supplied (one leafname is too big)\n");
#endif
            return false;
        }

        // copy the filename to look for
        memcpy(seekFilename, filename, slashPos - filename);
        seekFilename[slashPos - filename] = 0;
    }

#ifdef FATX_DEBUG
    VIDEO_ATTR=0xffc8c8c8;
    printk("_FATXFindFile : %s\n",filename);
#endif
    // OK, search through directory entries
    endOfDirectory = 0;
    while(clusterId != -1) {
            // load cluster data
            LoadFATXCluster(partition, clusterId, clusterData);

        // loop through it, outputing entries
        for(i=0; i< partition->clusterSize / FATX_DIRECTORYENTRY_SIZE; i++) {
            // work out the currentEntry
            curEntry = clusterData + (i * FATX_DIRECTORYENTRY_SIZE);

            // first of all, check that it isn't an end of directory marker
            if (checkForLastDirectoryEntry(curEntry)) {
                endOfDirectory = 1;
                break;
            }

            // get the filename size
            filenameSize = curEntry[0];

            // check if file is deleted
            if (filenameSize == 0xE5) {
                continue;
            }

            // check size is OK
            if ((filenameSize < 1) || (filenameSize > FATX_FILENAME_MAX)) {
#ifdef FATX_INFO
                printk("Invalid filename size: %i\n", filenameSize);
#endif
                return false;
            }

            // extract the filename
            memset(foundFilename, 0, 50);
            memcpy(foundFilename, curEntry+2, filenameSize);
            foundFilename[filenameSize] = 0;

            // get rest of data
            flags = curEntry[1];
            entryClusterId = *((u_int32_t*) (curEntry + 0x2c));
            fileSize = *((u_int32_t*) (curEntry + 0x30));

            // is it what we're looking for...
            if (strlen(seekFilename)==strlen(foundFilename) && _strncasecmp(foundFilename, seekFilename,strlen(seekFilename)) == 0) {
                // if we're looking for a directory and found a directory
                if (lookForDirectory) {
                    if (flags & FATX_FILEATTR_DIRECTORY) {
                        return _FATXFindFile(partition, slashPos+1, entryClusterId,fileinfo);
                    } else {
#ifdef FATX_INFO
                        printk("File not found\n");
#endif
                        return false;
                    }
                }

                // if we're looking for a file and found a file
                if (lookForFile) {
                    if (!(flags & FATX_FILEATTR_DIRECTORY)) {
                        fileinfo->clusterId = entryClusterId;
                        fileinfo->fileSize = fileSize;
                        memset(fileinfo->filename,0,sizeof(fileinfo->filename));
                        strcpy(fileinfo->filename,filename);
                        return true;
                    } else {
#ifdef FATX_INFO
                        printk("File not found %s\n",filename);
#endif
                        return false;
                    }
                }
            }
        }

        // have we hit the end of the directory yet?
        if (endOfDirectory) {
            break;
        }

        // Find next cluster
        clusterId = getNextClusterInChain(partition, clusterId);
    }

    // not found it!
#ifdef FATX_INFO
    printk("File not found\n");
#endif
    return false;
}



u_int32_t getNextClusterInChain(FATXPartition* partition, int clusterId) {
    int nextClusterId = 0;
    u_int32_t eocMarker = 0;
    u_int32_t rootFatMarker = 0;
    u_int32_t maxCluster = 0;

    // check
    if (clusterId < 1) {
        VIDEO_ATTR=0xffe8e8e8;
        printk("getNextClusterInChain : Attempt to access invalid cluster: %i\n", clusterId);
    }

    // get the next ID
    if (partition->chainMapEntrySize == 2) {
        nextClusterId = partition->clusterChainMap.words[clusterId];
            eocMarker = 0xffff;
        rootFatMarker = 0xfff8;
        maxCluster = 0xfff4;
    } else if (partition->chainMapEntrySize == 4) {
        nextClusterId = partition->clusterChainMap.dwords[clusterId];
        eocMarker = 0xffffffff;
        rootFatMarker = 0xfffffff8;
        maxCluster = 0xfffffff4;
    } else {
        VIDEO_ATTR=0xffe8e8e8;
        printk("getNextClusterInChain : Unknown cluster chain map entry size: %i\n", partition->chainMapEntrySize);
    }

    // is it the end of chain?
      if ((nextClusterId == eocMarker) || (nextClusterId == rootFatMarker)) {
        return -1;
    }
    
    // is it something else unknown?
    if (nextClusterId == 0) {
        VIDEO_ATTR=0xffe8e8e8;
        printk("getNextClusterInChain : Cluster chain problem: Next cluster after %i is unallocated!\n", clusterId);
        }
    if (nextClusterId > maxCluster) {
        printk("getNextClusterInChain : Cluster chain problem: Next cluster after %i has invalid value: %i\n", clusterId, nextClusterId);
    }
    
    // OK!
    return nextClusterId;
}

void LoadFATXCluster(FATXPartition* partition, int clusterId, unsigned char* clusterData) {
    u_int64_t clusterAddress;
    u_int64_t readSize;
    
    // work out the address of the cluster
    clusterAddress = partition->cluster1Address + ((unsigned long long)(clusterId - 1) * partition->clusterSize);

    // Now, load it
    readSize = FATXRawRead(partition->nDriveIndex, partition->partitionStart,
            clusterAddress, partition->clusterSize, clusterData);

        if (readSize != partition->clusterSize) {
        printk("LoadFATXCluster : Out of data while reading cluster %i\n", clusterId);
    }
}
        

int FATXRawRead(int drive, int sector, unsigned long long byte_offset, int byte_len, char *buf) {

    int byte_read;
    
    byte_read = 0;

//    printk("rawread: sector=0x%X, byte_offset=0x%X, len=%d\n", sector, byte_offset, byte_len);

        sector+=byte_offset/512;
        byte_offset%=512;

        while(byte_len) {
        int nThisTime=512;
        if(byte_len<512) nThisTime=byte_len;
                if(byte_offset) {
                    u8 ba[512];
            if(BootIdeReadSector(drive, buf, sector, 0, 512)) {
                VIDEO_ATTR=0xffe8e8e8;
                printk("Unable to get first sector\n");
                                return false;
            }
            memcpy(buf, &ba[byte_offset], nThisTime-byte_offset);
            buf+=nThisTime-byte_offset;
            byte_len-=nThisTime-byte_offset;
            byte_read += nThisTime-byte_offset;
            byte_offset=0;
        } else {
            if(BootIdeReadSector(drive, buf, sector, 0, nThisTime)) {
                VIDEO_ATTR=0xffe8e8e8;
                printk("Unable to get first sector\n");
                return false;
            }
            buf+=nThisTime;
            byte_len-=nThisTime;
            byte_read += nThisTime;
        }
        sector++;
    }
    return byte_read;
}

void CloseFATXPartition(FATXPartition* partition) {
    if(partition != NULL) {
        free(partition->clusterChainMap.words);
        free(partition);
        partition = NULL;
    }
}

void FATXCreateDirectoryEntry(u8 * buffer, char *entryName, u32 entryNumber, u32 cluster){
    u32 offset = entryNumber * 0x40;

    FATXDIRINFO *dirEntry = (FATXDIRINFO *)&buffer[offset];
    memset(&buffer[offset],0xff,0x40);

    dirEntry->FilenameLength = strlen(entryName);
    strncpy(dirEntry->Filename,entryName,42);
    if(dirEntry->FilenameLength < 42)                           //If filename is not of 42 characters
        dirEntry->Filename[dirEntry->FilenameLength] = 0xFF;    //Terminate it.
    dirEntry->Attrib = FATX_FILEATTR_DIRECTORY;
    dirEntry->FirstCluster = cluster;
    dirEntry->FileSize = 0;
    dirEntry->UpdateTime = 0x5153;
    dirEntry->CreationTime = 0x5153;
    dirEntry->AccessTime = 0x5153;
    dirEntry->UpdateDate = 0x0442;
    dirEntry->CreationDate = 0x0442;
    dirEntry->AccessDate = 0x0442;

    return;                                                     //buffer is updated, ready to be wrote on HDD.
}

/*
bool FATXCheckBRFR(u8 drive){
    u8 ba[512];
    if(BootIdeReadSector(drive, &ba[0], 0x03, 0, 512)) {
        VIDEO_ATTR=0xffe8e8e8;
//#ifdef FATX_INFO
        printk("\n\n\n           FATXCheckBRFR : Unable to read Boot block sector\n");
        wait_ms(3000);
//#endif
        return true;
    } else {
	if(ba[0] == 'B' && ba[1] == 'R' && ba[2] == 'F' && ba[3] == 'R')
		return true;
	else{
	    printk("\n\n\n           sector = 0x03,  content = %x , %x, %x, %x", ba[0],ba[1],ba[2],ba[3]);
	    wait_ms(5000);
	}
    }
    return false;
}
*/

void FATXSetBRFR(u8 drive){
	u8 buffer[512];
	u32 counter;
	
	memset(buffer, 0, 512);
        for(counter = 0; counter < 1024; counter++){             //Set first 512KB of HDD to 0x00.
            BootIdeWriteSector(drive, buffer, counter);        //512KB = 1024 sectors.
        }
        sprintf(buffer,"BRFR");
        BootIdeWriteSector(drive, buffer, 3);       //Write "BRFR" string and number of boots(0) at absolute offset 0x600
        tsaHarddiskInfo[drive].m_enumDriveType = EDT_XBOXFS;
	
}

bool FATXCheckMBR(u8 driveId, XboxPartitionTable *p_table){
    u8 *sourceTable = (u8 *)p_table;
    u8 i;
    u8 ba[512];
    if(BootIdeReadSector(driveId, &ba[0], 0x00, 0, 512)) {
        printk("\n\n\n           FATXCheckMBR : Unable to read MBR sector\n");
    }
    else{
        for(i = 0; i < 208; i++){               //first 208 bytes should always be identical for every Xbox.
            if(ba[i] != sourceTable[i])         //Contains generic MBR header + standard Xbox Partitions (C,E,X,Y,Z)
                return false;
        }
    }
    return true;
}

void FATXSetMBR(u8 driveId, XboxPartitionTable *p_table){
    u8 *sourceTable = (u8 *)p_table;
    BootIdeWriteSector(driveId,sourceTable,0);
}

void FATXSetInitMBR(u8 driveId){
    BootIdeWriteSector(driveId,(u8 *)&BackupPartTbl,0);
}

void FATXFormatCacheDrives(int nIndexDrive){
    u8 buffer[512], headerBuf[0x1000], chainmapBuf[512];
    u32 counter;
    u32 whichpartition;
    PARTITIONHEADER *header;

    if(tsaHarddiskInfo[nIndexDrive].m_enumDriveType != EDT_XBOXFS)
    	FATXSetBRFR(nIndexDrive);

    memset(headerBuf,0xff,0x1000);              //First sector(and only one used) of the Partition header area.
    header = (PARTITIONHEADER *)headerBuf;
    header->magic = FATX_PARTITION_MAGIC;       //Whooo, magic!
    header->volumeID = IoInputDword(0x8008);    //Goes with the HDD.
    header->clusterSize = 0x20;                 //16KB cluster, so 32 clusters per sector.
    header->nbFAT = 1;                          //Always 1.
    header->unknown = 0;                        //Always 0.
    memset(header->unused,0xff,4078);           //Fill unused area with 0xff.
    //Partition header is ready. It's the same for all 3 cache drives.
    //It's not necessary to fill unused area up to 0x1000 because we'll only write the first 512 bytes of headerBuf
    //onto the HDD. Let's do it for the exercise OK? A few wasted cycles isn't going to hurt anybody.

    memset(chainmapBuf,0x0,512);                //First sector of the Cluster chain map area.
    chainmapBuf[0]=0xf8;                        //First cluster is 0xFFF8 in word mode cluster.
    chainmapBuf[1]=0xff;

    memset(buffer,0xff,512);                    //Killer buffer.

    //Cycle through all 3 cache partitions.
    for(whichpartition = SECTOR_CACHE1; whichpartition <= SECTOR_CACHE3; whichpartition += (SECTOR_CACHE2 - SECTOR_CACHE1)){
        // Starting (from 0 to 512*8 = 0x1000). Erasing Partition header data.
        //4KB so 8*512 bytes sectors.
        for (counter=whichpartition;counter<(whichpartition+8); counter++) {
            BootIdeWriteSector(nIndexDrive,buffer,counter);
        }
        //Write Partition info on first sector. lest seven sectors of the first 0x1000 are already 0xff anyway.
        BootIdeWriteSector(nIndexDrive,headerBuf,whichpartition);   //Partition header write.

        // Cluster chain map area (from 512*8 = 0x1000 to 512*200 = 0x19000)
        memset(buffer,0x0,512); //wipe. Unused cluster == 0
        for (counter=(whichpartition+8);counter<(whichpartition+200); counter++) {
            BootIdeWriteSector(nIndexDrive,buffer,counter);
        }
        //Write initial cluster chain map.
        BootIdeWriteSector(nIndexDrive,chainmapBuf,whichpartition+8);   //Initial Cluster chain map write.

        // Root Dir (from 512*200 = 0x19000 to 0x1d000 = 512*232)
        memset(buffer,0xff,512);
        for (counter=(whichpartition+200);counter<(whichpartition+200+(32*10)); counter++) {
            BootIdeWriteSector(nIndexDrive,buffer,counter);
        }
    }
}

void FATXFormatDriveC(int nIndexDrive){
    u8 buffer[512], headerBuf[0x1000], chainmapBuf[512];
    u32 counter;
    PARTITIONHEADER *header;

    if(tsaHarddiskInfo[nIndexDrive].m_enumDriveType != EDT_XBOXFS)
        FATXSetBRFR(nIndexDrive);

    memset(headerBuf,0xff,0x1000);              //First sector(and only one used) of the Partition header area.
    header = (PARTITIONHEADER *)headerBuf;
    header->magic = FATX_PARTITION_MAGIC;       //Whooo, magic!
    header->volumeID = IoInputDword(0x8008);    //Goes with the HDD.
    header->clusterSize = 0x20;                 //16KB cluster, so 32 clusters per sector.
    header->nbFAT = 1;                          //Always 1.
    header->unknown = 0;                        //Always 0.
    memset(header->unused,0xff,4078);           //Fill unused area with 0xff.
    //Partition header is ready. It's the same for all 3 cache drives.
    //It's not necessary to fill unused area up to 0x1000 because we'll only write the first 512 bytes of headerBuf
    //onto the HDD. Let's do it for the exercise OK? A few wasted cycles isn't going to hurt anybody.

    memset(chainmapBuf,0x0,512);                //First sector of the Cluster chain map area.
    chainmapBuf[0]=0xf8;                        //First cluster is 0xFFF8 in word mode cluster.
    chainmapBuf[1]=0xff;

    memset(buffer,0xff,512);                    //Killer buffer.

    // Starting (from 0 to 512*8 = 0x1000). Erasing Partition header data.
    //4KB so 8*512 bytes sectors.
    for (counter=SECTOR_SYSTEM;counter<(SECTOR_SYSTEM+8); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }
    //Write Partition info on first sector. lest seven sectors of the first 0x1000 are already 0xff anyway.
    BootIdeWriteSector(nIndexDrive,headerBuf,SECTOR_SYSTEM);   //Partition header write.

    // Cluster chain map area (from 512*8 = 0x1000 to 512*136 = 0x11000)
    memset(buffer,0x0,512); //wipe. Unused cluster == 0
    for (counter=(SECTOR_SYSTEM+8);counter<(SECTOR_SYSTEM+136); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }
    //Write initial cluster chain map.
    BootIdeWriteSector(nIndexDrive,chainmapBuf,SECTOR_SYSTEM+8);   //Initial Cluster chain map write.

    // Root Dir (from 512*136 = 0x11000 )
    memset(buffer,0xff,512);
    for (counter=(SECTOR_SYSTEM+136);counter<(SECTOR_SYSTEM+136+(32*10)); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }
}

void FATXFormatDriveE(int nIndexDrive){
    u8 buffer[512], headerBuf[0x1000], chainmapBuf[512];
    u32 counter;
    PARTITIONHEADER *header;

    if(tsaHarddiskInfo[nIndexDrive].m_enumDriveType != EDT_XBOXFS)
        FATXSetBRFR(nIndexDrive);

    memset(headerBuf,0xff,0x1000);              //First sector(and only one used) of the Partition header area.
    header = (PARTITIONHEADER *)headerBuf;
    header->magic = FATX_PARTITION_MAGIC;       //Whooo, magic!
    header->volumeID = IoInputDword(0x8008);    //Goes with the HDD.
    header->clusterSize = 0x20;                 //16KB cluster, so 32 clusters per sector.
    header->nbFAT = 1;                          //Always 1.
    header->unknown = 0;                        //Always 0.
    memset(header->unused,0xff,4078);           //Fill unused area with 0xff.
    //Partition header is ready. It's the same for all 3 cache drives.
    //It's not necessary to fill unused area up to 0x1000 because we'll only write the first 512 bytes of headerBuf
    //onto the HDD. Let's do it for the exercise OK? A few wasted cycles isn't going to hurt anybody.

    memset(chainmapBuf,0x0,512);                //First sector of the Cluster chain map area.
    memset(chainmapBuf,0xff,4*7);               //We'll use clusters for base folders.
    chainmapBuf[0]=0xf8;                        //First cluster is 0xFFFFFFF8 in 4 byte mode cluster.

    memset(buffer,0xff,512);                    //Killer buffer.

    // Starting (from 0 to 512*8 = 0x1000). Erasing Partition header data.
    //4KB so 8*512 bytes sectors.
    for (counter=SECTOR_STORE;counter<(SECTOR_STORE+8); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }
    //Write Partition info on first sector. lest seven sectors of the first 0x1000 are already 0xff anyway.
    BootIdeWriteSector(nIndexDrive,headerBuf,SECTOR_STORE);   //Partition header write.

    // Cluster chain map area (from 512*8 = 0x1000 to 512*2456 = 0x133000)
    memset(buffer,0x0,512); //wipe. Unused cluster == 0
    for (counter=(SECTOR_STORE+8);counter<(SECTOR_STORE+2440); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }
    //Write initial cluster chain map.
    BootIdeWriteSector(nIndexDrive,chainmapBuf,SECTOR_STORE+8);   //Initial Cluster chain map write.

    // Marked as used from 2440 to 2456.
    memset(buffer,0xff,512);
    for (counter=(SECTOR_STORE+2440);counter<(SECTOR_STORE+2456); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }

    // Root Dir (from 512*2456 = 0x133000 to 0x1d000 = 512*232)
    // 10 cluster formatted.
    for (counter=(SECTOR_STORE+2456);counter<(SECTOR_STORE+2456+(32*10)); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }

    memset(buffer,0xff,512);
    // TDATA Dir points to Cluster 2
    FATXCreateDirectoryEntry(buffer,"TDATA",0,2);
    // UDATA Dir points to Cluster 4
    FATXCreateDirectoryEntry(buffer,"UDATA",1,4);
    // CACHE Dir points to Cluster 6
    FATXCreateDirectoryEntry(buffer,"CACHE",2,6);
    BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456);   // Write Cluster 1(E: Root).

    memset(buffer,0xff,512);
    //CACHE dir is empty to 0xff everywhere.
    BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456+32+32+32+32+32);   // Write Cluster 6(CACHE).
    // FFFE0000 Dir points to Cluster 3
    FATXCreateDirectoryEntry(buffer,"FFFE0000",0,3);
    BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456+32);   // Write Cluster 2(TDATA).

    memset(buffer,0xff,512);
    // Music Dir points to Cluster 5
    FATXCreateDirectoryEntry(buffer,"Music",0,5);
    BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456+32+32+32);   // Write Cluster 4(UDATA).
}
