/* vmsys.h -- host services the VM image calls, see vm_syscalls.asm and vmrcc.c */

#ifndef VM_SYSCALLS_H
#define VM_SYSCALLS_H

int vm_open(const char* path, const char* mode);
int vm_close(int handle);
int vm_read(int handle, void* buf, int n);
int vm_write(int handle, const void* buf, int n);
int vm_seek(int handle, int off, int whence);
int vm_tell(int handle);
void vm_exit(int status);
int vm_getenv(const char* name, char* buf, int n);
int vm_time(void);
int vm_ctime(int t, char* buf, int n);
int vm_argc(void);
int vm_argv(int i, char* buf, int n);
int vm_remove(const char* path);
void vm_poison(void* p, int n);
int vm_strtod(const char* s, double* out);
int vm_run(const char* path, int argc, char** argv);
/* the extra heap of the run, from -heap: its first and end addresses, 0 when there is none */
int vm_heap_start(void);
int vm_heap_end(void);

#endif
