/* lcc-run.c -- one executable that compiles a C program with the compiler running
 * in q3vm, links it with the VM runtime and runs it in q3vm.
 *
 * Usage: lcc-run file.c [arguments...]
 *
 * The executable holds everything it needs (payload.c, made by payload.py):
 * the compiler rcc.qvm, the assembler q3asm.qvm, the runtime library and the include
 * files of q3vm, and the preprocessor q3cpp. The steps are:
 *
 *   1. unpack the payload into a temporary directory, with the source file
 *   2. q3cpp preprocesses the source (a native program, from the payload)
 *   3. rcc.qvm, the compiler in the VM, writes LCC bytecode
 *   4. q3asm links the bytecode with the runtime library (native, from the payload)
 *   5. the program runs in the VM, with the arguments
 *
 * LCC_RUN_WIDTH (32 or 64) is set by the Makefile and must match rcc.qvm.
 */

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "payload.h"
#include "vmhost.h"

#ifndef LCC_RUN_WIDTH
#define LCC_RUN_WIDTH 32
#endif

#if LCC_RUN_WIDTH == 64
#define TARGET_OPTION "-target=bytecode64"
#else
#define TARGET_OPTION "-target=bytecode"
#endif

#define CPP_DEFINES_COMMON "-D__STDC__=1 -D__STRICT_ANSI__ -D__signed__=signed -DQ3_VM -D__LCC__"
/* the headers of q3vm leave out the 64-bit functions for a 32-bit target */
#if LCC_RUN_WIDTH == 64
#define CPP_DEFINES CPP_DEFINES_COMMON
#else
#define CPP_DEFINES CPP_DEFINES_COMMON " -DLCC_WIDE32"
#endif

static const struct payload_file* find_payload(const char* name)
{
    const struct payload_file* f;

    for (f = payload_files; f->name; f++)
    {
        if (strcmp(f->name, name) == 0)
        {
            return f;
        }
    }
    return NULL;
}

static int write_file(const char* path, const unsigned char* data, size_t size,
                      int executable)
{
    FILE* f = fopen(path, "wb");

    if (!f)
    {
        return -1;
    }
    if (size > 0 && fwrite(data, 1, size, f) != size)
    {
        fclose(f);
        return -1;
    }
    fclose(f);
    if (executable)
    {
        chmod(path, 0755);
    }
    return 0;
}

/* unpacks every payload file below dir, creating the directories it needs */
static int unpack(const char* dir)
{
    const struct payload_file* f;
    char                       path[PATH_MAX];

    for (f = payload_files; f->name; f++)
    {
        snprintf(path, sizeof path, "%s/%s", dir, f->name);
        if (strchr(f->name, '/'))
        {
            char sub[PATH_MAX];
            size_t n = strchr(f->name, '/') - f->name;

            snprintf(sub, sizeof sub, "%s/%.*s", dir, (int)n, f->name);
            mkdir(sub, 0755);
        }
        if (write_file(path, f->data, f->size, strncmp(f->name, "bin/", 4) == 0) != 0)
        {
            fprintf(stderr, "lcc-run: cannot write %s\n", path);
            return -1;
        }
    }
    return 0;
}

/* copies the source file and the headers next to it, so that quoted includes work */
static int copy_sources(const char* src, const char* srcdir, const char* base,
                        const char* dir)
{
    char          dst[PATH_MAX];
    FILE*         in;
    unsigned char buf[4096];
    size_t        n;
    FILE*         out;
    DIR*          d;
    struct dirent* e;

    snprintf(dst, sizeof dst, "%s/%s.c", dir, base);
    in = fopen(src, "rb");
    out = fopen(dst, "wb");
    if (!in || !out)
    {
        return -1;
    }
    while ((n = fread(buf, 1, sizeof buf, in)) > 0)
    {
        fwrite(buf, 1, n, out);
    }
    fclose(in);
    fclose(out);

    d = opendir(srcdir);
    if (!d)
    {
        return 0;
    }
    while ((e = readdir(d)) != NULL)
    {
        size_t len = strlen(e->d_name);
        char   from[PATH_MAX];
        char   to[PATH_MAX];

        if (len < 3 || strcmp(e->d_name + len - 2, ".h") != 0)
        {
            continue;
        }
        snprintf(from, sizeof from, "%s/%s", srcdir, e->d_name);
        snprintf(to, sizeof to, "%s/%s", dir, e->d_name);
        in = fopen(from, "rb");
        out = fopen(to, "wb");
        if (in && out)
        {
            while ((n = fread(buf, 1, sizeof buf, in)) > 0)
            {
                fwrite(buf, 1, n, out);
            }
        }
        if (in)
        {
            fclose(in);
        }
        if (out)
        {
            fclose(out);
        }
    }
    closedir(d);
    return 0;
}

static uint8_t* read_image(const char* path, int* size)
{
    FILE*    f = fopen(path, "rb");
    uint8_t* data;

    if (!f)
    {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    *size = (int)ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(*size);
    if (data && fread(data, 1, *size, f) != (size_t)*size)
    {
        free(data);
        data = NULL;
    }
    fclose(f);
    return data;
}

static void remove_dir(const char* dir)
{
    char cmd[PATH_MAX + 16];

    /* LCC_RUN_KEEP=1 keeps the temporary files, to look at them */
    if (getenv("LCC_RUN_KEEP"))
    {
        fprintf(stderr, "lcc-run: kept %s\n", dir);
        return;
    }
    snprintf(cmd, sizeof cmd, "rm -rf '%s'", dir);
    if (system(cmd) != 0)
    {
        fprintf(stderr, "lcc-run: cannot remove %s\n", dir);
    }
}

static void usage(const char* name)
{
    fprintf(stderr, "usage: %s file.c [arguments...]\n", name);
    exit(2);
}

int main(int argc, char** argv)
{
    const char*    src;
    char           srcdir[PATH_MAX];
    char           base[256];
    char           tmpl[PATH_MAX];
    char           dir[PATH_MAX];
    char           saved[PATH_MAX];
    char           cmd[2 * PATH_MAX + 256];
    char           iname[PATH_MAX];
    char           aname[PATH_MAX];
    const char*    name;
    const char*    tmproot;
    const char*    slash;
    const char*    dot;
    const struct payload_file* rcc;
    const struct payload_file* qasm;
    char*                      qargv[8];
    uint8_t*       image;
    int            size = 0;
    int            status;
    char*          rargv[4];
    char**         pargv;
    int            i;

    if (argc < 2)
    {
        usage(argv[0]);
    }
    src = argv[1];
    slash = strrchr(src, '/');
    if (slash)
    {
        size_t n = slash - src;
        snprintf(srcdir, sizeof srcdir, "%.*s", (int)(n ? n : 1), src);
    }
    else
    {
        snprintf(srcdir, sizeof srcdir, ".");
    }
    name = slash ? slash + 1 : src;
    dot = strrchr(name, '.');
    snprintf(base, sizeof base, "%.*s", dot ? (int)(dot - name) : (int)strlen(name), name);

    tmproot = getenv("TMPDIR");
    snprintf(tmpl, sizeof tmpl, "%s/lccrun.XXXXXX", tmproot ? tmproot : "/tmp");
    if (!mkdtemp(tmpl))
    {
        fprintf(stderr, "lcc-run: cannot make a temporary directory\n");
        return 1;
    }
    snprintf(dir, sizeof dir, "%s", tmpl);
    if (!getcwd(saved, sizeof saved))
    {
        remove_dir(dir);
        return 1;
    }

    if (unpack(dir) != 0 || copy_sources(src, srcdir, base, dir) != 0)
    {
        fprintf(stderr, "lcc-run: cannot prepare %s\n", src);
        remove_dir(dir);
        return 1;
    }
    if (chdir(dir) != 0)
    {
        remove_dir(dir);
        return 1;
    }

    /* 2. preprocess */
    snprintf(cmd, sizeof cmd, "./bin/q3cpp %s -Iinclude '%s.c' > '%s.i'", CPP_DEFINES, base, base);
    if (system(cmd) != 0)
    {
        fprintf(stderr, "lcc-run: preprocessing failed\n");
        status = 1;
        goto done;
    }

    /* 3. compile in the VM */
    rcc = find_payload("rcc.qvm");
    if (!rcc)
    {
        fprintf(stderr, "lcc-run: the compiler is missing from the executable\n");
        status = 1;
        goto done;
    }
    snprintf(iname, sizeof iname, "%s.i", base);
    snprintf(aname, sizeof aname, "%s.asm", base);
    rargv[0] = "rcc";
    rargv[1] = TARGET_OPTION;
    rargv[2] = iname;
    rargv[3] = aname;
    if (vmhost_run(rcc->data, (int)rcc->size, 4, rargv) != 0)
    {
        fprintf(stderr, "lcc-run: compilation failed\n");
        status = 1;
        goto done;
    }

    /* 4. link with the runtime library, the assembler runs in the VM too */
    qasm = find_payload("q3asm.qvm");
    if (!qasm)
    {
        fprintf(stderr, "lcc-run: the assembler is missing from the executable\n");
        status = 1;
        goto done;
    }
    qargv[0] = "q3asm";
    qargv[1] = "-o";
    qargv[2] = "prog";
    qargv[3] = "lib/vmmain";
    qargv[4] = "lib/vmlib";
    qargv[5] = "lib/bg_lib";
    qargv[6] = base;
    qargv[7] = "lib/vm_syscalls";
    if (vmhost_run(qasm->data, (int)qasm->size, 8, qargv) != 0)
    {
        fprintf(stderr, "lcc-run: linking failed\n");
        status = 1;
        goto done;
    }

    /* 5. run the program in the VM, its argv[0] is the source name */
    image = read_image("prog.qvm", &size);
    if (!image)
    {
        fprintf(stderr, "lcc-run: no program image\n");
        status = 1;
        goto done;
    }
    pargv = malloc(sizeof(char*) * (argc - 1));
    pargv[0] = (char*)base;
    for (i = 2; i < argc; i++)
    {
        pargv[i - 1] = argv[i];
    }
    fflush(stdout);
    status = vmhost_run(image, size, argc - 1, pargv);
    free(image);
    free(pargv);

done:
    if (chdir(saved) != 0)
    {
        fprintf(stderr, "lcc-run: cannot return to %s\n", saved);
    }
    remove_dir(dir);
    return status < 0 ? 2 : status;
}
