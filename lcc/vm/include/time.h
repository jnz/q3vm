/* time.h -- wall clock time from the host */

#ifndef VM_TIME_H
#define VM_TIME_H

#include <stddef.h>

typedef int time_t;

time_t time(time_t* t);
char* ctime(const time_t* t);

/* the calendar time: the VM has no local time zone, localtime is not available (NULL) */
struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

struct tm* localtime(const time_t* t);
size_t     strftime(char* buf, size_t max, const char* fmt, const struct tm* tm);

#endif
