/*
 * 64-bit integer and double test cases.
 *
 * This file is compiled twice: to bytecode with LCC (wide_test.qvm) and
 * natively with GCC (wide_host.c includes it). Every case returns a checksum
 * of its results, so both builds must agree exactly.
 *
 * vmMain must be the first function in the file (the VM starts there).
 */

typedef long long i64;
typedef unsigned long long u64;

static u64 test_int(void);
static u64 test_conv(void);
static u64 test_double(void);
static u64 test_call(void);
static u64 test_memory(void);
static u64 test_loop(void);
static u64 test_syscall(void);

/* native functions of the host, see wide_syscalls.asm for the ids */
i64    wide_add(i64 a, i64 b);
double wide_scale(i64 a, double d);

i64 vmMain(int command)
{
    switch (command)
    {
    case 0:
        return (i64)test_int();
    case 1:
        return (i64)test_conv();
    case 2:
        return (i64)test_double();
    case 3:
        return (i64)test_call();
    case 4:
        return (i64)test_memory();
    case 5:
        return (i64)test_loop();
    case 6:
        return (i64)test_syscall();
    default:
        return -1;
    }
}

static u64 mix(u64 acc, u64 v)
{
    return (acc ^ v) * 1099511628211ULL + 7;
}

static u64 dbits(double d)
{
    union {
        double d;
        u64    u;
    } x;
    x.d = d;
    return x.u;
}

/* integers: arithmetic, bitwise, shifts and comparisons on pairs of values */
static u64 test_int(void)
{
    static const i64 vals[8] = {
        0LL,
        1LL,
        -1LL,
        1234567890123LL,
        -9876543210987LL,
        9223372036854775807LL,
        -9223372036854775807LL - 1,
        0x7fffffffLL,
    };
    u64 acc = 0;
    int i, j;

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            i64 a = vals[i];
            i64 b = vals[j];
            int s = (int)(b & 63);

            /* signed overflow is undefined in C: use unsigned arithmetic */
            acc = mix(acc, (u64)a + (u64)b);
            acc = mix(acc, (u64)a - (u64)b);
            acc = mix(acc, (u64)a * (u64)b);
            acc = mix(acc, (u64)(a & b));
            acc = mix(acc, (u64)(a | b));
            acc = mix(acc, (u64)(a ^ b));
            acc = mix(acc, (u64)(~a));
            acc = mix(acc, (u64)0 - (u64)a);
            acc = mix(acc, (u64)a << s);
            acc = mix(acc, (u64)(a >> s));
            acc = mix(acc, (u64)((u64)a >> s));
            acc = mix(acc, (u64)(a < b));
            acc = mix(acc, (u64)(a <= b));
            acc = mix(acc, (u64)(a > b));
            acc = mix(acc, (u64)(a >= b));
            acc = mix(acc, (u64)(a == b));
            acc = mix(acc, (u64)(a != b));
            acc = mix(acc, (u64)((u64)a < (u64)b));
            acc = mix(acc, (u64)((u64)a <= (u64)b));
            acc = mix(acc, (u64)((u64)a > (u64)b));
            acc = mix(acc, (u64)((u64)a >= (u64)b));
            if (b != 0 && b != -1)
            {
                acc = mix(acc, (u64)(a / b));
                acc = mix(acc, (u64)(a % b));
                acc = mix(acc, (u64)((u64)a / (u64)b));
                acc = mix(acc, (u64)((u64)a % (u64)b));
            }
            if (a < b)
            {
                acc = mix(acc, 1);
            }
            else
            {
                acc = mix(acc, 2);
            }
        }
    }
    return acc;
}

/* conversions between 64-bit integers, 32-bit, 16-bit, 8-bit and doubles */
static u64 test_conv(void)
{
    int            i = -5;
    unsigned       u = 4000000000u;
    short          s = -3;
    char           c = -7;
    unsigned char  uc = 250;
    i64            big = 0x123456789ABCDEF0LL;
    u64            acc = 0;

    acc = mix(acc, (u64)(i64)i);
    acc = mix(acc, (u64)(i64)u);
    acc = mix(acc, (u64)(i64)s);
    acc = mix(acc, (u64)(i64)c);
    acc = mix(acc, (u64)(i64)uc);
    acc = mix(acc, (u64)(u64)u);
    acc = mix(acc, (u64)(int)big);
    acc = mix(acc, (u64)(unsigned)big);
    acc = mix(acc, (u64)(short)big);
    acc = mix(acc, (u64)(char)big);
    acc = mix(acc, (u64)(unsigned char)big);
    acc = mix(acc, (u64)(i64)(int)big);
    acc = mix(acc, (u64)(i64)(unsigned)big);
    acc = mix(acc, (u64)(i64)(-1));
    acc = mix(acc, (u64)(i64)(i * 3));
    acc = mix(acc, (u64)(i64)(char)(i * 40));

    acc = mix(acc, dbits((double)i));
    acc = mix(acc, dbits((double)big / 4096.0));
    acc = mix(acc, dbits((double)(i64)-3));
    acc = mix(acc, dbits((double)(float)1.5));
    acc = mix(acc, dbits((double)(float)big));
    acc = mix(acc, (u64)(i64)(-3.9));
    acc = mix(acc, (u64)(i64)(3.9));
    acc = mix(acc, (u64)(i64)(1e18));
    acc = mix(acc, (u64)(i64)(-2.5e18));
    acc = mix(acc, (u64)(int)(2.75));
    acc = mix(acc, (u64)(int)(-2.75));
    acc = mix(acc, (u64)(float)big == (u64)big);
    return acc;
}

/* doubles: arithmetic, comparisons and float conversions */
static u64 test_double(void)
{
    static const double vals[6] = { 0.0, 1.0, -2.5, 3.141592653589793, 1e-300,
                                    -7.25e7 };
    u64    acc = 0;
    int    i, j;

    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 6; j++)
        {
            double a = vals[i];
            double b = vals[j];

            acc = mix(acc, dbits(a + b));
            acc = mix(acc, dbits(a - b));
            acc = mix(acc, dbits(a * b));
            acc = mix(acc, dbits(-a));
            acc = mix(acc, (u64)(a == b));
            acc = mix(acc, (u64)(a != b));
            acc = mix(acc, (u64)(a < b));
            acc = mix(acc, (u64)(a <= b));
            acc = mix(acc, (u64)(a > b));
            acc = mix(acc, (u64)(a >= b));
            if (b != 0.0)
            {
                acc = mix(acc, dbits(a / b));
            }
            if (a < b)
            {
                acc = mix(acc, 11);
            }
            else
            {
                acc = mix(acc, 12);
            }
        }
    }
    acc = mix(acc, dbits((float)3.25));
    acc = mix(acc, dbits((double)(float)0.1));
    return acc;
}

/* function calls with mixed 32-bit and 64-bit arguments and results */
static i64 mixargs(int a, i64 b, int c, double d, unsigned u, i64 e)
{
    return a + b * c + (i64)d + (i64)u + e;
}

static double dmix(i64 a, double b, float f, i64 c)
{
    return (double)a + b + f + (double)c;
}

static i64 fact(i64 n)
{
    if (n <= 1)
    {
        return 1;
    }
    return n * fact(n - 1);
}

static u64 test_call(void)
{
    u64 acc = 0;
    int k;

    /* results that are discarded still take two stack cells */
    fact(5);
    dmix(1LL, 2.0, 3.0f, 4LL);
    acc = mix(acc, (u64)mixargs(1, 2LL, 3, 4.5, 5u, 6000000000LL));
    acc = mix(acc, (u64)mixargs(-1, -9876543210LL, -2, -7.75, 4000000000u,
                                -123LL));
    acc = mix(acc, dbits(dmix(1234567890123LL, 0.5, 0.25f, -9LL)));
    acc = mix(acc, (u64)fact(20));
    acc = mix(acc, (u64)fact(3) + (u64)fact(4));
    for (k = 0; k < 10; k++)
    {
        acc = mix(acc, (u64)mixargs(k, (i64)k * 1000000000LL, k, k * 0.5, k,
                                     (i64)-k));
    }
    return acc;
}

/* globals, arrays, pointers, structs and locals of 64-bit and double type */
static i64    g_arr[4];
static double g_darr[3];
static i64    g_val = 42LL;
static double g_dval = 0.125;

struct wide_s {
    int    tag;
    i64    count;
    double value;
};

static u64 test_memory(void)
{
    i64            loc[3];
    double         dloc[2];
    struct wide_s  s1;
    struct wide_s  s2;
    i64*           p = g_arr;
    double*        dp = g_darr;
    u64            acc = 0;
    int            i;

    for (i = 0; i < 4; i++)
    {
        g_arr[i] = (i64)i * 1000000000000LL - g_val;
    }
    for (i = 0; i < 3; i++)
    {
        g_darr[i] = g_dval * (double)(i + 1);
    }
    acc = mix(acc, (u64)p[2]);
    acc = mix(acc, (u64)*(p + 3));
    acc = mix(acc, dbits(dp[1]));
    p[1] += 5LL;
    dp[2] = dp[2] * 2.0;
    acc = mix(acc, (u64)g_arr[1]);
    acc = mix(acc, dbits(g_darr[2]));

    loc[0] = 1LL << 40;
    loc[1] = loc[0] + 7;
    loc[2] = loc[1] * -3;
    dloc[0] = 0.5;
    dloc[1] = dloc[0] * loc[2];
    acc = mix(acc, (u64)loc[2]);
    acc = mix(acc, dbits(dloc[1]));

    s1.tag = 3;
    s1.count = 77777777777LL;
    s1.value = -0.5;
    s2 = s1;
    s2.count += s2.tag;
    acc = mix(acc, (u64)s2.count);
    acc = mix(acc, dbits(s2.value));
    acc = mix(acc, (u64)s1.count);
    return acc;
}

/* calls to the host with 64-bit arguments and results */
static u64 test_syscall(void)
{
    i64    big = 0x123456789ABCDEF0LL;
    u64    acc = 0;

    wide_add(big, 1LL);
    wide_scale(big, 1.0);
    acc = mix(acc, (u64)wide_add(big, -2LL));
    acc = mix(acc, (u64)wide_add(wide_add(1LL, 2LL), 3LL) * 2);
    acc = mix(acc, dbits(wide_scale(big, 0.5)));
    acc = mix(acc, dbits(wide_scale(-7LL, 1.25) + 1.0));
    return acc;
}

/* loops and recursion with 64-bit values */
static u64 test_loop(void)
{
    i64 sum = 0;
    double dsum = 0.0;
    int i;

    for (i = 1; i <= 1000; i++)
    {
        sum += (i64)i * i * i;
        dsum += 1.0 / (double)i;
    }
    return mix(mix((u64)sum, dbits(dsum)), (u64)fact(15));
}
