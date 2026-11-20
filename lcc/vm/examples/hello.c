#include <stdio.h>
#include <stdlib.h>

int square(int x)
{
    return x * x;
}

int main(int argc, char** argv)
{
    int* a = malloc(4 * sizeof(int));
    int  i;

    for (i = 0; i < 4; i++)
        a[i] = square(i + 1);
    printf("argc %d first %s\n", argc, argv[1]);
    printf("sum %d\n", a[0] + a[1] + a[2] + a[3]);
    return 0;
}
