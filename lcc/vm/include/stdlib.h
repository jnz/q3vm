/* stdlib.h -- allocation, conversion, sorting and process exit for the VM */

#ifndef VM_STDLIB_H
#define VM_STDLIB_H

#include "stddef.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 0x7fff

void exit(int status);
void abort(void);
void* malloc(size_t n);
void* calloc(size_t n, size_t size);
void* realloc(void* p, size_t n);
void free(void* p);

int atoi(const char* s);
long atol(const char* s);
double atof(const char* s);
long strtol(const char* s, char** end, int base);
#ifndef LCC_WIDE32
unsigned long long strtoull(const char* s, char** end, int base);
long long atoll(const char* s);
#endif
double strtod(const char* s, char** end);

typedef int cmp_t(const void*, const void*);
void qsort(void* a, size_t n, size_t es, cmp_t* cmp);

char* getenv(const char* name);
int abs(int n);
int rand(void);
void srand(unsigned seed);

#endif
