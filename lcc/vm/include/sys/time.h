/* sys/time.h -- the time of day. The VM has a clock in seconds (time), so gettimeofday
 * gives whole seconds and microseconds of 0 (see vmlib.c). */

#ifndef VM_SYS_TIME_H
#define VM_SYS_TIME_H

#include <sys/types.h>

struct timeval
{
    int tv_sec;
    int tv_usec;
};

int gettimeofday(struct timeval* tv, void* tz);
int utimes(const char* path, const struct timeval times[2]); /* fails: no file times */

#endif
