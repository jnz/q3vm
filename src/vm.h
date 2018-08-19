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

/******************************************************************************
 * TYPEDEFS
 ******************************************************************************/

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

// #define DEBUG_VM

#define Q3_LITTLE_ENDIAN

// Max number of arguments to pass from engine to vm's vmMain function.
// command number + 12 arguments
#define MAX_VMMAIN_ARGS 13

#define MAX_TOKEN_CHARS     1024    // max length of an individual token

// Max number of arguments to pass from a vm to engine's syscall handler function for the vm.
// syscall number + 15 arguments
#define MAX_VMSYSCALL_ARGS 16

#define MAX_QPATH           64      // max length of a quake game pathname
#define MAX_OSPATH          256

// don't change, this is hardcoded into x86 VMs, opStack protection relies
// on this
#define OPSTACK_SIZE    1024
#define OPSTACK_MASK    (OPSTACK_SIZE-1)

#define QDECL

#define PAD(base, alignment)    (((base)+(alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment) (PAD((base), (alignment)) - (base))
#define PADP(base, alignment)   ((void *) PAD((intptr_t) (base), (alignment)))

#define ARRAY_LEN(x)            (sizeof(x) / sizeof(*(x)))

#define Q_ftol(v) ((long) (v))

// don't change
// Hardcoded in q3asm a reserved at end of bss
#define PROGRAM_STACK_SIZE  0x10000
#define PROGRAM_STACK_MASK  (PROGRAM_STACK_SIZE-1)

#define Com_Memset memset
#define Com_Memcpy memcpy

#define ID_INLINE inline

void CopyShortSwap (void *dest, void *src);
void CopyLongSwap (void *dest, void *src);
short ShortSwap (short l);
int LongSwap (int l);
float FloatSwap (const float *f);

#if defined( Q3_BIG_ENDIAN ) && defined( Q3_LITTLE_ENDIAN )
#error "Endianness defined as both big and little"
#elif defined( Q3_BIG_ENDIAN )

#define CopyLittleShort(dest, src) CopyShortSwap(dest, src)
#define CopyLittleLong(dest, src) CopyLongSwap(dest, src)
#define LittleShort(x) ShortSwap(x)
#define LittleLong(x) LongSwap(x)
#define LittleFloat(x) FloatSwap(&x)
#define BigShort
#define BigLong
#define BigFloat

#elif defined( Q3_LITTLE_ENDIAN )

#define CopyLittleShort(dest, src) Com_Memcpy(dest, src, 2)
#define CopyLittleLong(dest, src) Com_Memcpy(dest, src, 4)
#define LittleShort
#define LittleLong
#define LittleFloat
#define BigShort(x) ShortSwap(x)
#define BigLong(x) LongSwap(x)
#define BigFloat(x) FloatSwap(&x)

#elif defined( Q3_VM )

#define LittleShort
#define LittleLong
#define LittleFloat
#define BigShort
#define BigLong
#define BigFloat

#else
#error "Endianness not defined"
#endif

#define Com_Printf printf

// typedef int intptr_t;

// parameters to the main Error routine
typedef enum {
    ERR_FATAL,                  // exit the entire game with a popup window
    ERR_DROP,                   // print to console and disconnect from game
    ERR_SERVERDISCONNECT,       // don't kill server
    ERR_DISCONNECT,             // client disconnected from the server
    ERR_NEED_CD                 // pop up the need-cd dialog
} errorParm_t;

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
    OP_STORE4,              // *(stack[top-1]) = stack[top]
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

typedef union {
    float f;
    int i;
    unsigned int ui;
} floatint_t;

typedef int vmptr_t;

typedef struct vmSymbol_s {
    struct vmSymbol_s   *next;
    int     symValue;
    int     profileCount;
    char    symName[1];     // variable sized
} vmSymbol_t;

#define VM_OFFSET_PROGRAM_STACK     0
#define VM_OFFSET_SYSTEM_CALL       4

typedef struct vm_s vm_t;
typedef enum {qfalse, qtrue}    qboolean;

#define VM_MAGIC        0x12721444

typedef struct {
    int     vmMagic;

    int     instructionCount;

    int     codeOffset;
    int     codeLength;

    int     dataOffset;
    int     dataLength;
    int     litLength;          // ( dataLength - litLength ) should be byteswapped on load
    int     bssLength;          // zero filled memory appended to datalength

} vmHeader_t;

struct vm_s {
    // DO NOT MOVE OR CHANGE THESE WITHOUT CHANGING THE VM_OFFSET_* DEFINES
    // USED BY THE ASM CODE
    int         programStack;       // the vm may be recursively entered
    intptr_t    (*systemCall)( intptr_t *parms );

    //------------------------------------
   
    char        name[MAX_QPATH];
    void        *searchPath;                // hint for FS_ReadFileDir()

    // for interpreted modules
    qboolean    currentlyInterpreting;

    qboolean    compiled;
    uint8_t     *codeBase;
    int         entryOfs;
    int         codeLength;

    intptr_t    *instructionPointers;
    int         instructionCount;

    uint8_t     *dataBase;
    int         dataMask;
    int         dataAlloc;          // actually allocated

    int         stackBottom;        // if programStack < stackBottom, error

    int         numSymbols;
    struct vmSymbol_s   *symbols;

    int         callLevel;      // counts recursive VM_Call
    int         breakFunction;      // increment breakCount on function entry to this
    int         breakCount;
};

typedef enum {
    VMI_NATIVE,
    VMI_BYTECODE,
    VMI_COMPILED
} vmInterpret_t;

typedef enum {
    TRAP_MEMSET = 100,
    TRAP_MEMCPY,
    TRAP_STRNCPY,
    TRAP_SIN,
    TRAP_COS,
    TRAP_ATAN2,
    TRAP_SQRT,
    TRAP_MATRIXMULTIPLY,
    TRAP_ANGLEVECTORS,
    TRAP_PERPENDICULARVECTOR,
    TRAP_FLOOR,
    TRAP_CEIL,

    TRAP_TESTPRINTINT,
    TRAP_TESTPRINTFLOAT
} sharedTraps_t;

vm_t *VM_Create(const char *module,
                uint8_t* bytecode,
                intptr_t (*systemCalls)(intptr_t *));

void    VM_Free( vm_t *vm );
void    VM_Clear(void);
void    VM_Forced_Unload_Start(void);
void    VM_Forced_Unload_Done(void);

intptr_t QDECL VM_Call( vm_t *vm, int callNum, ... );

void    VM_Debug( int level );

void    *VM_ArgPtr( intptr_t intValue );
void    *VM_ExplicitArgPtr( vm_t *vm, intptr_t intValue );

#define VMA(x) VM_ArgPtr(args[x])
static ID_INLINE float _vmf(intptr_t x)
{
    floatint_t fi;
    fi.i = (int) x;
    return fi.f;
}
#define VMF(x)  _vmf(args[x])



extern  int vm_debugLevel;

void VM_Compile( vm_t *vm, vmHeader_t *header );
int VM_CallCompiled( vm_t *vm, int *args );

void VM_PrepareInterpreter( vm_t *vm, vmHeader_t *header );
int VM_CallInterpreted( vm_t *vm, int *args );

vmSymbol_t *VM_ValueToFunctionSymbol( vm_t *vm, int value );
int VM_SymbolToValue( vm_t *vm, const char *symbol );
const char *VM_ValueToSymbol( vm_t *vm, int value );
void VM_LogSyscalls( int *args );

void VM_BlockCopy(unsigned int dest, unsigned int src, size_t n);

typedef enum {
    G_PRINT,        // ( const char *string );
    // print message on the local console

    G_ERROR,        // ( const char *string );
    // abort the game


} syscallId_t;

