/* fcntl.h -- opening files. The VM has only the host's files, see vmfd.c */

#ifndef VM_FCNTL_H
#define VM_FCNTL_H

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2

int open(const char* path, int flags);
int creat(const char* path, int mode);

#endif
