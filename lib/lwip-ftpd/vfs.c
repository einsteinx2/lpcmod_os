/* Copyright (c) 2013, Philipp Tölke
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vfs.h"
#include "stdlib.h"

/* dirent that will be given to callers;
 * note: both APIs assume that only one dirent ever exists
 */
vfs_dirent_t dir_ent;

static FILEX guard_for_the_whole_fs;

int vfs_read (void* buffer, int dummy, int len, vfs_file_t* file) {
    int result = vroot_read(*file, buffer, len);

    if(0 > result)
    {
        result = 0;
    }
	return result;
}

vfs_dirent_t* vfs_readdir(vfs_dir_t* dir) {
	FileInfo fi;
#if _USE_LFN
	fi.lfname = NULL;
#endif
	fi = vroot_readdir(*dir);
	if ('\0' == fi.name[0] || 0 == fi.nameLength || FATX_FILENAME_MAX < fi.nameLength) return NULL;
	memcpy(dir_ent.name, fi.name, fi.nameLength);
	dir_ent.name[fi.nameLength] = '\0';
	return &dir_ent;
}

int vfs_stat(vfs_t* vfs, const char* filename, vfs_stat_t* st) {
#if _USE_LFN
	f.lfname = NULL;
#endif
	FileInfo f = vroot_stat(filename);
	if ('\0' == f.name[0] || 0 == f.nameLength || FATX_FILENAME_MAX < f.nameLength)
    {
		return 1;
	}
	st->st_size = f.size;
	st->st_mode = f.attributes;
	st->st_mtime.date = f.modDate;
	st->st_mtime.time = f.modTime;
	return 0;
}

void vfs_close(vfs_t* vfs) {
	if (vfs != &guard_for_the_whole_fs) {
		/* Close a file */
		vroot_close(*vfs);
		free(vfs);
	}
}

int vfs_write (void* buffer, int dummy, int len, vfs_file_t* file) {
	unsigned int byteswritten = vroot_write(*file, buffer, len);
	if (byteswritten != len) return 0;
	return byteswritten;
}

vfs_t* vfs_openfs(void) {
	return &guard_for_the_whole_fs;
}

vfs_file_t* vfs_open(vfs_t* vfs, const char* filename, const char* mode) {
    vfs_file_t *f = malloc(sizeof(vfs_file_t));
	BYTE flags = 0;
	while (*mode != '\0') {
		if (*mode == 'r') flags |= FileOpenMode_Read;
		if (*mode == 'w') flags |= FileOpenMode_Write | FileOpenMode_CreateAlways;
		mode++;
	}
	*f = vroot_open(filename, flags);
	if (0 == *f)
	{
		free(f);
		return NULL;
	}
	return f;
}

char* vfs_getcwd(vfs_t* vfs, void* dummy1, int dummy2) {
	char* cwd = malloc(255);
	const char* out = vroot_getcwd();
	strncpy(cwd, out, 255);
	if (0 == strlen(cwd)) {
		free(cwd);
		return NULL;
	}
	return cwd;
}

vfs_dir_t* vfs_opendir(vfs_t* vfs, const char* path) {
	vfs_dir_t* dir = malloc(sizeof(vfs_dir_t));
	*dir = vroot_opendir(path);
	if(0 == *dir) {
		free(dir);
		return NULL;
	}
	return dir;
}

void vfs_closedir(vfs_dir_t* dir) {
    vroot_closedir(*dir);
	free(dir);
}

struct tm dummy = {
	.tm_year = 70,
	.tm_mon  = 0,
	.tm_mday = 1,
	.tm_hour = 0,
	.tm_min  = 0
};
struct tm* gmtime(time_t* c_t) {
	return &dummy;
}
