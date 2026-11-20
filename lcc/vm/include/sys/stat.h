/* sys/stat.h -- file status. The VM has no file system calls for these, so stat
 * fails and mkdir does nothing (see vmlib.c) */

#ifndef VM_SYS_STAT_H
#define VM_SYS_STAT_H

#include <sys/types.h>

struct stat
{
    int st_mode;
    int st_size;
    int st_mtime;
};

#define S_IFMT  0170000
#define S_IFDIR 0040000
#define S_IFREG 0100000
#define S_IFLNK  0120000
#define S_IFIFO  0010000
#define S_IFCHR  0020000
#define S_IFBLK  0060000
#define S_IFSOCK 0140000
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

int stat(const char* path, struct stat* buf);
int lstat(const char* path, struct stat* buf);
int fstat(int fd, struct stat* buf);
int mkdir(const char* path, mode_t mode);
int chmod(const char* path, mode_t mode); /* always succeeds: the VM has no modes */

#endif
