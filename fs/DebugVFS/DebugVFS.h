/*
 * DebugVFS.h
 *
 *  Created on: May 14, 2018
 *      Author: cromwelldev
 */

#ifndef FS_DEBUGVFS_DEBUGVFS_H_
#define FS_DEBUGVFS_DEBUGVFS_H_

#include "FatFSAccessor.h"

void DebugVFS_init(void);

int getEntryName(unsigned char index, const char * *const  out);
FILEX open(const char* path, FileOpenMode mode);
int read(FILEX handle, unsigned char* out, unsigned int size);
int write(FILEX handle, const unsigned char* in, unsigned int size);
int close(FILEX handle);

int eof(FILEX handle);
FileInfo stat(const char* path);

int rename(const char* path, const char* newName);
int mkdir(const char* path);
int remove(const char* path);
int chdir(const char* path);

DIREX opendir(const char* path);
FileInfo readdir(DIREX handle);
int closedir(DIREX handle);

#endif /* FS_DEBUGVFS_DEBUGVFS_H_ */
