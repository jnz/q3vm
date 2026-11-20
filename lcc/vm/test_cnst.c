/* test_cnst.c -- the shape of cnsttree: a variadic function that switches on the type
 * and reads a long long, unsigned long long, double or pointer argument */
#include <stdio.h>
#include <stdarg.h>

typedef struct type_s { int op; int size; } *Type;
typedef struct tree_s { long long i; unsigned long long u; double d; void* p; } *Tree;

enum { INT = 1, UNSIGNED = 2, FLOAT = 3, POINTER = 4 };

static struct tree_s pool;
static struct type_s intT = { INT, 4 }, uT = { UNSIGNED, 4 }, fT = { FLOAT, 8 }, pT = { POINTER, 4 };

static Tree mk(Type ty, ...)
{
    va_list ap;
    Tree p = &pool;

    va_start(ap, ty);
    switch (ty->op)
    {
    case INT:
        p->i = va_arg(ap, long long);
        break;
    case UNSIGNED:
        p->u = va_arg(ap, unsigned long long);
        break;
    case FLOAT:
        p->d = va_arg(ap, double);
        break;
    case POINTER:
        p->p = va_arg(ap, void*);
        break;
    default:
        break;
    }
    va_end(ap);
    return p;
}

int main(int argc, char** argv)
{
    long long n = 4;
    unsigned long long un = 9;
    Tree t;

    t = mk(&intT, n);
    fprintf(stdout, "int %d\n", (int)t->i);
    t = mk(&intT, (long long)7);
    fprintf(stdout, "int %d\n", (int)t->i);
    t = mk(&uT, un);
    fprintf(stdout, "uns %d\n", (int)t->u);
    t = mk(&fT, 2.5);
    fprintf(stdout, "flt %d\n", (int)(t->d * 10.0));
    t = mk(&pT, (void*)0);
    fprintf(stdout, "ptr %d\n", t->p == 0);
    return 0;
}
