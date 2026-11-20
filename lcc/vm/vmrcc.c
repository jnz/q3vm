/* vmrcc.c -- host runner for a .qvm image: runs main() of the image in q3vm.
 *
 * Usage: vmrcc [-overlay[=DIR]] [-overlay-write=memory|disk] image.qvm [arguments...]
 *
 * The files of the VM are in memory, and the overlay is a host directory behind them
 * (see vfs.h). By default it is the current directory, and the files the VM writes are
 * saved to it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vfs.h"
#include "vmhost.h"

static uint8_t* readFile(const char* path, int* size)
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

int main(int argc, char** argv)
{
    uint8_t* image;
    int      size = 0;
    int      status;
    int      first = 1;

    /* the VM's files are the host's files in the current directory, written back to
     * it, unless the options say otherwise (see vfs.h) */
    vfs_init("/");
    vfs_option("-overlay");
    vfs_option("-overlay-write=disk");
    while (first < argc && argv[first][0] == '-')
    {
        int option = vfs_option(argv[first]);

        if (option == 0)
        {
            break;
        }
        if (option < 0)
        {
            fprintf(stderr, "%s: bad option %s\n", argv[0], argv[first]);
            return 2;
        }
        first++;
    }
    if (first >= argc || vfs_check())
    {
        fprintf(stderr, "usage: %s [-overlay[=DIR]] [-overlay-write=memory|disk] image.qvm [arguments]\n",
                argv[0]);
        if (vfs_check())
        {
            fprintf(stderr, "%s: %s\n", argv[0], vfs_check());
        }
        return 2;
    }
    image = readFile(argv[first], &size);
    if (!image)
    {
        fprintf(stderr, "cannot read %s\n", argv[first]);
        return 2;
    }
    /* the image's argv[0] is the name of the image, not of this runner */
    status = vmhost_run(image, size, argc - first, argv + first);
    free(image);
    return status < 0 ? 2 : status;
}
