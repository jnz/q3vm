#include "bg_lib.h"

void printf(const char* fmt, ...);
static void qr_decomposition(float* A, int m, int n, float* Q, float* R);
static float Q_rsqrt(float number);

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
    int i, j;
    int m = 3, n = 3;
    float A[] = {
        1, 2, 3,
        4, 5, 6,
        7, 8, 9
    };

    float Q[9] = {0};
    float R[9] = {0};

    qr_decomposition(A, m, n, Q, R);

    printf("Matrix Q:\n");
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%.3f ", Q[i * n + j]);
        }
        printf("\n");
    }

    printf("Matrix R:\n");
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%.3f ", R[i * n + j]);
        }
        printf("\n");
    }

    return 0;
}

static float Q_rsqrt(float number)
{
    int i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(int*)&y;                  // evil floating point bit level hacking
    i = 0x5f3759df - (i >> 1);           // what the f***?
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y)); // 1st iteration

    return y;
}

static void qr_decomposition(float* A, int m, int n, float* Q, float* R)
{
    int i, j, k;
    float norm;
    float dot_product;

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            Q[j * n + i] = A[j * n + i];
        }
        for (k = 0; k < i; k++) {
            dot_product = 0;
            for (j = 0; j < m; j++) {
                dot_product += A[j * n + i] * Q[j * n + k];
            }
            R[k * n + i] = dot_product;
            for (j = 0; j < m; j++) {
                Q[j * n + i] -= R[k * n + i] * Q[j * n + k];
            }
        }
        norm = 0;
        for (j = 0; j < m; j++) {
            norm += Q[j * n + i] * Q[j * n + i];
        }
        norm = 1.0f/Q_rsqrt(norm);
        R[i * n + i] = norm;
        for (j = 0; j < m; j++) {
            Q[j * n + i] /= norm;
        }
    }
}

void printf(const char* fmt, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, fmt);
    vsprintf(text, fmt, argptr);
    va_end(argptr);

    trap_Printf(text);
}

