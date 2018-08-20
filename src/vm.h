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

// #define DEBUG_VM
#define ID_INLINE           inline
#define MAX_QPATH           64      // max length of a pathname

// Redirect information output
#define Com_Printf          printf
#define Com_Memset          memset
#define Com_Memcpy          memcpy

/* stack macros (used in system calls) */
#define VMA(x) VM_ArgPtr(args[x])
#define VMF(x)  _vmf(args[x])

/* Define endianess of target platform */
#define Q3VM_LITTLE_ENDIAN

/******************************************************************************
 * TYPEDEFS
 ******************************************************************************/

typedef union {
    float f;
    int i;
    unsigned int ui;
} floatint_t;

typedef struct vmSymbol_s {
    struct vmSymbol_s   *next;
    int     symValue;
    int     profileCount;
    char    symName[1];     // variable sized
} vmSymbol_t;

typedef struct vm_s vm_t;
typedef enum {qfalse, qtrue}    qboolean;

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
    void        *searchPath;        // hint for FS_ReadFileDir()

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
    vmSymbol_t  *symbols;

    int         callLevel;          // counts recursive VM_Call
    int         breakFunction;      // increment breakCount on function entry to this
    int         breakCount;
};

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

void		Com_Error(int level, const char *error);
vm_t*		VM_Create(const char *module,
					  uint8_t* bytecode,
					  intptr_t (*systemCalls)(intptr_t *));
void		VM_Free(vm_t *vm);
intptr_t	VM_Call(vm_t *vm, int callNum, ...);
void		VM_Debug(int level);

void        *VM_ArgPtr(intptr_t intValue);

/******************************************************************************
 * INLINE
 ******************************************************************************/

static ID_INLINE float _vmf(intptr_t x)
{
    floatint_t fi;
    fi.i = (int) x;
    return fi.f;
}
