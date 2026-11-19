/* lcc-run.c -- one executable that compiles a C program with the compiler running
 * in q3vm, links it with the VM runtime and runs it in q3vm.
 *
 * Usage: lcc-run [-S] [-I dir]... file.c [arguments...]
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

static void usage(const char* name)
{
    fprintf(stderr, "usage: %s [-S] [-I dir]... file.c [arguments...]\n", name);
    exit(2);
}

int main(int argc, char** argv)
{
    const struct payload_file* driver;
    char*                      dargv[256];
    char                       srcdir[4096];
    const char*                src = NULL;
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
    vfs_init("/", 0);
    put_payload();

    /* the arguments for the driver: -I is done here, the rest goes on. The first
     * argument that is not an option is the source file. */
    dargv[dargc++] = argv[0];
    for (i = 1; i < argc && dargc < 256; i++)
    {
        if (strcmp(argv[i], "-I") == 0 && i + 1 < argc)
        {
            put_headers(argv[++i]);
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
    return status < 0 ? 2 : status;
}
