/* float.h -- floating point limits for the VM (IEEE single and double) */

#ifndef VM_FLOAT_H
#define VM_FLOAT_H

#define FLT_MAX 3.40282346638528859812e+38F
#define FLT_MIN 1.17549435082228750797e-38F
#define FLT_EPSILON 1.19209289550781250000e-07F
#define DBL_MAX 1.7976931348623157e+308
#define DBL_MIN 2.2250738585072014e-308
#define DBL_EPSILON 2.2204460492503131e-16
#define DBL_DIG 15
#define DBL_MANT_DIG 53

/* long double is the same 64-bit type as double on the VM */
#define LDBL_MAX DBL_MAX
#define LDBL_MIN DBL_MIN
#define LDBL_EPSILON DBL_EPSILON
#define LDBL_DIG DBL_DIG

#endif
