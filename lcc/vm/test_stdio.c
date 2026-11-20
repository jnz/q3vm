/* test_stdio.c -- checks the VM stdio with the same entry point as rcc: run with vmrcc */
#include <stdio.h>

int main(int argc, char** argv)
{
    FILE* f;

    fputc('a', stdout);
    f = freopen("test_stdio.out", "w", stdout);
    fputc('b', stdout);
    fputs("hello\n", stdout);
    fprintf(stdout, "%d %s\n", argc, argv[0]);
    return f == NULL;
}
