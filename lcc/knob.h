// Alexey Kutepov: This is a complete backward incompatible rewrite of https://github.com/tsoding/nobuild
// because I'm really unhappy with the direction it is going. It's gonna sit in this repo
// until it's matured enough and then I'll probably extract it to its own repo.

// This is a complete backward incompatible overwrite of https://github.com/tsoding/musializer/blob/master/src/nob.h
// because I intend on using it for "serious" programming projects(i.e. gamedev/edutainement)

// Copyright 2023 Alexey Kutepov <reximkut@gmail.com>
// Copyright 2023 Jean-Sébastien Nadeau <mundusnine@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef KNOB_H_
#define KNOB_H_

#define KNOB_ASSERT assert
#define KNOB_REALLOC realloc
#define KNOB_FREE free

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define _BSD_SOURCE
#define _XOPEN_SOURCE 700
#elif defined(__linux__)
// @TODO: The fact that we need to do this to remove warnings is just crazy, i.e. wth linux
// We need to test this with cosmocc
#ifndef __USE_XOPEN2K8
#define __USE_XOPEN2K8
#endif 
#include <string.h>
#undef __USE_XOPEN2K8
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define _XOPEN_SOURCE 700
#endif
#include <errno.h>
#include <ctype.h>

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#    include <direct.h>
#    include <shellapi.h>
typedef HANDLE Knob_Fd;
#else
typedef int Knob_Fd;
#    include <sys/types.h>
#    include <sys/wait.h>
#    include <fcntl.h>
// @TODO: The fact that we need to do this to remove warnings is just crazy, i.e. wth linux
// We need to test this with cosmocc
#define __USE_XOPEN_EXTENDED
#    include <unistd.h>
#undef __USE_XOPEN_EXTENDED
#ifndef __USE_MISC
#define __USE_MISC
#endif
#    include <sys/stat.h>
#endif

#ifdef _WIN32
#    define KNOB_LINE_END "\r\n"
#    define PATH_SEP "\\"
#    define DLL_NAME ".dll"
#else
#    define KNOB_LINE_END "\n"
#    define PATH_SEP "/"
#    define DLL_NAME ".so"
#endif

#ifdef __cplusplus
extern "C"{
#endif

typedef struct {
    Knob_Fd read;
    Knob_Fd write;
} Knob_Pipe;

Knob_Pipe knob_pipe_make(void);

#define KNOB_ARRAY_LEN(array) (sizeof(array)/sizeof(array[0]))
#define KNOB_ARRAY_GET(array, index) \
    (KNOB_ASSERT(index >= 0), KNOB_ASSERT(index < KNOB_ARRAY_LEN(array)), array[index])

typedef enum {
    KNOB_INFO,
    KNOB_WARNING,
    KNOB_ERROR,
} Knob_Log_Level;

void knob_log(Knob_Log_Level level, const char *fmt, ...);

// It is an equivalent of shift command from bash. It basically pops a command line
// argument from the beginning.
char *knob_shift_args(int *argc, char ***argv);

int knob_cstr_match(char* left, char* right);
int knob_cstr_ends(char* str, const char* end);

typedef struct {
    const char **items;
    size_t count;
    size_t capacity;
} Knob_File_Paths;

typedef enum {
    KNOB_FILE_REGULAR = 0,
    KNOB_FILE_DIRECTORY,
    KNOB_FILE_SYMLINK,
    KNOB_FILE_OTHER,
} Knob_File_Type;

bool knob_mkdir_if_not_exists(const char *path);
bool knob_copy_file(const char *src_path, const char *dst_path);
bool knob_copy_directory_recursively(const char *src_path, const char *dst_path);
bool knob_read_entire_dir(const char *parent, Knob_File_Paths *children);
bool knob_write_entire_file(const char *path, void *data, size_t size);
Knob_File_Type knob_get_file_type(const char *path);

#define knob_return_defer(value) do { result = (value); goto defer; } while(0)

// Initial capacity of a dynamic array
#define KNOB_DA_INIT_CAP 256

// Append an item to a dynamic array
#define knob_da_append(da, item)                                                          \
    do {                                                                                 \
        if ((da)->count >= (da)->capacity) {                                             \
            (da)->capacity = (da)->capacity == 0 ? KNOB_DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = KNOB_REALLOC((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            KNOB_ASSERT((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                                \
                                                                                         \
        (da)->items[(da)->count++] = (item);                                             \
    } while (0)

#define knob_da_free(da) KNOB_FREE((da).items)

// Append several items to a dynamic array
#define knob_da_append_many(da, new_items, new_items_count)                                  \
    do {                                                                                    \
        if ((da)->count + new_items_count > (da)->capacity) {                               \
            if ((da)->capacity == 0) {                                                      \
                (da)->capacity = KNOB_DA_INIT_CAP;                                           \
            }                                                                               \
            while ((da)->count + new_items_count > (da)->capacity) {                        \
                (da)->capacity *= 2;                                                        \
            }                                                                               \
            (da)->items = KNOB_REALLOC((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            KNOB_ASSERT((da)->items != NULL && "Buy more RAM lol");                          \
        }                                                                                   \
        memcpy((da)->items + (da)->count, new_items, new_items_count*sizeof(*(da)->items)); \
        (da)->count += new_items_count;                                                     \
    } while (0)

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} Knob_String_Builder;

bool knob_read_entire_file(const char *path, Knob_String_Builder *sb);

// Append a sized buffer to a string builder
#define knob_sb_append_buf(sb, buf, size) knob_da_append_many(sb, buf, size)

// Append a NULL-terminated string to a string builder
#define knob_sb_append_cstr(sb, cstr)  \
    do {                              \
        const char *s = (cstr);       \
        size_t n = strlen(s);         \
        knob_da_append_many(sb, s, n); \
    } while (0)

// Append a single NULL character at the end of a string builder. So then you can
// use it a NULL-terminated C string
#define knob_sb_append_null(sb) knob_da_append_many(sb, "", 1)

// Free the memory allocated by a string builder
#define knob_sb_free(sb) KNOB_FREE((sb).items)

// Process handle
#ifdef _WIN32
typedef HANDLE Knob_Proc;
#define KNOB_INVALID_PROC INVALID_HANDLE_VALUE
#else
typedef int Knob_Proc;
#define KNOB_INVALID_PROC (-1)
#endif // _WIN32

typedef struct {
    Knob_Proc *items;
    size_t count;
    size_t capacity;
} Knob_Procs;

bool knob_procs_wait(Knob_Procs procs);

// Wait until the process has finished
bool knob_proc_wait(Knob_Proc proc);

// A command - the main workhorse of Knob. Knob is all about building commands an running them
typedef struct {
    const char **items;
    size_t count;
    size_t capacity;
} Knob_Cmd;

// Render a string representation of a command into a string builder. Keep in mind the the
// string builder is not NULL-terminated by default. Use knob_sb_append_null if you plan to
// use it as a C string.
void knob_cmd_render(Knob_Cmd cmd, Knob_String_Builder *render);

#define knob_cmd_append(cmd, ...) \
    knob_da_append_many(cmd, ((const char*[]){__VA_ARGS__}), (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))
#define knob_da_mult_append(da, ...) \
    knob_da_append_many(da, ((const char*[]){__VA_ARGS__}), (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))

// Free all the memory allocated by command arguments
#define knob_cmd_free(cmd) KNOB_FREE(cmd.items)

// Run command asynchronously
Knob_Proc knob_cmd_run_async(Knob_Cmd cmd, Knob_Fd *fdin, Knob_Fd *fdout);

// Run command synchronously
bool knob_cmd_run_sync(Knob_Cmd cmd);

#ifndef KNOB_TEMP_CAPACITY
#define KNOB_TEMP_CAPACITY (8*1024*1024)
#endif // KNOB_TEMP_CAPACITY
char *knob_temp_strdup(const char *cstr);
void *knob_temp_alloc(size_t size);
char *knob_temp_sprintf(const char *format, ...);
void knob_temp_reset(void);
size_t knob_temp_save(void);
void knob_temp_rewind(size_t checkpoint);

int is_path1_modified_after_path2(const char *path1, const char *path2);
bool knob_rename(const char *old_path, const char *new_path);
int knob_needs_rebuild(const char *output_path, const char **input_paths, size_t input_paths_count);
int knob_needs_rebuild1(const char *output_path, const char *input_path);
int knob_file_exists(const char *file_path);
int knob_file_del(const char *file_path);
int knob_path_is_dir(const char *path);

typedef enum {
    TARGET_LINUX,
    TARGET_LINUX_MUSL,
    TARGET_WIN64_MINGW,
    TARGET_WIN64_MSVC,
    TARGET_MACOS,

    COUNT_TARGETS
} Knob_Target;

static_assert(5 == COUNT_TARGETS, "Amount of targets have changed");
static const char *target_names[] = {
    [TARGET_LINUX]       = "linux",
    [TARGET_LINUX_MUSL]  = "linux-musl",
    [TARGET_WIN64_MINGW] = "win64-mingw",
    [TARGET_WIN64_MSVC]  = "win64-msvc",
    [TARGET_MACOS]       = "macos",
};


typedef enum {
    COMPILER_COSMOCC,
    COMPILER_ZIG,
    COMPILER_CLANG,
    COMPILER_GCC,
    COMPILER_CL,
    COUNT_COMPILERS
} Knob_Compiler;

static_assert(5 == COUNT_COMPILERS, "Amount of compilers have changed");
static const char *compiler_names[][2] = {
    [COMPILER_COSMOCC]  = {"cosmocc","-L."},
    [COMPILER_ZIG]      = {"zig","cc"},
    [COMPILER_CLANG]    = {"clang","-L/dev/null"},
    [COMPILER_GCC]      = {"gcc","-L/dev/null"},
    [COMPILER_CL]       = {"cl.exe","-L."},
};
static const char *compilerpp_names[][2] = {
    [COMPILER_COSMOCC]  = {"cosmoc++","-L."},
    [COMPILER_ZIG]      = {"zig","c++"},
    [COMPILER_CLANG]    = {"clang++","-L/dev/null"},
    [COMPILER_GCC]      = {"g++","-L/dev/null"},
    [COMPILER_CL]       = {"cl.exe","-L."},
};
#define GET_COMPILER_NAME(cmp) compiler_names[cmp][0] ,compiler_names[cmp][1]
#define GET_COMPILERPP_NAME(cmp) compilerpp_names[cmp][0] ,compilerpp_names[cmp][1]


#define CONFIG_PATH "./build/config.h"
#define PLUG_PATH "./build/plug.h"

typedef enum {
    BIN_O,
    BIN_DLL,
    BIN_EXE
} Knob_Binary_out;

typedef struct {
    Knob_Target target;
    Knob_Compiler compiler;
    int debug_mode;
    const char* build_to;
    Knob_Binary_out output_type;
    Knob_File_Paths c_files;
    Knob_File_Paths cpp_files;
    Knob_File_Paths includes;
    Knob_File_Paths defines;
    Knob_File_Paths c_flags;
    Knob_File_Paths cpp_flags;
} Knob_Config;

void knob_create_default_config(const char* project_name,Knob_String_Builder *content,Knob_Cmd* user_conf);

/*
* Initialize build config
*/
void knob_config_init(Knob_Config *config);
/*
* Add a define to the config. It's the users responsibility to add `-D` to the define string
*/
void knob_config_add_define(Knob_Config* config,const char* define);
void knob_config_add_c_flag(Knob_Config* config,const char* flag);
void knob_config_add_cpp_flag(Knob_Config* config,const char* flag);
/*
* Add a define to the config. The implementation adds `-I`.
*/
void knob_config_add_includes(Knob_Config* config,const char* filepaths[],size_t len);
/*
* Add files with the extensions `*.c` and `*.cpp` to the build config.
*/
void knob_config_add_files(Knob_Config* config,const char* filepaths[],size_t len);
int knob_config_build(Knob_Config* config,Knob_File_Paths* outs,int debug);
void knob_create_plug(void);
//@TODO: Add reload_plug_here
void knob_cmd_add_compiler(Knob_Cmd* cmd,Knob_Config* config);
void knob_cmd_add_includes(Knob_Cmd* cmd,Knob_Config* config);
int knob_compile_run_submodule(const char* path,Knob_Config* config,Knob_File_Paths* files_to_link,Knob_Cmd* cmd_to_pass,const char* path_to_knobh);
// int knob_get_submodule()

#ifndef CC
#  define CC_PATH ""
#  if _WIN32
#    if defined(__GNUC__)
#       define CC "gcc"
#    elif defined(__clang__)
#       define CC "clang"
#    elif defined(_MSC_VER)
#       define CC "cl.exe"
#    endif
#  else
#   define CC "cc"
#  endif
#endif
// TODO: add MinGW support for Go Rebuild Urself™ Technology
#ifndef KNOB_REBUILD_URSELF
#define KNOB_REBUILD_URSELF(binary_path, source_path) CC_PATH CC, "-o", binary_path, source_path
#endif
typedef int (*submodule_entrypoint)(Knob_Config* /*parent_config*/, Knob_File_Paths* /*project_name##_link_files*/, int /*argc*/,char** /*argv*/);
#ifdef KNOB_SUBMODULE
#define MAIN(project_name) \
 static char* proj_name = #project_name; \
 int project_name##_entrypoint(Knob_Config* parent_config, Knob_File_Paths* project_name##_link_files, int argc,char** argv)
#else
#define MAIN(project_name) int main(int argc,char** argv)
#endif

// Go Rebuild Urself™ Technology
//
//   How to use it:
//     int main(int argc, char** argv) {
//         GO_REBUILD_URSELF(argc, argv);
//         // actual work
//         return 0;
//     }
//
//   After your added this macro every time you run ./nobuild it will detect
//   that you modified its original source code and will try to rebuild itself
//   before doing any actual work. So you only need to bootstrap your build system
//   once.
//
//   The modification is detected by comparing the last modified times of the executable
//   and its source code. The same way the make utility usually does it.
//
//   The rebuilding is done by using the REBUILD_URSELF macro which you can redefine
//   if you need a special way of bootstraping your build system. (which I personally
//   do not recommend since the whole idea of nobuild is to keep the process of bootstrapping
//   as simple as possible and doing all of the actual work inside of the nobuild)
//
#define KNOB_GO_REBUILD_URSELF(argc, argv)                                                    \
    do {                                                                                     \
        const char *source_path = __FILE__;                                                  \
        assert(argc >= 1);                                                                   \
        const char *binary_path = argv[0];                                                   \
                                                                                             \
        int rebuild_is_needed = knob_needs_rebuild(binary_path, &source_path, 1);             \
        if (rebuild_is_needed < 0) exit(1);                                                  \
        if (rebuild_is_needed) {                                                             \
            Knob_String_Builder sb = {0};                                                     \
            knob_sb_append_cstr(&sb, binary_path);                                            \
            knob_sb_append_cstr(&sb, ".old");                                                 \
            knob_sb_append_null(&sb);                                                         \
                                                                                             \
            if (!knob_rename(binary_path, sb.items)) exit(1);                                 \
            Knob_Cmd rebuild = {0};                                                           \
            knob_cmd_append(&rebuild, KNOB_REBUILD_URSELF(binary_path, source_path));          \
            bool rebuild_succeeded = knob_cmd_run_sync(rebuild);                              \
            knob_cmd_free(rebuild);                                                           \
            if (!rebuild_succeeded) {                                                        \
                knob_rename(sb.items, binary_path);                                           \
                exit(1);                                                                     \
            }                                                                                \
                                                                                             \
            Knob_Cmd cmd = {0};                                                               \
            knob_da_append_many(&cmd, argv, argc);                                            \
            if (!knob_cmd_run_sync(cmd)) exit(1);                                             \
            exit(0);                                                                         \
        }                                                                                    \
    } while(0)
// The implementation idea is stolen from https://github.com/zhiayang/nabs

typedef struct {
    size_t count;
    const char *data;
} Knob_String_View;

const char *knob_temp_sv_to_cstr(Knob_String_View sv);

Knob_String_View knob_sv_chop_by_delim(Knob_String_View *sv, char delim);
Knob_String_View knob_sv_trim(Knob_String_View sv);
bool knob_sv_eq(Knob_String_View a, Knob_String_View b);
Knob_String_View knob_sv_from_cstr(const char *cstr);
Knob_String_View knob_sv_from_parts(const char *data, size_t count);

// printf macros for String_View
#ifndef SV_Fmt
#define SV_Fmt "%.*s"
#endif // SV_Fmt
#ifndef SV_Arg
#define SV_Arg(sv) (int) (sv).count, (sv).data
#endif // SV_Arg
// USAGE:
//   String_View name = ...;
//   printf("Name: "SV_Fmt"\n", SV_Arg(name));


// minirent.h HEADER BEGIN ////////////////////////////////////////
// Copyright 2021 Alexey Kutepov <reximkut@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// ============================================================
//
// minirent — 0.0.1 — A subset of dirent interface for Windows.
//
// https://github.com/tsoding/minirent
//
// ============================================================
//
// ChangeLog (https://semver.org/ is implied)
//
//    0.0.2 Automatically include dirent.h on non-Windows
//          platforms
//    0.0.1 First Official Release

#ifndef _WIN32
#include <dirent.h>
#else // _WIN32

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

struct dirent
{
    char d_name[MAX_PATH+1];
};

typedef struct DIR DIR;

DIR *opendir(const char *dirpath);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
#endif // _WIN32
// minirent.h HEADER END ////////////////////////////////////////

#endif // KNOB_H_

// dynlib.h HEADER BEGIN ////////////////////////////////////////
// Copyright 2023 Jean-Sébastien Nadeau <mundusnine@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// ============================================================
/*
* Single-header library for dynamic library loading
* Define in only one .c or .cpp file DYNLIB_IMPL
*/

static char dynlib_last_err[256] = {0};
typedef void (*func_ptr)(void);

void* dynlib_load(const char *dllfile);
int dynlib_unload(void *handle);
void* dynlib_loadfunc(void *handle, const char *name);

// dynlib.h HEADER END ////////////////////////////////////////

int knob_sleep_ms(int milliseconds);

#ifdef __cplusplus
}
#endif

#if defined(KNOB_IMPLEMENTATION)

static size_t knob_temp_size = 0;
static char knob_temp[KNOB_TEMP_CAPACITY] = {0};

bool knob_mkdir_if_not_exists(const char *path)
{
#ifdef _WIN32
    int result = mkdir(path);
#else
    int result = mkdir(path, 0755);
#endif
    if (result < 0) {
        if (errno == EEXIST) {
            knob_log(KNOB_INFO, "directory `%s` already exists", path);
            return true;
        }
        knob_log(KNOB_ERROR, "could not create directory `%s`: %s", path, strerror(errno));
        return false;
    }

    knob_log(KNOB_INFO, "created directory `%s`", path);
    return true;
}

bool knob_copy_file(const char *src_path, const char *dst_path)
{
    knob_log(KNOB_INFO, "copying %s -> %s", src_path, dst_path);
#ifdef _WIN32
    if (!CopyFile(src_path, dst_path, FALSE)) {
        knob_log(KNOB_ERROR, "Could not copy file: %lu", GetLastError());
        return false;
    }
    return true;
#else
    int src_fd = -1;
    int dst_fd = -1;
    size_t buf_size = 32*1024;
    char *buf = KNOB_REALLOC(NULL, buf_size);
    KNOB_ASSERT(buf != NULL && "Buy more RAM lol!!");
    bool result = true;

    src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) {
        knob_log(KNOB_ERROR, "Could not open file %s: %s", src_path, strerror(errno));
        knob_return_defer(false);
    }

    struct stat src_stat;
    if (fstat(src_fd, &src_stat) < 0) {
        knob_log(KNOB_ERROR, "Could not get mode of file %s: %s", src_path, strerror(errno));
        knob_return_defer(false);
    }

    dst_fd = open(dst_path, O_CREAT | O_TRUNC | O_WRONLY, src_stat.st_mode);
    if (dst_fd < 0) {
        knob_log(KNOB_ERROR, "Could not create file %s: %s", dst_path, strerror(errno));
        knob_return_defer(false);
    }

    for (;;) {
        ssize_t n = read(src_fd, buf, buf_size);
        if (n == 0) break;
        if (n < 0) {
            knob_log(KNOB_ERROR, "Could not read from file %s: %s", src_path, strerror(errno));
            knob_return_defer(false);
        }
        char *buf2 = buf;
        while (n > 0) {
            ssize_t m = write(dst_fd, buf2, n);
            if (m < 0) {
                knob_log(KNOB_ERROR, "Could not write to file %s: %s", dst_path, strerror(errno));
                knob_return_defer(false);
            }
            n    -= m;
            buf2 += m;
        }
    }

defer:
    free(buf);
    close(src_fd);
    close(dst_fd);
    return result;
#endif
}

void knob_cmd_render(Knob_Cmd cmd, Knob_String_Builder *render)
{
    for (size_t i = 0; i < cmd.count; ++i) {
        const char *arg = cmd.items[i];
        if (arg == NULL) break;
        if (i > 0) knob_sb_append_cstr(render, " ");
        if (!strchr(arg, ' ')) {
            knob_sb_append_cstr(render, arg);
        } else {
            knob_da_append(render, '\'');
            knob_sb_append_cstr(render, arg);
            knob_da_append(render, '\'');
        }
    }
}

Knob_Proc knob_cmd_run_async(Knob_Cmd cmd, Knob_Fd *fdin, Knob_Fd *fdout)
{
#ifdef _WIN32
    // https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    // NOTE: theoretically setting NULL to std handles should not be a problem
    // https://docs.microsoft.com/en-us/windows/console/getstdhandle?redirectedfrom=MSDN#attachdetach-behavior
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    // TODO(#32): check for errors in GetStdHandle
    siStartInfo.hStdOutput = fdout ? *fdout : GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = fdin ? *fdin : GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    BOOL bSuccess =
        CreateProcess(
            NULL,
            // TODO(#33): cmd_run_async on Windows does not render command line properly
            // It may require wrapping some arguments with double-quotes if they contains spaces, etc.
            cstr_array_join(" ", cmd.line),
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &siStartInfo,
            &piProcInfo
        );

    if (!bSuccess) {
        knob_log(KNOB_ERROR,"Could not create child process %s: %s\n",
              cmd_show(cmd), GetLastErrorAsString());
    }

    CloseHandle(piProcInfo.hThread);

    return piProcInfo.hProcess;
#else
    pid_t cpid = fork();
    if (cpid < 0) {
        knob_log(KNOB_ERROR,"Could not fork child process: %s",strerror(errno));
    }

    if (cpid == 0) {
        // NOTE: This leaks a bit of memory in the child process.
        // But do we actually care? It's a one off leak anyway...
        Knob_Cmd cmd_null = {0};
        knob_da_append_many(&cmd_null, cmd.items, cmd.count);
        knob_cmd_append(&cmd_null, NULL);

        if (fdin) {
            if (dup2(*fdin, STDIN_FILENO) < 0) {
                knob_log(KNOB_ERROR,"Could not setup stdin for child process: %s", strerror(errno));
            }
            close(*fdin);
        }

        if (fdout) {
            if (dup2(*fdout, STDOUT_FILENO) < 0) {
                knob_log(KNOB_ERROR,"Could not setup stdout for child process: %s", strerror(errno));
            }
            close(*fdout);
        }

        if (execvp(cmd.items[0], (char * const*) cmd_null.items) < 0) {
            knob_log(KNOB_ERROR, "Could not exec child process: %s", strerror(errno));
            exit(1);
        }
    }

    return cpid;
#endif // _WIN32
}

bool knob_procs_wait(Knob_Procs procs)
{
    bool success = true;
    for (size_t i = 0; i < procs.count; ++i) {
        success = knob_proc_wait(procs.items[i]) && success;
    }
    return success;
}

bool knob_proc_wait(Knob_Proc proc)
{
    if (proc == KNOB_INVALID_PROC) return false;

#ifdef _WIN32
    DWORD result = WaitForSingleObject(
                       proc,    // HANDLE hHandle,
                       INFINITE // DWORD  dwMilliseconds
                   );

    if (result == WAIT_FAILED) {
        knob_log(KNOB_ERROR, "could not wait on child process: %lu", GetLastError());
        return false;
    }

    DWORD exit_status;
    if (!GetExitCodeProcess(proc, &exit_status)) {
        knob_log(KNOB_ERROR, "could not get process exit code: %lu", GetLastError());
        return false;
    }

    if (exit_status != 0) {
        knob_log(KNOB_ERROR, "command exited with exit code %lu", exit_status);
        return false;
    }

    CloseHandle(proc);

    return true;
#else
    for (;;) {
        int wstatus = 0;
        if (waitpid(proc, &wstatus, 0) < 0) {
            knob_log(KNOB_ERROR, "could not wait on command (pid %d): %s", proc, strerror(errno));
            return false;
        }

        if (WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);
            if (exit_status != 0) {
                knob_log(KNOB_ERROR, "command exited with exit code %d", exit_status);
                return false;
            }

            break;
        }

        if (WIFSIGNALED(wstatus)) {
            knob_log(KNOB_ERROR, "command process was terminated by %s", strsignal(WTERMSIG(wstatus)));
            return false;
        }
    }

    return true;
#endif
}

bool knob_cmd_run_sync(Knob_Cmd cmd)
{
    Knob_Proc p = knob_cmd_run_async(cmd,NULL,NULL);
    if (p == KNOB_INVALID_PROC) return false;
    return knob_proc_wait(p);
}

char *knob_shift_args(int *argc, char ***argv)
{
    KNOB_ASSERT(*argc > 0);
    char *result = **argv;
    (*argv) += 1;
    (*argc) -= 1;
    return result;
}

int knob_cstr_match(char* left, char* right){
    int i =0;
    while(left[i] != '\0' && right[i] != '\0'){
        if(left[i] != right[i]){
            return 0;
        }
        ++i;
    }
    if(left[i] != '\0' || right[i] != '\0')
        return 0;
    return 1;
}

int knob_cstr_ends(char* str,const char* end){
    int e_len = strlen(end);
    int s_len = strlen(str);
    int i =1;
    while(e_len-i > -1){
        if(str[s_len-i] != end[e_len-i]){
            return 0;
        }
        i++;
    } 
    return 1;
}
Knob_Pipe knob_pipe_make(void)
{
    Knob_Pipe pip = {0};

#ifdef _WIN32
    // https://docs.microsoft.com/en-us/windows/win32/ProcThread/creating-a-child-process-with-redirected-input-and-output

    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    if (!CreatePipe(&pip.read, &pip.write, &saAttr, 0)) {
        knob_log(KNOB_ERROR,"Could not create pipe: %s", GetLastErrorAsString());
    }
#else
    Knob_Fd pipefd[2];
    if (pipe(pipefd) < 0) {
        knob_log(KNOB_ERROR,"Could not create pipe: %s", strerror(errno));
    }

    pip.read = pipefd[0];
    pip.write = pipefd[1];
#endif // _WIN32

    return pip;
}

Knob_Fd knob_fd_open_for_read(const char* path)
{
#ifndef _WIN32
    Knob_Fd result = open(path, O_RDONLY);
    if (result < 0) {
        knob_log(KNOB_ERROR,"Could not open file %s: %s", path, strerror(errno));
    }
    return result;
#else
    // https://docs.microsoft.com/en-us/windows/win32/fileio/opening-a-file-for-reading-or-writing
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    Knob_Fd result = CreateFile(
                    path,
                    GENERIC_READ,
                    0,
                    &saAttr,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_READONLY,
                    NULL);

    if (result == INVALID_HANDLE_VALUE) {
        knob_log(KNOB_ERROR,"Could not open file %s", path);
    }

    return result;
#endif // _WIN32
}

Knob_Fd knob_fd_open_for_write(const char* path)
{
#ifndef _WIN32
    Knob_Fd result = open(path,
                     O_WRONLY | O_CREAT | O_TRUNC,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (result < 0) {
        knob_log(KNOB_ERROR,"could not open file %s: %s", path, strerror(errno));
    }
    return result;
#else
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    Knob_Fd result = CreateFile(
                    path,                  // name of the write
                    GENERIC_WRITE,         // open for writing
                    0,                     // do not share
                    &saAttr,               // default security
                    CREATE_NEW,            // create new file only
                    FILE_ATTRIBUTE_NORMAL, // normal file
                    NULL                   // no attr. template
                );

    if (result == INVALID_HANDLE_VALUE) {
        knob_log(KNOB_ERROR,"Could not open file %s: %s", path, GetLastErrorAsString());
    }

    return result;
#endif // _WIN32
}

void knob_fd_close(Knob_Fd fd)
{
#ifdef _WIN32
    CloseHandle(fd);
#else
    close(fd);
#endif // _WIN32
}

void knob_log(Knob_Log_Level level, const char *fmt, ...)
{
    switch (level) {
    case KNOB_INFO:
        fprintf(stderr, "[INFO] ");
        break;
    case KNOB_WARNING:
        fprintf(stderr, "[WARNING] ");
        break;
    case KNOB_ERROR:
        fprintf(stderr, "[ERROR] ");
        break;
    default:
        KNOB_ASSERT(0 && "unreachable");
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

bool knob_read_entire_dir(const char *parent, Knob_File_Paths *children)
{
    bool result = true;
    DIR *dir = NULL;

    dir = opendir(parent);
    if (dir == NULL) {
        knob_log(KNOB_ERROR, "Could not open directory %s: %s", parent, strerror(errno));
        knob_return_defer(false);
    }

    errno = 0;
    struct dirent *ent = readdir(dir);
    while (ent != NULL) {
        knob_da_append(children, knob_temp_strdup(ent->d_name));
        ent = readdir(dir);
    }

    if (errno != 0) {
        knob_log(KNOB_ERROR, "Could not read directory %s: %s", parent, strerror(errno));
        knob_return_defer(false);
    }

defer:
    if (dir) closedir(dir);
    return result;
}

bool knob_write_entire_file(const char *path, void *data, size_t size)
{
    bool result = true;

    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        knob_log(KNOB_ERROR, "Could not open file %s for writing: %s\n", path, strerror(errno));
        knob_return_defer(false);
    }

    //           len
    //           v
    // aaaaaaaaaa
    //     ^
    //     data

    char *buf = data;
    while (size > 0) {
        size_t n = fwrite(buf, 1, size, f);
        if (ferror(f)) {
            knob_log(KNOB_ERROR, "Could not write into file %s: %s\n", path, strerror(errno));
            knob_return_defer(false);
        }
        size -= n;
        buf  += n;
    }

defer:
    if (f) fclose(f);
    return result;
}

Knob_File_Type knob_get_file_type(const char *path)
{
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        knob_log(KNOB_ERROR, "Could not get file attributes of %s: %lu", path, GetLastError());
        return -1;
    }

    if (attr & FILE_ATTRIBUTE_DIRECTORY) return KNOB_FILE_DIRECTORY;
    // TODO: detect symlinks on Windows (whatever that means on Windows anyway)
    return KNOB_FILE_REGULAR;
#else // _WIN32
    struct stat statbuf;
    if (stat(path, &statbuf) < 0) {
        knob_log(KNOB_ERROR, "Could not get stat of %s: %s", path, strerror(errno));
        return -1;
    }

    switch (statbuf.st_mode & S_IFMT) {
        case S_IFDIR:  return KNOB_FILE_DIRECTORY;
        case S_IFREG:  return KNOB_FILE_REGULAR;
        case S_IFLNK:  return KNOB_FILE_SYMLINK;
        default:       return KNOB_FILE_OTHER;
    }
#endif // _WIN32
}

bool knob_copy_directory_recursively(const char *src_path, const char *dst_path)
{
    bool result = true;
    Knob_File_Paths children = {0};
    Knob_String_Builder src_sb = {0};
    Knob_String_Builder dst_sb = {0};
    size_t temp_checkpoint = knob_temp_save();

    Knob_File_Type type = knob_get_file_type(src_path);
    if (type < 0) return false;

    switch (type) {
        case KNOB_FILE_DIRECTORY: {
            if (!knob_mkdir_if_not_exists(dst_path)) knob_return_defer(false);
            if (!knob_read_entire_dir(src_path, &children)) knob_return_defer(false);

            for (size_t i = 0; i < children.count; ++i) {
                if (strcmp(children.items[i], ".") == 0) continue;
                if (strcmp(children.items[i], "..") == 0) continue;

                src_sb.count = 0;
                knob_sb_append_cstr(&src_sb, src_path);
                knob_sb_append_cstr(&src_sb, "/");
                knob_sb_append_cstr(&src_sb, children.items[i]);
                knob_sb_append_null(&src_sb);

                dst_sb.count = 0;
                knob_sb_append_cstr(&dst_sb, dst_path);
                knob_sb_append_cstr(&dst_sb, "/");
                knob_sb_append_cstr(&dst_sb, children.items[i]);
                knob_sb_append_null(&dst_sb);

                if (!knob_copy_directory_recursively(src_sb.items, dst_sb.items)) {
                    knob_return_defer(false);
                }
            }
        } break;

        case KNOB_FILE_REGULAR: {
            if (!knob_copy_file(src_path, dst_path)) {
                knob_return_defer(false);
            }
        } break;

        case KNOB_FILE_SYMLINK: {
            knob_log(KNOB_WARNING, "TODO: Copying symlinks is not supported yet");
        } break;

        case KNOB_FILE_OTHER: {
            knob_log(KNOB_ERROR, "Unsupported type of file %s", src_path);
            knob_return_defer(false);
        } break;

        default: KNOB_ASSERT(0 && "unreachable");
    }

defer:
    knob_temp_rewind(temp_checkpoint);
    knob_da_free(src_sb);
    knob_da_free(dst_sb);
    knob_da_free(children);
    return result;
}

char *knob_temp_strdup(const char *cstr)
{
    size_t n = strlen(cstr);
    char *result = knob_temp_alloc(n + 1);
    KNOB_ASSERT(result != NULL && "Increase KNOB_TEMP_CAPACITY");
    memcpy(result, cstr, n);
    result[n] = '\0';
    return result;
}

void *knob_temp_alloc(size_t size)
{
    if (knob_temp_size + size > KNOB_TEMP_CAPACITY) return NULL;
    void *result = &knob_temp[knob_temp_size];
    knob_temp_size += size;
    return result;
}

char *knob_temp_sprintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vsnprintf(NULL, 0, format, args);
    va_end(args);

    KNOB_ASSERT(n >= 0);
    char *result = knob_temp_alloc(n + 1);
    KNOB_ASSERT(result != NULL && "Extend the size of the temporary allocator");
    // TODO: use proper arenas for the temporary allocator;
    va_start(args, format);
    vsnprintf(result, n + 1, format, args);
    va_end(args);

    return result;
}

void knob_temp_reset(void)
{
    knob_temp_size = 0;
}

size_t knob_temp_save(void)
{
    return knob_temp_size;
}

void knob_temp_rewind(size_t checkpoint)
{
    knob_temp_size = checkpoint;
}

const char *knob_temp_sv_to_cstr(Knob_String_View sv)
{
    char *result = knob_temp_alloc(sv.count + 1);
    KNOB_ASSERT(result != NULL && "Extend the size of the temporary allocator");
    memcpy(result, sv.data, sv.count);
    result[sv.count] = '\0';
    return result;
}

int knob_needs_rebuild(const char *output_path, const char **input_paths, size_t input_paths_count)
{
#ifdef _WIN32
    BOOL bSuccess;

    HANDLE output_path_fd = CreateFile(output_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (output_path_fd == INVALID_HANDLE_VALUE) {
        // NOTE: if output does not exist it 100% must be rebuilt
        if (GetLastError() == ERROR_FILE_NOT_FOUND) return 1;
        knob_log(KNOB_ERROR, "Could not open file %s: %lu", output_path, GetLastError());
        return -1;
    }
    FILETIME output_path_time;
    bSuccess = GetFileTime(output_path_fd, NULL, NULL, &output_path_time);
    CloseHandle(output_path_fd);
    if (!bSuccess) {
        knob_log(KNOB_ERROR, "Could not get time of %s: %lu", output_path, GetLastError());
        return -1;
    }

    for (size_t i = 0; i < input_paths_count; ++i) {
        const char *input_path = input_paths[i];
        HANDLE input_path_fd = CreateFile(input_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
        if (input_path_fd == INVALID_HANDLE_VALUE) {
            // NOTE: non-existing input is an error cause it is needed for building in the first place
            knob_log(KNOB_ERROR, "Could not open file %s: %lu", input_path, GetLastError());
            return -1;
        }
        FILETIME input_path_time;
        bSuccess = GetFileTime(input_path_fd, NULL, NULL, &input_path_time);
        CloseHandle(input_path_fd);
        if (!bSuccess) {
            knob_log(KNOB_ERROR, "Could not get time of %s: %lu", input_path, GetLastError());
            return -1;
        }

        // NOTE: if even a single input_path is fresher than output_path that's 100% rebuild
        if (CompareFileTime(&input_path_time, &output_path_time) == 1) return 1;
    }

    return 0;
#else
    struct stat statbuf = {0};

    if (stat(output_path, &statbuf) < 0) {
        // NOTE: if output does not exist it 100% must be rebuilt
        if (errno == ENOENT) return 1;
        knob_log(KNOB_ERROR, "could not stat %s: %s", output_path, strerror(errno));
        return -1;
    }
    int output_path_time = statbuf.st_mtime;

    for (size_t i = 0; i < input_paths_count; ++i) {
        const char *input_path = input_paths[i];
        if (stat(input_path, &statbuf) < 0) {
            // NOTE: non-existing input is an error cause it is needed for building in the first place
            knob_log(KNOB_ERROR, "could not stat %s: %s", input_path, strerror(errno));
            return -1;
        }
        int input_path_time = statbuf.st_mtime;
        // NOTE: if even a single input_path is fresher than output_path that's 100% rebuild
        if (input_path_time > output_path_time) return 1;
    }

    return 0;
#endif
}

int knob_needs_rebuild1(const char *output_path, const char *input_path)
{
    return knob_needs_rebuild(output_path, &input_path, 1);
}

bool knob_rename(const char *old_path, const char *new_path)
{
    knob_log(KNOB_INFO, "renaming %s -> %s", old_path, new_path);
#ifdef _WIN32
    if (!MoveFileEx(old_path, new_path, MOVEFILE_REPLACE_EXISTING)) {
        knob_log(KNOB_ERROR, "could not rename %s to %s: %lu", old_path, new_path, GetLastError());
        return false;
    }
#else
    if (rename(old_path, new_path) < 0) {
        knob_log(KNOB_ERROR, "could not rename %s to %s: %s", old_path, new_path, strerror(errno));
        return false;
    }
#endif // _WIN32
    return true;
}

bool knob_read_entire_file(const char *path, Knob_String_Builder *sb)
{
    bool result = true;
    size_t buf_size = 32*1024;
    char *buf = KNOB_REALLOC(NULL, buf_size);
    KNOB_ASSERT(buf != NULL && "Buy more RAM lool!!");
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        knob_log(KNOB_ERROR, "Could not open %s for reading: %s", path, strerror(errno));
        knob_return_defer(false);
    }

    size_t n = fread(buf, 1, buf_size, f);
    while (n > 0) {
        knob_sb_append_buf(sb, buf, n);
        n = fread(buf, 1, buf_size, f);
    }
    if (ferror(f)) {
        knob_log(KNOB_ERROR, "Could not read %s: %s\n", path, strerror(errno));
        knob_return_defer(false);
    }

defer:
    KNOB_FREE(buf);
    if (f) fclose(f);
    return result;
}

Knob_String_View knob_sv_chop_by_delim(Knob_String_View *sv, char delim)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim) {
        i += 1;
    }

    Knob_String_View result = knob_sv_from_parts(sv->data, i);

    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data  += i + 1;
    } else {
        sv->count -= i;
        sv->data  += i;
    }

    return result;
}

Knob_String_View knob_sv_from_parts(const char *data, size_t count)
{
    Knob_String_View sv;
    sv.count = count;
    sv.data = data;
    return sv;
}

Knob_String_View knob_sv_trim_left(Knob_String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[i])) {
        i += 1;
    }

    return knob_sv_from_parts(sv.data + i, sv.count - i);
}

Knob_String_View knob_sv_trim_right(Knob_String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
        i += 1;
    }

    return knob_sv_from_parts(sv.data, sv.count - i);
}

Knob_String_View knob_sv_trim(Knob_String_View sv)
{
    return knob_sv_trim_right(knob_sv_trim_left(sv));
}

Knob_String_View knob_sv_from_cstr(const char *cstr)
{
    return knob_sv_from_parts(cstr, strlen(cstr));
}

bool knob_sv_eq(Knob_String_View a, Knob_String_View b)
{
    if (a.count != b.count) {
        return false;
    } else {
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

// RETURNS:
//  0 - file does not exists
//  1 - file exists
// -1 - error while checking if file exists. The error is logged
int knob_file_exists(const char *file_path)
{
#if _WIN32
    // TODO: distinguish between "does not exists" and other errors
    DWORD dwAttrib = GetFileAttributesA(file_path);
    return dwAttrib != INVALID_FILE_ATTRIBUTES;
#else
    struct stat statbuf;
    if (stat(file_path, &statbuf) < 0) {
        if (errno == ENOENT) return 0;
        knob_log(KNOB_ERROR, "Could not check if file %s exists: %s", file_path, strerror(errno));
        return -1;
    }
    return 1;
#endif
}

// RETURNS:
//  0 - file was not deleted
//  1 - file was deleted
int knob_file_del(const char *file_path){
    #if _WIN32
    assert(0 && "TODO: Not implemented");
    #else
    int res = remove(file_path);
    if(res != 0){
        knob_log(KNOB_ERROR, "Could not delete file %s : %s", file_path, strerror(errno));
    }
    return  res == 0;
    #endif
}

int knob_path_is_dir(const char *path)
{
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributes(path);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat statbuf = {0};
    if (stat(path, &statbuf) < 0) {
        if (errno == ENOENT) {
            errno = 0;
            return 0;
        }

        knob_log(KNOB_ERROR,"could not retrieve information about file %s: %s",
              path, strerror(errno));
    }

    return S_ISDIR(statbuf.st_mode);
#endif // _WIN32
}


void knob_create_default_config(const char* project_name,Knob_String_Builder *content,Knob_Cmd* user_conf)
{
    knob_sb_append_cstr(content, "//// Build target. Pick only one!\n");
#ifdef _WIN32
#   if defined(_MSC_VER)
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_LINUX\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_LINUX_MUSL\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_WIN64_MINGW\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("#define %s_TARGET TARGET_WIN64_MSVC\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_MACOS\n",project_name));
#   else
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_LINUX\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("#define %s_TARGET TARGET_WIN64_MINGW\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_WIN64_MSVC\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_MACOS\n",project_name));
#   endif
#else
#   if defined (__APPLE__) || defined (__MACH__)
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_LINUX\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_LINUX_MUSL\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_WIN64_MINGW\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_WIN64_MSVC\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("#define %s_TARGET TARGET_MACOS\n",project_name));
#   else
    knob_sb_append_cstr(content, knob_temp_sprintf("#define %s_TARGET TARGET_LINUX\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_LINUX_MUSL\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_WIN64_MINGW\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_WIN64_MSVC\n",project_name));
    knob_sb_append_cstr(content, knob_temp_sprintf("// #define %s_TARGET TARGET_MACOS\n",project_name));
#   endif
#endif
    knob_sb_append_cstr(content, "//// Build compiler. Pick only one!\n");
    knob_sb_append_cstr(content,"#ifdef CC\n");
    knob_sb_append_cstr(content,"#undef CC\n");
    knob_sb_append_cstr(content,"#endif\n");
    #ifdef _WIN32
#   if defined(_MSC_VER)
    knob_sb_append_cstr(content, "// #define CC COMPILER_COSMOCC\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_ZIG\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_GCC\n");
    knob_sb_append_cstr(content, "#define CC COMPILER_CL\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_CLANG\n");
#   else
    knob_sb_append_cstr(content, "// #define CC COMPILER_COSMOCC\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_ZIG\n");
    knob_sb_append_cstr(content, "#define CC COMPILER_GCC\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_CL\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_CLANG\n");
#   endif
#else
#   if defined (__APPLE__) || defined (__MACH__)
    knob_sb_append_cstr(content, "// #define CC COMPILER_COSMOCC\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_ZIG\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_GCC\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_CL\n");
    knob_sb_append_cstr(content, "#define CC COMPILER_CLANG\n");
#   else
    knob_sb_append_cstr(content, "// #define CC COMPILER_COSMOCC\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_ZIG\n");
    knob_sb_append_cstr(content, "#define CC COMPILER_GCC\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_CL\n");
    knob_sb_append_cstr(content, "// #define CC COMPILER_CLANG\n");
#   endif
#endif
    knob_sb_append_cstr(content,"//For the crazy people that version their C compiler :)\n");
    knob_sb_append_cstr(content,"#ifdef CC_PATH\n");
    knob_sb_append_cstr(content,"#undef CC_PATH\n");
    knob_sb_append_cstr(content,"#endif\n");
    knob_sb_append_cstr(content,"#define CC_PATH \"\"\n");
    knob_sb_append_cstr(content, "\n");
    knob_sb_append_cstr(content, "//// Moves everything in src/plug.c to a separate \"DLL\" so it can be hotreloaded. Works only for Linux right now\n");
    knob_sb_append_cstr(content, "// #define KNOB_HOTRELOAD\n");
    knob_sb_append_cstr(content, "\n");
    knob_sb_append_cstr(content, "//// PROJECT SPECIFIC CONFIGS.\n");
    for(int i =0; i < user_conf->count; ++i){
        const char* config = user_conf->items[i];
        knob_sb_append_cstr(content,config);
        knob_sb_append_cstr(content, "\n");
    }
}

void knob_config_init(Knob_Config *config)
{
    memset(config, 0, sizeof(Knob_Config));
#ifdef _WIN32
#   if defined(_MSC_VER)
        config->target = TARGET_WIN64_MSVC;
        config->compiler = COMPILER_CL;
#   else
        config->target = TARGET_WIN64_MINGW;
        config->compiler = COMPILER_GCC;
#   endif
#else
#   if defined (__APPLE__) || defined (__MACH__)
        config->target = TARGET_MACOS;
        config->compiler = COMPILER_CLANG;
#   else
        config->target = TARGET_LINUX;
        config->compiler = COMPILER_GCC;
#   endif
#endif
    config->debug_mode = 1;
    config->output_type = BIN_EXE;
    config->build_to = "./build";
    memset(&config->c_files,0,sizeof(Knob_File_Paths));
    memset(&config->cpp_files,0,sizeof(Knob_File_Paths));
    memset(&config->includes,0,sizeof(Knob_File_Paths));
    memset(&config->defines,0,sizeof(Knob_File_Paths));
    memset(&config->c_flags,0,sizeof(Knob_File_Paths));
    memset(&config->cpp_flags,0,sizeof(Knob_File_Paths));
}

void knob_config_add_define(Knob_Config* config,const char* define){
    knob_da_append(&config->defines,define);
}

void knob_config_add_c_flag(Knob_Config* config,const char* flag){
    knob_da_append(&config->c_flags,flag);
}
void knob_config_add_cpp_flag(Knob_Config* config,const char* flag){
    knob_da_append(&config->cpp_flags,flag);
}

void knob_config_add_includes(Knob_Config* config,const char* filepaths[],size_t len){
    for(int i =0; i < len;++i){
        const char* path = filepaths[i]; 
        // if(!knob_path_is_dir((char*)path)){
        //     knob_log(KNOB_ERROR,"Path supplied isn't a directory and can't be added... %s", path);
        //     exit(1);
        // }
        knob_da_append(&config->includes,path);
    }
}
void knob_config_add_files(Knob_Config* config,const char* filepaths[],size_t len){
    //@TODO: Add filters, so weird people that use cxx or whatever are supported
    // This would also enable adding objc .m files
    for(int y =0; y < len;++y){
        char* path = (char*)filepaths[y]; 
        if(knob_path_is_dir(path)){
            Knob_File_Paths childs = {0};
            knob_read_entire_dir(path,&childs);
            for(int i =0 ; i < childs.count;++i){
                char* filename = (char*)childs.items[i];  
                if(knob_cstr_match(filename,"..") || knob_cstr_match(filename,".")) continue;
                const char* fname = knob_temp_sprintf("%s"PATH_SEP"%s",path,filename);
                knob_config_add_files(config,(const char*[]){
                    fname
                }, 1);
            }
            knob_da_free(childs);
        }
        else {
            if(knob_cstr_ends(path,".cpp")){
                knob_da_append(&config->cpp_files,path);
            }
            else if(knob_cstr_ends(path,".c")){
                knob_da_append(&config->c_files,path);
            }
        }
    }
}
int knob_config_build(Knob_Config* config,Knob_File_Paths* outs,int debug){
    bool result = true;
    Knob_Cmd cmd = {0};
    Knob_Procs procs = {0};

    const char* filepath = NULL;
    const char* config_file = knob_temp_sprintf("%s"PATH_SEP"config.h",config->build_to);
    
    for(int i =0; i < config->cpp_files.count;++i){
        cmd.count = 0;
        filepath = config->cpp_files.items[i];
        Knob_String_View sv = {
            .data = filepath,
            .count = strlen(filepath)
        };
        size_t last_count =0;
        while(last_count != sv.count && strstr(sv.data,PATH_SEP) != NULL){
            last_count = sv.count;
            knob_sv_chop_by_delim(&sv,PATH_SEP[0]);
        }
        char* out = knob_temp_sprintf("%s"PATH_SEP"%s.o",config->build_to,sv.count > 0 ?  sv.data : filepath);
        if(knob_needs_rebuild1(out,filepath) || (knob_file_exists(config_file) && knob_needs_rebuild1(out,config_file))){
            knob_cmd_append(&cmd,GET_COMPILERPP_NAME(config->compiler));
            if(config->debug_mode){
                knob_cmd_append(&cmd, "-ggdb3");
            }
            knob_cmd_add_includes(&cmd,config);
            for(int i=0; i < config->defines.count;++i){
                knob_cmd_append(&cmd,config->defines.items[i]);
            }
            for(int i=0; i < config->cpp_flags.count;++i){
                knob_cmd_append(&cmd,config->cpp_flags.items[i]);
            }
            knob_cmd_append(&cmd, "-c",filepath);
            knob_cmd_append(&cmd,"-o",out);
            if(debug){
                Knob_String_Builder render = {0};
                knob_cmd_render(cmd,&render);
                knob_log(KNOB_INFO,"CPP CMD: %s",render.items);
            }
            Knob_Proc proc = knob_cmd_run_async(cmd,NULL,NULL);
            knob_da_append(&procs, proc);
        }
        knob_da_append(outs,out);
    }
    for(int i =0; i < config->c_files.count;++i){
        cmd.count = 0;
        filepath = config->c_files.items[i];
        Knob_String_View sv = {
            .data = filepath,
            .count = strlen(filepath)
        };
        size_t last_count =0;
        while(last_count != sv.count && strstr(sv.data,PATH_SEP) != NULL){
            last_count = sv.count;
            knob_sv_chop_by_delim(&sv,PATH_SEP[0]);
        }
        char* out = knob_temp_sprintf("%s"PATH_SEP"%s.o",config->build_to,sv.count > 0 ?  sv.data : filepath);
        if(knob_needs_rebuild1(out,filepath) || (knob_file_exists(config_file) && knob_needs_rebuild1(out,config_file))){
            knob_cmd_append(&cmd,GET_COMPILER_NAME(config->compiler));
            if(config->debug_mode){
                knob_cmd_append(&cmd, "-ggdb3");
            }
            knob_cmd_add_includes(&cmd,config);
            for(int i=0; i < config->defines.count;++i){
                knob_cmd_append(&cmd,config->defines.items[i]);
            }
            for(int i=0; i < config->c_flags.count;++i){
                knob_cmd_append(&cmd,config->c_flags.items[i]);
            }
            knob_cmd_append(&cmd, "-c",filepath);
            knob_cmd_append(&cmd,"-o",out);
            if(debug){
                Knob_String_Builder render = {0};
                knob_cmd_render(cmd,&render);
                knob_log(KNOB_INFO,"C CMD: %s",render.items);
            }
            Knob_Proc proc = knob_cmd_run_async(cmd,NULL,NULL);
            knob_da_append(&procs, proc);
        }
        knob_da_append(outs,out);
    }
    if (!knob_procs_wait(procs)) knob_return_defer(false);

defer:
    knob_cmd_free(cmd);
    return result;
}


void knob_create_plug(void){
const char default_source[] = 
"#pragma once\n\n"
"//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"//~~~IMPORTANT~~~~: User applications need to define the actual struct Plug in their own code.\n"
"//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"typedef struct Plug Plug_t;\n"
"#define LIST_OF_PLUGS \\\n"
"    PLUG(plug_init, void, void) \\\n"
"    PLUG(plug_pre_reload, void*, void) \\\n"
"    PLUG(plug_post_reload, void, void*) \\\n"
"    PLUG(plug_update, void, void)\n\n"
"#define PLUG(name, ret, ...) typedef ret (name##_t)(__VA_ARGS__);\n"
"LIST_OF_PLUGS\n"
"#undef PLUG\n";
    knob_write_entire_file(PLUG_PATH,(void*)default_source,strlen(default_source));
}

void knob_cmd_add_compiler(Knob_Cmd* cmd,Knob_Config* config){
    knob_cmd_append(cmd, GET_COMPILER_NAME(config->compiler));
    if(config->target == TARGET_LINUX || config->target == TARGET_LINUX_MUSL){// @TODO: Should we add Mingw for plugins ?
        if(config->output_type == BIN_DLL){
            knob_cmd_append(cmd, "-shared","-fPIC");
        }
        if(config->debug_mode && (config->compiler == COMPILER_CLANG || config->compiler == COMPILER_GCC || config->compiler == COMPILER_COSMOCC)){
            knob_cmd_append(cmd, "-ggdb3");
        }
    }
    if(config->compiler == COMPILER_ZIG){
        if(config->debug_mode){
            knob_cmd_append(cmd,"--debug");
        }
        knob_cmd_append(cmd,"-std=c11", "-fno-sanitize=undefined","-fno-omit-frame-pointer");
    }
}

void knob_cmd_add_includes(Knob_Cmd* cmd,Knob_Config* config){
    for(int i=0; i < config->includes.count;++i){
        knob_cmd_append(cmd,knob_temp_sprintf("-I%s",config->includes.items[i]));
    }
}

int knob_compile_run_submodule(const char* path,Knob_Config* config,Knob_File_Paths* files_to_link,Knob_Cmd* cmd_to_pass,const char* path_to_knobh){
    Knob_Cmd cmd = {0};
    Knob_Config conf = *config;
    conf.output_type = BIN_DLL;
    knob_cmd_add_compiler(&cmd,&conf);
    knob_cmd_append(&cmd,knob_temp_sprintf("-I%s",path_to_knobh));
    int path_len = strlen(path);
    #ifdef _WIN32
    char temp_path[260] = {0};
    int y = -1;
    while(y < path_len){
        ++y;
        if(path[y] == '/'){
            temp_path[y] = PATH_SEP[0];
            continue;
        }
        temp_path[y] = path[y];
    }
    path = temp_path;
    #endif
    char* sep = path[path_len-1] == PATH_SEP[0] ? "" : PATH_SEP;

    char project_name[64] = {0};
    int i = path_len-2;
    while(path[i] != PATH_SEP[0] ){
        i--;
    }
    i++;
    const char* sub = &path[i];
    strcpy(project_name,sub);
    if(strlen(sep) == 0){
        project_name[path_len - i  - 1] = '\0';
    }

    char temp[260] = {0};
    char cFile[260] = {0};
    snprintf(cFile,260,"%s%sknob.c",path,sep);
    knob_cmd_append(&cmd,cFile);

    knob_cmd_append(&cmd, "-DKNOB_SUBMODULE");
    int n = snprintf(temp,260,"%s%s%s%s","./build",sep,project_name,DLL_NAME);
    knob_cmd_append(&cmd,"-o",temp);
    if(!knob_cmd_run_sync(cmd)) return 0;
    knob_sleep_ms(500);
    void* dll_handle = dynlib_load(temp);
    if(dll_handle == NULL) return 0;
    
    memset(temp,0,n);
    n = snprintf(temp,260,"%s_entrypoint",project_name);
    submodule_entrypoint func = (submodule_entrypoint)dynlib_loadfunc(dll_handle,temp);
    if(func == NULL){
        exit(1);
    }
    
    memset(temp,0,n);
    getcwd(temp,260);
    chdir(path);
    if(!func(&conf,files_to_link,cmd_to_pass->count,(char**)cmd_to_pass->items)) return 0;
    chdir(temp);
    
    return 1;
    
}

// minirent.h SOURCE BEGIN ////////////////////////////////////////
#ifdef _WIN32
struct DIR
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    struct dirent *dirent;
};

DIR *opendir(const char *dirpath)
{
    assert(dirpath);

    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\*", dirpath);

    DIR *dir = (DIR*)calloc(1, sizeof(DIR));

    dir->hFind = FindFirstFile(buffer, &dir->data);
    if (dir->hFind == INVALID_HANDLE_VALUE) {
        // TODO: opendir should set errno accordingly on FindFirstFile fail
        // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        errno = ENOSYS;
        goto fail;
    }

    return dir;

fail:
    if (dir) {
        free(dir);
    }

    return NULL;
}

struct dirent *readdir(DIR *dirp)
{
    assert(dirp);

    if (dirp->dirent == NULL) {
        dirp->dirent = (struct dirent*)calloc(1, sizeof(struct dirent));
    } else {
        if(!FindNextFile(dirp->hFind, &dirp->data)) {
            if (GetLastError() != ERROR_NO_MORE_FILES) {
                // TODO: readdir should set errno accordingly on FindNextFile fail
                // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
                errno = ENOSYS;
            }

            return NULL;
        }
    }

    memset(dirp->dirent->d_name, 0, sizeof(dirp->dirent->d_name));

    strncpy(
        dirp->dirent->d_name,
        dirp->data.cFileName,
        sizeof(dirp->dirent->d_name) - 1);

    return dirp->dirent;
}

int closedir(DIR *dirp)
{
    assert(dirp);

    if(!FindClose(dirp->hFind)) {
        // TODO: closedir should set errno accordingly on FindClose fail
        // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        errno = ENOSYS;
        return -1;
    }

    if (dirp->dirent) {
        free(dirp->dirent);
    }
    free(dirp);

    return 0;
}
#endif // _WIN32
// minirent.h SOURCE END ////////////////////////////////////////

// dynlib.h SOURCE BEGIN ////////////////////////////////////////
#if defined(_WIN32)
#include <stdlib.h>

static LPWSTR utfconv_utf8towc(const char *str) {
  LPWSTR output;
  int len;

  // len includes \0
  len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
  if (len == 0)
    return NULL;

  output = (LPWSTR) malloc(sizeof(WCHAR) * len);
  if (output == NULL)
    return NULL;

  len = MultiByteToWideChar(CP_UTF8, 0, str, -1, output, len);
  if (len == 0) {
    free(output);
    return NULL;
  }

  return output;
}

void *dynlib_load(const char *dllfile){
    void *handle;

    if (dllfile == NULL) {
        setbuf(stderr,dynlib_last_err);
        fprintf(stderr,"Param dllfile can't be NULL.");
        setbuf(stderr,NULL);
        return NULL;
    }
#ifdef __WINRT__
    LPWSTR wstr;//@TODO: test this I guess... a UWP thing.
    wstr = utfconv_utf8towc(dllfile);
    handle = (void *)LoadPackagedLibrary(wstr, 0);
    free(wstr);
#else
    handle = (void *)LoadLibraryA(dllfile);
#endif
    if (handle == NULL) {
        LPVOID errMsg;
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,GetLastError(),0,(LPSTR)&errMsg,0,NULL);
        strcpy(dynlib_last_err,errMsg);
        setbuf(stderr,dynlib_last_err);
        fprintf(stderr,"Failed loading %s: %s", dllfile, errMsg);
        setbuf(stderr,NULL);
        LocalFree(errMsg);
    }
    return handle;
}
int dynlib_unload(void *handle){
    if (handle != NULL) {
        return FreeLibrary((HMODULE)handle);
    }
    return 0;
}
void* dynlib_loadfunc(void *handle, const char *name){
    void *symbol = (void *)GetProcAddress((HMODULE)handle, name);
    if(symbol == NULL){
        setbuf(stderr,dynlib_last_err);
        fprintf(stderr,"Failed loading func %s",name);
    }
    return symbol;
}
#else
#if defined(__linux__)
#define __USE_GNU
#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>
#else
// #define _BSD_SOURCE // If it doesn't compile, just uncomment and see.(musllibc based)
#include <dlfcn.h>// FreeBSD usually.
#endif

// static size_t d_count = 0;
// static void* addr;
// int print_library_info(struct dl_phdr_info* info, size_t size, void* data)
// {
//     if (info->dlpi_name[0] != '\0') {
//         void* handle = dlopen(info->dlpi_name, RTLD_NOW | RTLD_NOLOAD);
//         if (handle != NULL) {
//           // if(strcmp(info->dlpi_name,"./engine.so") == 0){
//           //   d_count = info->dlpi_phdr[0].p_memsz;
//           //   addr = (void*)info->dlpi_addr;
//           // }
//           dynlib_unload(handle);
//           const char *error = dlerror();
//           if (error != NULL) {
//               fprintf(stderr, "Error unloading library %s: %s\n",info->dlpi_name,error);
//           }
//           else {
//             printf("Unloaded: %s\n",info->dlpi_name);
//           }
//         }
//     }
//     return 0;
// }

void *dynlib_load(const char *dllfile)
{
    void *handle;

    handle = dlopen(dllfile, RTLD_NOW /*| RTLD_LOCAL*/);
    if (handle == NULL) {
        setbuf(stderr,dynlib_last_err);
        fprintf(stderr,"Failed loading %s: %s", dllfile, dlerror());
        setbuf(stderr,NULL);
    }
    return handle;
}

int dynlib_unload(void *handle)
{
    // if (handle != NULL) {
    //     int count =-1;
    //     Dl_info info;
    //     int result = 0;
    //     while(dlinfo(handle,RTLD_DI_LINKMAP,&info) == 0 && result == 0){
    //         result = dlclose(handle);
    //         count++;
    //     }
    //     return count ==1;
    // }
    int res = dlclose(handle);
    return res == 0;
}

void* dynlib_loadfunc(void *handle, const char *name)
{
    void *symbol = dlsym(handle, name);
    if (symbol == NULL) {
        char _name[128] = {0};
        snprintf(_name,128,"_%s",name);
        symbol = dlsym(handle, _name);
        if (symbol == NULL) {
            setbuf(stderr,dynlib_last_err);
            fprintf(stderr,"Failed loading %s: %s\n", name, dlerror());
        }
    }
    return symbol;
}
#endif
// dynlib.h SOURCE END ////////////////////////////////////////

// sleep_ms SOURCE START //////////////////////////////////////

#ifndef WIN32
  #ifndef __GLIBC__// musl
      #define _GNU_SOURCE 
  #endif
  #if _POSIX_C_SOURCE >= 199309L
      #include <time.h>   // for nanosleep
  #else
      #include <unistd.h> // for usleep
  #endif
#endif

int knob_sleep_ms(int milliseconds){ // cross-platform sleep function
#ifdef WIN32
    DWORD milli = (DWORD)milliseconds;
    Sleep(milli);
    return 1;
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    return nanosleep(&ts, NULL);
#else
    if (milliseconds >= 1000)
      sleep(milliseconds / 1000);
    return usleep((milliseconds % 1000) * 1000);
#endif
}
// sleep_ms SOURCE END //////////////////////////////////////

#endif
