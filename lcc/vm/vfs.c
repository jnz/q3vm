/* vfs.c -- the files of the VM. Every file lives in memory under its absolute path.
 * With an overlay, a host directory is behind the memory: the VM's "/" is that
 * directory. A file that is not in memory is read from the host, and with the
 * write mode "disk" a file the VM changed is saved to the host on close and on
 * vfs_flush. Without an overlay the files exist only in memory, as in a browser.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vfs.h"

#define VFS_PATH_MAX 4096

typedef struct vfs_node
{
    char*            path;
    unsigned char*   data;
    size_t           size;
    size_t           cap;
    int              dirty; /* changed since it was last saved to the host */
    struct vfs_node* next;
} vfs_node;

struct vfs_file
{
    vfs_node* node;
    size_t    pos;
    int       writable;
    int       append;
};

static vfs_node* nodes;
static char      cwd[VFS_PATH_MAX] = "/";
static char      overlay[VFS_PATH_MAX]; /* the host directory of "/", as a prefix */
static int       has_overlay;
static int       write_disk;

void vfs_init(const char* dir)
{
    snprintf(cwd, sizeof cwd, "%s", dir);
    has_overlay = 0;
    write_disk = 0;
}

int vfs_option(const char* arg)
{
    if (strcmp(arg, "-overlay") == 0)
    {
        snprintf(overlay, sizeof overlay, "./");
        has_overlay = 1;
        return 1;
    }
    if (strncmp(arg, "-overlay=", 9) == 0)
    {
        snprintf(overlay, sizeof overlay, "%s", arg + 9);
        has_overlay = 1;
        return 1;
    }
    if (strcmp(arg, "-overlay-write=memory") == 0)
    {
        write_disk = 0;
        return 1;
    }
    if (strcmp(arg, "-overlay-write=disk") == 0)
    {
        write_disk = 1;
        return 1;
    }
    if (strncmp(arg, "-overlay", 8) == 0)
    {
        return -1;
    }
    return 0;
}

const char* vfs_check(void)
{
    if (write_disk && !has_overlay)
    {
        return "-overlay-write=disk needs -overlay=DIR";
    }
    return NULL;
}

/* the host path of an absolute path of the VM: the overlay directory, then the path
 * without its leading slash, so that "./" and "dir" both give "./x" and "dir/x" */
static int host_path(const char* path, char* out)
{
    size_t len = strlen(overlay);
    int    n;

    if (len > 0 && overlay[len - 1] == '/')
    {
        n = snprintf(out, VFS_PATH_MAX, "%s%s", overlay, path + 1);
    }
    else
    {
        n = snprintf(out, VFS_PATH_MAX, "%s%s", overlay, path);
    }
    return n < 0 || n >= VFS_PATH_MAX ? -1 : 0;
}

/* the absolute path of name, with "." and ".." removed; -1 when it is too long */
static int normalize(const char* name, char* out)
{
    char        full[VFS_PATH_MAX];
    const char* p;
    const char* start;

    if (name[0] == '/')
    {
        snprintf(full, sizeof full, "%s", name);
    }
    else
    {
        snprintf(full, sizeof full, "%s/%s", cwd, name);
    }
    out[0] = '\0';
    for (p = full; *p;)
    {
        while (*p == '/')
        {
            p++;
        }
        start = p;
        while (*p && *p != '/')
        {
            p++;
        }
        if (p == start)
        {
            break;
        }
        if (p - start == 1 && start[0] == '.')
        {
            continue;
        }
        if (p - start == 2 && start[0] == '.' && start[1] == '.')
        {
            char* slash = strrchr(out, '/');

            if (slash)
            {
                *slash = '\0';
            }
            continue;
        }
        if (strlen(out) + 1 + (size_t)(p - start) >= VFS_PATH_MAX)
        {
            return -1;
        }
        strcat(out, "/");
        strncat(out, start, p - start);
    }
    if (out[0] == '\0')
    {
        strcpy(out, "/");
    }
    return 0;
}

static vfs_node* find(const char* path)
{
    vfs_node* n;

    for (n = nodes; n; n = n->next)
    {
        if (strcmp(n->path, path) == 0)
        {
            return n;
        }
    }
    return NULL;
}

static vfs_node* add(const char* path)
{
    vfs_node* n = calloc(1, sizeof(vfs_node));

    if (!n)
    {
        return NULL;
    }
    n->path = strdup(path);
    if (!n->path)
    {
        free(n);
        return NULL;
    }
    n->next = nodes;
    nodes = n;
    return n;
}

static void unlink_node(vfs_node* n)
{
    vfs_node** p;

    for (p = &nodes; *p; p = &(*p)->next)
    {
        if (*p == n)
        {
            *p = n->next;
            break;
        }
    }
    free(n->data);
    free(n->path);
    free(n);
}

/* makes room for size bytes in the node */
static int reserve(vfs_node* n, size_t size)
{
    size_t       cap;
    unsigned char* data;

    if (size <= n->cap)
    {
        return 0;
    }
    cap = n->cap ? n->cap * 2 : 256;
    if (cap < size)
    {
        cap = size;
    }
    data = realloc(n->data, cap);
    if (!data)
    {
        return -1;
    }
    n->data = data;
    n->cap = cap;
    return 0;
}

/* reads the host file into memory, NULL when the host has no such file */
static vfs_node* load(const char* path)
{
    char      host[VFS_PATH_MAX];
    FILE*     f;
    long      size;
    vfs_node* n;
    size_t    got;

    if (host_path(path, host) != 0)
    {
        return NULL;
    }
    f = fopen(host, "rb");
    if (!f)
    {
        return NULL;
    }
    size = fseek(f, 0, SEEK_END) == 0 ? ftell(f) : -1;
    if (size < 0 || fseek(f, 0, SEEK_SET) != 0)
    {
        /* not a regular file, a directory for example */
        fclose(f);
        return NULL;
    }
    n = add(path);
    if (!n || reserve(n, (size_t)size) != 0)
    {
        fclose(f);
        if (n)
        {
            unlink_node(n);
        }
        return NULL;
    }
    got = fread(n->data, 1, (size_t)size, f);
    fclose(f);
    n->size = got;
    return n;
}

/* saves the node to the host */
static int save(vfs_node* n)
{
    char   host[VFS_PATH_MAX];
    FILE*  f;
    size_t put;

    if (host_path(n->path, host) != 0)
    {
        return -1;
    }
    f = fopen(host, "wb");
    if (!f)
    {
        return -1;
    }
    put = n->size ? fwrite(n->data, 1, n->size, f) : 0;
    fclose(f);
    if (put != n->size)
    {
        return -1;
    }
    n->dirty = 0;
    return 0;
}

vfs_file_t* vfs_open(const char* name, const char* mode)
{
    char        path[VFS_PATH_MAX];
    vfs_node*   n;
    vfs_file_t* f;
    int         writable = mode[0] == 'w' || mode[0] == 'a';

    if (normalize(name, path) != 0)
    {
        return NULL;
    }
    n = find(path);
    if (mode[0] == 'w')
    {
        if (!n)
        {
            n = add(path);
        }
        if (!n)
        {
            return NULL;
        }
        n->size = 0;
        n->dirty = 1;
    }
    else if (!n && has_overlay)
    {
        n = load(path);
    }
    if (!n)
    {
        return NULL;
    }
    f = calloc(1, sizeof(vfs_file_t));
    if (!f)
    {
        return NULL;
    }
    f->node = n;
    f->writable = writable;
    f->append = mode[0] == 'a';
    f->pos = f->append ? n->size : 0;
    return f;
}

size_t vfs_read(vfs_file_t* f, void* buf, size_t n)
{
    size_t avail;

    if (f->pos >= f->node->size)
    {
        return 0;
    }
    avail = f->node->size - f->pos;
    if (n > avail)
    {
        n = avail;
    }
    memcpy(buf, f->node->data + f->pos, n);
    f->pos += n;
    return n;
}

size_t vfs_write(vfs_file_t* f, const void* buf, size_t n)
{
    vfs_node* node = f->node;

    if (!f->writable)
    {
        return 0;
    }
    if (f->append)
    {
        f->pos = node->size;
    }
    if (reserve(node, f->pos + n) != 0)
    {
        return 0;
    }
    if (f->pos > node->size)
    {
        memset(node->data + node->size, 0, f->pos - node->size);
    }
    memcpy(node->data + f->pos, buf, n);
    f->pos += n;
    if (f->pos > node->size)
    {
        node->size = f->pos;
    }
    node->dirty = 1;
    return n;
}

int vfs_seek(vfs_file_t* f, long off, int whence)
{
    long base;

    if (whence == SEEK_SET)
    {
        base = 0;
    }
    else if (whence == SEEK_CUR)
    {
        base = (long)f->pos;
    }
    else
    {
        base = (long)f->node->size;
    }
    if (base + off < 0)
    {
        return -1;
    }
    f->pos = (size_t)(base + off);
    return 0;
}

long vfs_tell(vfs_file_t* f)
{
    return (long)f->pos;
}

int vfs_close(vfs_file_t* f)
{
    int result = 0;

    if (f->writable && f->node->dirty && write_disk)
    {
        result = save(f->node);
    }
    free(f);
    return result;
}

int vfs_remove(const char* name)
{
    char      path[VFS_PATH_MAX];
    char      host[VFS_PATH_MAX];
    vfs_node* n;
    int       removed = 0;

    if (normalize(name, path) != 0)
    {
        return -1;
    }
    n = find(path);
    if (n)
    {
        unlink_node(n);
        removed = 1;
    }
    if (write_disk && host_path(path, host) == 0 && remove(host) == 0)
    {
        removed = 1;
    }
    return removed ? 0 : -1;
}

void vfs_put(const char* name, const void* data, size_t n)
{
    char      path[VFS_PATH_MAX];
    vfs_node* node;

    if (normalize(name, path) != 0)
    {
        return;
    }
    node = find(path);
    if (!node)
    {
        node = add(path);
    }
    if (!node || reserve(node, n) != 0)
    {
        return;
    }
    memcpy(node->data, data, n);
    node->size = n;
    /* a file from the host is the same as its host copy: only the VM's writes are
     * saved back to the host */
    node->dirty = 0;
}

long vfs_size(const char* name)
{
    char      path[VFS_PATH_MAX];
    vfs_node* n;

    if (normalize(name, path) != 0)
    {
        return -1;
    }
    n = find(path);
    return n ? (long)n->size : -1;
}

long vfs_get(const char* name, void* buf, size_t n)
{
    char      path[VFS_PATH_MAX];
    vfs_node* node;

    if (normalize(name, path) != 0)
    {
        return -1;
    }
    node = find(path);
    if (!node)
    {
        return -1;
    }
    memcpy(buf, node->data, node->size < n ? node->size : n);
    return (long)node->size;
}

void vfs_flush(void)
{
    vfs_node* n;

    if (!write_disk)
    {
        return;
    }
    for (n = nodes; n; n = n->next)
    {
        if (n->dirty)
        {
            save(n);
        }
    }
}
