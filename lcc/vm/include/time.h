/* time.h -- wall clock time from the host */

#ifndef VM_TIME_H
#define VM_TIME_H

typedef int time_t;

time_t time(time_t* t);
char* ctime(const time_t* t);

#endif
