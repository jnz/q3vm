/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// #include "q_shared.h"
// #include "qcommon.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

// #define QDECL __cdecl
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
#define VM_MAGIC_VER2   0x12721445

typedef struct {
    int     vmMagic;

    int     instructionCount;

    int     codeOffset;
    int     codeLength;

    int     dataOffset;
    int     dataLength;
    int     litLength;          // ( dataLength - litLength ) should be byteswapped on load
    int     bssLength;          // zero filled memory appended to datalength

    //!!! below here is VM_MAGIC_VER2 !!!
    int     jtrgLength;         // number of jump table targets
} vmHeader_t;

struct vm_s {
    // DO NOT MOVE OR CHANGE THESE WITHOUT CHANGING THE VM_OFFSET_* DEFINES
    // USED BY THE ASM CODE
    int         programStack;       // the vm may be recursively entered
    intptr_t            (*systemCall)( intptr_t *parms );

    //------------------------------------
   
    char        name[MAX_QPATH];
    void    *searchPath;                // hint for FS_ReadFileDir()

    // for dynamic linked modules
    void        *dllHandle;
    intptr_t            (QDECL *entryPoint)( int callNum, ... );
    void (*destroy)(vm_t* self);

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

    uint8_t     *jumpTableTargets;
    int         numJumpTableTargets;
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

void    VM_Init( void );
vm_t    *VM_Create( const char *module, vmInterpret_t interpret );

void    VM_Free( vm_t *vm );
void    VM_Clear(void);
void    VM_Forced_Unload_Start(void);
void    VM_Forced_Unload_Done(void);
vm_t    *VM_Restart(vm_t *vm, qboolean unpure);

intptr_t        QDECL VM_Call( vm_t *vm, int callNum, ... );

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



extern  vm_t    *currentVM;
extern  int     vm_debugLevel;

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
    //============== general Quake services ==================

    G_PRINT,        // ( const char *string );
    // print message on the local console

    G_ERROR,        // ( const char *string );
    // abort the game

    G_MILLISECONDS, // ( void );
    // get current time for profiling reasons
    // this should NOT be used for any game related tasks,
    // because it is not journaled

    // console variable interaction
    G_CVAR_REGISTER,    // ( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
    G_CVAR_UPDATE,  // ( vmCvar_t *vmCvar );
    G_CVAR_SET,     // ( const char *var_name, const char *value );
    G_CVAR_VARIABLE_INTEGER_VALUE,  // ( const char *var_name );

    G_CVAR_VARIABLE_STRING_BUFFER,  // ( const char *var_name, char *buffer, int bufsize );

    G_ARGC,         // ( void );
    // ClientCommand and ServerCommand parameter access

    G_ARGV,         // ( int n, char *buffer, int bufferLength );

    G_FS_FOPEN_FILE,    // ( const char *qpath, fileHandle_t *file, fsMode_t mode );
    G_FS_READ,      // ( void *buffer, int len, fileHandle_t f );
    G_FS_WRITE,     // ( const void *buffer, int len, fileHandle_t f );
    G_FS_FCLOSE_FILE,       // ( fileHandle_t f );

    G_SEND_CONSOLE_COMMAND, // ( const char *text );
    // add commands to the console as if they were typed in
    // for map changing, etc


    //=========== server specific functionality =============

    G_LOCATE_GAME_DATA,     // ( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
    //                          playerState_t *clients, int sizeofGameClient );
    // the game needs to let the server system know where and how big the gentities
    // are, so it can look at them directly without going through an interface

    G_DROP_CLIENT,      // ( int clientNum, const char *reason );
    // kick a client off the server with a message

    G_SEND_SERVER_COMMAND,  // ( int clientNum, const char *fmt, ... );
    // reliably sends a command string to be interpreted by the given
    // client.  If clientNum is -1, it will be sent to all clients

    G_SET_CONFIGSTRING, // ( int num, const char *string );
    // config strings hold all the index strings, and various other information
    // that is reliably communicated to all clients
    // All of the current configstrings are sent to clients when
    // they connect, and changes are sent to all connected clients.
    // All confgstrings are cleared at each level start.

    G_GET_CONFIGSTRING, // ( int num, char *buffer, int bufferSize );

    G_GET_USERINFO,     // ( int num, char *buffer, int bufferSize );
    // userinfo strings are maintained by the server system, so they
    // are persistant across level loads, while all other game visible
    // data is completely reset

    G_SET_USERINFO,     // ( int num, const char *buffer );

    G_GET_SERVERINFO,   // ( char *buffer, int bufferSize );
    // the serverinfo info string has all the cvars visible to server browsers

    G_SET_BRUSH_MODEL,  // ( gentity_t *ent, const char *name );
    // sets mins and maxs based on the brushmodel name

    G_TRACE,    // ( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
    // collision detection against all linked entities

    G_POINT_CONTENTS,   // ( const vec3_t point, int passEntityNum );
    // point contents against all linked entities

    G_IN_PVS,           // ( const vec3_t p1, const vec3_t p2 );

    G_IN_PVS_IGNORE_PORTALS,    // ( const vec3_t p1, const vec3_t p2 );

    G_ADJUST_AREA_PORTAL_STATE, // ( gentity_t *ent, qboolean open );

    G_AREAS_CONNECTED,  // ( int area1, int area2 );

    G_LINKENTITY,       // ( gentity_t *ent );
    // an entity will never be sent to a client or used for collision
    // if it is not passed to linkentity.  If the size, position, or
    // solidity changes, it must be relinked.

    G_UNLINKENTITY,     // ( gentity_t *ent );      
    // call before removing an interactive entity

    G_ENTITIES_IN_BOX,  // ( const vec3_t mins, const vec3_t maxs, gentity_t **list, int maxcount );
    // EntitiesInBox will return brush models based on their bounding box,
    // so exact determination must still be done with EntityContact

    G_ENTITY_CONTACT,   // ( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
    // perform an exact check against inline brush models of non-square shape

    // access for bots to get and free a server client (FIXME?)
    G_BOT_ALLOCATE_CLIENT,  // ( void );

    G_BOT_FREE_CLIENT,  // ( int clientNum );

    G_GET_USERCMD,  // ( int clientNum, usercmd_t *cmd )

    G_GET_ENTITY_TOKEN, // qboolean ( char *buffer, int bufferSize )
    // Retrieves the next string token from the entity spawn text, returning
    // false when all tokens have been parsed.
    // This should only be done at GAME_INIT time.

    G_FS_GETFILELIST,
    G_DEBUG_POLYGON_CREATE,
    G_DEBUG_POLYGON_DELETE,
    G_REAL_TIME,
    G_SNAPVECTOR,

    G_TRACECAPSULE, // ( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
    G_ENTITY_CONTACTCAPSULE,    // ( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
    
    // 1.32
    G_FS_SEEK,

} gameImport_t;

