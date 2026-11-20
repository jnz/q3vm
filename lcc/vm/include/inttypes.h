/* inttypes.h -- printf formats for the integer types of stdint.h, in the VM's printf
 * (which takes the 64-bit formats as %llu and %llx) */

#ifndef VM_INTTYPES_H
#define VM_INTTYPES_H

#include "stdint.h"

#define PRId32 "d"
#define PRIu32 "u"
#define PRIx32 "x"
#define PRIX32 "X"
#define PRId64 "lld"
#define PRIu64 "llu"
#define PRIx64 "llx"
#define PRIX64 "llX"
#define PRIdPTR "d"
#define PRIuPTR "u"
#define PRIxPTR "x"
#define PRIXPTR "X"

#endif
