/* stdint.h -- integer types for the VM. The 64-bit types need the bytecode64 target. */

#ifndef VM_STDINT_H
#define VM_STDINT_H

typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned int       uint32_t;
#ifndef LCC_WIDE32
typedef long long          int64_t;
typedef unsigned long long uint64_t;
#define INT64_MAX 0x7fffffffffffffffLL
#define INT64_MIN (-INT64_MAX - 1)
#endif
typedef int                intptr_t;
typedef unsigned int       uintptr_t;
#ifndef LCC_WIDE32
typedef long long          intmax_t;
typedef unsigned long long uintmax_t;
#endif

#endif
