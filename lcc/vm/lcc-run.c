/* lcc-run.c -- one executable that compiles a C program with the compiler running
 * in q3vm, links it with the VM runtime and runs it in q3vm.
 *
 * Usage: lcc-run [-S | -o out.qvm] [-I dir]... [-f file]... [-overlay[=DIR]]
 *        [-overlay-write=memory|disk] file.c|file.qvm [arguments...]
 *
 * -S compiles only: the program is not linked or run. -I dir adds the headers of dir.
 *
 * The executable holds everything it needs (payload.c, made by payload.py): the
 * driver driver.qvm, the compiler rcc.qvm, the assembler q3asm.qvm, the preprocessor
 * q3cpp.qvm, the runtime library and the include files of q3vm. All of them are VM
 * images. This program only puts the files in the virtual file system (vfs.c) and
 * runs the driver (driver.c) in the VM, which does the steps: preprocess, compile,
 * link and run. The files named on the command line are put in the file system too.
 */

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "payload.h"
#include "vfs.h"
#include "vmhost.h"

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

/* reads a whole host file, NULL when it cannot be read */
static uint8_t* read_file(const char* path, long* size)
{
    FILE*    f = fopen(path, "rb");
    uint8_t* data;

    if (!f)
    {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (*size = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0)
    {
        fclose(f);
        return NULL;
    }
    data = malloc(*size ? *size : 1);
    if (data && fread(data, 1, *size, f) != (size_t)*size)
    {
        free(data);
        data = NULL;
    }
    fclose(f);
    return data;
}

/* puts a host file in the virtual file system, in its directory, which is the root
 * of the VM: the files are named by their base names. False when it cannot be read. */
static int put_host_file(const char* path)
{
    const char* name = strrchr(path, '/');
    long        size;
    uint8_t*    data = read_file(path, &size);

    if (!data)
    {
        return 0;
    }
    vfs_put(name ? name + 1 : path, data, size);
    free(data);
    return 1;
}

/* the payload: the images, the runtime library and the headers */
static void put_payload(void)
{
    const struct payload_file* f;

    for (f = payload_files; f->name; f++)
    {
        vfs_put(f->name, f->data, f->size);
    }
}

/* the headers of a directory, so that quoted includes work */
static void put_headers(const char* dir)
{
    DIR*           d = opendir(dir);
    struct dirent* e;

    if (!d)
    {
        return;
    }
    while ((e = readdir(d)) != NULL)
    {
        size_t len = strlen(e->d_name);
        char   path[4096];

        if (len < 3 || strcmp(e->d_name + len - 2, ".h") != 0)
        {
            continue;
        }
        snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
        put_host_file(path);
    }
    closedir(d);
}

#ifndef LCC_RUN_WIDTH
#define LCC_RUN_WIDTH 32
#endif

static void usage(const char* name)
{
    fprintf(stderr, "lcc-run: %d-bit target (the compiler and the programs it builds)\n",
            LCC_RUN_WIDTH);
    fprintf(stderr, "usage: %s [-S | -o out.qvm] [-I dir]... [-f file]... [-s file.c]... [-overlay[=DIR]]\n"
            "       [-overlay-write=memory|disk] [-heap=SIZE] file.c|file.qvm [arguments...]\n", name);
    exit(2);
}

/* runs a .qvm image of the host with the arguments: argv[0] is the image, as the
 * name of the program, and the rest are its arguments */
static int run_image(int argc, char** argv)
{
    long      size;
    uint8_t*  image = read_file(argv[0], &size);
    int       status;

    if (!image)
    {
        fprintf(stderr, "lcc-run: cannot read %s\n", argv[0]);
        return 1;
    }
    fflush(stdout);
    status = vmhost_run(image, (int)size, argc, argv);
    free(image);
    return status < 0 ? 2 : status;
}

/* writes the linked program of the file system to a host file, as a .qvm image */
static int save_program(const char* path)
{
    long  size = vfs_size("prog.qvm");
    void* data;
    FILE* f;

    if (size < 0)
    {
        fprintf(stderr, "lcc-run: there is no program to save\n");
        return 1;
    }
    data = malloc((size_t)size);
    if (!data)
    {
        return 1;
    }
    vfs_get("prog.qvm", data, (size_t)size);
    f = fopen(path, "wb");
    if (!f)
    {
        fprintf(stderr, "lcc-run: cannot write %s\n", path);
        free(data);
        return 1;
    }
    fwrite(data, 1, (size_t)size, f);
    fclose(f);
    free(data);
    return 0;
}

int main(int argc, char** argv)
{
    const struct payload_file* driver;
    char*                      dargv[256];
    char                       srcdir[4096];
    const char*                src = NULL;
    const char*                out = NULL;
    const char*                slash;
    int                        dargc = 0;
    int                        i;
    int                        status;

    if (argc < 2)
    {
        usage(argv[0]);
    }
    driver = find_payload("driver.qvm");
    if (!driver)
    {
        fprintf(stderr, "lcc-run: the driver is missing from the executable\n");
        return 1;
    }

    /* the files of the VM are in the virtual file system, relative names start from "/" */
    vfs_init("/");

    /* the options before the file: -f puts a host file in the file system, for the
     * program to open, and -overlay reads the host files behind it. A .qvm file is
     * run as it is, with the arguments after it. */
    for (i = 1; i < argc && argv[i][0] == '-'; i++)
    {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
        {
            if (!put_host_file(argv[++i]))
            {
                fprintf(stderr, "lcc-run: cannot read %s\n", argv[i]);
                return 1;
            }
        }
        else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc)
        {
            if (!put_host_file(argv[++i]))
            {
                fprintf(stderr, "lcc-run: cannot read %s\n", argv[i]);
                return 1;
            }
        }
        else if (strcmp(argv[i], "-I") == 0 || strcmp(argv[i], "-o") == 0)
        {
            i++;
        }
        else if (vfs_option(argv[i]) < 0 || vmhost_option(argv[i]) < 0)
        {
            usage(argv[0]);
        }
    }
    if (vfs_check())
    {
        fprintf(stderr, "lcc-run: %s\n", vfs_check());
        return 2;
    }
    if (i < argc && strlen(argv[i]) > 4 && strcmp(argv[i] + strlen(argv[i]) - 4, ".qvm") == 0)
    {
        return run_image(argc - i, argv + i);
    }
    put_payload();

    /* the arguments for the driver: -I is done here, the rest goes on. The first
     * argument that is not an option is the source file. */
    dargv[dargc++] = argv[0];
    for (i = 1; i < argc && dargc < 256; i++)
    {
        if (!src && strcmp(argv[i], "-f") == 0 && i + 1 < argc)
        {
            /* already put in the file system above */
            i++;
            continue;
        }
        if (!src && strcmp(argv[i], "-s") == 0 && i + 1 < argc)
        {
            /* a source for the driver, put in the file system above */
            dargv[dargc++] = argv[i];
            dargv[dargc++] = argv[++i];
            continue;
        }
        if (!src && (vfs_option(argv[i]) == 1 || vmhost_option(argv[i]) == 1))
        {
            /* already set above */
            continue;
        }
        if (strcmp(argv[i], "-I") == 0 && i + 1 < argc)
        {
            put_headers(argv[++i]);
            continue;
        }
        if (!src && strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            /* -o links the program and saves it, the program is not run */
            out = argv[++i];
            dargv[dargc++] = "-c";
            continue;
        }
        dargv[dargc++] = argv[i];
        if (argv[i][0] == '-')
        {
            continue;
        }
        if (!src)
        {
            src = argv[i];
            if (!put_host_file(src))
            {
                fprintf(stderr, "lcc-run: cannot read %s\n", src);
                return 1;
            }
            slash = strrchr(src, '/');
            snprintf(srcdir, sizeof srcdir, "%.*s", slash ? (int)(slash - src) : 1,
                     slash ? src : ".");
            put_headers(srcdir);
        }
        else
        {
            put_host_file(argv[i]);
        }
    }
    if (!src)
    {
        usage(argv[0]);
    }

    fflush(stdout);
    status = vmhost_run(driver->data, (int)driver->size, dargc, dargv);
    if (status == 0 && out)
    {
        return save_program(out);
    }
    return status < 0 ? 2 : status;
}
