/*
 * DebugVFS.h
 *
 *  Created on: May 14, 2018
 *      Author: cromwelldev
 */

#ifndef FS_DEBUGVFS_DEBUGVFS_H_
#define FS_DEBUGVFS_DEBUGVFS_H_

#include "FatFSAccessor.h"

void debugvfs_init(void);

int debugvfsgetEntryName(unsigned char index, const char * *const  out);
FILEX debugvfsopen(const char* path, FileOpenMode mode);
int debugvfsread(FILEX handle, unsigned char* out, unsigned int size);
int debugvfswrite(FILEX handle, const unsigned char* in, unsigned int size);
int debugvfsclose(FILEX handle);

int debugvfseof(FILEX handle);
FileInfo debugvfsstat(const char* path);

int debugvfsrename(const char* path, const char* newName);
int debugvfsmkdir(const char* path);
int debugvfsremove(const char* path);
int debugvfschdir(const char* path);

DIREX debugvfsopendir(const char* path);
FileInfo debugvfsreaddir(DIREX handle);
int debugvfsclosedir(DIREX handle);

#endif /* FS_DEBUGVFS_DEBUGVFS_H_ */
