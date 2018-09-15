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

#include <stdint.h>
#include <stdio.h>  /* remove this if Com_Printf does not point to printf */
#include <string.h> /* remove this if Com_Mem*** does not point to memcpy */

/******************************************************************************
 * DEFINES
 ******************************************************************************/

// #define DEBUG_VM /**< ifdef: enable debug functions and additional checks */

/** File start magic number for .qvm files (4 bytes, little endian) */
#define VM_MAGIC 0x12721444

/** Don't change stack size: Hardcoded in q3asm and reserved at end of BSS */
#define VM_PROGRAM_STACK_SIZE 0x10000

/** Max. number of bytes in .qvm */
#define VM_MAX_IMAGE_SIZE 0x400000

/**< Maximum length of a pathname, 64 to be Q3 compatible */
#define VM_MAX_QPATH 64

/** Redirect printf() calls with this macro */
#define Com_Printf printf

/** Redirect memset() calls with this macro */
#define Com_Memset memset

/** Redirect memcpy() calls with this macro */
#define Com_Memcpy memcpy

/** Translate from virtual machine memory to real machine memory. */
#define VMA(x, vm) VMA_(args[x], vm)

/** Get float argument in syscall (used in system calls) and don't cast it */
#define VMF(x) VM_IntToFloat(args[x])

/******************************************************************************
 * TYPEDEFS
 ******************************************************************************/

/** VM error codes */
typedef enum {
    VM_NO_ERROR                    = 0,   /**< 0 = OK */
    VM_INVALID_POINTER             = -1,  /**< NULL pointer for vm_t */
    VM_FAILED_TO_LOAD_BYTECODE     = -2,  /**< Invalid byte code */
    VM_NO_SYSCALL_CALLBACK         = -3,  /**< Syscall pointer missing */
    VM_FREE_ON_RUNNING_VM          = -4,  /**< Call to VM_Free while running */
    VM_BLOCKCOPY_OUT_OF_RANGE      = -5,  /**< VM tries to escape sandbox */
    VM_PC_OUT_OF_RANGE             = -6,  /**< Program counter out of range */
    VM_JUMP_TO_INVALID_INSTRUCTION = -7,  /**< VM tries to escape sandbox */
    VM_STACK_OVERFLOW              = -8,  /**< Only in DEBUG_VM mode */
    VM_STACK_MISALIGNED            = -9,  /**< Stack not aligned (DEBUG_VM) */
    VM_OP_LOAD4_MISALIGNED         = -10, /**< Access misaligned (DEBUG_VM) */
    VM_STACK_ERROR                 = -11, /**< Stack corrupted after call */
    VM_DATA_OUT_OF_RANGE           = -12, /**< Syscall pointer not in sandbox */
    VM_MALLOC_FAILED               = -13, /**< Not enough memory */
    VM_BAD_INSTRUCTION             = -14, /**< Unknown OP code in bytecode */
    VM_NOT_LOADED                  = -15, /**< VM not loaded */
} vmErrorCode_t;

/** VM alloc type */
typedef enum {
    VM_ALLOC_CODE_SEC             = 0, /**< Bytecode code section */
    VM_ALLOC_DATA_SEC             = 1, /**< Bytecode data section */
    VM_ALLOC_INSTRUCTION_POINTERS = 2, /**< Bytecode instruction pointers */
    VM_ALLOC_DEBUG                = 3, /**< DEBUG_VM functions */
    VM_ALLOC_TYPE_MAX                  /**< Last item in vmMallocType_t */
} vmMallocType_t;

/** File header of a bytecode .qvm file. Can be directly mapped to the start of
 *  the file. This is always little endian encoded in the file. */
typedef struct
{
    int32_t vmMagic;          /**< Bytecode image shall start with VM_MAGIC */
    int32_t instructionCount; /**< Number of instructions in .qvm */
    int32_t codeOffset;       /**< Byte offset in .qvm file of .code segment */
    int32_t codeLength;       /**< Bytes in code segment */
    int32_t dataOffset;       /**< Byte offset in .qvm file of .data segment */
    int32_t dataLength;       /**< Bytes in .data segment */
    int32_t litLength; /**< Bytes in strings segment (after .data segment) */
    int32_t bssLength; /**< How many bytes should be used for .bss segment */
} vmHeader_t;

/** For debugging (DEBUG_VM): symbol list */
typedef struct vmSymbol_s
{
    struct vmSymbol_s* next; /**< Linked list of symbols */

    int symValue; /**< Value of symbol that we want to have the ASCII name for
                     */
    int  profileCount; /**< For the runtime profiler. +1 for each call. */
    char symName[1];   /**< Variable sized symbol name. Space is reserved by
                          malloc at load time. */
} vmSymbol_t;

/** Main struct (think of a kind of a main class) to keep all information of
 * the virtual machine together. Has pointer to the bytecode, the stack and
 * everything. Call VM_Create(...) to initialize this struct. Call VM_Free(...)
 * to cleanup this struct and free the memory. */
typedef struct vm_s
{
    // DO NOT MOVE OR CHANGE THESE WITHOUT CHANGING THE VM_OFFSET_* DEFINES
    // USED BY THE ASM CODE (IF WE ADD THE Q3 JIT COMPILER IN THE FUTURE)

    int programStack; /**< Stack pointer into .data segment. */

    /** Function pointer to callback function for native functions called by
     * the bytecode. The function is identified by an integer id that
     * corresponds to the bytecode function ids defined in g_syscalls.asm.
     * Note however that parms equals to (-1 - function_id).
     * So -1 in g_syscalls.asm equals to 0 in the systemCall parms argument,
     *    -2 in g_syscalls.asm equals to 1 in the systemCall parms argument,
     *    -3 in g_syscalls.asm equals to 2 in the systemCall parms argument
     * and so on. You can convert it back to -1, -2, -3, but the 0 based
     * index might help a lookup table. */
    intptr_t (*systemCall)(struct vm_s* vm, intptr_t* parms);

    //------------------------------------

    char      name[VM_MAX_QPATH];    /** File name of the bytecode */
    void*     searchPath;            /**< unused */
    int       currentlyInterpreting; /**< Is the vm currently running? */
    int       compiled;   /**< Is a JIT active? Otherwise interpreted */
    uint8_t*  codeBase;   /**< Bytecode code segment */
    int       entryOfs;   /**< unused */
    int       codeLength; /**< Number of bytes in code segment */
    intptr_t* instructionPointers;
    int       instructionCount; /**< Number of instructions for VM */
    uint8_t*  dataBase;         /**< Start of .data memory segment */
    int       dataMask;         /**< VM mask to protect access to dataBase */
    int       dataAlloc;        /**< Number of bytes allocated for dataBase */
    int       stackBottom;      /**< If programStack < stackBottom, error */

    //------------------------------------

    /* DEBUG_VM */
    int         numSymbols;    /**< Number of symbols from VM_LoadSymbols */
    vmSymbol_t* symbols;       /**< By VM_LoadSymbols: names for debugging */
    int         callLevel;     /**< Counts recursive VM_Call */
    int         breakFunction; /**< For debugging: break at this function */
    int         breakCount;  /**< Used for breakpoints, triggered by OP_BREAK */
    vmErrorCode_t lastError; /**< Last known error */
} vm_t;

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

/** Initialize a virtual machine.
 * @param[out] vm Pointer to a virtual machine to initialize.
 * @param[in] module Path to the bytecode file. Used to load the
 *                   symbols. Otherwise not strictly required.
 * @param[in] bytecode Pointer to the bytecode. Directly byte by byte
 *                     the content of the .qvm file.
 * @param[in] length Number of bytes in the bytecode array.
 * @param[in] systemCalls Function pointer to callback function for native
 *   functions called by the bytecode. The function is identified by an integer
 *   id that corresponds to the bytecode function ids defined in g_syscalls.asm.
 *   Note however that parms equals to (-1 - function_id). So -1 in
 *   g_syscalls.asm equals to 0 in the systemCall parms argument, -2 in
 *   g_syscalls.asm is 1 in parms, -3 is 2 and so on.
 * @return 0 if everything is OK. -1 if something went wrong. */
int VM_Create(vm_t* vm, const char* module, const uint8_t* bytecode, int length,
              intptr_t (*systemCalls)(vm_t*, intptr_t*));

/** Free the memory of the virtual machine.
 * @param[in] vm Pointer to initialized virtual machine. */
void VM_Free(vm_t* vm);

/** Run a function from the virtual machine.
 * Use the command argument to tell the VM what to do.
 * You can supply additional (up to 12) parameters to pass to the bytecode.
 * @param[in] vm Pointer to initialized virtual machine.
 * @param[in] command Basic parameter passed to the bytecode.
 * @return Return value of the function call by the VM. */
intptr_t VM_Call(vm_t* vm, int command, ...);

/** Helper function for syscalls VMA(x) macro:
 * Translate from virtual machine memory to real machine memory.
 * If this is a memory range, use the VM_MemoryRangeValid() function to
 * make sure that this syscall does not escape from the sandbox.
 * @param[in] vmAddr Address in virtual machine memory
 * @param[in,out] vm Current VM
 * @return translated address. */
void* VMA_(intptr_t vmAddr, vm_t* vm);

/** Helper function for syscalls VMF(x) macro:
 * Get float argument in syscall (used in system calls) and don't cast it.
 * E.g. if the VM calls a native function with a float argument: don't
 * cast the int argument to a float, but rather interpret it directly
 * as a floating point variable.
 * @param[in] x Argument on call stack.
 * @return Value as float. */
float VM_IntToFloat(int32_t x);

/** Helper function for syscalls:
 * Convert a 32-bit floating point variable to an integer variable without
 * converting it.
 * @param[in] f Floating point number.
 * @return Floating point number as integer */
int32_t VM_FloatToInt(float f);

/** Helper function for syscalls:
 * Check if address + range in in the valid VM memory range.
 * Use this function in the syscall callback to keep the VM in its sandbox.
 * @param[in] vmAddr Address in virtual machine memory
 * @param[in] len Length in bytes
 * @param[in] vm Current VM
 * @return 0 if valid (!), -1 if invalid. */
int VM_MemoryRangeValid(intptr_t vmAddr, size_t len, const vm_t* vm);

/** Print call statistics for every function. Only works with DEBUG_VM.
 * Does nothing if DEBUG_VM is not defined.
 * @param[in] vm VM to profile */
void VM_VmProfile_f(const vm_t* vm);

/** Set the printf debug level. Only useful with DEBUG_VM.
 * Set to 1 for general informations and 2 to output every opcode name.
 * @param[in] level If level is 0: be quiet (default). */
void VM_Debug(int level);

/******************************************************************************
 * CALLBACK FUNCTIONS (USER DEFINED IN HOST APPLICATION)
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

#endif /* __Q3VM_H */
