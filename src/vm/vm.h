/*
      ___   _______     ____  __
     / _ \ |___ /\ \   / /  \/  |
    | | | |  |_ \ \ \ / /| |\/| |
    | |_| |____) | \ V / | |  | |
     \__\_______/   \_/  |_|  |_|


   Quake III Arena Virtual Machine
*/

#ifndef __Q3VM_H
#define __Q3VM_H

/******************************************************************************
 * SYSTEM INCLUDE FILES
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/******************************************************************************
 * PROJECT INCLUDE FILES
 ******************************************************************************/

/******************************************************************************
 * DEFINES
 ******************************************************************************/

// #define DEBUG_VM /**< ifdef: enable debug functions and additional checks */
#define ID_INLINE inline /**< Mark function as inline */
#define MAX_QPATH 64     /**< Maximum length of a pathname */

/** Redirect printf() calls with this macro */
#define Com_Printf printf
/** Redirect memset() calls with this macro */
#define Com_Memset memset
/** Redirect memcpy() calls with this macro */
#define Com_Memcpy memcpy

/** Translate pointer from VM memory to system memory */
#define VMA(x, vm) VM_ArgPtr(args[x], vm)
/** Get float argument in syscall (used in system calls) and
 * don't cast it. */
#define VMF(x) _vmf(args[x])

/** Define endianess of target platform */
#define Q3VM_LITTLE_ENDIAN

/******************************************************************************
 * TYPEDEFS
 ******************************************************************************/

/** VM error codes */
typedef enum {
    VM_NO_ERROR                    = 0,
    VM_INVALID_POINTER             = -1,
    VM_FAILED_TO_LOAD_BYTECODE     = -2,
    VM_NO_SYSCALL_CALLBACK         = -3,
    VM_FREE_ON_RUNNING_VM          = -4,
    VM_BLOCKCOPY_OUT_OF_RANGE      = -5,
    VM_PC_OUT_OF_RANGE             = -6, /**< program counter out of range */
    VM_JUMP_TO_INVALID_INSTRUCTION = -7,
    VM_STACK_OVERFLOW              = -8,
    VM_STACK_MISALIGNED            = -9,
    VM_OP_LOAD4_MISALIGNED         = -10,
    VM_STACK_ERROR                 = -11,
    VM_DATA_OUT_OF_RANGE           = -12,
} vmErrorCode_t;

/** VM alloc type */
typedef enum {
    VM_ALLOC_CODE_SEC             = 0, /**< Bytecode code section */
    VM_ALLOC_DATA_SEC             = 1, /**< Bytecode data section */
    VM_ALLOC_INSTRUCTION_POINTERS = 2, /**< Bytecode instruction pointers */
    VM_ALLOC_DEBUG                = 3, /**< DEBUG_VM functions */
    VM_ALLOC_TYPE_MAX                  /**< Make this the last item */
} vmMallocType_t;

/** For debugging: symbols */
typedef struct vmSymbol_s
{
    struct vmSymbol_s* next;     /**< Linked list of symbols */
    int                symValue; /**< Value of symbol that we want to have the
                                   ASCII name for */
    int profileCount;            /**< For the runtime profiler. +1 for each
                                   call. */
    char symName[1];             /**< Variable sized symbol name. Space is
                                   reserved by malloc at load time. */
} vmSymbol_t;

/** File header of a bytecode .qvm file. Can be directly mapped to the start of
 *  the file. */
typedef struct
{
    int32_t vmMagic; /**< Bytecode image shall start with VM_MAGIC */

    int32_t instructionCount;

    int32_t codeOffset;
    int32_t codeLength;

    int32_t dataOffset;
    int32_t dataLength;
    int32_t
            litLength; /**< (dataLength-litLength) should be byteswapped on load */
    int32_t bssLength; /**< Zero filled memory appended to datalength */
} vmHeader_t;

/** Main struct (think of a kind of a main class) to keep all information of
 * the virtual machine together. Has pointer to the bytecode, the stack and
 * everything. Call VM_Create(...) to initialize this struct. Call VM_Free(...)
 * to cleanup this struct and free the memory. */
typedef struct vm_s
{
    // DO NOT MOVE OR CHANGE THESE WITHOUT CHANGING THE VM_OFFSET_* DEFINES
    // USED BY THE ASM CODE
    int programStack; /**< the vm may be recursively entered */
    /** Function pointer to callback function for native functions called by
     * the bytecode. The function is identified by an integer id
     * that corresponds to the bytecode function ids defined in g_syscalls.asm.
     * Note however that parms equals to (-1 - function_id). So -1 in
     * g_syscalls.asm
     * equals to 0 in the systemCall parms argument, -2 in g_syscalls.asm is 1
     * in parms,
     * -3 is 2 and so on. */
    intptr_t (*systemCall)(struct vm_s* vm, intptr_t* parms);

    //------------------------------------

    char  name[MAX_QPATH]; /** File name of the bytecode */
    void* searchPath;      /** unused */

    int currentlyInterpreting; /** Is the interpreter currently running? */

    int      compiled; /** Is a JIT active? Otherwise interpreted */
    uint8_t* codeBase; /** Bytecode code segment */
    int      entryOfs; /** unused */
    int      codeLength;

    intptr_t* instructionPointers;
    int       instructionCount;

    uint8_t* dataBase;
    int      dataMask;
    int      dataAlloc; /**< Number of bytes allocated for dataBase */

    int stackBottom; /**< If programStack < stackBottom, error */

    int         numSymbols; /**< Number of loaded symbols */
    vmSymbol_t* symbols;    /**< Loaded symbols for debugging */

    int callLevel;     /**< Counts recursive VM_Call */
    int breakFunction; /**< Debug breakpoints: increment breakCount on function
                         entry to this */
    int breakCount;    /**< Used for breakpoints (triggered by OP_BREAK) */
    vmErrorCode_t lastError; /**< Last known error */
} vm_t;

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

/** Implement this error callback function for error callbacks in the host
 * application.
 * @param[in] level Error identifier, see vmErrorCode_t.
 * @param[in] error Human readable error text. */
void Com_Error(vmErrorCode_t level, const char* error);

/** Implement this memory allocation function in the host application.
 * A simple implementation in the host app can just call malloc() or new[]
 * and ignore the vm and type parameters.
 * The type information can be used as a hint for static memory allocation
 * if needed.
 * @param[in] size Number of bytes to allocate.
 * @param[in] vm Pointer to vm requesting the memory.
 * @param[in] type What purpose has the requested memory, see vmMallocType_t.
 * @return pointer to allocated memory. */
void* Com_malloc(size_t size, vm_t* vm, vmMallocType_t type);

/** Implement this memory free function in the host application.
 * A simple implementation in the host app can just call free() or delete[]
 * and ignore the vm and type parameters.
 * @param[in,out] p Pointer of memory allocated by Com_malloc to be released.
 * @param[in] vm Pointer to vm releasing the memory.
 * @param[in] type What purpose has the memory, see vmMallocType_t. */
void Com_free(void* p, vm_t* vm, vmMallocType_t type);

/** Initialize a virtual machine.
 * @param[out] vm Pointer to a virtual machine to initialize.
 * @param[in] module Path to the bytecode file. Used to load the
 *                   symbols. Otherwise not strictly required.
 * @param[in] bytecode Pointer to the bytecode. Directly byte by byte
 *                     the content of the .qvm file.
 * @param[in] systemCalls Function pointer to callback function for native
 *   functions called by the bytecode. The function is identified by an integer
 *   id that corresponds to the bytecode function ids defined in g_syscalls.asm.
 *   Note however that parms equals to (-1 - function_id). So -1 in
 *   g_syscalls.asm equals to 0 in the systemCall parms argument, -2 in
 *   g_syscalls.asm is 1 in parms, -3 is 2 and so on.
 * @return 0 if everything is OK. -1 if something went wrong. */
int VM_Create(vm_t* vm, const char* module, uint8_t* bytecode,
              intptr_t (*systemCalls)(vm_t*, intptr_t*));

/** Free the memory of the virtual machine.
 * @param[in] vm Pointer to initialized virtual machine. */
void VM_Free(vm_t* vm);

/** Run a function from the virtual machine.
 * @param[in] vm Pointer to initialized virtual machine.
 * @param[in] command Basic parameter passed to the bytecode.
 * You can supply additional (up to 12) parameters to pass to the bytecode.
 * @return Return value of the function call. */
intptr_t VM_Call(vm_t* vm, int command, ...);

/**< Translate from virtual machine memory to real machine memory
 * @param[in] vmAddr address in virtual machine memory
 * @param[in,out] vm Current VM
 * @param[in] len Length in bytes
 * @return translated address. */
void* VM_ArgPtr(intptr_t vmAddr, vm_t* vm);

/**< Check if address + range in in the valid VM memory range.
 * @param[in] vmAddr address in virtual machine memory
 * @param[in] len Length in bytes
 * @param[in] vm Current VM
 * @return 0 if valid, -1 if invalid. */
int VM_MemoryRangeValid(intptr_t vmAddr, size_t len, const vm_t* vm);

/** Print profile statistics. Only useful with #define DEBUG_VM.
 * Does nothing if DEBUG_VM is not defined.
 * @param[in] vm VM to profile */
void VM_VmProfile_f(const vm_t* vm);

/** Set the printf debug level. Only useful with #define DEBUG_VM.
 * @param[in] level If level is 0: be quiet (default). */
void VM_Debug(int level);

/******************************************************************************
 * INLINE
 ******************************************************************************/

/** Helper function for VMF. Used for float arguments on system calls.
 * @param[in] x Argument on call stack.
 * @return Value as float. */
static ID_INLINE float _vmf(intptr_t x)
{
    union {
        float    f;  /**< float IEEE 754 32-bit single */
        int32_t  i;  /**< int32 part */
        uint32_t ui; /**< unsigned int32 part */
    } fi;
    fi.i = (int)x;
    return fi.f;
}

#endif /* __Q3VM_H */
