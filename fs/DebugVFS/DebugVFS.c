/*
 * DebugVFS.c
 *
 *  Created on: May 14, 2018
 *      Author: cromwelldev
 */

#include "DebugVFS.h"


int getEntryName(unsigned char index, const char * *  out)
{

}

FILEX open(const char* path, FileOpenMode mode)
{

}

int read(FILEX handle, unsigned char* out, unsigned int size)
{

}

int write(FILEX handle, const unsigned char* in, unsigned int size)
{

}


int eof(FILEX handle)
{

}

FileInfo stat(const char* path)
{

}

int rename(const char* path, const char* newName)
{

}

int mkdir(const char* path)
{

}

int remove(const char* path)
{

}

int chdir(const char* path)
{

}

DIREX opendir(const char* path)
{

}

FileInfo readdir(DIREX handle)
{

}

int closedir(DIREX handle)
{

}

