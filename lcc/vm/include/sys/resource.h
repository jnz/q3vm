/* sys/resource.h -- the resource usage of the process. The VM does not count it:
 * getrusage fails (see vmlib.c). */

#ifndef VM_SYS_RESOURCE_H
#define VM_SYS_RESOURCE_H

#include <sys/time.h>

#define RUSAGE_SELF 0

struct rusage
{
    struct timeval ru_utime;
    struct timeval ru_stime;
    long           ru_maxrss;
};

int getrusage(int who, struct rusage* usage);

#endif
