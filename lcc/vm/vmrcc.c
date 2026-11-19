/* vmrcc.c -- host runner for a .qvm image: runs main() of the image in q3vm.
 *
 * Usage: vmrcc image.qvm [arguments...]
 */

#include <stdio.h>
#include <stdlib.h>

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

    if (argc < 2)
    {
        fprintf(stderr, "usage: %s image.qvm [arguments]\n", argv[0]);
        return 2;
    }
    image = readFile(argv[1], &size);
    if (!image)
    {
        fprintf(stderr, "cannot read %s\n", argv[1]);
        return 2;
    }
    /* the image's argv[0] is the name of the image, not of this runner */
    status = vmhost_run(image, size, argc - 1, argv + 1);
    free(image);
    return status < 0 ? 2 : status;
}
