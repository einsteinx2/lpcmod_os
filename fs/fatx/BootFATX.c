// Functions for processing FATX partitions
// (c) 2001 Andrew de Quincey

#include "boot.h"
#include "BootFATX.h"
#include "BootIde.h"
#include "stdlib.h"
#include "string.h"
#include "Gentoox.h"
#include "string.h"
#include "cromwell.h"
#include "sortHelpers.h"

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

int checkForLastDirectoryEntry(unsigned char* entry)
{

    // if the filename length byte is 0 or 0xff,
    // this is the last entry
    if ((entry[0] == 0xff) || (entry[0] == 0))
    {
        return 1;
    }

    // wasn't last entry
    return 0;
}

int FATXListDir(FATXPartition *partition, int clusterId, char **res, int reslen, char *prefix)
{
    unsigned char* curEntry;
    unsigned char clusterData[partition->clusterSize];
    char *tempSortPtr;
    int i = 0;
    int c = 0;
    int sortResult;
    unsigned long filenameSize;
    int sortNotOver = 1;

    while(clusterId != -1)
    {
        // load cluster data
        LoadFATXCluster(partition, clusterId, clusterData);

        // loop through it, outputing entries
        for(i=0; i< partition->clusterSize / FATX_DIRECTORYENTRY_SIZE; i++)
        {
            // work out the currentEntry
            curEntry = clusterData + (i * FATX_DIRECTORYENTRY_SIZE);

            // first of all, check that it isn't an end of directory marker
            if (checkForLastDirectoryEntry(curEntry))
            {
                goto exit;
            }

            // get the filename size
            filenameSize = curEntry[0];

            // check if file is deleted
            if (filenameSize == 0xE5)
            {
                continue;
            }

            // check size is OK
            if ((filenameSize < 1) || (filenameSize > FATX_FILENAME_MAX))
            {
#ifdef FATX_INFO
                printk("     Invalid filename size: %i\n", filenameSize);
#endif
                continue;
            }

            res[c] = malloc (filenameSize + 1 + strlen (prefix));
            strcpy (res[c], prefix);
            memcpy(res[c]+strlen (prefix), curEntry+2, filenameSize);
            res[c][filenameSize + strlen (prefix)] = '\0';

            c++;
            if (c >= reslen)
            {
                goto exit;
            }

        }
        // Find next cluster
        clusterId = getNextClusterInChain(partition, clusterId);
    }
    
    //place quicksort here.

    //TODO: Freakin bubble sort for now... Just to test "strcmpbynum" function
exit:
printk("\n\n");
    while(sortNotOver && c >= 2)
    {
    	
        sortNotOver = 0;
        for(i = 0; i < (c - 1); i++)
        {
            sortResult = strcmpbynum(res[i], res[i + 1]);

            if(sortResult > 0)
            {
                sortNotOver++;
                tempSortPtr = res[i];
                res[i] = res[i + 1];
                res[i + 1] = tempSortPtr;
            }
        }
    }

    return c;
}

int FATXFindDir(FATXPartition *partition, int clusterId, char *dir)
{
    unsigned char* curEntry;
    unsigned char clusterData[partition->clusterSize];
    int i = 0;
    unsigned long filenameSize;
    unsigned long flags;
    unsigned long entryClusterId;
    char foundFilename[50];

    while(clusterId != -1)
    {
        // load cluster data
        LoadFATXCluster(partition, clusterId, clusterData);

        // loop through it, outputing entries
        for(i=0; i< partition->clusterSize / FATX_DIRECTORYENTRY_SIZE; i++)
        {
            // work out the currentEntry
            curEntry = clusterData + (i * FATX_DIRECTORYENTRY_SIZE);

            // first of all, check that it isn't an end of directory marker
            if (checkForLastDirectoryEntry(curEntry))
            {
                return -1;
            }

            // get the filename size
            filenameSize = curEntry[0];

            // check if file is deleted
            if (filenameSize == 0xE5)
            {
                continue;
            }

            // check size is OK
            if ((filenameSize < 1) || (filenameSize > FATX_FILENAME_MAX))
            {
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
            entryClusterId = *((unsigned long*) (curEntry + 0x2c));

            // is it what we're looking for...  We use _strncasecmp since fatx
            // isnt case sensitive.
            if (strlen(dir)==strlen(foundFilename) && _strncasecmp(foundFilename, dir,strlen(dir)) == 0)
            {
                if (flags & FATX_FILEATTR_DIRECTORY)
                {
                    return entryClusterId;
                }
                else
                {
                    return -1;
                }
            }
        }
        // Find next cluster
        clusterId = getNextClusterInChain(partition, clusterId);
    }
    return 0;           //Keep compiler happy.
}

int LoadFATXFile(FATXPartition *partition,char *filename, FATXFILEINFO *fileinfo)
{

    if(partition == NULL)
    {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("LoadFATXFile : no open FATX partition\n");
#endif
    }
    else
    {
        if(FATXFindFile(partition,filename,FATX_ROOT_FAT_CLUSTER,fileinfo))
        {
#ifdef FATX_DEBUG
            printk("ClusterID : %d\n",fileinfo->clusterId);
            printk("fileSize  : %d\n",fileinfo->fileSize);
#endif
            fileinfo->buffer = malloc(fileinfo->fileSize);
            memset(fileinfo->buffer,0,fileinfo->fileSize);
            if(FATXLoadFromDisk(partition, fileinfo))
            {
                return true;
            }
            else
            {
#ifdef FATX_INFO
                printk("LoadFATXFile : error loading %s\n",filename);
#endif
                free(fileinfo->buffer);
                return false;
            }
        }
        else
        {
#ifdef FATX_INFO
            printk("LoadFATXFile : file %s not found\n",filename);
#endif
            return false;
        }
    }
    return false;
}

void PrintFATXPartitionTable(int nDriveIndex)
{
    FATXPartition *partition = NULL;
    FATXFILEINFO fileinfo;

    VIDEO_ATTR=0xffe8e8e8;
    printk("FATX Partition Table:\n");
    memset(&fileinfo,0,sizeof(FATXFILEINFO));

    if(FATXSignature(nDriveIndex,SECTOR_SYSTEM))
    {
        VIDEO_ATTR=0xffe8e8e8;
        printk("Partition SYSTEM\n");
        partition = OpenFATXPartition(nDriveIndex,SECTOR_SYSTEM,SYSTEM_SIZE);
        if(partition == NULL)
        {
            VIDEO_ATTR=0xffe8e8e8;
            printk("PrintFAXPartitionTable : error on opening STORE\n");
        }
        else
        {
            DumpFATXTree(partition);
        }
    }

    VIDEO_ATTR=0xffc8c8c8;
}

int FATXSignature(int nDriveIndex,unsigned int block)
{
    unsigned char ba[512];

    if(BootIdeReadSector(0, &ba[0], block, 0, 512))
    {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("FATXSignature : Unable to read FATX sector\n");
#endif
        return false;
    }
    else
    {
        if( (ba[0]=='F') && (ba[1]=='A') && (ba[2]=='T') && (ba[3]=='X') )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

FATXPartition *OpenFATXPartition(int nDriveIndex,
    unsigned int partitionOffset,
    unsigned long long partitionSize)
{
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

    if (readSize != FATX_PARTITION_HEADERSIZE)
    {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("OpenFATXPartition : Out of data while reading partition header\n");
#endif
        return NULL;
    }

    // check the magic
    if (*((unsigned long*) &partitionInfo) != FATX_PARTITION_MAGIC)
    {
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
    if (partition == NULL)
    {
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
    if (chainTableSize % FATX_CHAINTABLE_BLOCKSIZE)
    {
        // round up to nearest FATX_CHAINTABLE_BLOCKSIZE bytes
        chainTableSize = ((chainTableSize / FATX_CHAINTABLE_BLOCKSIZE) + 1)
                * FATX_CHAINTABLE_BLOCKSIZE;
    }

#ifdef FATX_DEBUG
    printk("OpenFATXPartition : Allocating chaintable struct\n");
#endif
      // Load the cluster chain map table
    partition->clusterChainMap.words = (unsigned short*) malloc(chainTableSize);
    if (partition->clusterChainMap.words == NULL)
    {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("OpenFATXPartition : Out of memory\n");
#endif	
        free(partition);
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

    if (readSize != chainTableSize)
    {
        VIDEO_ATTR=0xffe8e8e8;
#ifdef FATX_INFO
        printk("Out of data while reading cluster chain map table\n");
#endif
    }
    partition->cluster1Address = ( ( FATX_PARTITION_HEADERSIZE + chainTableSize) );

    return partition;
}

void DumpFATXTree(FATXPartition *partition)
{
    // OK, start off the recursion at the root FAT
    _DumpFATXTree(partition, FATX_ROOT_FAT_CLUSTER, 0);
}

void _DumpFATXTree(FATXPartition* partition, int clusterId, int nesting)
{

    int endOfDirectory;
    unsigned char* curEntry;
    unsigned char clusterData[partition->clusterSize];
    int i,j;
    char writeBuf[512];
    char filename[50];
    unsigned long filenameSize;
    unsigned long fileSize;
    unsigned long entryClusterId;
    unsigned char flags;
    char flagsStr[5];

    // OK, output all the directory entries
    endOfDirectory = 0;
    while(clusterId != -1)
    {
        LoadFATXCluster(partition, clusterId, clusterData);

        // loop through it, outputing entries
        for(i=0; i< partition->clusterSize / FATX_DIRECTORYENTRY_SIZE; i++)
        {

            // work out the currentEntry
            curEntry = clusterData + (i * FATX_DIRECTORYENTRY_SIZE);

            // first of all, check that it isn't an end of directory marker
            if (checkForLastDirectoryEntry(curEntry))
            {
                endOfDirectory = 1;
                break;
            }

            // get the filename size
            filenameSize = curEntry[0];

            // check if file is deleted
            if (filenameSize == 0xE5)
            {
                continue;
            }

            // check size is OK
            if ((filenameSize < 1) || (filenameSize > FATX_FILENAME_MAX))
            {
                VIDEO_ATTR=0xffe8e8e8;
                printk("_DumpFATXTree : Invalid filename size: %i\n", filenameSize);
            }

            // extract the filename
            memset(filename, 0, 50);
            memcpy(filename, curEntry+2, filenameSize);
            filename[filenameSize] = 0;

            // get rest of data
            flags = curEntry[1];
            entryClusterId = *((unsigned long*) (curEntry + 0x2c));
            fileSize = *((unsigned long*) (curEntry + 0x30));

            // wipe fileSize
            if (flags & FATX_FILEATTR_DIRECTORY)
            {
                fileSize = 0;
            }

            // zap flagsStr
            strcpy(flagsStr, "    ");

            // work out other flags
            if (flags & FATX_FILEATTR_READONLY)
            {
                          flagsStr[0] = 'R';
            }
            if (flags & FATX_FILEATTR_HIDDEN)
            {
                flagsStr[1] = 'H';
            }
            if (flags & FATX_FILEATTR_SYSTEM)
            {
                flagsStr[2] = 'S';
            }
            if (flags & FATX_FILEATTR_ARCHIVE)
            {
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
            for(j=0; j< nesting; j++)
            {
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

        if (endOfDirectory)
        {
            break;
        }    
        clusterId = getNextClusterInChain(partition, clusterId);
    }
}

int FATXLoadFromDisk(FATXPartition* partition, FATXFILEINFO *fileinfo)
{

    unsigned char clusterData[partition->clusterSize];
    int fileSize = fileinfo->fileSize;
    int written;
    int clusterId = fileinfo->clusterId;
    unsigned char *ptr;

    fileinfo->fileRead = 0;
    ptr = fileinfo->buffer;

    // loop, outputting clusters
    while(clusterId != -1)
    {
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
    if (fileSize != 0)
    {
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
                    int clusterId, FATXFILEINFO *fileinfo)
{

    int i = 0;
#ifdef FATX_DEBUG
    VIDEO_ATTR=0xffc8c8c8;
    printk("FATXFindFile : %s\n",filename);
#endif

    // convert any '\' to '/' characters
    while(filename[i] != 0)
    {
        if (filename[i] == '\\')
        {
              filename[i] = '/';
        }

        i++;
    }

    // skip over any leading / characters
    i=0;
    while((filename[i] != 0) && (filename[i] == '/'))
    {
        i++;
    }

    return _FATXFindFile(partition,&filename[i],clusterId,fileinfo);
}

int _FATXFindFile(FATXPartition* partition,
                    char* filename,
                    int clusterId, FATXFILEINFO *fileinfo)
{
    unsigned char* curEntry;
    unsigned char clusterData[partition->clusterSize];
    int i = 0;
    int endOfDirectory;
    unsigned long filenameSize;
    unsigned long flags;
    unsigned long entryClusterId;
    unsigned long fileSize;
    char seekFilename[50];
    char foundFilename[50];
    char* slashPos;
    int lookForDirectory = 0;
    int lookForFile = 0;


    // work out the filename we're looking for
    slashPos = strrchr0(filename, '/');
    if (slashPos == NULL)
    {
    // looking for file
        lookForFile = 1;

        // check seek filename size
        if (strlen(filename) > FATX_FILENAME_MAX)
        {
#ifdef FATX_INFO
            printk("Bad filename supplied (one leafname is too big)\n");
#endif
            return false;
        }

        // copy the filename to look for
        strcpy(seekFilename, filename);
    }
    else
    {
        // looking for directory
        lookForDirectory = 1;

        // check seek filename size
        if ((slashPos - filename) > FATX_FILENAME_MAX)
        {
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
    while(clusterId != -1)
    {
            // load cluster data
        LoadFATXCluster(partition, clusterId, clusterData);

        // loop through it, outputing entries
        for(i=0; i< partition->clusterSize / FATX_DIRECTORYENTRY_SIZE; i++)
        {
            // work out the currentEntry
            curEntry = clusterData + (i * FATX_DIRECTORYENTRY_SIZE);

            // first of all, check that it isn't an end of directory marker
            if (checkForLastDirectoryEntry(curEntry))
            {
                endOfDirectory = 1;
                break;
            }

            // get the filename size
            filenameSize = curEntry[0];

            // check if file is deleted
            if (filenameSize == 0xE5)
            {
                continue;
            }

            // check size is OK
            if ((filenameSize < 1) || (filenameSize > FATX_FILENAME_MAX))
            {
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
            entryClusterId = *((unsigned long*) (curEntry + 0x2c));
            fileSize = *((unsigned long*) (curEntry + 0x30));

            // is it what we're looking for...
            if (strlen(seekFilename)==strlen(foundFilename) && _strncasecmp(foundFilename, seekFilename,strlen(seekFilename)) == 0)
            {
                // if we're looking for a directory and found a directory
                if (lookForDirectory)
                {
                    if (flags & FATX_FILEATTR_DIRECTORY)
                    {
                        return _FATXFindFile(partition, slashPos+1, entryClusterId,fileinfo);
                    }
                    else
                    {
#ifdef FATX_INFO
                        printk("File not found\n");
#endif
                        return false;
                    }
                }

                // if we're looking for a file and found a file
                if (lookForFile)
                {
                    if (!(flags & FATX_FILEATTR_DIRECTORY))
                    {
                        fileinfo->clusterId = entryClusterId;
                        fileinfo->fileSize = fileSize;
                        memset(fileinfo->filename,0,sizeof(fileinfo->filename));
                        strcpy(fileinfo->filename,filename);
                        return true;
                    }
                    else
                    {
#ifdef FATX_INFO
                        printk("File not found %s\n",filename);
#endif
                        return false;
                    }
                }
            }
        }

        // have we hit the end of the directory yet?
        if (endOfDirectory)
        {
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



unsigned long getNextClusterInChain(FATXPartition* partition, int clusterId)
{
    int nextClusterId = 0;
    unsigned long eocMarker = 0;
    unsigned long rootFatMarker = 0;
    unsigned long maxCluster = 0;

    // check
    if (clusterId < 1)
    {
        VIDEO_ATTR=0xffe8e8e8;
        printk("getNextClusterInChain : Attempt to access invalid cluster: %i\n", clusterId);
    }

    // get the next ID
    if (partition->chainMapEntrySize == 2)
    {
        nextClusterId = partition->clusterChainMap.words[clusterId];
        eocMarker = 0xffff;
        rootFatMarker = 0xfff8;
        maxCluster = 0xfff4;
    }
    else if (partition->chainMapEntrySize == 4)
    {
        nextClusterId = partition->clusterChainMap.dwords[clusterId];
        eocMarker = 0xffffffff;
        rootFatMarker = 0xfffffff8;
        maxCluster = 0xfffffff4;
    }
    else
    {
        VIDEO_ATTR=0xffe8e8e8;
        printk("getNextClusterInChain : Unknown cluster chain map entry size: %i\n", partition->chainMapEntrySize);
    }

    // is it the end of chain?
    if ((nextClusterId == eocMarker) || (nextClusterId == rootFatMarker))
    {
        return -1;
    }
    
    // is it something else unknown?
    if (nextClusterId == 0)
    {
        VIDEO_ATTR=0xffe8e8e8;
        printk("getNextClusterInChain : Cluster chain problem: Next cluster after %i is unallocated!\n", clusterId);
    }
    if (nextClusterId > maxCluster)
    {
        printk("getNextClusterInChain : Cluster chain problem: Next cluster after %i has invalid value: %i\n", clusterId, nextClusterId);
    }
    
    // OK!
    return nextClusterId;
}

void LoadFATXCluster(FATXPartition* partition, int clusterId, unsigned char* clusterData)
{
    unsigned long long clusterAddress;
    unsigned long long readSize;
    
    // work out the address of the cluster
    clusterAddress = partition->cluster1Address + ((unsigned long long)(clusterId - 1) * partition->clusterSize);

    // Now, load it
    readSize = FATXRawRead(partition->nDriveIndex, partition->partitionStart,
            clusterAddress, partition->clusterSize, clusterData);

    if (readSize != partition->clusterSize)
    {
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
                    //unsigned char ba[512];
            if(BootIdeReadSector(drive, buf, sector, 0, 512)) {
                VIDEO_ATTR=0xffe8e8e8;
                printk("Unable to get first sector\n");
                                return false;
            }
            //memcpy(buf, &ba[byte_offset], nThisTime-byte_offset);
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

void FATXCreateDirectoryEntry(unsigned char * buffer, char *entryName, unsigned int entryNumber, unsigned int cluster){
    unsigned int offset = entryNumber * 0x40;

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

    return;                                                     //buffer is updated, ready to be written on HDD.
}

/********** Old way, Done at drive Init now. Leave for legacy. **********
bool FATXCheckBRFR(unsigned char drive){
    unsigned char ba[512];
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

void FATXSetBRFR(unsigned char drive){
	unsigned char buffer[512];
	unsigned int counter;
	
	memset(buffer, 0, 512);
        for(counter = 1; counter < 1024; counter++){             //Set first 512KB of HDD to 0x00.
            if(BootIdeWriteSector(drive, buffer, counter, DEFAULT_WRITE_RETRY)){        //512KB = 1024 sectors.
                printk("\n           FATXSetBRFR: Write error, sector %u   ", counter);
                cromwellWarning();
                return;
            }
        }
        sprintf(buffer,"BRFR");
        if(BootIdeWriteSector(drive, buffer, 3, DEFAULT_WRITE_RETRY)){       //Write "BRFR" string and number of boots(0) at absolute offset 0x600
            printk("\n           FATXSetBRFR: Write error, sector %u   ", 3);
            cromwellWarning();
            return;
        }
        tsaHarddiskInfo[drive].m_enumDriveType = EDT_XBOXFS;
	
}

bool FATXCheckMBR(unsigned char driveId)
{
    unsigned char *sourceTable = (unsigned char *)&BackupPartTbl;
    unsigned char i;
    unsigned char ba[512];
    if(BootIdeReadSector(driveId, &ba[0], 0x00, 0, 512)) {
        printk("\n\n\n           FATXCheckMBR : Unable to read MBR sector\n");
        //debugSPIPrint("Unable to read MBR sector (0).\n");
        return 0;
    }
    else{
        for(i = 0; i < 48; i++){
            if(ba[i] != sourceTable[i]){         //Contains generic MBR header
                //debugSPIPrint("Partition table header not properly constructed.\n");
                return 0;                       //First 48 bytes should always be identical for every Part tables.
            }
        }
        for(i = 48; i < 208; i++){
            if(ba[i] != sourceTable[i]){         //Contains standard Xbox Partitions (C,E,X,Y,Z)
                //debugSPIPrint("Partition table base entries(C,E,X,Y,Z) not standard.\n");
                return -1;                      //If basic partition entries contains unconventional values, return error.
            }
        }
    }
    //debugSPIPrint("Drive has valid MBR partition table.\n");
    return 1;
}

int FATXCheckFATXMagic(unsigned char driveId)
{
    unsigned char ba[512];
    if(BootIdeReadSector(driveId, &ba[0], 0x03, 0, 512)) {
        printk("\n\n\n           FATXCheckFATXMagic : Unable to read MBR sector\n");
        //debugSPIPrint("Unable to read FATX sector (3).");
        return 0;
    }
    if((ba[0]=='B') && (ba[1]=='R') && (ba[2]=='F') && (ba[3]=='R')){
        tsaHarddiskInfo[driveId].m_enumDriveType=EDT_XBOXFS;
    }
    //Everything went fine.
    return 1;
}


void FATXSetMBR(unsigned char driveId, XboxPartitionTable *p_table){
    unsigned char *sourceTable = (unsigned char *)p_table;
    if(BootIdeWriteSector(driveId,sourceTable, 0, DEFAULT_WRITE_RETRY)){    //Write on sector 0
        printk("\n           FATXSetMBR: Write error, sector %u   ", 0);
        cromwellWarning();
        return;
    }
    tsaHarddiskInfo[driveId].m_fHasMbr = 1;
}

void FATXSetInitMBR(unsigned char driveId){
    if(BootIdeWriteSector(driveId,(unsigned char *)&BackupPartTbl, 0, DEFAULT_WRITE_RETRY)){   //Write on sector 0
        printk("\n           FATXSetInitMBR: Write error, sector %u   ", 0);
        cromwellWarning();
        return;
    }
    tsaHarddiskInfo[driveId].m_fHasMbr = 1;
}

void FATXFormatCacheDrives(int nIndexDrive, bool verbose){
    unsigned char buffer[512], headerBuf[0x1000], driveLetter[3];
/********** Old way, sector by sector. Leave for legacy. **********
    unsigned char chainmapBuf[512];
**********************************************************************/
    unsigned char *ptrBuffer;
    unsigned int counter;
    unsigned int whichpartition;
    PARTITIONHEADER *header;

    VIDEO_ATTR=0xffd8d8d8;

    if(tsaHarddiskInfo[nIndexDrive].m_enumDriveType != EDT_XBOXFS)
    	FATXSetBRFR(nIndexDrive);

    memset(headerBuf,0xff,0x1000);              //First sector(and only one used) of the Partition header area.
    header = (PARTITIONHEADER *)headerBuf;
    header->magic = FATX_PARTITION_MAGIC;       //Whooo, magic!
    header->volumeID = 'X';					    //Goes with the HDD.
    header->clusterSize = 0x20;                 //16KB cluster, so 32 clusters per sector.
    header->nbFAT = 1;                          //Always 1.
    header->unknown = 0;                        //Always 0.
    memset(header->unused,0xff,4078);           //Fill unused area with 0xff.
    //Partition header is ready. It's the same for all "stock" partitions.
    //It's not necessary to fill unused area up to 0x1000 because we'll only write the first 512 bytes of headerBuf
    //onto the HDD. Let's do it for the exercise OK? A few wasted cycles isn't going to hurt anybody.

/********** Old way, sector by sector. Leave for legacy. **********
    memset(chainmapBuf,0x0,512);                //First sector of the Cluster chain map area.
    chainmapBuf[0]=0xf8;                        //First 2 clusters are 0xFFF8 in word mode (FATX16).
    chainmapBuf[1]=0xff;
    chainmapBuf[2]=0xff;
    chainmapBuf[3]=0xff;
**********************************************************************/
    ptrBuffer = (unsigned char *)malloc(192 * 512);    //chainmap buffer total length.
    memset(ptrBuffer,0x0,512 * 192);
    ptrBuffer[0]=0xf8;                        //First 2 clusters are 0xFFF8 in word mode (FATX16).
    ptrBuffer[1]=0xff;
    ptrBuffer[2]=0xff;
    ptrBuffer[3]=0xff;

    //Cycle through all 3 cache partitions.
    for(whichpartition = SECTOR_CACHE1; whichpartition <= SECTOR_CACHE3; whichpartition += (SECTOR_CACHE2 - SECTOR_CACHE1)){
        if(whichpartition == SECTOR_CACHE1)
        {
            sprintf(driveLetter, "%s", "X:");
            header->volumeID = 'X';
        }
        else if(whichpartition == SECTOR_CACHE2)
        {
            sprintf(driveLetter, "%s", "Y:");
            header->volumeID = 'Y';
        }
        else
        {
            sprintf(driveLetter, "%s", "Z:");
            header->volumeID = 'Z';
        }

        memset(buffer,0xff,512);                    //Killer buffer.
        
        if(verbose)
        {
            printk("\n\n           %s  Writing Boot Block.   ", driveLetter);
        }
        // Starting (from 0 to 512*8 = 0x1000). Erasing Partition header data.
        //4KB so 8*512 bytes sectors.
        for (counter=whichpartition;counter<(whichpartition+8); counter++) {
            if(BootIdeWriteSector(nIndexDrive,buffer,counter, DEFAULT_WRITE_RETRY)){
                printk("\n           Write error, sector %u   ", counter);
                cromwellWarning();
                return;
            }
        }
        //Write Partition info on first sector. Last seven sectors of the first 0x1000 are already 0xff anyway.
        if(BootIdeWriteSector(nIndexDrive,headerBuf,whichpartition, DEFAULT_WRITE_RETRY)){   //Partition header write.
            printk("\n           Write error, sector %u   ", whichpartition);
            cromwellWarning();
            return;
        }
        if(verbose)
            cromwellSuccess();

        if(verbose)
            printk("           %s  Writing Cluster Chain map.   ", driveLetter);
/********** Old way, sector by sector. Leave for legacy. **********
        // Cluster chain map area (from 512*8 = 0x1000 to 512*200 = 0x19000)
        memset(buffer,0x0,512); //wipe. Unused cluster == 0
        for (counter=(whichpartition+8);counter<(whichpartition+200); counter++) {
            if(BootIdeWriteSector(nIndexDrive,buffer,counter, DEFAULT_WRITE_RETRY)){
                printk("\n           Write error, sector %u   ", counter);
                cromwellWarning();
                return;
            }
        }
        //Write initial cluster chain map.
        if(BootIdeWriteSector(nIndexDrive,chainmapBuf,whichpartition+8, DEFAULT_WRITE_RETRY)){   //Initial Cluster chain map write.
            printk("\n           Write error, sector %u   ", whichpartition+8);
            cromwellWarning();
            return;
        }
**********************************************************************/
        if(BootIdeWriteMultiple(nIndexDrive, ptrBuffer, whichpartition+8, 192, DEFAULT_WRITE_RETRY)){   //Initial Cluster chain map write.
            printk("\n           Write error, Cluster Chainmap   ");                                   //Length for cache drive is fixed at 192 sectors
            cromwellWarning();
            return;
        }
        if(verbose)
            cromwellSuccess();

        if(verbose)
            printk("           %s  Finalizing.   ", driveLetter);
        // Root Dir (from 512*200 = 0x19000 to 0x1d000 = 512*232)
        memset(buffer,0x00,512);
        //Format 2 first clusters
        for (counter=(whichpartition+200);counter<(whichpartition+200+(32*2)); counter++) {
            if(BootIdeWriteSector(nIndexDrive,buffer,counter, DEFAULT_WRITE_RETRY)){
                printk("\n           Write error, sector %u   ", counter);
                cromwellWarning();
                return;
            }
        }
        //For a total of 264 sectors written, each partition.
        if(verbose)
            cromwellSuccess();
    }
    free(ptrBuffer);
}

void FATXFormatDriveC(int nIndexDrive, bool verbose){
    unsigned char buffer[512], headerBuf[0x1000];
/********** Old way, sector by sector. Leave for legacy. **********
    unsigned char chainmapBuf[512];
**********************************************************************/
    unsigned char *ptrBuffer;


    unsigned int counter;
    PARTITIONHEADER *header;

    if(tsaHarddiskInfo[nIndexDrive].m_enumDriveType != EDT_XBOXFS)
        FATXSetBRFR(nIndexDrive);

    memset(headerBuf,0xff,0x1000);              //First sector(and only one used) of the Partition header area.
    header = (PARTITIONHEADER *)headerBuf;
    header->magic = FATX_PARTITION_MAGIC;       //Whooo, magic!
    header->volumeID = 'C';    					//Goes with the HDD.
    header->clusterSize = 0x20;                 //16KB cluster, so 32 clusters per sector.
    header->nbFAT = 1;                          //Always 1.
    header->unknown = 0;                        //Always 0.
    memset(header->unused,0xff,4078);           //Fill unused area with 0xff.
    //Partition header is ready. It's the same for all "stock" partitions.
    //It's not necessary to fill unused area up to 0x1000 because we'll only write the first 512 bytes of headerBuf
    //onto the HDD. Let's do it for the exercise OK? A few wasted cycles isn't going to hurt anybody.

    ptrBuffer = (unsigned char *)malloc(128 * 512);    //chainmap buffer total length.
    memset(ptrBuffer,0x0,512 * 128);
/********** Old way, sector by sector. Leave for legacy. **********
    memset(chainmapBuf,0x0,512);                //First sector of the Cluster chain map area.
    chainmapBuf[0]=0xf8;                        //First cluster is 0xFFF8 in word mode cluster.
    chainmapBuf[1]=0xff;
    chainmapBuf[2]=0xff;
    chainmapBuf[3]=0xff;
**********************************************************************/

    ptrBuffer[0]=0xf8;                        //First 2 clusters are 0xFFF8  and 0xFFFF in word mode cluster.
    ptrBuffer[1]=0xff;
    ptrBuffer[2]=0xff;
    ptrBuffer[3]=0xff;

    memset(buffer,0xff,512);                    //Killer buffer.

    if(verbose)
        printk("\n\n           Writing Boot Block.   ");
    // Starting (from 0 to 512*8 = 0x1000). Erasing Partition header data.
    //4KB so 8*512 bytes sectors.
    for (counter=SECTOR_SYSTEM;counter<(SECTOR_SYSTEM+8); counter++) {
        if(BootIdeWriteSector(nIndexDrive,buffer,counter, DEFAULT_WRITE_RETRY)){
            printk("\n           Write error, sector %u   ", counter);
            cromwellWarning();
            return;
        }
    }
    //Write Partition info on first sector. lest seven sectors of the first 0x1000 are already 0xff anyway.
    if(BootIdeWriteSector(nIndexDrive,headerBuf,SECTOR_SYSTEM, DEFAULT_WRITE_RETRY)){   //Partition header write.
        printk("\n           Write error, sector %u   ", SECTOR_SYSTEM);
        cromwellWarning();
        return;
    }
    if(verbose)
        cromwellSuccess();

    if(verbose)
        printk("\n\n           Writing Cluster Chain map.   ");


/********** Old way, sector by sector. Leave for legacy. **********
    // Cluster chain map area (from 512*8 = 0x1000 to 512*136 = 0x11000)
    memset(buffer,0x0,512); //wipe. Unused cluster == 0
    for (counter=(SECTOR_SYSTEM+8);counter<(SECTOR_SYSTEM+136); counter++) {
        if(BootIdeWriteSector(nIndexDrive,buffer,counter, DEFAULT_WRITE_RETRY)){
            printk("\n           Write error, sector %u   ", counter);
            cromwellWarning();
            return;
        }
    }

    //Write initial cluster chain map.
    if(BootIdeWriteSector(nIndexDrive,chainmapBuf,SECTOR_SYSTEM+8, DEFAULT_WRITE_RETRY)){   //Initial Cluster chain map write.
        printk("\n           Write error, sector %u   ", SECTOR_SYSTEM+8);
        cromwellWarning();
        return;
    }
**********************************************************************/

    if(BootIdeWriteMultiple(nIndexDrive, ptrBuffer, SECTOR_SYSTEM+8, 128, DEFAULT_WRITE_RETRY)){   //Initial Cluster chain map write.
            printk("\n           Write error, Cluster Chainmap   ");                               //Length for C: drive is fixed at 128 sectors
            cromwellWarning();
            return;
    }
    free(ptrBuffer);



    if(verbose)
        cromwellSuccess();

    if(verbose)
        printk("\n\n           Finalizing.   ");
    // Root Dir (from 512*136 = 0x11000 )
    memset(buffer,0x00,512);
    //Format 2 first clusters
    for (counter=(SECTOR_SYSTEM+136);counter<(SECTOR_SYSTEM+136+(32*2)); counter++) {
        if(BootIdeWriteSector(nIndexDrive,buffer,counter, DEFAULT_WRITE_RETRY)){
            printk("\n           Write error, sector %u   ", counter);
            cromwellWarning();
            return;
        }
    }
    //For a total of 200 consecutive sectors written.
    if(verbose)
        cromwellSuccess();
}

void FATXFormatDriveE(int nIndexDrive, bool verbose){
    unsigned char buffer[512], headerBuf[0x1000], i;
/********** Old way, sector by sector. Leave for legacy. **********
    unsigned char chainmapBuf[512];
**********************************************************************/
    unsigned char *ptrBuffer;
    unsigned int counter;
    PARTITIONHEADER *header;

    if(tsaHarddiskInfo[nIndexDrive].m_enumDriveType != EDT_XBOXFS)
        FATXSetBRFR(nIndexDrive);

    memset(headerBuf,0xff,0x1000);              //First sector(and only one used) of the Partition header area.
    header = (PARTITIONHEADER *)headerBuf;
    header->magic = FATX_PARTITION_MAGIC;       //Whooo, magic!
    header->volumeID = 'E';					    //Goes with the HDD.
    header->clusterSize = 0x20;                 //16KB cluster, so 32 clusters per sector.
    header->nbFAT = 1;                          //Always 1.
    header->unknown = 0;                        //Always 0.
    memset(header->unused,0xff,4078);           //Fill unused area with 0xff.
    //Partition header is ready. It's the same for all "stock" paritions.
    //It's not necessary to fill unused area up to 0x1000 because we'll only write the first 512 bytes of headerBuf
    //onto the HDD. Let's do it for the exercise OK? A few wasted cycles isn't going to hurt anybody.

/********** Old way, sector by sector. Leave for legacy. **********
    memset(chainmapBuf,0x0,512);                //First sector of the Cluster chain map area.
    memset(chainmapBuf,0xff,4*7);               //We'll use 5 clusters for base folders.
    chainmapBuf[0]=0xf8;                        //First cluster is 0xFFFFFFF8 in 4 byte mode cluster.
**********************************************************************/
    ptrBuffer = (unsigned char *)malloc(256 * 512);    //chainmap buffer. Length is of a single MULTIPLE WRITE ATA command.
    memset(ptrBuffer,0x0,512 * 256);

    memset(buffer,0xff,512);                    //Killer buffer.

    if(verbose)
        printk("\n\n           Writing Boot Block.   ");
    // Starting (from 0 to 512*8 = 0x1000). Erasing Partition header data.
    //4KB so 8*512 bytes sectors.
    for (counter=SECTOR_STORE;counter<(SECTOR_STORE+8); counter++) {
        if(BootIdeWriteSector(nIndexDrive,buffer,counter, DEFAULT_WRITE_RETRY)){
            printk("\n           Write error, sector %u   ", counter);
            cromwellWarning();
            return;
        }
    }
    //Write Partition info on first sector. last seven sectors of the first 0x1000 are already 0xff anyway.
    if(BootIdeWriteSector(nIndexDrive,headerBuf,SECTOR_STORE, DEFAULT_WRITE_RETRY)){   //Partition header write.
        printk("\n           Write error, sector %u   ", SECTOR_STORE);
        cromwellWarning();
    }
    if(verbose)
        cromwellSuccess();

    if(verbose)
        printk("\n\n           Writing Cluster Chain map.   ");
/********** Old way, sector by sector. Leave for legacy. **********
    // Cluster chain map area (from 512*8 = 0x1000 to 512*2456 = 0x133000)
    memset(buffer,0x0,512); //wipe. Unused cluster == 0
    for (counter=(SECTOR_STORE+8);counter<(SECTOR_STORE+2456); counter++) {
        if(BootIdeWriteSector(nIndexDrive,buffer,counter, DEFAULT_WRITE_RETRY)){
            printk("\n           Write error, sector %u   ", counter);
            cromwellWarning();
            return;
        }
    }
    //Write initial cluster chain map.
    if(BootIdeWriteSector(nIndexDrive,chainmapBuf,SECTOR_STORE+8, DEFAULT_WRITE_RETRY)){   //Initial Cluster chain map write.
        printk("\n           Write error, sector %u   ", SECTOR_STORE+8);
        cromwellWarning();
        return;
    }
**********************************************************************/
    for(i = 0; i < 9; i++){                                    //Must be done 9 times as WRITE MULTIPLE will only take buffers of 256 sectors long.
        //Start by writing 0 everywhere, skip the first 144 sectors to write 9*256 sectors up to the end,
        //Reuse 9 times memory allocated (all set to 0x00) of 256*512 bytes in size
        if(BootIdeWriteMultiple(nIndexDrive, ptrBuffer, SECTOR_STORE+8+144+(i << 8), 256, DEFAULT_WRITE_RETRY)){   //Initial Cluster chain map write.
            printk("\n           Write error, Cluster Chainmap   ");                               //Length for E: drive is fixed at 2448 sectors
            cromwellWarning();
            return;
        }
    }
    memset(ptrBuffer,0xff,4*7);               //We'll use 5 clusters for base folders(+2 first clusters used as specs).
    ptrBuffer[0]=0xf8;                        //First cluster is 0xFFFFFFF8 in 4 byte mode cluster.

    //One last time for the first 144 sectors, with initial cluster entries.
    if(BootIdeWriteMultiple(nIndexDrive, ptrBuffer, SECTOR_STORE+8, 144, DEFAULT_WRITE_RETRY)){   //Initial Cluster chain map write.
        printk("\n           Write error, Cluster Chainmap   ");                               //Length for E: drive is fixed at 2448 sectors
        cromwellWarning();
        return;
    }
    free(ptrBuffer);

    if(verbose)
        cromwellSuccess();

    if(verbose)
        printk("\n\n           Finalizing, creating directories.   ");
    // Root Dir (from 512*2456 = 0x133000 to 0x1d000 = 512*232)
    //memset(buffer,0xff,512);
    //Format 6 first clusters
    ptrBuffer = (unsigned char *)malloc(224 * 512);    //chainmap buffer. Length is of a single MULTIPLE WRITE ATA command.
    memset(ptrBuffer,0x00,512 * 224);
/********** Old way, sector by sector. Leave for legacy. **********
    for (counter=(SECTOR_STORE+2456);counter<(SECTOR_STORE+2456+(32*6)); counter++) {
        if(BootIdeWriteSector(nIndexDrive,buffer,counter, DEFAULT_WRITE_RETRY)){
            printk("\n           Write error, sector %u   ", counter);
            cromwellWarning();
            return;
        }
    }
**********************************************************************/
    if(BootIdeWriteMultiple(nIndexDrive, ptrBuffer, SECTOR_STORE+2456, 224, DEFAULT_WRITE_RETRY)){   //Format 7 first clusters.
        printk("\n           Write error, Clusters 0 to 6   ");                               //Length is fixed at 224 sectors
        cromwellWarning();
        return;
    }
    free(ptrBuffer);

    // memset(buffer,0x00,512);
    // TDATA Dir points to Cluster 2
    FATXCreateDirectoryEntry(buffer,"TDATA",0,2);
    // UDATA Dir points to Cluster 4
    FATXCreateDirectoryEntry(buffer,"UDATA",1,4);
    // CACHE Dir points to Cluster 6
    FATXCreateDirectoryEntry(buffer,"CACHE",2,6);
    if(BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456, DEFAULT_WRITE_RETRY)){   // Write Cluster 1(E: Root).
        printk("\n           Write error, sector %u   ", SECTOR_STORE+2456);
        cromwellWarning();
        return;
    }

    memset(buffer,0x00,512);
    //CACHE dir is empty to 0x00 everywhere.
    //BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456+32+32+32+32+32+32);   // Write Cluster 6(CACHE).
    // FFFE0000 Dir points to Cluster 3
    FATXCreateDirectoryEntry(buffer,"FFFE0000",0,3);
    if(BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456+32, DEFAULT_WRITE_RETRY)){   // Write Cluster 2(TDATA).
        printk("\n           Write error, sector %u   ", SECTOR_STORE+2456+32);
        cromwellWarning();
        return;
    }
    memset(buffer,0x00,512);
    // Music Dir points to Cluster 5
    FATXCreateDirectoryEntry(buffer,"Music",0,5);
    if(BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456+32+32+32, DEFAULT_WRITE_RETRY)){   // Write Cluster 4(UDATA).
        printk("\n           Write error, sector %u   ", SECTOR_STORE+2456+32+32+32);
        cromwellWarning();
        return;
    }
    //For a total of 2648 sectors written
    if(verbose)
        cromwellSuccess();
}

void FATXFormatExtendedDrive(unsigned char driveId, unsigned char partition, unsigned int lbaStart, unsigned int lbaSize){
    unsigned char buffer[512], headerBuf[0x1000];
    unsigned int i;
/********** Old way, sector by sector. Leave for legacy. **********
    unsigned char chainmapBuf[512];
**********************************************************************/
    unsigned char *ptrBuffer;
    unsigned long counter, chainmapSize = 0;
    PARTITIONHEADER *header;
    unsigned char clusterSize = 32;                //16KB cluster by default(32 sectors * 512 bytes)

    VIDEO_ATTR=0xffd8d8d8;

    XboxPartitionTable * mbr = (XboxPartitionTable *)buffer;

    if(tsaHarddiskInfo[driveId].m_enumDriveType != EDT_XBOXFS)
       FATXSetBRFR(driveId);

    if(lbaSize >= LBASIZE_512GB)         //Need 64K clusters
        clusterSize = 128;               //Clustersize in number of 512-byte sectors
    else if(lbaSize >= LBASIZE_256GB)
        clusterSize = 64;
    else
        clusterSize = 32;
        
    //debugSPIPrint("Cluster size is %uKB.\n", clusterSize / 2);
    
    //Calculate size of FAT, in number of 512-byte sectors.
    chainmapSize = (lbaSize / clusterSize);       //Divide total of sectors(512 bytes) by number of sector contained in a cluster
    chainmapSize = chainmapSize * ((lbaSize < FATX16_MAXLBA) ? 2 : 4);      //Multiply by length(in bytes) of a single entry in FAT.
                                                                            //FATX16 has 2 byte FAT entries and FATX32 has 4 bytes entries.
    chainmapSize = (chainmapSize >> 9);                                     //Divide by 512bytes,

    while((chainmapSize % 8) != 0)                    //Round it to 4096 byte boundary.
        chainmapSize += 1;
        
    //debugSPIPrint("Chainmap size is %u sectors.\n", chainmapSize);
	
    if(tsaHarddiskInfo[driveId].m_fHasMbr == 1) {                           //MBR is present on HDD
        if(BootIdeReadSector(driveId, &buffer[0], 0x00, 0, 512)) {
            VIDEO_ATTR=0xffff0000;
            printk("\n\1                Unable to read MBR sector");
            cromwellWarning();
            return;
        }
        //debugSPIPrint("MBR read on HDD, will be updating it.\n");
    }
    else
    {                                                   //If no MBR already on disk
        memcpy(mbr, &BackupPartTbl, sizeof(BackupPartTbl)); //Copy backup in working buffer and work from there.
    	//debugSPIPrint("No MBR on HDD. Will take default one and start from there.\n");
    }

    printk("\n\n\n           Writing partition table in MBR.   ");
    mbr->TableEntries[partition].Flags = PE_PARTFLAGS_IN_USE;
    mbr->TableEntries[partition].LBAStart = lbaStart;
    mbr->TableEntries[partition].LBASize = lbaSize;
    //debugSPIPrint("MBR entry writing. Starts at 0x%X. Size is 0x%X.\n", lbaStart, lbaSize);
    FATXSetMBR(driveId, mbr);
    cromwellSuccess();

    memset(headerBuf,0xff,0x1000);              //First sector(and only one used) of the Partition header area.
    header = (PARTITIONHEADER *)headerBuf;
    header->magic = FATX_PARTITION_MAGIC;       //Whoop, magic!
    header->volumeID = partition <= 5 ? 'F' : 'G';    //Goes with the HDD.
    header->clusterSize = clusterSize;   //16KB = 32 sector/cluster. 32KB = 64 sector/cluster. 64KB = 128 sector/cluster.
    header->nbFAT = 1;                          //Always 1.
    header->unknown = 0;                        //Always 0.
    memset(header->unused,0xff,4078);           //Fill unused area with 0xff.
    //Partition header is ready.
    //It's not necessary to fill unused area up to 0x1000 because we'll only write the first 512 bytes of headerBuf
    //onto the HDD. Let's do it for the exercise OK? A few wasted cycles isn't going to hurt anybody.

/********** Old way, sector by sector. Leave for legacy. **********
    memset(chainmapBuf,0x0,512);                //First sector of the Cluster chain map area.
    chainmapBuf[0]=0xf8;                        //First cluster is 0xFFFFFFF8 in 4 byte mode cluster.
    chainmapBuf[1]=0xff;
    chainmapBuf[3]=0xff;
    chainmapBuf[4]=0xff;
    if(lbaSize >= FATX16_MAXLBA){               //FATX16 stops there. Only 2-byte entries in cluster chain.
        chainmapBuf[5]=0xff;
        chainmapBuf[6]=0xff;
        chainmapBuf[7]=0xff;
        chainmapBuf[8]=0xff;
    }
**********************************************************************/
    ptrBuffer = (unsigned char *)malloc(256 * 512);    //chainmap buffer. Length is of a single MULTIPLE WRITE ATA command.
    memset(ptrBuffer,0x0,512 * 256);

    if((chainmapSize % 256) == 0){          //If we'll only issue WRITE MULTIPLE ATA commands with full 256 sectors writes.
                                            //We'll put the initial clusters reservation right now.
        ptrBuffer[0]=0xf8;                        //First cluster is 0xFFFFFFF8 in 4 byte mode cluster.
        ptrBuffer[1]=0xff;
        ptrBuffer[3]=0xff;
        ptrBuffer[4]=0xff;
        if(lbaSize >= FATX16_MAXLBA){               //FATX16 stops there. Only 2-byte entries in cluster chain.
            ptrBuffer[5]=0xff;
            ptrBuffer[6]=0xff;
            ptrBuffer[7]=0xff;
            ptrBuffer[8]=0xff;
        }
    }

    memset(buffer,0xff,512);                    //Killer buffer.

    printk("\n\n           Writing Boot Block.   ");
    // Starting (from 0 to 512*8 = 0x1000). Erasing Partition header data.
    //4KB so 8*512 bytes sectors.
    for (counter=lbaStart;counter<(lbaStart+8); counter++) {
        if(BootIdeWriteSector(driveId,buffer,counter, DEFAULT_WRITE_RETRY)){
            printk("\n           Write error, sector %u   ", counter);
            cromwellWarning();
            return;
        }
    }

    //Write Partition info on first sector. last seven sectors of the first 0x1000 are already 0xff anyway.
    if(BootIdeWriteSector(driveId,headerBuf,lbaStart, DEFAULT_WRITE_RETRY)){   //Partition header write.
        printk("\n           Write error, sector %u   ", lbaStart);
        cromwellWarning();
        return;
    }

    cromwellSuccess();

    printk("\n\n           Writing Cluster Chain map.   ");
/********** Old way, sector by sector. Leave for legacy. **********
    // Cluster chain map area (from 512*8 = 0x1000 to 512*8 + chainmapSize)
    memset(buffer,0x0,512); //wipe. Unused cluster == 0
    for (counter=(lbaStart+8);counter<(lbaStart+8+chainmapSize); counter++) {
        if(BootIdeWriteSector(driveId,buffer,counter, DEFAULT_WRITE_RETRY)){
            printk("\n           Write error, sector %u   ", counter);
            cromwellWarning();
            return;
        }
    }
    //Write initial cluster chain map.
    if(BootIdeWriteSector(driveId,chainmapBuf,lbaStart+8, DEFAULT_WRITE_RETRY)){   //Initial Cluster chain map write.
        printk("\n           Write error, sector %u   ", lbaStart+8);
        cromwellWarning();
        return;
    }
**********************************************************************/

    for(i = 0; i < (chainmapSize >> 8); i++){                         //Must be done multiple times as WRITE MULTIPLE will only take buffers of 256 sectors long.
                                                                      //In fact, the number of time chainmapSize can fit 256.
        //Start by writing 0 everywhere, skip the first 144 sectors to write 9*256 sectors up to the end,
        //Reuse n times memory allocated (all set to 0x00) of 256*512 bytes in size in the event (chainmapSize % 256) != 0.
        //If (chainmapSize % 256) == 0, start of buffer contains initial chainmap initialization.
        if(BootIdeWriteMultiple(driveId, ptrBuffer, lbaStart+8+(chainmapSize % 256)+(i << 8), 256, DEFAULT_WRITE_RETRY)){   //Initial Cluster chain map write.
            printk("\n           Write error, Cluster Chainmap   ");                               //Length for E: drive is fixed at 2448 sectors
            cromwellWarning();
            return;
        }
    }

    if(chainmapSize % 256){                       //If there's a partial WRITE MULTIPLE COMMAND to be issued.
        if(lbaSize >= FATX16_MAXLBA)
            memset(ptrBuffer,0xff,4*2);          //FATX32 partition, cluster entry is Dword-sized
        else
            memset(ptrBuffer,0xff,2*2);
        ptrBuffer[0]=0xf8;                        //FATX16 partitions, cluster entry is word-sized

        //One last time for the first 144 sectors, with initial cluster entries.
        if(BootIdeWriteMultiple(driveId, ptrBuffer, lbaStart+8, (chainmapSize % 256), DEFAULT_WRITE_RETRY)){   //Initial Cluster chain map write.
            printk("\n           Write error, Cluster Chainmap   ");                               //Length for E: drive is fixed at 2448 sectors
            cromwellWarning();
            return;
        }
    }
    free(ptrBuffer);
    cromwellSuccess();

    // Root Dir
    // 2 clusters formatted.
    printk("\n\n           Finalizing.   ");
/********** Old way, sector by sector. Leave for legacy. **********
    memset(buffer,0xff,512);
    for (counter=(lbaStart+8+chainmapSize);counter<(lbaStart+8+chainmapSize+(clusterSize*2)); counter++) {
        if(BootIdeWriteSector(driveId,buffer,counter, DEFAULT_WRITE_RETRY)){
            printk("\n           Write error, sector %u   ", counter);
            cromwellWarning();
            return;
        }
    }
**********************************************************************/
    ptrBuffer = (unsigned char *)malloc(clusterSize*2 * 512);    //2 first clusters.
    memset(ptrBuffer,0xff,512 * clusterSize*2);

    if(BootIdeWriteMultiple(driveId, ptrBuffer, lbaStart+8+chainmapSize, clusterSize*2, DEFAULT_WRITE_RETRY)){   //Format 6 first clusters.
        printk("\n           Write error, Clusters 0 to 1   ");                               //Length is fixed at 192 sectors
        cromwellWarning();
        return;
    }

    free(ptrBuffer);
    //for a total of 8 + chainmapSize + (clustersize * 2) sectors written.
    cromwellSuccess();


    return;
}
