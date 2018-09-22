#ifdef Q3_VM
#include "bg_lib.h"
void printf(const char* fmt, ...);
#else
#include <stdio.h>
#include <string.h>
#define trap_Error(x) printf("%s\n", x)
#endif

/** @brief Simple function to sum up character values.
  This is used for a test in the makefile.
  @param[in] f pointer to string.
  @return test number. */
int testCase(const char* f);

/* this call is supposed to fail */
int badcall(int i);

/* test float system calls */
float floatff(float f);

/* test recursive calls */
int recursive(int i);

volatile int        bssTest;         /* don't initialize, should be zero */
volatile static int dataTest = -999; /* don't change, should be 999 */

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .qvm file
================
*/
int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4,
           int arg5, int arg6, int arg7, int arg8, int arg9, int arg10,
           int arg11)
{
    volatile int            i;
    volatile int            iloop = 20000000;
    char                    str[] = "Hello %s\n";
    volatile float          f     = 0.0f;
    volatile float          df    = 0.0001f;
    volatile int            xi    = 128;
    volatile unsigned       xu    = 128;
    volatile unsigned       xu2   = 3;
    volatile unsigned short us    = 1;
    volatile short          si    = 1;
    volatile int            j;
    static unsigned char    mem1[8];
    static unsigned char    mem2[8] = "Hello"; /* don't change this string */
    int                     doStupidStuff = 0; /* misbehave and see if the interpreter deals correctly with that */

    void (*fun_ptr)(int) = (void*)0xffffff;

    /*
    printf("cmd:   %i\n", command);
    printf("arg0:  %i\n", arg0);
    printf("arg1:  %i\n", arg1);
    printf("arg2:  %i\n", arg2);
    printf("arg3:  %i\n", arg3);
    printf("arg4:  %i\n", arg4);
    printf("arg5:  %i\n", arg5);
    printf("arg6:  %i\n", arg6);
    printf("arg7:  %i\n", arg7);
    printf("arg8:  %i\n", arg8);
    printf("arg9:  %i\n", arg9);
    printf("arg10: %i\n", arg10);
    printf("arg11: %i\n", arg11);
    */

    if (command == 1)
    {
        return arg0; /* just return arg0, used for "recursive()" test */
    }
    if (command == 2)
    {
        printf("Invalid function pointer call...\n");
        (*fun_ptr)(10);
        printf("Invalid function pointer accepted.\n");
        return 0;
    }

    printf(str, "World");
    trap_Error("Testing Error Callback\n");

    doStupidStuff = (arg1 == 1);
    if (doStupidStuff)
    {
        printf("Misbehave mode is enabled\n");
        badcall(9001);
    }

    if (doStupidStuff)
    {
        /* call a native function that will call us back here with command == 1 */
        printf("Test recursive VM call... ");
        if (recursive(666) != 666) /* we expect our input back */
        {
            printf("failed\n");
            return -1;
        }
        printf("passed\n");
    }

    /* float */
    for (i = 0; i < iloop; i++)
    {
        f += df;
        f = -f;
        f = -f;
        f /= 2.0f;
        f *= 2.0f;
    }
    printf("Result (should be 2048.000000): %f\n", f);

    /* memcpy/memset */
    if (doStupidStuff)
    {
        memset(mem1, 0, sizeof(mem1));
        memcpy(mem1, mem2, sizeof(mem2));
        if (mem1[0] != 'H' || mem1[1] != 'e' || mem1[2] != 'l' || mem1[3] != 'l' ||
            mem1[4] != 'o' || mem1[5] != '\0' || mem1[6] != 0)
        {
            printf("memcpy / memset error\n");
            return -1;
        }
    }

    /* integer stuff */
    for (j = 0; j < 32; j++)
    {
        xi = us;
        xi = si;
        xi = j;
        xi = xi << 3;
        xi = xi >> 2;
        xi = xi * 3;
        xi = xi / 3;
        xi = xi % 2;
        xi = xi & 3;
        xi = xi | 4;
        xi = xi ^ 4;
        xu = xu * 3;
        xu = xu / 3;
        xu = xu + 2;
        xu = xu - 2;
        xu = xu % 2;
        xu = xu << xu2;
        xu = xu >> xu2;
        xu = xu / xu2;
        xu = xu % xu2;
        xu = xu * xu2;
        us += 2;
        us = us << 3;
        xi = us + 3;

        xu = ~xu;
        xi = (short)xi;
        f  = j * j;

        if (j > 0)
        {
            xi = -xi;
        }
        else
        {
            xi = 2 * xi;
        }
        if (j >= 0)
        {
            xi = -xi;
        }
        else
        {
            xi = 2 * xi;
        }
        if (j < 5)
        {
            xi = -xi;
        }
        else
        {
            xi = 2 * xi;
        }
        if (j <= 5)
        {
            xi = -xi;
        }
        else
        {
            xi = 2 * xi;
        }
        xu = j;
        if (xu > 5U)
        {
            xi = -xi;
        }
        else
        {
            xi = 2 * xi;
        }
        if (xu >= 5U)
        {
            xi = -xi;
        }
        else
        {
            xi = 2 * xi;
        }
        if (xu < 5U)
        {
            xi = -xi;
        }
        else
        {
            xi = 2 * xi;
        }
        if (xu <= 5U)
        {
            xi = -xi;
        }
        else
        {
            xi = 2 * xi;
        }
        if (f > 0)
        {
            f = -f;
        }
        else
        {
            f = 2 * f;
        }
        if (f >= 0)
        {
            f = -f;
        }
        else
        {
            f = 2 * f;
        }
        if (f < 1)
        {
            f = -f;
        }
        else
        {
            f = 2 * f;
        }
        if (f <= 1)
        {
            f = -f;
        }
        else
        {
            f = 2 * f;
        }
        f = 0.1f;
        if (f == 0.0f)
        {
            f = -f;
        }
        else
        {
            f = 0.0f;
        }
        if (f == df)
        {
            f = -f;
        }
        else
        {
            f = 2 * f;
        }
        if (f != 0.0f)
        {
            f = 128 * f;
        }
    }

    /* test bss section: should be zero */
    printf("bssTest: %i\n", bssTest);
    if (bssTest != 0)
    {
        return -1;
    }

    /* test data section: should be -999 */
    printf("dataTest: %i\n", dataTest);
    if (dataTest != -999)
    {
        return -1;
    }

    if (doStupidStuff)
    {
        printf("Float system call test: ");
        if (floatff(3.33f) != 6.66f)
        {
            printf("failed\n");
            return -1;
        }
        printf("passed\n");
    }

#ifdef Q3_VM
    if (doStupidStuff)
    {
        printf(str, "Trying to copy outside of vm sandbox:\n");
        memcpy(mem1, mem2, 1000000); /* try to escape the sandbox */
    }
#endif
    if (arg0 != 0 || arg1 != 1 || arg2 != 2 || arg3 != 3 || arg4 != 4 ||
        arg5 != 5 || arg6 != 6 || arg7 != 7 || arg8 != 8 || arg9 != 9 ||
        arg10 != 10 || arg11 != 11)
    {
        return 0;
    }
    else
    {
        return 333; /* test case expects 333 in this case */
    }
}

#ifndef Q3_VM
int main(int argc, char** argv)
{
    return vmMain(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int badcall(int i)
{
    return 0;
}

float floatff(float f)
{
    return 2.0f * f;
}

int recursive(int i)
{
    return vmMain(1, i, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
#else
void printf(const char* fmt, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, fmt);
    vsprintf(text, fmt, argptr);
    va_end(argptr);

    trap_Printf(text);
}
#endif
