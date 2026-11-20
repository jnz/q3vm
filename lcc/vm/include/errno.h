/* errno.h -- error number for the VM; the host does not set it */

#ifndef VM_ERRNO_H
#define VM_ERRNO_H

extern int errno;

#define ENOENT 2
#define EACCES 13
#define EINVAL 22
#define EEXIST 17
#define ERANGE 34

#endif
