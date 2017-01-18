/*
 * string.h
 *
 *  Created on: Aug 8, 2016
 *      Author: bennyboy
 */


#ifdef __PC_SIMULATOR__
#include </usr/include/string.h>
#else
#ifndef INCLUDE_STRING_H_
#define INCLUDE_STRING_H_

#include <stdarg.h>
#include <stddef.h>

void *memset (void *__s, int __c, size_t __n);

int memcmp(const void * cs,const void * ct,size_t count);

char * strcpy(char * dest,const char *src);

char * strncpy(char * dest,const char *src,int count);

char * strstr(const char * s1,const char * s2);

char * strpbrk(const char * cs,const char * ct);

char * strsep(char **s, const char *ct);

int strncmp(const char * cs,const char * ct,size_t count);

int atoi(const char *str);

int strcmp(const char* s1,const char* s2);

char *strrchr0(char *string, char ch);

void chrreplace(char *string, char search, char ch);

void *memmove(void *dest, const void *src, size_t count);

void *memcpy (void *__restrict __dest, const void *__restrict __src, size_t __n);

size_t strlen (const char *__s);

int sprintf(char * buf, const char *fmt, ...);

char *strchr(const char *s, int c);

#endif /* INCLUDE_STRING_H_ */
#endif
