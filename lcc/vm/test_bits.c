/* test_bits.c -- static initialization of bitfields, compared with a plain expectation */
#include <stdio.h>

struct flags
{
    unsigned little:1;
    unsigned mul:1;
    unsigned callb:1;
    unsigned argb:1;
    unsigned ltr:1;
    unsigned dag:1;
    unsigned uc:1;
};

static struct flags onlyLtr = { 0, 0, 0, 0, 1, 0, 0 };
static struct flags onlyLittle = { 1, 0, 0, 0, 0, 0, 0 };

static struct flags runtime;

int main(int argc, char** argv)
{
    unsigned char* p = (unsigned char*)&onlyLtr;
    unsigned char* q = (unsigned char*)&onlyLittle;

    runtime.ltr = 1;
    fprintf(stdout, "ltr bytes %d, little bytes %d, runtime bytes %d\n", p[0], q[0],
            ((unsigned char*)&runtime)[0]);
    fprintf(stdout, "ltr field %d\n", onlyLtr.ltr);
    return 0;
}
