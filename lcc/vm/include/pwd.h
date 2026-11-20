/* pwd.h -- the user database. The VM has none: getpwuid finds no user (see vmlib.c). */

#ifndef VM_PWD_H
#define VM_PWD_H

#include <sys/types.h>

struct passwd
{
    char* pw_name;
    char* pw_dir;
};

struct passwd* getpwuid(int uid);

#endif
