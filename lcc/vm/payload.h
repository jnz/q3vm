/* payload.h -- the files packed into lcc-run, see payload.py */

#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <stddef.h>

struct payload_file
{
    const char*          name; /* where lcc-run puts it, e.g. bin/q3asm */
    const unsigned char* data;
    size_t               size;
};

/* terminated by an entry with a NULL name */
extern const struct payload_file payload_files[];

#endif
