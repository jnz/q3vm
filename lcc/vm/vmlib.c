/* vmlib.c -- the part of the C library that the compiler needs, for code compiled
 * to the VM. Files, environment and time go to the host through vm_syscalls.asm.
 * The string functions, qsort and the conversions are in test/bg_lib.c.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <utime.h>
#include "vmsys.h"

int vsprintf(char* buffer, const char* fmt, va_list argptr);
double atof(const char* string);

int errno;
FILE vm_files[3] = { { 0, 0, 0 }, { 1, 0, 0 }, { 2, 0, 0 } };

/* ---------- memory ---------- */

/* The heap is in bss. A block has an 8 byte header, its capacity (a multiple of 8)
 * and the offset of the next free block in its bin (0 for none). A block of up to
 * VM_BIN_MAX bytes goes on the bin of its capacity when it is freed, and is reused
 * for a request of the same capacity. The other blocks come from the top of the
 * heap. Offsets are used instead of pointers. */
#ifndef VM_HEAP_SIZE
#define VM_HEAP_SIZE (6 * 1024 * 1024)
#endif
#define VM_ALIGN 8
#define VM_BIN_MAX 1024
#define VM_BINS (VM_BIN_MAX / VM_ALIGN + 1)

static char   vm_heap[VM_HEAP_SIZE];
/* the heap: the array above, or the extra heap of -heap (see vm_heap_start). The
 * first malloc sets it. */
static char*  vm_base;
static size_t vm_size;
static size_t vm_heap_top; /* offset of the next block, set by the first malloc */
static int    vm_heap_ready;
static size_t vm_bin[VM_BINS];

void* memcpy(void* dest, const void* src, size_t n)
{
    char*       d = dest;
    const char* s = src;

    while (n-- > 0)
    {
        *d++ = *s++;
    }
    return dest;
}

void* memset(void* dest, int c, size_t n)
{
    char* d = dest;

    while (n-- > 0)
    {
        *d++ = (char)c;
    }
    return dest;
}

int memcmp(const void* a, const void* b, size_t n)
{
    const unsigned char* p = a;
    const unsigned char* q = b;

    for (; n > 0; n--, p++, q++)
    {
        if (*p != *q)
        {
            return *p < *q ? -1 : 1;
        }
    }
    return 0;
}

void bzero(void* dest, size_t n)
{
    memset(dest, 0, n);
}

void* malloc(size_t n)
{
    size_t  cap = (n + VM_ALIGN - 1) & ~(VM_ALIGN - 1);
    size_t* block;

    if (!vm_heap_ready)
    {
        /* the data of a block must be 8 aligned, and the bss may not be: the first
         * block starts after the padding that aligns it. The offset 0 is never a
         * block, so that 0 can mean no block in a bin. */
        int start = vm_heap_start();
        int end   = vm_heap_end();

        if (start != 0 && end > start)
        {
            vm_base = (char*)start;
            vm_size = (size_t)(end - start);
        }
        else
        {
            vm_base = vm_heap;
            vm_size = VM_HEAP_SIZE;
        }
        vm_heap_top   = VM_ALIGN + ((VM_ALIGN - ((size_t)vm_base & (VM_ALIGN - 1))) & (VM_ALIGN - 1));
        vm_heap_ready = 1;
    }
    if (cap == 0)
    {
        cap = VM_ALIGN;
    }
    if (cap <= VM_BIN_MAX && vm_bin[cap / VM_ALIGN])
    {
        block = (size_t*)(vm_base + vm_bin[cap / VM_ALIGN]);
        vm_bin[cap / VM_ALIGN] = block[1];
    }
    else
    {
        if (cap > vm_size - vm_heap_top - VM_ALIGN)
        {
            fprintf(stderr, "vm heap exhausted: %u bytes requested, %u used\n", (unsigned)cap, (unsigned)vm_heap_top);
            vm_exit(EXIT_FAILURE);
        }
        block = (size_t*)(vm_base + vm_heap_top);
        vm_heap_top += VM_ALIGN + cap;
    }
    block[0] = cap;
    block[1] = 0;
    /* a checked VM reports reads of the block before it is written */
    vm_poison(block + 2, cap);
    return block + 2;
}

void* calloc(size_t n, size_t size)
{
    void* p = malloc(n * size);

    return memset(p, 0, n * size);
}

void* realloc(void* p, size_t n)
{
    size_t* old = (size_t*)p - 2;
    void*   q;

    if (!p)
    {
        return malloc(n);
    }
    if (n <= old[0])
    {
        return p;
    }
    q = malloc(n);
    memcpy(q, p, old[0]);
    free(p);
    return q;
}

void free(void* p)
{
    size_t* block;

    if (!p)
    {
        return;
    }
    block = (size_t*)p - 2;
    if (block[0] <= VM_BIN_MAX)
    {
        block[1] = vm_bin[block[0] / VM_ALIGN];
        vm_bin[block[0] / VM_ALIGN] = (size_t)((char*)block - vm_base);
    }
}

/* returns the address of the next variable argument of the given size and
 * moves ap past it. Arguments of more than 4 bytes are aligned to 8. */
void* vm_va_next(va_list* ap, int size)
{
    unsigned a = (unsigned)*ap;

    if (size > 4)
    {
        a = (a + 7) & ~7u;
    }
    else
    {
        a = (a + 3) & ~3u;
    }
    *ap = (va_list)(a + ((size + 3) & ~3));
    return (void*)a;
}

/* ---------- strings ---------- */

char* strncpy(char* dest, const char* src, size_t n)
{
    size_t i;

    for (i = 0; i < n && src[i]; i++)
    {
        dest[i] = src[i];
    }
    for (; i < n; i++)
    {
        dest[i] = 0;
    }
    return dest;
}

int strncmp(const char* a, const char* b, size_t n)
{
    for (; n > 0; n--, a++, b++)
    {
        if (*a != *b)
        {
            return *a < *b ? -1 : 1;
        }
        if (!*a)
        {
            return 0;
        }
    }
    return 0;
}

char* strrchr(const char* s, int c)
{
    const char* last = NULL;

    for (; *s; s++)
    {
        if (*s == (char)c)
        {
            last = s;
        }
    }
    return (char*)(c == 0 ? s : last);
}

/* ---------- ctype ---------- */

int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

int isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}

int islower(int c)
{
    return c >= 'a' && c <= 'z';
}

int isalpha(int c)
{
    return isupper(c) || islower(c);
}

int isalnum(int c)
{
    return isalpha(c) || isdigit(c);
}

int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

int isprint(int c)
{
    return c >= ' ' && c < 127;
}

/* ---------- conversions ---------- */

long strtol(const char* s, char** end, int base)
{
    long value = 0;
    int  negative = 0;

    while (isspace(*s))
    {
        s++;
    }
    if (*s == '-' || *s == '+')
    {
        negative = *s == '-';
        s++;
    }
    if (base == 0)
    {
        base = 10;
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        {
            base = 16;
            s += 2;
        }
        else if (s[0] == '0')
        {
            base = 8;
        }
    }
    for (;; s++)
    {
        int digit;

        if (isdigit(*s))
        {
            digit = *s - '0';
        }
        else if (isalpha(*s))
        {
            digit = tolower(*s) - 'a' + 10;
        }
        else
        {
            break;
        }
        if (digit >= base)
        {
            break;
        }
        value = value * base + digit;
    }
    if (end)
    {
        *end = (char*)s;
    }
    return negative ? -value : value;
}

long atol(const char* s)
{
    return strtol(s, NULL, 10);
}

/* the end pointer is set to the terminator, which is wrong for a partial parse
 * but the compiler only passes NULL or a complete number */
/* the value of an IEEE double, from its bits: hi and lo are its two words. It is built
 * with arithmetic, so that it is right for a double of 8 bytes and for a float. */
static double vm_double_from_bits(unsigned int hi, unsigned int lo)
{
    int    sign = (hi >> 31) ? -1 : 1;
    int    e = (int)((hi >> 20) & 0x7ff);
    double f;
    double v;

    if (e == 0)
    {
        return 0;
    }
    if (e == 0x7ff)
    {
        return sign * 1e30;
    }
    f = (double)(hi & 0xfffff) / 1048576.0 + (double)lo / 4503599627370496.0;
    v = 1 + f;
    for (e -= 1023; e > 0; e--)
    {
        v *= 2;
    }
    for (; e < 0; e++)
    {
        v *= 0.5;
    }
    return sign * v;
}

double strtod(const char* s, char** end)
{
    unsigned int raw[2]; /* the 8 bytes of a double, low word first */

    /* the host converts, so the result is the same as the native compiler's */
    vm_strtod(s, (double*)raw);
    if (end)
    {
        *end = (char*)s + strlen(s);
    }
    return vm_double_from_bits(raw[1], raw[0]);
}

/* ---------- process and environment ---------- */

void exit(int status)
{
    vm_exit(status);
}

void abort(void)
{
    vm_exit(EXIT_FAILURE);
}

void vm_assert_fail(const char* expr, const char* file, int line)
{
    fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", expr, file, line);
    abort();
}

char* getenv(const char* name)
{
    static char value[512];

    if (vm_getenv(name, value, sizeof value) < 0)
    {
        return NULL;
    }
    return value;
}

time_t time(time_t* t)
{
    time_t now = vm_time();

    if (t)
    {
        *t = now;
    }
    return now;
}

char* ctime(const time_t* t)
{
    static char text[64];

    vm_ctime(*t, text, sizeof text);
    return text;
}

/* ---------- stdio ---------- */

static char vm_print_buffer[4096];

static FILE* vm_new_file(int handle)
{
    FILE* f = malloc(sizeof(FILE));

    f->handle = handle;
    f->eof = 0;
    f->error = 0;
    return f;
}

FILE* fopen(const char* name, const char* mode)
{
    int handle = vm_open(name, mode);

    if (handle < 0)
    {
        return NULL;
    }
    return vm_new_file(handle);
}

/* the VM has no file permissions: a file that can be opened can be read, and
 * the other modes are not checked (see unistd.h) */
int access(const char* path, int mode)
{
    FILE* f;

    (void)mode;
    f = fopen(path, "r");
    if (!f)
    {
        return -1;
    }
    fclose(f);
    return 0;
}

FILE* freopen(const char* name, const char* mode, FILE* f)
{
    int handle = vm_open(name, mode);

    if (handle < 0)
    {
        return NULL;
    }
    /* the standard streams keep their FILE, only the host handle changes */
    if (f->handle > 2)
    {
        vm_close(f->handle);
    }
    f->handle = handle;
    f->eof = 0;
    f->error = 0;
    return f;
}

int fclose(FILE* f)
{
    if (f->handle > 2)
    {
        vm_close(f->handle);
    }
    return 0;
}

size_t fread(void* ptr, size_t size, size_t n, FILE* f)
{
    int    total = size * n;
    int    got = 0;
    char*  p = ptr;

    while (got < total)
    {
        int r = vm_read(f->handle, p + got, total - got);

        if (r <= 0)
        {
            f->eof = 1;
            break;
        }
        got += r;
    }
    return size ? got / size : 0;
}

size_t fwrite(const void* ptr, size_t size, size_t n, FILE* f)
{
    int total = size * n;
    int r = vm_write(f->handle, ptr, total);

    if (r != total)
    {
        f->error = 1;
    }
    return size ? r / size : 0;
}

int fgetc(FILE* f)
{
    unsigned char c;

    if (fread(&c, 1, 1, f) != 1)
    {
        return EOF;
    }
    return c;
}

int getc(FILE* f)
{
    return fgetc(f);
}

int getchar(void)
{
    return fgetc(stdin);
}

char* fgets(char* s, int n, FILE* f)
{
    int i = 0;

    while (i < n - 1)
    {
        int c = fgetc(f);

        if (c == EOF)
        {
            break;
        }
        s[i++] = (char)c;
        if (c == '\n')
        {
            break;
        }
    }
    if (i == 0)
    {
        return NULL;
    }
    s[i] = 0;
    return s;
}

int fputc(int c, FILE* f)
{
    char ch = (char)c;

    fwrite(&ch, 1, 1, f);
    return c;
}

int putc(int c, FILE* f)
{
    return fputc(c, f);
}

int putchar(int c)
{
    return fputc(c, stdout);
}

int fputs(const char* s, FILE* f)
{
    fwrite(s, 1, strlen(s), f);
    return 0;
}

int puts(const char* s)
{
    fputs(s, stdout);
    return fputc('\n', stdout);
}

int fflush(FILE* f)
{
    (void)f;
    return 0;
}

int setvbuf(FILE* f, char* buf, int mode, size_t size)
{
    (void)f;
    (void)buf;
    (void)mode;
    (void)size;
    return 0;
}

int fseek(FILE* f, long off, int whence)
{
    f->eof = 0;
    return vm_seek(f->handle, off, whence) < 0 ? -1 : 0;
}

long ftell(FILE* f)
{
    return vm_tell(f->handle);
}

void rewind(FILE* f)
{
    fseek(f, 0, SEEK_SET);
    f->error = 0;
}

int feof(FILE* f)
{
    return f->eof;
}

int ferror(FILE* f)
{
    return f->error;
}

void setbuf(FILE* f, char* buf)
{
    (void)f;
    (void)buf;
}

int remove(const char* name)
{
    return vm_remove(name);
}

void perror(const char* s)
{
    fprintf(stderr, "%s: error %d\n", s, errno);
}

static int vprintf_to(FILE* f, const char* fmt, va_list ap)
{
    int len = vsprintf(vm_print_buffer, fmt, ap);

    fwrite(vm_print_buffer, 1, len, f);
    return len;
}

int printf(const char* fmt, ...)
{
    va_list ap;
    int     len;

    va_start(ap, fmt);
    len = vprintf_to(stdout, fmt, ap);
    va_end(ap);
    return len;
}

int fprintf(FILE* f, const char* fmt, ...)
{
    va_list ap;
    int     len;

    va_start(ap, fmt);
    len = vprintf_to(f, fmt, ap);
    va_end(ap);
    return len;
}

int sprintf(char* buf, const char* fmt, ...)
{
    va_list ap;
    int     len;

    va_start(ap, fmt);
    len = vsprintf(buf, fmt, ap);
    va_end(ap);
    return len;
}

int vprintf(const char* fmt, va_list ap)
{
    return vprintf_to(stdout, fmt, ap);
}

int vfprintf(FILE* f, const char* fmt, va_list ap)
{
    return vprintf_to(f, fmt, ap);
}

/* ---------- more conversions and strings, for the assembler ---------- */

#ifndef LCC_WIDE32
unsigned long long strtoull(const char* s, char** end, int base)
{
    unsigned long long value = 0;
    int                negative = 0;

    while (isspace(*s))
    {
        s++;
    }
    /* as in C, a minus sign negates the unsigned result, so "-5" is 2^64 - 5 */
    if (*s == '-' || *s == '+')
    {
        negative = *s == '-';
        s++;
    }
    if (base == 0)
    {
        base = 10;
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        {
            base = 16;
            s += 2;
        }
        else if (s[0] == '0')
        {
            base = 8;
        }
    }
    for (;; s++)
    {
        int digit;

        if (isdigit(*s))
        {
            digit = *s - '0';
        }
        else if (isalpha(*s))
        {
            digit = tolower(*s) - 'a' + 10;
        }
        else
        {
            break;
        }
        if (digit >= base)
        {
            break;
        }
        value = value * base + digit;
    }
    if (end)
    {
        *end = (char*)s;
    }
    return negative ? 0 - value : value;
}

long long atoll(const char* s)
{
    int                negative = 0;
    unsigned long long value;

    while (isspace(*s))
    {
        s++;
    }
    if (*s == '-' || *s == '+')
    {
        negative = *s == '-';
        s++;
    }
    value = strtoull(s, NULL, 10);
    return negative ? -(long long)value : (long long)value;
}
#endif

char* strncat(char* dest, const char* src, size_t n)
{
    char* d = dest + strlen(dest);

    while (n-- > 0 && *src)
    {
        *d++ = *src++;
    }
    *d = 0;
    return dest;
}

char* strerror(int errnum)
{
    (void)errnum;
    return "error";
}

/* the VM has no current directory and no file status of its own */
char* getcwd(char* buf, int size)
{
    if (size < 2)
    {
        return NULL;
    }
    strcpy(buf, "/");
    return buf;
}

int chdir(const char* path)
{
    (void)path;
    return -1;
}

int mkdir(const char* path, mode_t mode)
{
    (void)path;
    (void)mode;
    return -1;
}

int stat(const char* path, struct stat* buf)
{
    (void)path;
    (void)buf;
    return -1;
}

int lstat(const char* path, struct stat* buf)
{
    return stat(path, buf);
}

int fstat(int fd, struct stat* buf)
{
    (void)fd;
    (void)buf;
    return -1;
}

/* ---------- the POSIX calls the VM has no use for: they fail, see the headers ---------- */

int gettimeofday(struct timeval* tv, void* tz)
{
    (void)tz;
    tv->tv_sec  = time(NULL);
    tv->tv_usec = 0;
    return 0;
}

int getrusage(int who, struct rusage* usage)
{
    (void)who;
    (void)usage;
    return -1;
}

int ioctl(int fd, unsigned long request, void* arg)
{
    (void)fd;
    (void)request;
    (void)arg;
    return -1;
}

int isatty(int fd)
{
    (void)fd;
    return 0;
}

int getpid(void)
{
    return 1;
}

int usleep(unsigned int usec)
{
    (void)usec;
    return 0;
}

sighandler_t signal(int sig, sighandler_t handler)
{
    (void)sig;
    (void)handler;
    return SIG_DFL;
}

int raise(int sig)
{
    (void)sig;
    return -1;
}

DIR* opendir(const char* name)
{
    (void)name;
    return NULL;
}

struct dirent* readdir(DIR* dir)
{
    (void)dir;
    return NULL;
}

int closedir(DIR* dir)
{
    (void)dir;
    return -1;
}

struct passwd* getpwuid(int uid)
{
    (void)uid;
    return NULL;
}

int utime(const char* path, const struct utimbuf* times)
{
    (void)path;
    (void)times;
    return -1;
}

int system(const char* command)
{
    (void)command;
    return -1;
}

char* realpath(const char* path, char* resolved)
{
    (void)path;
    (void)resolved;
    return NULL;
}

char* strdup(const char* s)
{
    size_t n = strlen(s) + 1;
    char*  p = malloc(n);

    if (p)
    {
        memcpy(p, s, n);
    }
    return p;
}

static int in_set(char c, const char* set)
{
    for (; *set; set++)
    {
        if (*set == c)
        {
            return 1;
        }
    }
    return 0;
}

size_t strspn(const char* s, const char* accept)
{
    size_t n = 0;

    while (s[n] && in_set(s[n], accept))
    {
        n++;
    }
    return n;
}

size_t strcspn(const char* s, const char* reject)
{
    size_t n = 0;

    while (s[n] && !in_set(s[n], reject))
    {
        n++;
    }
    return n;
}

void* memchr(const void* s, int c, size_t n)
{
    const unsigned char* p = s;

    for (; n > 0; n--, p++)
    {
        if (*p == (unsigned char)c)
        {
            return (void*)p;
        }
    }
    return NULL;
}

#ifndef LCC_WIDE32
long long strtoll(const char* s, char** end, int base)
{
    return (long long)strtoull(s, end, base);
}
#endif

/* snprintf formats into a scratch buffer, then copies at most n - 1 bytes. A result
 * longer than VM_FORMAT_MAX bytes is not safe: keep the formats of snprintf short. */
#define VM_FORMAT_MAX 8192
static char vm_format_buf[VM_FORMAT_MAX];

int vsnprintf(char* buf, size_t n, const char* fmt, va_list ap)
{
    int len = vsprintf(vm_format_buf, fmt, ap);

    if (n > 0)
    {
        size_t copy = (size_t)len < n ? (size_t)len : n - 1;

        memcpy(buf, vm_format_buf, copy);
        buf[copy] = '\0';
    }
    return len;
}

int snprintf(char* buf, size_t n, const char* fmt, ...)
{
    va_list ap;
    int     len;

    va_start(ap, fmt);
    len = vsnprintf(buf, n, fmt, ap);
    va_end(ap);
    return len;
}

/* the VM has no local time, host names, environment, links, file modes or processes:
 * these calls fail, or do nothing where failing would be wrong */
struct tm* localtime(const time_t* t)
{
    (void)t;
    return NULL;
}

size_t strftime(char* buf, size_t max, const char* fmt, const struct tm* tm)
{
    (void)buf;
    (void)max;
    (void)fmt;
    (void)tm;
    return 0;
}

int gethostname(char* name, size_t len)
{
    (void)name;
    (void)len;
    return -1;
}

int getuid(void)
{
    return 0;
}

int unlink(const char* path)
{
    return vm_remove(path) == 0 ? 0 : -1;
}

int symlink(const char* target, const char* path)
{
    (void)target;
    (void)path;
    return -1;
}

int readlink(const char* path, char* buf, size_t size)
{
    (void)path;
    (void)buf;
    (void)size;
    return -1;
}

int chmod(const char* path, mode_t mode)
{
    (void)path;
    (void)mode;
    return 0;
}

int utimes(const char* path, const struct timeval times[2])
{
    (void)path;
    (void)times;
    return -1;
}

int setenv(const char* name, const char* value, int overwrite)
{
    (void)name;
    (void)value;
    (void)overwrite;
    return -1;
}

int unsetenv(const char* name)
{
    (void)name;
    return -1;
}

int atexit(void (*fn)(void))
{
    (void)fn;
    return 0;
}

FILE* popen(const char* command, const char* mode)
{
    (void)command;
    (void)mode;
    return NULL;
}

int pclose(FILE* f)
{
    (void)f;
    return -1;
}

int signbit(double x)
{
    return x < 0;
}

/* ---------- math, see include/math.h ----------
 * Plain C, with series and Newton steps. The integer part is taken with a long, so
 * floor, ceil and fmod are exact for |x| < 2^30, and they are not used beyond that.
 * On the 32-bit target a double is a float, and the results have the precision of a
 * float. */

#define VM_PI 3.14159265358979323846
#define VM_LN2 0.69314718055994530942

static double vm_trunc(double x)
{
    if (x > -1073741824.0 && x < 1073741824.0)
    {
        return (double)(long)x;
    }
    return x;
}

/* fabs is in test/bg_lib.c, with the other shared functions */

double floor(double x)
{
    double t = vm_trunc(x);

    return t > x ? t - 1 : t;
}

double ceil(double x)
{
    double t = vm_trunc(x);

    return t < x ? t + 1 : t;
}

double fmod(double x, double y)
{
    if (y == 0)
    {
        return 0;
    }
    return x - y * vm_trunc(x / y);
}

double sqrt(double x)
{
    double g;
    int    i;

    if (x <= 0)
    {
        return 0;
    }
    g = x > 1 ? x : 1;
    for (i = 0; i < 60; i++)
    {
        g = 0.5 * (g + x / g);
    }
    return g;
}

/* the argument is reduced to [-pi, pi] first */
double sin(double x)
{
    double r = x - 2 * VM_PI * floor(x / (2 * VM_PI) + 0.5);
    double term = r;
    double sum = r;
    int    n;

    for (n = 1; n < 12; n++)
    {
        term *= -r * r / ((2 * n) * (2 * n + 1));
        sum += term;
    }
    return sum;
}

double cos(double x)
{
    double r = x - 2 * VM_PI * floor(x / (2 * VM_PI) + 0.5);
    double term = 1;
    double sum = 1;
    int    n;

    for (n = 1; n < 12; n++)
    {
        term *= -r * r / ((2 * n - 1) * (2 * n));
        sum += term;
    }
    return sum;
}

double tan(double x)
{
    return sin(x) / cos(x);
}

/* atan for |x| <= 1: atan(x) = 2 atan(x / (1 + sqrt(1 + x*x))), three times, then a
 * series, and the result is multiplied by 2^3 */
static double vm_atan_small(double x)
{
    double y = x;
    double y2;
    double term;
    double sum;
    int    k;
    int    n;

    for (k = 0; k < 3; k++)
    {
        y = y / (1 + sqrt(1 + y * y));
    }
    y2 = y * y;
    term = y;
    sum = y;
    for (n = 1; n < 12; n++)
    {
        term *= -y2;
        sum += term / (2 * n + 1);
    }
    return 8 * sum;
}

double atan(double x)
{
    if (x < 0)
    {
        return -atan(-x);
    }
    if (x > 1)
    {
        return VM_PI / 2 - vm_atan_small(1 / x);
    }
    return vm_atan_small(x);
}

double atan2(double y, double x)
{
    if (x > 0)
    {
        return atan(y / x);
    }
    if (x < 0)
    {
        return y >= 0 ? atan(y / x) + VM_PI : atan(y / x) - VM_PI;
    }
    if (y > 0)
    {
        return VM_PI / 2;
    }
    if (y < 0)
    {
        return -VM_PI / 2;
    }
    return 0;
}

/* exp(x) = 2^k * exp(r), with x = k ln 2 + r */
double exp(double x)
{
    double k = floor(x / VM_LN2 + 0.5);
    double r = x - k * VM_LN2;
    double term = 1;
    double sum = 1;
    int    n;

    for (n = 1; n < 20; n++)
    {
        term *= r / n;
        sum += term;
    }
    for (; k > 0; k--)
    {
        sum *= 2;
    }
    for (; k < 0; k++)
    {
        sum *= 0.5;
    }
    return sum;
}

/* log(x) = k ln 2 + 2 atanh(y), with x = 2^k m, 1 <= m < 2 and y = (m - 1) / (m + 1).
 * There is no negative infinity, so log of a number that is not positive is 0. */
double log(double x)
{
    double k = 0;
    double m;
    double y;
    double y2;
    double term;
    double sum;
    int    n;

    if (x <= 0)
    {
        return 0;
    }
    m = x;
    while (m >= 2)
    {
        m *= 0.5;
        k++;
    }
    while (m < 1)
    {
        m *= 2;
        k--;
    }
    y = (m - 1) / (m + 1);
    y2 = y * y;
    term = y;
    sum = y;
    for (n = 1; n < 30; n++)
    {
        term *= y2;
        sum += term / (2 * n + 1);
    }
    return k * VM_LN2 + 2 * sum;
}

/* x to the power of y, for a negative x only with an integer y */
double pow(double x, double y)
{
    double r;
    long   n;

    if (y == 0)
    {
        return 1;
    }
    if (x == 0)
    {
        return 0;
    }
    if (x < 0)
    {
        if (floor(y) != y)
        {
            return 0;
        }
        r = pow(-x, y);
        n = (long)vm_trunc(y);
        return (n % 2) ? -r : r;
    }
    return exp(y * log(x));
}
