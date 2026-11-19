/* vfs.h -- the files of the VM: an in-memory file system, with the host's files
 * behind it as an optional backup (see vfs.c) */

#ifndef VFS_H
#define VFS_H

#include <stddef.h>

typedef struct vfs_file vfs_file_t;

/* cwd is the absolute directory that relative names start from. With os_backup,
 * a file missing from memory is read from the host, and a file the VM writes is
 * saved to the host when it is closed. */
void         vfs_init(const char* cwd, int os_backup);
vfs_file_t*  vfs_open(const char* name, const char* mode);
size_t       vfs_read(vfs_file_t* f, void* buf, size_t n);
size_t       vfs_write(vfs_file_t* f, const void* buf, size_t n);
int          vfs_seek(vfs_file_t* f, long off, int whence);
long         vfs_tell(vfs_file_t* f);
int          vfs_close(vfs_file_t* f);
int          vfs_remove(const char* name);
/* the host side of the file system: put a file in memory, look at its size and copy it */
void         vfs_put(const char* name, const void* data, size_t n);
long         vfs_size(const char* name);
long         vfs_get(const char* name, void* buf, size_t n);
/* saves the changed files to the host, when there is a backup */
void         vfs_flush(void);

#endif
