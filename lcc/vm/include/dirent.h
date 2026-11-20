/* dirent.h -- directories. The VM has none: opendir fails, so a directory is empty
 * (see vmlib.c). */

#ifndef VM_DIRENT_H
#define VM_DIRENT_H

typedef struct VM_DIR DIR;

struct dirent
{
    char d_name[256];
};

DIR*           opendir(const char* name);
struct dirent* readdir(DIR* dir);
int            closedir(DIR* dir);

#endif
