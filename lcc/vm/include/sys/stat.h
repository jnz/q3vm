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

int stat(const char* path, struct stat* buf);
int mkdir(const char* path, mode_t mode);

#endif
