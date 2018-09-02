/*
      ___   _______     ____  __
     / _ \ |___ /\ \   / /  \/  |
    | | | |  |_ \ \ \ / /| |\/| |
    | |_| |____) | \ V / | |  | |
     \__\_______/   \_/  |_|  |_|


   Quake III Arena Virtual Machine
*/

/******************************************************************************
 * SYSTEM INCLUDE FILES
 ******************************************************************************/

#include "stdarg.h"

/******************************************************************************
 * PROJECT INCLUDE FILES
 ******************************************************************************/

#include "vm.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

/** File start magic number for .qvm files */
#define VM_MAGIC 0x12721444

/** Don't change stack size: Hardcoded in q3asm a reserved at end of bss */
#define PROGRAM_STACK_SIZE 0x10000
#define PROGRAM_STACK_MASK (PROGRAM_STACK_SIZE - 1)

#define OPSTACK_SIZE 1024

/** Max number of arguments to pass from a vm to engine's syscall handler
 * function for the vm.
 * syscall number + 15 arguments */
#define MAX_VMSYSCALL_ARGS 16

/* Max number of arguments to pass from engine to vm's vmMain function.
 * command number + 12 arguments */
#define MAX_VMMAIN_ARGS 13

#if defined(Q3VM_BIG_ENDIAN) && defined(Q3VM_LITTLE_ENDIAN)
#error "Endianness defined as both big and little"
#elif defined(Q3VM_BIG_ENDIAN)
/** Helper function to swap bytes for the big endian version.
 * @param[in] l Input value
 * @return swapped output value. */
static ID_INLINE int LongSwap(int l)
{
    uint8_t b1, b2, b3, b4;

    b1 = l & 255;
    b2 = (l >> 8) & 255;
    b3 = (l >> 16) & 255;
    b4 = (l >> 24) & 255;

    return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}
#define LittleLong(x) LongSwap(x)
#elif defined(Q3VM_LITTLE_ENDIAN)
#define LittleLong
#else
#error "Endianness not defined"
#endif

/* GCC can do computed gotos */
#ifdef __GNUC__
#ifndef DEBUG_VM           /* can't use computed gotos in debug mode */
#define USE_COMPUTED_GOTOS /**< use computed gotos instead of a switch */
#endif
#endif

/******************************************************************************
 * TYPEDEFS
 ******************************************************************************/

/** Enum for the virtual machine op codes */
typedef enum {
    OP_UNDEF,

    OP_IGNORE,

    OP_BREAK,

    OP_ENTER,
    OP_LEAVE,
    OP_CALL,
    OP_PUSH,
    OP_POP,

    OP_CONST,
    OP_LOCAL,

    OP_JUMP,

    //-------------------

    OP_EQ,
    OP_NE,

    OP_LTI,
    OP_LEI,
    OP_GTI,
    OP_GEI,

    OP_LTU,
    OP_LEU,
    OP_GTU,
    OP_GEU,

    OP_EQF,
    OP_NEF,

    OP_LTF,
    OP_LEF,
    OP_GTF,
    OP_GEF,

    //-------------------

    OP_LOAD1,
    OP_LOAD2,
    OP_LOAD4,
    OP_STORE1,
    OP_STORE2,
    OP_STORE4, // *(stack[top-1]) = stack[top]
    OP_ARG,

    OP_BLOCK_COPY,

    //-------------------

    OP_SEX8,
    OP_SEX16,

    OP_NEGI,
    OP_ADD,
    OP_SUB,
    OP_DIVI,
    OP_DIVU,
    OP_MODI,
    OP_MODU,
    OP_MULI,
    OP_MULU,

    OP_BAND,
    OP_BOR,
    OP_BXOR,
    OP_BCOM,

    OP_LSH,
    OP_RSHI,
    OP_RSHU,

    OP_NEGF,
    OP_ADDF,
    OP_SUBF,
    OP_DIVF,
    OP_MULF,

    OP_CVIF,
    OP_CVFI
} opcode_t;

#ifndef USE_COMPUTED_GOTOS
/* for the the computed gotos we need labels,
 * but for the normal switch case we need the cases */
#define goto_OP_BREAK case OP_BREAK
#define goto_OP_BREAK case OP_BREAK
#define goto_OP_ENTER case OP_ENTER
#define goto_OP_LEAVE case OP_LEAVE
#define goto_OP_CALL case OP_CALL
#define goto_OP_PUSH case OP_PUSH
#define goto_OP_POP case OP_POP
#define goto_OP_CONST case OP_CONST
#define goto_OP_LOCAL case OP_LOCAL
#define goto_OP_JUMP case OP_JUMP
#define goto_OP_EQ case OP_EQ
#define goto_OP_NE case OP_NE
#define goto_OP_LTI case OP_LTI
#define goto_OP_LEI case OP_LEI
#define goto_OP_GTI case OP_GTI
#define goto_OP_GEI case OP_GEI
#define goto_OP_LTU case OP_LTU
#define goto_OP_LEU case OP_LEU
#define goto_OP_GTU case OP_GTU
#define goto_OP_GEU case OP_GEU
#define goto_OP_EQF case OP_EQF
#define goto_OP_NEF case OP_NEF
#define goto_OP_LTF case OP_LTF
#define goto_OP_LEF case OP_LEF
#define goto_OP_GTF case OP_GTF
#define goto_OP_GEF case OP_GEF
#define goto_OP_LOAD1 case OP_LOAD1
#define goto_OP_LOAD2 case OP_LOAD2
#define goto_OP_LOAD4 case OP_LOAD4
#define goto_OP_STORE1 case OP_STORE1
#define goto_OP_STORE2 case OP_STORE2
#define goto_OP_STORE4 case OP_STORE4
#define goto_OP_ARG case OP_ARG
#define goto_OP_BLOCK_COPY case OP_BLOCK_COPY
#define goto_OP_SEX8 case OP_SEX8
#define goto_OP_SEX16 case OP_SEX16
#define goto_OP_NEGI case OP_NEGI
#define goto_OP_ADD case OP_ADD
#define goto_OP_SUB case OP_SUB
#define goto_OP_DIVI case OP_DIVI
#define goto_OP_DIVU case OP_DIVU
#define goto_OP_MODI case OP_MODI
#define goto_OP_MODU case OP_MODU
#define goto_OP_MULI case OP_MULI
#define goto_OP_MULU case OP_MULU
#define goto_OP_BAND case OP_BAND
#define goto_OP_BOR case OP_BOR
#define goto_OP_BXOR case OP_BXOR
#define goto_OP_BCOM case OP_BCOM
#define goto_OP_LSH case OP_LSH
#define goto_OP_RSHI case OP_RSHI
#define goto_OP_RSHU case OP_RSHU
#define goto_OP_NEGF case OP_NEGF
#define goto_OP_ADDF case OP_ADDF
#define goto_OP_SUBF case OP_SUBF
#define goto_OP_DIVF case OP_DIVF
#define goto_OP_MULF case OP_MULF
#define goto_OP_CVIF case OP_CVIF
#define goto_OP_CVFI case OP_CVFI
#endif

/******************************************************************************
 * GLOBAL DATA DEFINITIONS
 ******************************************************************************/

/******************************************************************************
 * LOCAL DATA DEFINITIONS
 ******************************************************************************/

static int vm_debugLevel; /**< 0: be quiet, otherwise print informations */

/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************/

/** @brief Safe strncpy that ensures a trailing zero.
 * @param[out] dest Output string.
 * @param[in] src Input string.
 * @param[in] destsize Number of free bytes in dest. */
static void Q_strncpyz(char* dest, const char* src, int destsize);

/** Helper function for VM_Create: Set up the virtual machine during loading.
 * Ensure consistency and prepare the jumps.
 * @param[in,out] vm Pointer to virtual machine, prepared by VM_Create.
 * @param[in] header Header of .qvm bytecode.
 * @return 0 if everything is OK. -1 otherwise. */
static int VM_PrepareInterpreter(vm_t* vm, const vmHeader_t* header);

/** Helper function for VM_Create: Set up the virtual machine during loading.
 * Copy the data from the file input (bytecode) to the vm.
 * @param[in,out] vm Pointer to virtual machine, prepared by VM_Create.
 * @param[in] bytecode Pointer to bytecode.
 * @return Pointer to start/header of vm bytecode. */
static vmHeader_t* VM_LoadQVM(vm_t* vm, uint8_t* bytecode);

/** Run a function from the virtual machine with the interpreter (i.e. no JIT).
 * @param[in] vm Pointer to initialized virtual machine.
 * @param[in] args Arguments for function call.
 * @return Return value of the function call. */
static int VM_CallInterpreted(vm_t* vm, int* args);

/** Executes a block copy operation (memcpy) within currentVM data space.
 * @param[out] dest Pointer (in VM space).
 * @param[in] src Pointer (in VM space).
 * @param[in] n Number of bytes.
 * @param[in,out] vm Current VM */
static void VM_BlockCopy(unsigned int dest, unsigned int src, size_t n,
                         vm_t* vm);

/******************************************************************************
 * DEBUG FUNCTIONS
 ******************************************************************************/

#ifdef DEBUG_VM
#include <stdio.h>           /* fopen to read symbols */
#define MAX_TOKEN_CHARS 1024 /**< max length of an individual token */
/* WARNING: DEBUG_VM is not thread safe */
static char com_token[MAX_TOKEN_CHARS]; /**< helper for COM_Parse */
static int  com_lines;                  /**< helper for COM_Parse */
static int  com_tokenline;              /**< helper for COM_Parse */
static int ParseHex(const char* text);  /**< helper for VM_LoadSymbols */
static void COM_StripExtension(const char* in,
                               char* out); /**< helper for VM_LoadSymbols */
static char* VM_Indent(vm_t* vm);
/** For profiling, find the symbol behind this value */
static vmSymbol_t* VM_ValueToFunctionSymbol(vm_t* vm, int value);
static const char* VM_ValueToSymbol(vm_t* vm, int value);
/** Load a .map file for the virtual machine. The .map file
 * should have the same path as the .qvm file. */
static void VM_LoadSymbols(vm_t* vm);
/** Load the file into a malloc'd buffer.
 * @param[in] filepath File to load.
 * @return file content in buffer. Call Com_free() to cleanup. */
static uint8_t* loadImage(const char* filepath);
/** Print a stack trace on OP_ENTER if vm_debugLevel is > 0 */
static void VM_StackTrace(vm_t* vm, int programCounter, int programStack);
#endif

/******************************************************************************
 * LOCAL INLINE FUNCTIONS AND FUNCTION MACROS
 ******************************************************************************/

#define ARRAY_LEN(x) (sizeof(x) / sizeof(*(x)))
#define PAD(base, alignment) (((base) + (alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment) (PAD((base), (alignment)) - (base))
#define PADP(base, alignment) ((void*)PAD((intptr_t)(base), (alignment)))
#define Q_ftol(v) ((long)(v))

/******************************************************************************
 * FUNCTION BODIES
 ******************************************************************************/

static void Q_strncpyz(char* dest, const char* src, int destsize)
{
    if (!dest || !src || destsize < 1)
    {
        return;
    }
    strncpy(dest, src, destsize - 1);
    dest[destsize - 1] = 0;
}

int VM_Create(vm_t* vm, const char* name, uint8_t* bytecode,
              intptr_t (*systemCalls)(vm_t*, intptr_t*))
{
    vmHeader_t* header;

    if (!vm)
    {
        Com_Error(VM_INVALID_POINTER, "Invalid vm pointer");
        return -1;
    }
    if (!systemCalls)
    {
        vm->lastError = VM_NO_SYSCALL_CALLBACK;
        Com_Error(vm->lastError, "No systemcalls provided");
        return -1;
    }

    Com_Memset(vm, 0, sizeof(vm_t));
    Q_strncpyz(vm->name, name, sizeof(vm->name));
    header = VM_LoadQVM(vm, bytecode);
    if (!header)
    {
        vm->lastError = VM_FAILED_TO_LOAD_BYTECODE;
        Com_Error(vm->lastError, "Failed to load bytecode");
        return -1;
    }

    vm->systemCall = systemCalls;

    // allocate space for the jump targets, which will be filled in by the
    // compile/prep functions
    vm->instructionCount    = header->instructionCount;
    vm->instructionPointers = (intptr_t*)Com_malloc(
        vm->instructionCount * sizeof(*vm->instructionPointers), vm,
        VM_ALLOC_INSTRUCTION_POINTERS);

    // copy or compile the instructions
    vm->codeLength = header->codeLength;

    vm->compiled = 0; /* no JIT */
    if (!vm->compiled)
    {
        if (VM_PrepareInterpreter(vm, header) != 0)
        {
            return -1;
        }
    }

#ifdef DEBUG_VM
    // load the map file
    VM_LoadSymbols(vm);
#endif

    // the stack is implicitly at the end of the image
    vm->programStack = vm->dataMask + 1;
    vm->stackBottom  = vm->programStack - PROGRAM_STACK_SIZE;

    return 0;
}

static vmHeader_t* VM_LoadQVM(vm_t* vm, uint8_t* bytecode)
{
    int dataLength;
    int i;
    union {
        vmHeader_t* h;
        void*       v;
    } header;

    // load the image
    Com_Printf("Loading vm file %s...\n", vm->name);

    header.v = bytecode;
    if (!header.h)
    {
        Com_Printf("Failed.\n");
        VM_Free(vm);
        return NULL;
    }

    if (LittleLong(header.h->vmMagic) == VM_MAGIC)
    {
        // byte swap the header
        // sizeof( vmHeader_t ) - sizeof( int ) is the 1.32b vm header size
        for (i = 0; i < (sizeof(vmHeader_t) - sizeof(int)) / 4; i++)
        {
            ((int*)header.h)[i] = LittleLong(((int*)header.h)[i]);
        }

        // validate
        if (header.h->bssLength < 0 || header.h->dataLength < 0 ||
            header.h->litLength < 0 || header.h->codeLength <= 0)
        {
            Com_Printf("Warning: %s has bad header\n", vm->name);
            return NULL;
        }
    }
    else
    {
        Com_Printf("Warning: Invalid magic number in header of \"%s\". "
                   "Read: 0x%x, expected: 0x%x\n",
                   vm->name, LittleLong(header.h->vmMagic), VM_MAGIC);
        return NULL;
    }

    // round up to next power of 2 so all data operations can
    // be mask protected
    dataLength =
        header.h->dataLength + header.h->litLength + header.h->bssLength;
    for (i = 0; dataLength > (1 << i); i++)
    {
    }
    dataLength = 1 << i;

    // allocate zero filled space for initialized and uninitialized data
    // leave some space beyond data mask so we can secure all mask operations
    vm->dataAlloc = dataLength + 4;
    vm->dataBase  = (uint8_t*)Com_malloc(vm->dataAlloc, vm, VM_ALLOC_DATA_SEC);
    vm->dataMask  = dataLength - 1;

    if (vm->dataBase == NULL)
    {
        Com_Printf("Out of memory\n");
        return NULL;
    }

    // copy the intialized data
    Com_Memcpy(vm->dataBase, (uint8_t*)header.h + header.h->dataOffset,
               header.h->dataLength + header.h->litLength);

    // byte swap the longs
    for (i = 0; i < header.h->dataLength; i += 4)
    {
        *(int*)(vm->dataBase + i) = LittleLong(*(int*)(vm->dataBase + i));
    }

    return header.h;
}

void VM_Free(vm_t* vm)
{
    if (!vm)
    {
        return;
    }

    vmSymbol_t* sym = vm->symbols;
    while (sym)
    {
        vmSymbol_t* next = sym->next;
        Com_free(sym, NULL, VM_ALLOC_DEBUG);
        sym = next;
    }

    if (vm->callLevel)
    {
        vm->lastError = VM_FREE_ON_RUNNING_VM;
        Com_Error(vm->lastError, "VM_Free on running vm");
        return;
    }

    if (vm->codeBase)
    {
        Com_free(vm->codeBase, vm, VM_ALLOC_CODE_SEC);
    }
    if (vm->dataBase)
    {
        Com_free(vm->dataBase, vm, VM_ALLOC_DATA_SEC);
    }
    if (vm->instructionPointers)
    {
        Com_free(vm->instructionPointers, vm, VM_ALLOC_INSTRUCTION_POINTERS);
    }

    Com_Memset(vm, 0, sizeof(*vm));
}

void* VM_ArgPtr(intptr_t vmAddr, vm_t* vm)
{
    if (!vmAddr)
    {
        return NULL;
    }
    if (vm == NULL)
    {
        Com_Error(VM_INVALID_POINTER, "Invalid VM pointer");
        return NULL;
    }

    return (void*)(vm->dataBase + (vmAddr & vm->dataMask));
}

int VM_MemoryRangeValid(intptr_t vmAddr, size_t len, const vm_t* vm)
{
    if (!vmAddr || !vm)
    {
        return -1;
    }
    const unsigned dest = vmAddr;
    const unsigned dataMask = vm->dataMask;
    if (((dest + len) & dataMask) != dest + len)
    {
        Com_Error(VM_DATA_OUT_OF_RANGE, "Memory access out of range");
        return -1;
    }
    else
    {
        return 0;
    }
}

intptr_t VM_Call(vm_t* vm, int command, ...)
{
    intptr_t r;
    int      args[MAX_VMMAIN_ARGS];
    va_list  ap;
    int      i;

    if (!vm)
    {
        Com_Error(VM_INVALID_POINTER, "VM_Call with NULL vm");
        return -1;
    }

    args[0] = command;
    va_start(ap, command);
    for (i = 1; i < ARRAY_LEN(args); i++)
    {
        args[i] = va_arg(ap, int);
    }
    va_end(ap);

    ++vm->callLevel;
    r = VM_CallInterpreted(vm, args);
    --vm->callLevel;

    return r;
}

static void VM_BlockCopy(unsigned int dest, unsigned int src, size_t n,
                         vm_t* vm)
{
    unsigned int dataMask = vm->dataMask;

    if ((dest & dataMask) != dest || (src & dataMask) != src ||
        ((dest + n) & dataMask) != dest + n ||
        ((src + n) & dataMask) != src + n)
    {
        vm->lastError = VM_BLOCKCOPY_OUT_OF_RANGE;
        Com_Error(vm->lastError, "OP_BLOCK_COPY out of range");
        return;
    }

    Com_Memcpy(vm->dataBase + dest, vm->dataBase + src, n);
}

#ifdef DEBUG_VM
const static char* opnames[256] = {
    "OP_UNDEF",

    "OP_IGNORE", "OP_BREAK",  "OP_ENTER", "OP_LEAVE",      "OP_CALL",
    "OP_PUSH",   "OP_POP",    "OP_CONST", "OP_LOCAL",      "OP_JUMP",
    "OP_EQ",     "OP_NE",     "OP_LTI",   "OP_LEI",        "OP_GTI",
    "OP_GEI",    "OP_LTU",    "OP_LEU",   "OP_GTU",        "OP_GEU",
    "OP_EQF",    "OP_NEF",    "OP_LTF",   "OP_LEF",        "OP_GTF",
    "OP_GEF",    "OP_LOAD1",  "OP_LOAD2", "OP_LOAD4",      "OP_STORE1",
    "OP_STORE2", "OP_STORE4", "OP_ARG",   "OP_BLOCK_COPY", "OP_SEX8",
    "OP_SEX16",  "OP_NEGI",   "OP_ADD",   "OP_SUB",        "OP_DIVI",
    "OP_DIVU",   "OP_MODI",   "OP_MODU",  "OP_MULI",       "OP_MULU",
    "OP_BAND",   "OP_BOR",    "OP_BXOR",  "OP_BCOM",       "OP_LSH",
    "OP_RSHI",   "OP_RSHU",   "OP_NEGF",  "OP_ADDF",       "OP_SUBF",
    "OP_DIVF",   "OP_MULF",   "OP_CVIF",  "OP_CVFI"
};
#endif

static ID_INLINE int loadWord(void* addr)
{
    int word;
    Com_Memcpy(&word, addr, 4);
    return LittleLong(word);
}

static int VM_PrepareInterpreter(vm_t* vm, const vmHeader_t* header)
{
    int      op;
    int      byte_pc;
    int      int_pc;
    uint8_t* code;
    int      instruction;
    int*     codeBase;

    vm->codeBase = (uint8_t*)Com_malloc(
        vm->codeLength * 4, vm, VM_ALLOC_CODE_SEC); // we're now int aligned
    Com_Memcpy(vm->codeBase, (uint8_t*)header + header->codeOffset,
               vm->codeLength);

    // we don't need to translate the instructions, but we still need
    // to find each instructions starting point for jumps
    int_pc = byte_pc = 0;
    instruction      = 0;
    code             = (uint8_t*)header + header->codeOffset;
    codeBase         = (int*)vm->codeBase;

    // Copy and expand instructions to words while building instruction table
    while (instruction < header->instructionCount)
    {
        vm->instructionPointers[instruction] = int_pc;
        instruction++;

        op               = (int)code[byte_pc];
        codeBase[int_pc] = op;
        if (byte_pc > header->codeLength)
        {
            vm->lastError = VM_PC_OUT_OF_RANGE;
            Com_Error(vm->lastError,
                      "VM_PrepareInterpreter: pc > header->codeLength");
            return -1;
        }

        byte_pc++;
        int_pc++;

        // these are the only opcodes that aren't a single byte
        switch (op)
        {
        case OP_ENTER:
        case OP_CONST:
        case OP_LOCAL:
        case OP_LEAVE:
        case OP_EQ:
        case OP_NE:
        case OP_LTI:
        case OP_LEI:
        case OP_GTI:
        case OP_GEI:
        case OP_LTU:
        case OP_LEU:
        case OP_GTU:
        case OP_GEU:
        case OP_EQF:
        case OP_NEF:
        case OP_LTF:
        case OP_LEF:
        case OP_GTF:
        case OP_GEF:
        case OP_BLOCK_COPY:
            codeBase[int_pc] = loadWord(&code[byte_pc]);
            byte_pc += 4;
            int_pc++;
            break;
        case OP_ARG:
            codeBase[int_pc] = (int)code[byte_pc];
            byte_pc++;
            int_pc++;
            break;
        default:
            break;
        }
    }
    int_pc      = 0;
    instruction = 0;

    // Now that the code has been expanded to int-sized opcodes, we'll translate
    // instruction index
    // into an index into codeBase[], which contains opcodes and operands.
    while (instruction < header->instructionCount)
    {
        op = codeBase[int_pc];
        instruction++;
        int_pc++;

        switch (op)
        {
        // These ops need to translate addresses in jumps from instruction index
        // to int index
        case OP_EQ:
        case OP_NE:
        case OP_LTI:
        case OP_LEI:
        case OP_GTI:
        case OP_GEI:
        case OP_LTU:
        case OP_LEU:
        case OP_GTU:
        case OP_GEU:
        case OP_EQF:
        case OP_NEF:
        case OP_LTF:
        case OP_LEF:
        case OP_GTF:
        case OP_GEF:
            if (codeBase[int_pc] < 0 || codeBase[int_pc] > vm->instructionCount)
            {
                vm->lastError = VM_JUMP_TO_INVALID_INSTRUCTION;
                Com_Error(vm->lastError,
                          "VM_PrepareInterpreter: Jump to invalid "
                          "instruction number");
                return -1;
            }

            // codeBase[pc] is the instruction index. Convert that into an
            // offset into
            // the int-aligned codeBase[] by the lookup table.
            codeBase[int_pc] = vm->instructionPointers[codeBase[int_pc]];
            int_pc++;
            break;

        // These opcodes have an operand that isn't an instruction index
        case OP_ENTER:
        case OP_CONST:
        case OP_LOCAL:
        case OP_LEAVE:
        case OP_BLOCK_COPY:
        case OP_ARG:
            int_pc++;
            break;

        default:
            break;
        }
    }
    return 0;
}

/*
==============
VM_CallInterpreted

Upon a system call, the stack will look like:

sp+32   parm1
sp+28   parm0
sp+24   return stack
sp+20   return address
sp+16   local1
sp+14   local0
sp+12   arg1
sp+8    arg0
sp+4    return stack
sp      return address

An interpreted function will immediately execute
an OP_ENTER instruction, which will subtract space for
locals from sp
==============
*/

static int VM_CallInterpreted(vm_t* vm, int* args)
{
    uint8_t  stack[OPSTACK_SIZE + 15];
    int*     opStack;
    uint8_t  opStackOfs;
    int      programCounter;
    int      programStack;
    int      stackOnEntry;
    uint8_t* image;
    int*     codeImage;
    int      v1;
    int      dataMask;
    int      arg;
#ifdef DEBUG_VM
    vmSymbol_t* profileSymbol;
#endif

    // interpret the code
    vm->currentlyInterpreting = 1;

    // we might be called recursively, so this might not be the very top
    programStack = stackOnEntry = vm->programStack;

#ifdef DEBUG_VM
    profileSymbol = VM_ValueToFunctionSymbol(vm, 0);
    // uncomment this for debugging breakpoints
    vm->breakFunction = 0;
#endif
    // set up the stack frame

    image     = vm->dataBase;
    codeImage = (int*)vm->codeBase;
    dataMask  = vm->dataMask;

    programCounter = 0;

    programStack -= (8 + 4 * MAX_VMMAIN_ARGS);

    for (arg = 0; arg < MAX_VMMAIN_ARGS; arg++)
    {
        *(int*)&image[programStack + 8 + arg * 4] = args[arg];
    }

    *(int*)&image[programStack + 4] = 0;  // return stack
    *(int*)&image[programStack]     = -1; // will terminate the loop on return

    // leave a free spot at start of stack so
    // that as long as opStack is valid, opStack-1 will
    // not corrupt anything
    opStack    = PADP(stack, 16);
    *opStack   = 0xDEADBEEF;
    opStackOfs = 0;

    // main interpreter loop, will exit when a LEAVE instruction
    // grabs the -1 program counter

    int opcode, r0, r1;
#define r2 codeImage[programCounter]

#ifdef USE_COMPUTED_GOTOS
    static const void* dispatch_table[] = {
        &&goto_OP_LEAVE, /* OP_UNDEF */
        &&goto_OP_LEAVE, /* OP_IGNORE */
        &&goto_OP_BREAK,  &&goto_OP_ENTER,  &&goto_OP_LEAVE,
        &&goto_OP_CALL,   &&goto_OP_PUSH,   &&goto_OP_POP,
        &&goto_OP_CONST,  &&goto_OP_LOCAL,  &&goto_OP_JUMP,
        &&goto_OP_EQ,     &&goto_OP_NE,     &&goto_OP_LTI,
        &&goto_OP_LEI,    &&goto_OP_GTI,    &&goto_OP_GEI,
        &&goto_OP_LTU,    &&goto_OP_LEU,    &&goto_OP_GTU,
        &&goto_OP_GEU,    &&goto_OP_EQF,    &&goto_OP_NEF,
        &&goto_OP_LTF,    &&goto_OP_LEF,    &&goto_OP_GTF,
        &&goto_OP_GEF,    &&goto_OP_LOAD1,  &&goto_OP_LOAD2,
        &&goto_OP_LOAD4,  &&goto_OP_STORE1, &&goto_OP_STORE2,
        &&goto_OP_STORE4, &&goto_OP_ARG,    &&goto_OP_BLOCK_COPY,
        &&goto_OP_SEX8,   &&goto_OP_SEX16,  &&goto_OP_NEGI,
        &&goto_OP_ADD,    &&goto_OP_SUB,    &&goto_OP_DIVI,
        &&goto_OP_DIVU,   &&goto_OP_MODI,   &&goto_OP_MODU,
        &&goto_OP_MULI,   &&goto_OP_MULU,   &&goto_OP_BAND,
        &&goto_OP_BOR,    &&goto_OP_BXOR,   &&goto_OP_BCOM,
        &&goto_OP_LSH,    &&goto_OP_RSHI,   &&goto_OP_RSHU,
        &&goto_OP_NEGF,   &&goto_OP_ADDF,   &&goto_OP_SUBF,
        &&goto_OP_DIVF,   &&goto_OP_MULF,   &&goto_OP_CVIF,
        &&goto_OP_CVFI,
    };
#define DISPATCH2()                                                            \
    opcode = codeImage[programCounter++];                                      \
    goto* dispatch_table[opcode]
#define DISPATCH()                                                             \
    r0 = opStack[opStackOfs];                                                  \
    r1 = opStack[(uint8_t)(opStackOfs - 1)];                                   \
    DISPATCH2()
    DISPATCH(); /* initial jump into the loop */
#else
#define DISPATCH2() goto nextInstruction2
#define DISPATCH() goto nextInstruction
#endif

    while (1)
    {
#ifndef USE_COMPUTED_GOTOS
    nextInstruction:
        r0 = opStack[opStackOfs];
        r1 = opStack[(uint8_t)(opStackOfs - 1)];
    nextInstruction2:
        opcode = codeImage[programCounter++];

#ifdef DEBUG_VM
        if ((unsigned)programCounter >= vm->codeLength)
        {
            vm->lastError = VM_PC_OUT_OF_RANGE;
            Com_Error(vm->lastError, "VM pc out of range");
            return -1;
        }

        if (programStack <= vm->stackBottom)
        {
            vm->lastError = VM_STACK_OVERFLOW;
            Com_Error(vm->lastError, "VM stack overflow");
            return -1;
        }

        if (programStack & 3)
        {
            vm->lastError = VM_STACK_MISALIGNED;
            Com_Error(vm->lastError, "VM program stack misaligned");
            return -1;
        }

        if (vm_debugLevel > 1)
        {
            Com_Printf("%s%i %s\n", VM_Indent(vm), opStackOfs, opnames[opcode]);
        }
        profileSymbol->profileCount++;
#endif /* DEBUG_VM */
        switch (opcode)
#endif /* !USE_COMPUTED_GOTOS */
        {
        goto_OP_BREAK:
            vm->breakCount++;
            DISPATCH2();
        goto_OP_CONST:
            opStackOfs++;
            r1 = r0;
            r0 = opStack[opStackOfs] = r2;

            programCounter += 1;
            DISPATCH2();
        goto_OP_LOCAL:
            opStackOfs++;
            r1 = r0;
            r0 = opStack[opStackOfs] = r2 + programStack;

            programCounter += 1;
            DISPATCH2();
        goto_OP_LOAD4:
#ifdef DEBUG_VM
            if (opStack[opStackOfs] & 3)
            {
                vm->lastError = VM_OP_LOAD4_MISALIGNED;
                Com_Error(vm->lastError, "OP_LOAD4 misaligned");
                return -1;
            }
#endif
            r0 = opStack[opStackOfs] = *(int*)&image[r0 & dataMask];
            DISPATCH2();
        goto_OP_LOAD2:
            r0 = opStack[opStackOfs] = *(unsigned short*)&image[r0 & dataMask];
            DISPATCH2();
        goto_OP_LOAD1:
            r0 = opStack[opStackOfs] = image[r0 & dataMask];
            DISPATCH2();

        goto_OP_STORE4:
            *(int*)&image[r1 & dataMask] = r0;
            opStackOfs -= 2;
            DISPATCH();
        goto_OP_STORE2:
            *(short*)&image[r1 & dataMask] = r0;
            opStackOfs -= 2;
            DISPATCH();
        goto_OP_STORE1:
            image[r1 & dataMask] = r0;
            opStackOfs -= 2;
            DISPATCH();
        goto_OP_ARG:
            // single byte offset from programStack
            *(int*)&image[(codeImage[programCounter] + programStack) &
                          dataMask] = r0;
            opStackOfs--;
            programCounter += 1;
            DISPATCH();
        goto_OP_BLOCK_COPY:
            VM_BlockCopy(r1, r0, r2, vm);
            programCounter += 1;
            opStackOfs -= 2;
            DISPATCH();
        goto_OP_CALL:
            // save current program counter
            *(int*)&image[programStack] = programCounter;

            // jump to the location on the stack
            programCounter = r0;
            opStackOfs--;
            if (programCounter < 0)
            {
                // system call
                int r;
#ifdef DEBUG_VM
                if (vm_debugLevel)
                {
                    Com_Printf("%s%i---> systemcall(%i)\n", VM_Indent(vm),
                               opStackOfs, -1 - programCounter);
                }
#endif
                // save the stack to allow recursive VM entry
                vm->programStack = programStack - 4;
#ifdef DEBUG_VM
                int stomped = *(int*)&image[programStack + 4];
#endif
                *(int*)&image[programStack + 4] = -1 - programCounter;

                // the vm has ints on the stack, we expect
                // pointers so we might have to convert it
                if (sizeof(intptr_t) != sizeof(int))
                {
                    intptr_t argarr[MAX_VMSYSCALL_ARGS];
                    int*     imagePtr = (int*)&image[programStack];
                    int      i;
                    for (i = 0; i < ARRAY_LEN(argarr); ++i)
                    {
                        argarr[i] = *(++imagePtr);
                    }
                    r = vm->systemCall(vm, argarr);
                }
                else
                {
                    intptr_t* argptr = (intptr_t*)&image[programStack + 4];
                    r                = vm->systemCall(vm, argptr);
                }

#ifdef DEBUG_VM
                // this is just our stack frame pointer, only needed
                // for debugging
                *(int*)&image[programStack + 4] = stomped;
#endif

                // save return value
                opStackOfs++;
                opStack[opStackOfs] = r;
                programCounter      = *(int*)&image[programStack];
#ifdef DEBUG_VM
                if (vm_debugLevel)
                {
                    Com_Printf("%s%i<--- %s\n", VM_Indent(vm), opStackOfs,
                               VM_ValueToSymbol(vm, programCounter));
                }
#endif
            }
            else if ((unsigned)programCounter >= vm->instructionCount)
            {
                vm->lastError = VM_PC_OUT_OF_RANGE;
                Com_Error(vm->lastError,
                          "VM program counter out of range in OP_CALL");
                return -1;
            }
            else
            {
                programCounter = vm->instructionPointers[programCounter];
            }
            DISPATCH();
        // push and pop are only needed for discarded or bad function return
        // values
        goto_OP_PUSH:
            opStackOfs++;
            DISPATCH();
        goto_OP_POP:
            opStackOfs--;
            DISPATCH();
        goto_OP_ENTER:
            // get size of stack frame
            v1 = r2;

            programCounter += 1;
            programStack -= v1;
#ifdef DEBUG_VM
            profileSymbol = VM_ValueToFunctionSymbol(vm, programCounter);
            //
            // save old stack frame for debugging traces
            *(int*)&image[programStack + 4] = programStack + v1;
            if (vm_debugLevel)
            {
                Com_Printf("%s%i---> %s\n", VM_Indent(vm), opStackOfs,
                           VM_ValueToSymbol(vm, programCounter - 5));
                if (vm->breakFunction &&
                    programCounter - 5 == vm->breakFunction)
                {
                    // this is to allow setting breakpoints here in the debugger
                    vm->breakCount++;
                    //                  vm_debugLevel = 2;
                    VM_StackTrace(vm, programCounter, programStack);
                }
            }
#endif
            DISPATCH();
        goto_OP_LEAVE:
            // remove our stack frame
            v1 = r2;

            programStack += v1;

            // grab the saved program counter
            programCounter = *(int*)&image[programStack];
#ifdef DEBUG_VM
            profileSymbol = VM_ValueToFunctionSymbol(vm, programCounter);
            if (vm_debugLevel)
            {
                Com_Printf("%s%i<--- %s\n", VM_Indent(vm), opStackOfs,
                           VM_ValueToSymbol(vm, programCounter));
            }
#endif
            // check for leaving the VM
            if (programCounter == -1)
            {
                goto done;
            }
            else if ((unsigned)programCounter >= vm->codeLength)
            {
                vm->lastError = VM_PC_OUT_OF_RANGE;
                Com_Error(vm->lastError,
                          "VM program counter out of range in OP_LEAVE");
                return -1;
            }
            DISPATCH();

        /*
           ===================================================================
           BRANCHES
           ===================================================================
           */

        goto_OP_JUMP:
            if ((unsigned)r0 >= vm->instructionCount)
            {
                vm->lastError = VM_PC_OUT_OF_RANGE;
                Com_Error(vm->lastError,
                          "VM program counter out of range in OP_JUMP");
                return -1;
            }

            programCounter = vm->instructionPointers[r0];

            opStackOfs--;
            DISPATCH();
        goto_OP_EQ:
            opStackOfs -= 2;
            if (r1 == r0)
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_NE:
            opStackOfs -= 2;
            if (r1 != r0)
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LTI:
            opStackOfs -= 2;
            if (r1 < r0)
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LEI:
            opStackOfs -= 2;
            if (r1 <= r0)
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GTI:
            opStackOfs -= 2;
            if (r1 > r0)
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GEI:
            opStackOfs -= 2;
            if (r1 >= r0)
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LTU:
            opStackOfs -= 2;
            if (((unsigned)r1) < ((unsigned)r0))
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LEU:
            opStackOfs -= 2;
            if (((unsigned)r1) <= ((unsigned)r0))
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GTU:
            opStackOfs -= 2;
            if (((unsigned)r1) > ((unsigned)r0))
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GEU:
            opStackOfs -= 2;
            if (((unsigned)r1) >= ((unsigned)r0))
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_EQF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)(opStackOfs + 1)] ==
                ((float*)opStack)[(uint8_t)(opStackOfs + 2)])
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_NEF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)(opStackOfs + 1)] !=
                ((float*)opStack)[(uint8_t)(opStackOfs + 2)])
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LTF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)(opStackOfs + 1)] <
                ((float*)opStack)[(uint8_t)(opStackOfs + 2)])
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LEF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)((uint8_t)(opStackOfs + 1))] <=
                ((float*)opStack)[(uint8_t)((uint8_t)(opStackOfs + 2))])
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GTF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)(opStackOfs + 1)] >
                ((float*)opStack)[(uint8_t)(opStackOfs + 2)])
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GEF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)(opStackOfs + 1)] >=
                ((float*)opStack)[(uint8_t)(opStackOfs + 2)])
            {
                programCounter = r2; // vm->instructionPointers[r2];
                DISPATCH();
            }
            else
            {
                programCounter += 1;
                DISPATCH();
            }

        //===================================================================

        goto_OP_NEGI:
            opStack[opStackOfs] = -r0;
            DISPATCH();
        goto_OP_ADD:
            opStackOfs--;
            opStack[opStackOfs] = r1 + r0;
            DISPATCH();
        goto_OP_SUB:
            opStackOfs--;
            opStack[opStackOfs] = r1 - r0;
            DISPATCH();
        goto_OP_DIVI:
            opStackOfs--;
            opStack[opStackOfs] = r1 / r0;
            DISPATCH();
        goto_OP_DIVU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) / ((unsigned)r0);
            DISPATCH();
        goto_OP_MODI:
            opStackOfs--;
            opStack[opStackOfs] = r1 % r0;
            DISPATCH();
        goto_OP_MODU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) % ((unsigned)r0);
            DISPATCH();
        goto_OP_MULI:
            opStackOfs--;
            opStack[opStackOfs] = r1 * r0;
            DISPATCH();
        goto_OP_MULU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) * ((unsigned)r0);
            DISPATCH();
        goto_OP_BAND:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) & ((unsigned)r0);
            DISPATCH();
        goto_OP_BOR:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) | ((unsigned)r0);
            DISPATCH();
        goto_OP_BXOR:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) ^ ((unsigned)r0);
            DISPATCH();
        goto_OP_BCOM:
            opStack[opStackOfs] = ~((unsigned)r0);
            DISPATCH();
        goto_OP_LSH:
            opStackOfs--;
            opStack[opStackOfs] = r1 << r0;
            DISPATCH();
        goto_OP_RSHI:
            opStackOfs--;
            opStack[opStackOfs] = r1 >> r0;
            DISPATCH();
        goto_OP_RSHU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) >> r0;
            DISPATCH();
        goto_OP_NEGF:
            ((float*)opStack)[opStackOfs] = -((float*)opStack)[opStackOfs];
            DISPATCH();
        goto_OP_ADDF:
            opStackOfs--;
            ((float*)opStack)[opStackOfs] =
                ((float*)opStack)[opStackOfs] +
                ((float*)opStack)[(uint8_t)(opStackOfs + 1)];
            DISPATCH();
        goto_OP_SUBF:
            opStackOfs--;
            ((float*)opStack)[opStackOfs] =
                ((float*)opStack)[opStackOfs] -
                ((float*)opStack)[(uint8_t)(opStackOfs + 1)];
            DISPATCH();
        goto_OP_DIVF:
            opStackOfs--;
            ((float*)opStack)[opStackOfs] =
                ((float*)opStack)[opStackOfs] /
                ((float*)opStack)[(uint8_t)(opStackOfs + 1)];
            DISPATCH();
        goto_OP_MULF:
            opStackOfs--;
            ((float*)opStack)[opStackOfs] =
                ((float*)opStack)[opStackOfs] *
                ((float*)opStack)[(uint8_t)(opStackOfs + 1)];
            DISPATCH();
        goto_OP_CVIF:
            ((float*)opStack)[opStackOfs] = (float)opStack[opStackOfs];
            DISPATCH();
        goto_OP_CVFI:
            opStack[opStackOfs] = Q_ftol(((float*)opStack)[opStackOfs]);
            DISPATCH();
        goto_OP_SEX8:
            opStack[opStackOfs] = (signed char)opStack[opStackOfs];
            DISPATCH();
        goto_OP_SEX16:
            opStack[opStackOfs] = (short)opStack[opStackOfs];
            DISPATCH();
        }
    }

done:
    vm->currentlyInterpreting = 0;

    if (opStackOfs != 1 || *opStack != 0xDEADBEEF)
    {
        vm->lastError = VM_STACK_ERROR;
        Com_Error(vm->lastError, "Interpreter stack error");
    }

    vm->programStack = stackOnEntry;

    // return the result
    return opStack[opStackOfs];
}

/* DEBUG FUNCTIONS */
/* --------------- */

void VM_Debug(int level)
{
    vm_debugLevel = level;
}

#ifdef DEBUG_VM
static char* VM_Indent(vm_t* vm)
{
    static char* string = "                                        ";
    if (vm->callLevel > 20)
    {
        return string;
    }
    return string + 2 * (20 - vm->callLevel);
}

static const char* VM_ValueToSymbol(vm_t* vm, int value)
{
    vmSymbol_t* sym;
    static char text[MAX_TOKEN_CHARS];

    sym = vm->symbols;
    if (!sym)
    {
        return "NO SYMBOLS";
    }

    // find the symbol
    while (sym->next && sym->next->symValue <= value)
    {
        sym = sym->next;
    }

    if (value == sym->symValue)
    {
        return sym->symName;
    }

    snprintf(text, sizeof(text), "%s+%i", sym->symName, value - sym->symValue);

    return text;
}

static vmSymbol_t* VM_ValueToFunctionSymbol(vm_t* vm, int value)
{
    vmSymbol_t*       sym;
    static vmSymbol_t nullSym;

    sym = vm->symbols;
    if (!sym)
    {
        return &nullSym;
    }

    while (sym->next && sym->next->symValue <= value)
    {
        sym = sym->next;
    }

    return sym;
}

static int ParseHex(const char* text)
{
    int value;
    int c;

    value = 0;
    while ((c = *text++) != 0)
    {
        if (c >= '0' && c <= '9')
        {
            value = value * 16 + c - '0';
            continue;
        }
        if (c >= 'a' && c <= 'f')
        {
            value = value * 16 + 10 + c - 'a';
            continue;
        }
        if (c >= 'A' && c <= 'F')
        {
            value = value * 16 + 10 + c - 'A';
            continue;
        }
    }

    return value;
}

static void COM_StripExtension(const char* in, char* out)
{
    while (*in && *in != '.')
    {
        *out++ = *in++;
    }
    *out = 0;
}

static uint8_t* loadImage(const char* filepath)
{
    FILE*    f;            /* input file */
    uint8_t* image = NULL; /* buffer */
    size_t   sz;           /* bytecode file size */

    f = fopen(filepath, "rb");
    if (!f)
    {
        return NULL;
    }
    /* calculate file size */
    fseek(f, 0L, SEEK_END);
    sz = ftell(f);
    rewind(f);

    image = (uint8_t*)Com_malloc(sz, NULL, VM_ALLOC_DEBUG);
    if (!image)
    {
        fclose(f);
        return NULL;
    }

    if (fread(image, 1, sz, f) != sz)
    {
        Com_free(image, NULL, VM_ALLOC_DEBUG);
        fclose(f);
        return NULL;
    }

    fclose(f);
    return image;
}

static char* SkipWhitespace(char* data, int* hasNewLines)
{
    int c;

    while ((c = *data) <= ' ')
    {
        if (!c)
        {
            return NULL;
        }
        if (c == '\n')
        {
            com_lines++;
            *hasNewLines = 1;
        }
        data++;
    }

    return data;
}

static char* COM_Parse(char** data_p)
{
    int   c           = 0, len;
    int   hasNewLines = 0;
    char* data;
    int   allowLineBreaks = 1;

    data          = *data_p;
    len           = 0;
    com_token[0]  = 0;
    com_tokenline = 0;

    // make sure incoming data is valid
    if (!data)
    {
        *data_p = NULL;
        return com_token;
    }

    while (1)
    {
        // skip whitespace
        data = SkipWhitespace(data, &hasNewLines);
        if (!data)
        {
            *data_p = NULL;
            return com_token;
        }
        if (hasNewLines && !allowLineBreaks)
        {
            *data_p = data;
            return com_token;
        }

        c = *data;

        // skip double slash comments
        if (c == '/' && data[1] == '/')
        {
            data += 2;
            while (*data && *data != '\n')
            {
                data++;
            }
        }
        // skip /* */ comments
        else if (c == '/' && data[1] == '*')
        {
            data += 2;
            while (*data && (*data != '*' || data[1] != '/'))
            {
                if (*data == '\n')
                {
                    com_lines++;
                }
                data++;
            }
            if (*data)
            {
                data += 2;
            }
        }
        else
        {
            break;
        }
    }

    // token starts on this line
    com_tokenline = com_lines;

    // handle quoted strings
    if (c == '\"')
    {
        data++;
        while (1)
        {
            c = *data++;
            if (c == '\"' || !c)
            {
                com_token[len] = 0;
                *data_p        = (char*)data;
                return com_token;
            }
            if (c == '\n')
            {
                com_lines++;
            }
            if (len < MAX_TOKEN_CHARS - 1)
            {
                com_token[len] = c;
                len++;
            }
        }
    }

    // parse a regular word
    do
    {
        if (len < MAX_TOKEN_CHARS - 1)
        {
            com_token[len] = c;
            len++;
        }
        data++;
        c = *data;
    } while (c > 32);

    com_token[len] = 0;

    *data_p = (char*)data;
    return com_token;
}

static void VM_LoadSymbols(vm_t* vm)
{
    union {
        char* c;
        void* v;
    } mapfile;
    char *       text_p, *token;
    char         name[MAX_QPATH];
    char         symbols[MAX_QPATH];
    vmSymbol_t **prev, *sym;
    int          count;
    int          value;
    int          chars;
    int          segment;
    int          numInstructions;

    COM_StripExtension(vm->name, name);
    snprintf(symbols, sizeof(symbols), "%s.map", name);
    Com_Printf("Loading symbol file: %s...\n", symbols);
    mapfile.v = loadImage(symbols);

    if (!mapfile.c)
    {
        Com_Printf("Couldn't load symbol file: %s\n", symbols);
        return;
    }

    numInstructions = vm->instructionCount;

    // parse the symbols
    text_p = mapfile.c;
    prev   = &vm->symbols;
    count  = 0;

    while (1)
    {
        token = COM_Parse(&text_p);
        if (!token[0])
        {
            break;
        }
        segment = ParseHex(token);
        if (segment)
        {
            COM_Parse(&text_p);
            COM_Parse(&text_p);
            continue; // only load code segment values
        }

        token = COM_Parse(&text_p);
        if (!token[0])
        {
            Com_Printf("WARNING: incomplete line at end of file\n");
            break;
        }
        value = ParseHex(token);

        token = COM_Parse(&text_p);
        if (!token[0])
        {
            Com_Printf("WARNING: incomplete line at end of file\n");
            break;
        }
        chars     = strlen(token);
        sym       = Com_malloc(sizeof(*sym) + chars, NULL, VM_ALLOC_DEBUG);
        *prev     = sym;
        prev      = &sym->next;
        sym->next = NULL;

        // convert value from an instruction number to a code offset
        if (value >= 0 && value < numInstructions)
        {
            value = vm->instructionPointers[value];
        }

        sym->symValue = value;
        Q_strncpyz(sym->symName, token, chars + 1);

        count++;
    }

    vm->numSymbols = count;
    Com_Printf("%i symbols parsed from %s\n", count, symbols);
    Com_free(mapfile.v, NULL, VM_ALLOC_DEBUG);
}

static void VM_StackTrace(vm_t* vm, int programCounter, int programStack)
{
    int count;

    count = 0;
    do
    {
        Com_Printf("%s\n", VM_ValueToSymbol(vm, programCounter));
        programStack   = *(int*)&vm->dataBase[programStack + 4];
        programCounter = *(int*)&vm->dataBase[programStack];
    } while (programCounter != -1 && ++count < 32);
}

static int VM_ProfileSort(const void* a, const void* b)
{
    vmSymbol_t *sa, *sb;

    sa = *(vmSymbol_t**)a;
    sb = *(vmSymbol_t**)b;

    if (sa->profileCount < sb->profileCount)
    {
        return -1;
    }
    if (sa->profileCount > sb->profileCount)
    {
        return 1;
    }
    return 0;
}

void VM_VmProfile_f(const vm_t* vm)
{
    vmSymbol_t **sorted, *sym;
    int          i;
    double       total;

    if (!vm)
    {
        return;
    }

    if (vm->numSymbols < 1)
    {
        return;
    }

    sorted = Com_malloc(vm->numSymbols * sizeof(*sorted), NULL, VM_ALLOC_DEBUG);
    sorted[0] = vm->symbols;
    total     = sorted[0]->profileCount;
    for (i = 1; i < vm->numSymbols; i++)
    {
        sorted[i] = sorted[i - 1]->next;
        total += sorted[i]->profileCount;
    }

    qsort(sorted, vm->numSymbols, sizeof(*sorted), VM_ProfileSort);

    for (i = 0; i < vm->numSymbols; i++)
    {
        int perc;

        sym = sorted[i];

        perc = 100 * (float)sym->profileCount / total;
        Com_Printf("%2i%% %9i %s\n", perc, sym->profileCount, sym->symName);
        sym->profileCount = 0;
    }

    Com_Printf("    %9.0f total\n", total);

    Com_free(sorted, NULL, VM_ALLOC_DEBUG);
}
#else
void VM_VmProfile_f(const vm_t* vm)
{
    (void)vm;
}
#endif
