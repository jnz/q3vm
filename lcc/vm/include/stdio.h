/* stdio.h -- stdio for the VM, unbuffered: every call goes to a host syscall */

#ifndef VM_STDIO_H
#define VM_STDIO_H

#include "stddef.h"
#include "stdarg.h"

typedef struct vm_file_s {
    int handle; /* index in the host file table, -1 when closed */
    int eof;
    int error;
} FILE;

extern FILE vm_files[3];
#define stdin (&vm_files[0])
#define stdout (&vm_files[1])
#define stderr (&vm_files[2])

#define EOF (-1)
#define BUFSIZ 512
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int printf(const char* fmt, ...);
int fprintf(FILE* f, const char* fmt, ...);
int sprintf(char* buf, const char* fmt, ...);
int sscanf(const char* buf, const char* fmt, ...);

FILE* fopen(const char* name, const char* mode);
FILE* freopen(const char* name, const char* mode, FILE* f);
int fclose(FILE* f);
size_t fread(void* ptr, size_t size, size_t n, FILE* f);
size_t fwrite(const void* ptr, size_t size, size_t n, FILE* f);
char* fgets(char* s, int n, FILE* f);
int fgetc(FILE* f);
int getc(FILE* f);
int getchar(void);
int fputc(int c, FILE* f);
int putc(int c, FILE* f);
int putchar(int c);
int fputs(const char* s, FILE* f);
int puts(const char* s);
int fflush(FILE* f);
int fseek(FILE* f, long off, int whence);
long ftell(FILE* f);
void rewind(FILE* f);
int feof(FILE* f);
int ferror(FILE* f);
void setbuf(FILE* f, char* buf);
void perror(const char* s);
int remove(const char* name);

#endif
