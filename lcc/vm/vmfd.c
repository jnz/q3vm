/* vmfd.c -- the file descriptor calls of the preprocessor (cpp), on the host's files.
 * A descriptor is a host file handle from vm_open. Descriptor 1 is standard output
 * unless dup2 has put a file in its place, which is how cpp writes its output file.
 */

#include <fcntl.h>
#include <unistd.h>
#include "vmsys.h"

static int out_fd = 1;

int open(const char* path, int flags)
{
    /* cpp only reads with open */
    return vm_open(path, "rb");
}

int creat(const char* path, int mode)
{
    return vm_open(path, "wb");
}

int read(int fd, void* buf, int n)
{
    return vm_read(fd, buf, n);
}

int write(int fd, const void* buf, int n)
{
    if (fd == 1)
    {
        fd = out_fd;
    }
    return vm_write(fd, buf, n);
}

int close(int fd)
{
    return vm_close(fd);
}

int dup2(int oldfd, int newfd)
{
    if (newfd == 1)
    {
        out_fd = oldfd;
    }
    return newfd;
}
