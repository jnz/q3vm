/* vmglue.c -- what sqlite needs from its operating system layer when it is built with
 * SQLITE_OS_OTHER, for the VM.
 *
 * sqlite's memdb VFS (the "memory file system", used by ":memory:" and by
 * "file:NAME?vfs=memdb") sits on top of the default VFS, so a default VFS must be
 * registered. This one has no files: every open fails, so only memdb databases can be
 * opened. It gives the time and random bytes that sqlite asks for. */

#include <string.h>
#include <time.h>
#include "sqlite3.h"

static int vm_open(sqlite3_vfs* vfs, sqlite3_filename name, sqlite3_file* file, int flags, int* outFlags)
{
    (void)vfs;
    (void)name;
    (void)file;
    (void)flags;
    (void)outFlags;
    return SQLITE_CANTOPEN;
}

static int vm_delete(sqlite3_vfs* vfs, const char* name, int syncDir)
{
    (void)vfs;
    (void)name;
    (void)syncDir;
    return SQLITE_IOERR_DELETE_NOENT;
}

static int vm_access(sqlite3_vfs* vfs, const char* name, int flags, int* result)
{
    (void)vfs;
    (void)name;
    (void)flags;
    *result = 0;
    return SQLITE_OK;
}

static int vm_fullpathname(sqlite3_vfs* vfs, const char* name, int nOut, char* out)
{
    (void)vfs;
    strncpy(out, name, nOut - 1);
    out[nOut - 1] = '\0';
    return SQLITE_OK;
}

static int vm_randomness(sqlite3_vfs* vfs, int nByte, char* out)
{
    int i;

    (void)vfs;
    for (i = 0; i < nByte; i++)
    {
        out[i] = (char)(i * 31 + 7);
    }
    return nByte;
}

static int vm_sleep(sqlite3_vfs* vfs, int microseconds)
{
    (void)vfs;
    (void)microseconds;
    return 0;
}

/* the Julian day: 2440587.5 is the Unix epoch, and a day has 86400 seconds */
static int vm_current_time(sqlite3_vfs* vfs, double* now)
{
    (void)vfs;
    *now = 2440587.5 + (double)time(NULL) / 86400.0;
    return SQLITE_OK;
}

static int vm_current_time64(sqlite3_vfs* vfs, sqlite3_int64* now)
{
    (void)vfs;
    *now = (sqlite3_int64)time(NULL) * 1000 + 210866760000000LL;
    return SQLITE_OK;
}

static int vm_last_error(sqlite3_vfs* vfs, int n, char* out)
{
    (void)vfs;
    (void)n;
    (void)out;
    return 0;
}

static sqlite3_vfs vm_vfs;

int sqlite3_os_init(void)
{
    memset(&vm_vfs, 0, sizeof vm_vfs);
    vm_vfs.iVersion = 3;
    vm_vfs.szOsFile = sizeof(sqlite3_file);
    vm_vfs.mxPathname = 512;
    vm_vfs.zName = "vm";
    vm_vfs.xOpen = vm_open;
    vm_vfs.xDelete = vm_delete;
    vm_vfs.xAccess = vm_access;
    vm_vfs.xFullPathname = vm_fullpathname;
    vm_vfs.xRandomness = vm_randomness;
    vm_vfs.xSleep = vm_sleep;
    vm_vfs.xCurrentTime = vm_current_time;
    vm_vfs.xGetLastError = vm_last_error;
    vm_vfs.xCurrentTimeInt64 = vm_current_time64;
    return sqlite3_vfs_register(&vm_vfs, 1);
}

int sqlite3_os_end(void)
{
    return SQLITE_OK;
}

int sqlite3_enable_load_extension(sqlite3* db, int onoff)
{
    (void)db;
    (void)onoff;
    return SQLITE_ERROR;
}

int sqlite3_load_extension(sqlite3* db, const char* file, const char* proc, char** err)
{
    (void)db;
    (void)file;
    (void)proc;
    (void)err;
    return SQLITE_ERROR;
}
