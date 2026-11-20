/* stdint.h -- integer types for the VM. The 64-bit types need the bytecode64 target. */

#ifndef VM_STDINT_H
#define VM_STDINT_H

typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned int       uint32_t;
#define INT8_MAX  127
#define INT8_MIN  (-127 - 1)
#define UINT8_MAX 255
#define INT16_MAX 32767
#define INT16_MIN (-32767 - 1)
#define UINT16_MAX 65535
#define INT32_MAX 2147483647
#define INT32_MIN (-2147483647 - 1)
#define UINT32_MAX 4294967295U
#define INT8_C(x)   x
#define UINT8_C(x)  x##U
#define INT16_C(x)  x
#define UINT16_C(x) x##U
#define INT32_C(x)  x
#define UINT32_C(x) x##U

#ifndef LCC_WIDE32
typedef long long          int64_t;
typedef unsigned long long uint64_t;
#define INT64_MAX 0x7fffffffffffffffLL
#define INT64_MIN (-INT64_MAX - 1)
#define UINT64_MAX 0xffffffffffffffffULL
#define INT64_C(x)  x##LL
#define UINT64_C(x) x##ULL
#endif
typedef int                intptr_t;
typedef unsigned int       uintptr_t;
#ifndef LCC_WIDE32
typedef long long          intmax_t;
typedef unsigned long long uintmax_t;
#endif

#endif
