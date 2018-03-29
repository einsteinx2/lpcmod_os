/*
 * PartTable.h
 *
 *  Created on: Mar 27, 2018
 *      Author: cromwelldev
 */

#ifndef FS_FF12B_SRC_FATFSACCESSOR_H_
#define FS_FF12B_SRC_FATFSACCESSOR_H_

#define NbDrivesSupported 2
#define HDD_Master  0
#define HDD_Slave   0

#define NbFATXPartPerHDD 7
#define Part_C    1
#define Part_E    0
#define Part_F    5
#define Part_G    6
#define Part_X    2
#define Part_Y    3
#define Part_Z    4

typedef enum
{
    XboxDiskLayout_Base,
    XboxDiskLayout_FOnly,
    XboxDiskLayout_F120GRest,
    XboxDiskLayout_FGSplit
}XboxDiskLayout;

typedef int FILE;

typedef enum
{
    FileOpenMode_Read = FA_READ,
    FileOpenMode_Write = FA_WRITE,
    FileOpenMode_OpenExistingOnly = FA_OPEN_EXISTING,
    FileOpenMode_CreateNewOnly = FA_CREATE_NEW,
    FileOpenMode_CreateAlways = FA_CREATE_ALWAYS,
    FileOpenMode_OpenAlways = FA_OPEN_ALWAYS,
    FileOpenMode_OpenAppend = FA_OPEN_APPEND
}FileOpenMode;

typedef int DIRE;


void FatFS_init(void);
int mountAll(unsigned char driveNumber);
int fdisk(unsigned char driveNumber, XboxDiskLayout xboxDiskLayout);
int fatxmkfs(unsigned char driveNumber, unsigned char partNumber);
FILE fopen(const char* path, FileOpenMode mode);
int fclose(FILE handle);
int fread(FILE handle, unsigned char* out, unsigned int size);
int fwrite(FILE handle, const unsigned char* in, unsigned int size);
int fseek(FILE handle, unsigned int offset);
int fsync(FILE handle);
int mkdir(const char* path);
DIRE fopendir(const char* path);
//TODO: readdir with output struct
//TODO: findfirst
//TODO: findnext
int frewinddir(DIRE handle);
int fclosedir(DIRE handle);
int fdelete(const char* path);
int frename(const char* path, const char* newName);
int fchdir(const char* path);
int fchdrive(const char* path);
const char* getcwd(void);
int fgetfree(const char* path);
int getclustersize(unsigned char driveNumber, unsigned char partNumber);
int fputc(FILE handle, char c);
int fputs(FILE handle, const char* sz);
int fprintf(FILE handle, const char* sz, ...);
int fgets(FILE handle, unsigned int len);
int ftell(FILE handle);
int feof(FILE handle);
int fsize(FILE handle);

#endif /* FS_FF12B_SRC_FATFSACCESSOR_H_ */
