/* unistd.h -- the directory calls that the assembler's sources use, and the file
 * descriptor calls of the preprocessor (vmfd.c). The VM has no current directory
 * of its own: getcwd returns "/" and chdir fails. */

#ifndef VM_UNISTD_H
#define VM_UNISTD_H

char* getcwd(char* buf, int size);
int   chdir(const char* path);

int read(int fd, void* buf, int n);
int write(int fd, const void* buf, int n);
int close(int fd);
int dup2(int oldfd, int newfd);

#endif
