#ifndef stdlib_h
#define stdlib_h

#include <stddef.h>
#include <limits.h>

/* haha, don't need ctype.c */
#define isdigit(c) ((c) - '0' + 0U <= 9U)
#define isalpha(c) (((c) | 32) - 'a' + 0U <= 'z' - 'a' + 0U)
#define is_digit isdigit
#define isxdigit(c)    (((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define islower(c)    ((c) >= 'a' && (c) <= 'z')
#define toupper(c) __toupper(c)
#define isupper(c) ((c - 'A') < 26u)
#define isspace(c) (c == ' ' || c == '\t' || c == '\r' || c == '\n')

int tolower(int ch);

long strtol(const char *nptr, char **ptr, int base);
unsigned long strtoul(const char *nptr, char **ptr, int base);

void free(void *ptr);
void * malloc(size_t size);
void *calloc (size_t __nmemb, size_t __size);

#define RAND_MAX INT_MAX
int rand(void);

#endif /* #ifndef stdlib.h */
