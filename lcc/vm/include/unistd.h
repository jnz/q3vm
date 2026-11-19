/* unistd.h -- the directory calls that the assembler's sources use. The VM has no
 * current directory of its own: getcwd returns "/" and chdir fails. */

#ifndef VM_UNISTD_H
#define VM_UNISTD_H

char* getcwd(char* buf, int size);
int   chdir(const char* path);

#endif
