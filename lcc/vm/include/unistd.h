/* unistd.h -- the directory calls that the assembler's sources use, and the file
 * descriptor calls of the preprocessor (vmfd.c). The VM has no current directory
 * of its own: getcwd returns "/" and chdir fails. */

#ifndef VM_UNISTD_H
#define VM_UNISTD_H

#include <stddef.h>

char* getcwd(char* buf, int size);
int   chdir(const char* path);

/* access: mode is not checked, a file is accessible when it can be opened to read */
int access(const char* path, int mode);

int read(int fd, void* buf, int n);
int write(int fd, const void* buf, int n);
int close(int fd);
int dup2(int oldfd, int newfd);
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int isatty(int fd); /* always 0: the VM has no terminal */
int gethostname(char* name, size_t len); /* fails: the VM has no host name */
int getuid(void);                        /* always 0 */
int unlink(const char* path);            /* removes the file of the VM */
int symlink(const char* target, const char* path); /* fails: no links */
int readlink(const char* path, char* buf, size_t size); /* fails: no links */
int getpid(void);
int usleep(unsigned int usec);

#endif
