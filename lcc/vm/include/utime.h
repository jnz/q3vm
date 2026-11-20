/* utime.h -- file times. The VM has none: utime fails (see vmlib.c). */

#ifndef VM_UTIME_H
#define VM_UTIME_H

#include <time.h>

struct utimbuf
{
    time_t actime;
    time_t modtime;
};

int utime(const char* path, const struct utimbuf* times);

#endif
