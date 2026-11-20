/* stdarg.h -- variable arguments for the VM. The caller puts every argument in
 * a 4 byte slot on the VM stack, and 8 byte arguments (long long, double) in
 * an 8 byte slot aligned to 8. vm_va_next in vmlib.c does the alignment. */

#ifndef VM_STDARG_H
#define VM_STDARG_H

typedef char* va_list;

void* vm_va_next(va_list* ap, int size);

#define va_start(ap, v) (ap = (va_list)&v + ((sizeof(v) + 3) & ~3))
#define va_arg(ap, t) (*(t*)vm_va_next(&(ap), sizeof(t)))
#define va_end(ap) (ap = (va_list)0)

#endif
