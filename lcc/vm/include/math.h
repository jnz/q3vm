/* math.h -- the common functions of the C math library, in the runtime library of the VM
 * (vmlib.c). They are approximations: about 1e-12 relative on a 64-bit double, and
 * the precision of a float on the 32-bit target. floor, ceil and fmod are exact for
 * |x| < 2^30. sqrt of a negative number, log of a number that is not positive and
 * exp of a large number are not the IEEE results (there is no NaN or infinity). */

#ifndef VM_MATH_H
#define VM_MATH_H

#define M_PI 3.14159265358979323846

/* INFINITY and HUGE_VAL are the largest double, the VM has no infinity: a value
 * that overflows is the largest one, and it is not an IEEE infinity */
#define HUGE_VAL 1.7976931348623157e308
#define INFINITY HUGE_VAL

double fabs(double x);
double floor(double x);
double ceil(double x);
double fmod(double x, double y);
double sqrt(double x);
double sin(double x);
double cos(double x);
double tan(double x);
double atan(double x);
double atan2(double y, double x);
double exp(double x);
double log(double x);
double pow(double x, double y);
int    signbit(double x);

#endif
