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

#include <string.h>

/******************************************************************************
 * PROJECT INCLUDE FILES
 ******************************************************************************/

#include "vm.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define MAX_VM                      3
#define VM_MAGIC                    0x12721444
#define VM_OFFSET_PROGRAM_STACK     0
#define VM_OFFSET_SYSTEM_CALL       4

// don't change
// Hardcoded in q3asm a reserved at end of bss
#define PROGRAM_STACK_SIZE          0x10000
#define PROGRAM_STACK_MASK          (PROGRAM_STACK_SIZE-1)

// don't change, this is hardcoded into x86 VMs, opStack protection relies
// on this
#define OPSTACK_SIZE                1024
#define OPSTACK_MASK                (OPSTACK_SIZE-1)

// Max number of arguments to pass from a vm to engine's syscall handler function for the vm.
// syscall number + 15 arguments
#define MAX_VMSYSCALL_ARGS          16


// Max number of arguments to pass from engine to vm's vmMain function.
// command number + 12 arguments
#define MAX_VMMAIN_ARGS             13

#define MAX_TOKEN_CHARS             1024    // max length of an individual token

#define MAX_OSPATH                  256

#if defined( Q3VM_BIG_ENDIAN ) && defined( Q3VM_LITTLE_ENDIAN )
#error "Endianness defined as both big and little"
#elif defined( Q3VM_BIG_ENDIAN )
static ID_INLINE int LongSwap (int l)
{
    uint8_t    b1,b2,b3,b4;

    b1 = l&255;
    b2 = (l>>8)&255;
    b3 = (l>>16)&255;
    b4 = (l>>24)&255;

    return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}
#define LittleLong(x) LongSwap(x)
#elif defined( Q3VM_LITTLE_ENDIAN )
#define LittleLong
#else
#error "Endianness not defined"
#endif

#define DEBUGSTR va("%s%i", VM_Indent(vm), opStackOfs)

/******************************************************************************
 * TYPEDEFS
 ******************************************************************************/

typedef enum
{
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

/******************************************************************************
 * GLOBAL DATA DEFINITIONS
 ******************************************************************************/

int vm_debugLevel;
vm_t vmTable[MAX_VM];

/******************************************************************************
 * LOCAL DATA DEFINITIONS
 ******************************************************************************/

static vm_t *currentVM = NULL;
static vm_t *lastVM    = NULL;

/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************/

static void VM_VmInfo_f( void );
static void VM_VmProfile_f( void );
static void Q_strncpyz( char *dest, const char *src, int destsize );
static void VM_PrepareInterpreter( vm_t *vm, vmHeader_t *header );
static int VM_CallInterpreted( vm_t *vm, int *args );
static void VM_BlockCopy(unsigned int dest, unsigned int src, size_t n);

/******************************************************************************
 * LOCAL INLINE FUNCTIONS AND FUNCTION MACROS
 ******************************************************************************/

#define ARRAY_LEN(x)            (sizeof(x) / sizeof(*(x)))
#define PAD(base, alignment)    (((base)+(alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment) (PAD((base), (alignment)) - (base))
#define PADP(base, alignment)   ((void *) PAD((intptr_t) (base), (alignment)))
#define Q_ftol(v)               ((long) (v))



/******************************************************************************
 * FUNCTION BODIES
 ******************************************************************************/

/*
vmSymbol_t *VM_ValueToFunctionSymbol( vm_t *vm, int value );
int VM_SymbolToValue( vm_t *vm, const char *symbol );
const char *VM_ValueToSymbol( vm_t *vm, int value );
void VM_LogSyscalls( int *args );
*/

/* @brief Safe strncpy that ensures a trailing zero */
static void Q_strncpyz( char *dest, const char *src, int destsize )
{
    if ( !dest || !src || destsize < 1) {
        return;
    }
    strncpy( dest, src, destsize-1 );
    dest[destsize-1] = 0;
}

#ifdef DEBUG_VM

char *VM_Indent( vm_t *vm ) {
    static char *string = "                                        ";
    if ( vm->callLevel > 20 ) {
        return string;
    }
    return string + 2 * ( 20 - vm->callLevel );
}

/*
===============
VM_ValueToSymbol

Assumes a program counter value
===============
*/
const char *VM_ValueToSymbol( vm_t *vm, int value ) {
    vmSymbol_t  *sym;
    static char     text[MAX_TOKEN_CHARS];

    sym = vm->symbols;
    if ( !sym ) {
        return "NO SYMBOLS";
    }

    // find the symbol
    while ( sym->next && sym->next->symValue <= value ) {
        sym = sym->next;
    }

    if ( value == sym->symValue ) {
        return sym->symName;
    }

    snprintf( text, sizeof( text ), "%s+%i", sym->symName, value - sym->symValue );

    return text;
}

/*
===============
VM_ValueToFunctionSymbol

For profiling, find the symbol behind this value
===============
*/
vmSymbol_t *VM_ValueToFunctionSymbol( vm_t *vm, int value ) {
    vmSymbol_t  *sym;
    static vmSymbol_t   nullSym;

    sym = vm->symbols;
    if ( !sym ) {
        return &nullSym;
    }

    while ( sym->next && sym->next->symValue <= value ) {
        sym = sym->next;
    }

    return sym;
}

/*
===============
VM_SymbolToValue
===============
*/
int VM_SymbolToValue( vm_t *vm, const char *symbol ) {
    vmSymbol_t  *sym;

    for ( sym = vm->symbols ; sym ; sym = sym->next ) {
        if ( !strcmp( symbol, sym->symName ) ) {
            return sym->symValue;
        }
    }
    return 0;
}

/*
===============
ParseHex
===============
*/
static int ParseHex( const char *text ) {
    int     value;
    int     c;

    value = 0;
    while ( ( c = *text++ ) != 0 ) {
        if ( c >= '0' && c <= '9' ) {
            value = value * 16 + c - '0';
            continue;
        }
        if ( c >= 'a' && c <= 'f' ) {
            value = value * 16 + 10 + c - 'a';
            continue;
        }
        if ( c >= 'A' && c <= 'F' ) {
            value = value * 16 + 10 + c - 'A';
            continue;
        }
    }

    return value;
}

/*
===============
VM_LoadSymbols
===============
*/
void VM_LoadSymbols( vm_t *vm ) {
    union {
        char    *c;
        void    *v;
    } mapfile;
    char *text_p, *token;
    char    name[MAX_QPATH];
    char    symbols[MAX_QPATH];
    vmSymbol_t  **prev, *sym;
    int     count;
    int     value;
    int     chars;
    int     segment;
    int     numInstructions;

    // don't load symbols if not developer
    if ( !com_developer->integer ) {
        return;
    }

    COM_StripExtension(vm->name, name, sizeof(name));
    sprintf( symbols, sizeof( symbols ), "vm/%s.map", name );
    FS_ReadFile( symbols, &mapfile.v );
    if ( !mapfile.c ) {
        Com_Printf( "Couldn't load symbol file: %s\n", symbols );
        return;
    }

    numInstructions = vm->instructionCount;

    // parse the symbols
    text_p = mapfile.c;
    prev = &vm->symbols;
    count = 0;

    while ( 1 ) {
        token = COM_Parse( &text_p );
        if ( !token[0] ) {
            break;
        }
        segment = ParseHex( token );
        if ( segment ) {
            COM_Parse( &text_p );
            COM_Parse( &text_p );
            continue;       // only load code segment values
        }

        token = COM_Parse( &text_p );
        if ( !token[0] ) {
            Com_Printf( "WARNING: incomplete line at end of file\n" );
            break;
        }
        value = ParseHex( token );

        token = COM_Parse( &text_p );
        if ( !token[0] ) {
            Com_Printf( "WARNING: incomplete line at end of file\n" );
            break;
        }
        chars = strlen( token );
        sym = Hunk_Alloc( sizeof( *sym ) + chars, h_high );
        *prev = sym;
        prev = &sym->next;
        sym->next = NULL;

        // convert value from an instruction number to a code offset
        if ( value >= 0 && value < numInstructions ) {
            value = vm->instructionPointers[value];
        }

        sym->symValue = value;
        Q_strncpyz( sym->symName, token, chars + 1 );

        count++;
    }

    vm->numSymbols = count;
    Com_Printf( "%i symbols parsed from %s\n", count, symbols );
    FS_FreeFile( mapfile.v );
}

//=================================================================

static int VM_ProfileSort( const void *a, const void *b ) {
    vmSymbol_t  *sa, *sb;

    sa = *(vmSymbol_t **)a;
    sb = *(vmSymbol_t **)b;

    if ( sa->profileCount < sb->profileCount ) {
        return -1;
    }
    if ( sa->profileCount > sb->profileCount ) {
        return 1;
    }
    return 0;
}

/*
==============
VM_VmProfile_f

==============
*/
void VM_VmProfile_f( void ) {
    vm_t        *vm;
    vmSymbol_t  **sorted, *sym;
    int         i;
    double      total;

    if ( !lastVM ) {
        return;
    }

    vm = lastVM;

    if ( !vm->numSymbols ) {
        return;
    }

    sorted = Z_Malloc( vm->numSymbols * sizeof( *sorted ) );
    sorted[0] = vm->symbols;
    total = sorted[0]->profileCount;
    for ( i = 1 ; i < vm->numSymbols ; i++ ) {
        sorted[i] = sorted[i-1]->next;
        total += sorted[i]->profileCount;
    }

    qsort( sorted, vm->numSymbols, sizeof( *sorted ), VM_ProfileSort );

    for ( i = 0 ; i < vm->numSymbols ; i++ ) {
        int     perc;

        sym = sorted[i];

        perc = 100 * (float) sym->profileCount / total;
        Com_Printf( "%2i%% %9i %s\n", perc, sym->profileCount, sym->symName );
        sym->profileCount = 0;
    }

    Com_Printf("    %9.0f total\n", total );

    Z_Free( sorted );
}

/*
==============
VM_VmInfo_f

==============
*/
void VM_VmInfo_f( void ) {
    vm_t    *vm;
    int     i;

    Com_Printf( "Registered virtual machines:\n" );
    for ( i = 0 ; i < MAX_VM ; i++ ) {
        vm = &vmTable[i];
        if ( !vm->name[0] ) {
            break;
        }
        Com_Printf( "%s : ", vm->name );
        if ( vm->compiled ) {
            Com_Printf( "compiled on load\n" );
        } else {
            Com_Printf( "interpreted\n" );
        }
        Com_Printf( "    code length : %7i\n", vm->codeLength );
        Com_Printf( "    table length: %7i\n", vm->instructionCount*4 );
        Com_Printf( "    data length : %7i\n", vm->dataMask + 1 );
    }
}

/*
===============
VM_LogSyscalls

Insert calls to this while debugging the vm compiler
===============
*/
void VM_LogSyscalls( int *args ) {
    static  int     callnum;
    static  FILE    *f;

    if ( !f ) {
        f = fopen("syscalls.log", "w" );
    }
    callnum++;
    fprintf(f, "%i: %p (%i) = %i %i %i %i\n", callnum, (void*)(args - (int *)currentVM->dataBase),
        args[0], args[1], args[2], args[3], args[4] );
}

void VM_StackTrace( vm_t *vm, int programCounter, int programStack ) {
    int     count;

    count = 0;
    do {
        Com_Printf( "%s\n", VM_ValueToSymbol( vm, programCounter ) );
        programStack =  *(int *)&vm->dataBase[programStack+4];
        programCounter = *(int *)&vm->dataBase[programStack];
    } while ( programCounter != -1 && ++count < 32 );

}

#endif

/*
=================
VM_LoadQVM

Load a .qvm file
=================
*/
vmHeader_t *VM_LoadQVM(vm_t *vm, uint8_t* bytecode)
{
    int  dataLength;
    int  i;
    union {
        vmHeader_t *h;
        void *v;
    } header;

    // load the image
    Com_Printf( "Loading vm file %s...\n", vm->name );

    header.v = bytecode;

    if ( !header.h ) {
        Com_Printf( "Failed.\n" );
        VM_Free( vm );
        return NULL;
    }

    if( LittleLong( header.h->vmMagic ) == VM_MAGIC ) {
        // byte swap the header
        // sizeof( vmHeader_t ) - sizeof( int ) is the 1.32b vm header size
        for ( i = 0 ; i < ( sizeof( vmHeader_t ) - sizeof( int ) ) / 4 ; i++ ) {
            ((int *)header.h)[i] = LittleLong( ((int *)header.h)[i] );
        }

        // validate
        if ( header.h->bssLength < 0
            || header.h->dataLength < 0
            || header.h->litLength < 0
            || header.h->codeLength <= 0 )
        {
            Com_Printf("Warning: %s has bad header\n", vm->name);
            return NULL;
        }
    } else {
        Com_Printf("Warning: %s does not have a recognisable "
                "magic number in its header\n", vm->name);
        return NULL;
    }

    // round up to next power of 2 so all data operations can
    // be mask protected
    dataLength = header.h->dataLength + header.h->litLength +
        header.h->bssLength;
    for ( i = 0 ; dataLength > ( 1 << i ) ; i++ ) {
    }
    dataLength = 1 << i;

    // allocate zero filled space for initialized and uninitialized data
    // leave some space beyond data mask so we can secure all mask operations
    vm->dataAlloc = dataLength + 4;
    vm->dataBase = (uint8_t*)malloc(vm->dataAlloc);
    vm->dataMask = dataLength - 1;

    // copy the intialized data
    Com_Memcpy( vm->dataBase, (uint8_t *)header.h + header.h->dataOffset,
        header.h->dataLength + header.h->litLength );

    // byte swap the longs
    for ( i = 0 ; i < header.h->dataLength ; i += 4 ) {
        *(int *)(vm->dataBase + i) = LittleLong( *(int *)(vm->dataBase + i ) );
    }


    return header.h;
}

/*
================
VM_Create

If image ends in .qvm it will be interpreted, otherwise
it will attempt to load as a system dll
================
*/
vm_t *VM_Create(const char *module,
                uint8_t* bytecode,
                intptr_t (*systemCalls)(intptr_t *))
{
    vm_t        *vm;
    vmHeader_t  *header;

    if ( !module || !module[0] ) {
        Com_Error(-1, "VM_Create: bad parms" );
    }

    vm = &vmTable[0];

    Q_strncpyz(vm->name, module, sizeof(vm->name));
    header = VM_LoadQVM(vm, bytecode);
    if (!header)
    {
        Com_Error(-1, "Failed to load bytecode.\n");
    }

    // VM_Free overwrites the name on failed load
    Q_strncpyz(vm->name, module, sizeof(vm->name));

    vm->systemCall = systemCalls;

    // allocate space for the jump targets, which will be filled in by the compile/prep functions
    vm->instructionCount = header->instructionCount;
    vm->instructionPointers = (intptr_t*)malloc(vm->instructionCount * sizeof(*vm->instructionPointers));

    // copy or compile the instructions
    vm->codeLength = header->codeLength;

    vm->compiled = qfalse;

    // VM_Compile may have reset vm->compiled if compilation failed
    if (!vm->compiled)
    {
        VM_PrepareInterpreter( vm, header );
    }

    // load the map file
    // VM_LoadSymbols( vm );

    // the stack is implicitly at the end of the image
    vm->programStack = vm->dataMask + 1;
    vm->stackBottom = vm->programStack - PROGRAM_STACK_SIZE;

    return vm;
}

/*
==============
VM_Free
==============
*/
void VM_Free( vm_t *vm ) {

    if(!vm) {
        return;
    }

    if(vm->callLevel) {
        Com_Error(-1, "VM_Free on running vm" );
        return;
    }

    if ( vm->codeBase ) {
        free( vm->codeBase );
    }
    if ( vm->dataBase ) {
        free( vm->dataBase );
    }
    if ( vm->instructionPointers ) {
        free( vm->instructionPointers );
    }

    Com_Memset( vm, 0, sizeof( *vm ) );

    currentVM = NULL;
    lastVM = NULL;
}

void *VM_ArgPtr(intptr_t intValue)
{
    // currentVM is missing on reconnect
    if ( !intValue || currentVM == NULL)
	{
        return NULL;
    }
    return (void *)(currentVM->dataBase + (intValue & currentVM->dataMask));
}

/*
==============
VM_Call


Upon a system call, the stack will look like:

sp+32   parm1
sp+28   parm0
sp+24   return value
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

intptr_t VM_Call( vm_t *vm, int callnum, ... )
{
    vm_t    *oldVM;
    intptr_t r;

    if(!vm || !vm->name[0])
    {
        Com_Error(-1, "VM_Call with NULL vm");
    }

    oldVM = currentVM;
    currentVM = vm;
    lastVM = vm;

    if ( vm_debugLevel ) {
      Com_Printf( "VM_Call( %d )\n", callnum );
    }

    ++vm->callLevel;
    r = VM_CallInterpreted( vm, (int*)&callnum );
    --vm->callLevel;

    if ( oldVM != NULL )
      currentVM = oldVM;
    return r;
}

/*
=================
VM_BlockCopy
Executes a block copy operation within currentVM data space
=================
*/

static void VM_BlockCopy(unsigned int dest, unsigned int src, size_t n)
{
    unsigned int dataMask = currentVM->dataMask;

    if ((dest & dataMask) != dest
    || (src & dataMask) != src
    || ((dest + n) & dataMask) != dest + n
    || ((src + n) & dataMask) != src + n)
    {
        Com_Error(-1, "OP_BLOCK_COPY out of range!");
    }

    Com_Memcpy(currentVM->dataBase + dest, currentVM->dataBase + src, n);
}


void VM_Debug( int level )
{
    vm_debugLevel = level;
}

#ifdef DEBUG_VM
const static char *opnames[256] = {
    "OP_UNDEF",

    "OP_IGNORE",

    "OP_BREAK",

    "OP_ENTER",
    "OP_LEAVE",
    "OP_CALL",
    "OP_PUSH",
    "OP_POP",

    "OP_CONST",

    "OP_LOCAL",

    "OP_JUMP",

    //-------------------

    "OP_EQ",
    "OP_NE",

    "OP_LTI",
    "OP_LEI",
    "OP_GTI",
    "OP_GEI",

    "OP_LTU",
    "OP_LEU",
    "OP_GTU",
    "OP_GEU",

    "OP_EQF",
    "OP_NEF",

    "OP_LTF",
    "OP_LEF",
    "OP_GTF",
    "OP_GEF",

    //-------------------

    "OP_LOAD1",
    "OP_LOAD2",
    "OP_LOAD4",
    "OP_STORE1",
    "OP_STORE2",
    "OP_STORE4",
    "OP_ARG",

    "OP_BLOCK_COPY",

    //-------------------

    "OP_SEX8",
    "OP_SEX16",

    "OP_NEGI",
    "OP_ADD",
    "OP_SUB",
    "OP_DIVI",
    "OP_DIVU",
    "OP_MODI",
    "OP_MODU",
    "OP_MULI",
    "OP_MULU",

    "OP_BAND",
    "OP_BOR",
    "OP_BXOR",
    "OP_BCOM",

    "OP_LSH",
    "OP_RSHI",
    "OP_RSHU",

    "OP_NEGF",
    "OP_ADDF",
    "OP_SUBF",
    "OP_DIVF",
    "OP_MULF",

    "OP_CVIF",
    "OP_CVFI"
};
#endif

static ID_INLINE int loadWord(void *addr) {
    int word;
    Com_Memcpy(&word, addr, 4);
    return LittleLong(word);
}

/*
====================
VM_PrepareInterpreter
====================
*/
static void VM_PrepareInterpreter( vm_t *vm, vmHeader_t *header ) {
    int     op;
    int     byte_pc;
    int     int_pc;
    uint8_t *code;
    int     instruction;
    int     *codeBase;

    vm->codeBase = (uint8_t*)malloc( vm->codeLength*4 );          // we're now int aligned
    Com_Memcpy( vm->codeBase, (uint8_t *)header + header->codeOffset, vm->codeLength );

    // we don't need to translate the instructions, but we still need
    // to find each instructions starting point for jumps
    int_pc = byte_pc = 0;
    instruction = 0;
    code = (uint8_t *)header + header->codeOffset;
    codeBase = (int *)vm->codeBase;

    // Copy and expand instructions to words while building instruction table
    while ( instruction < header->instructionCount ) {
        vm->instructionPointers[ instruction ] = int_pc;
        instruction++;

        op = (int)code[ byte_pc ];
        codeBase[int_pc] = op;
        if(byte_pc > header->codeLength)
            Com_Error(-1, "VM_PrepareInterpreter: pc > header->codeLength");

        byte_pc++;
        int_pc++;

        // these are the only opcodes that aren't a single byte
        switch ( op ) {
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
    int_pc = 0;
    instruction = 0;

    // Now that the code has been expanded to int-sized opcodes, we'll translate instruction index
    //into an index into codeBase[], which contains opcodes and operands.
    while ( instruction < header->instructionCount ) {
        op = codeBase[ int_pc ];
        instruction++;
        int_pc++;

        switch ( op ) {
        // These ops need to translate addresses in jumps from instruction index to int index
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
            if(codeBase[int_pc] < 0 || codeBase[int_pc] > vm->instructionCount)
                Com_Error(-1, "VM_PrepareInterpreter: Jump to invalid instruction number");

            // codeBase[pc] is the instruction index. Convert that into an offset into
            //the int-aligned codeBase[] by the lookup table.
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
}

/*
==============
VM_Call


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

static int VM_CallInterpreted( vm_t *vm, int *args ) {
    uint8_t stack[OPSTACK_SIZE + 15];
    int     *opStack;
    uint8_t opStackOfs;
    int     programCounter;
    int     programStack;
    int     stackOnEntry;
    uint8_t *image;
    int     *codeImage;
    int     v1;
    int     dataMask;
    int     arg;
#ifdef DEBUG_VM
    vmSymbol_t  *profileSymbol;
#endif

    // interpret the code
    vm->currentlyInterpreting = qtrue;

    // we might be called recursively, so this might not be the very top
    programStack = stackOnEntry = vm->programStack;

#ifdef DEBUG_VM
    profileSymbol = VM_ValueToFunctionSymbol( vm, 0 );
    // uncomment this for debugging breakpoints
    vm->breakFunction = 0;
#endif
    // set up the stack frame

    image = vm->dataBase;
    codeImage = (int *)vm->codeBase;
    dataMask = vm->dataMask;

    programCounter = 0;

    programStack -= ( 8 + 4 * MAX_VMMAIN_ARGS );

    for ( arg = 0; arg < MAX_VMMAIN_ARGS; arg++ )
        *(int *)&image[ programStack + 8 + arg * 4 ] = args[ arg ];

    *(int *)&image[ programStack + 4 ] = 0; // return stack
    *(int *)&image[ programStack ] = -1;    // will terminate the loop on return

    VM_Debug(0);

    // leave a free spot at start of stack so
    // that as long as opStack is valid, opStack-1 will
    // not corrupt anything
    opStack = PADP(stack, 16);
    *opStack = 0xDEADBEEF;
    opStackOfs = 0;

//  vm_debugLevel=2;
    // main interpreter loop, will exit when a LEAVE instruction
    // grabs the -1 program counter

#define r2 codeImage[programCounter]

    while ( 1 ) {
        int     opcode, r0, r1;
//      unsigned int    r2;

nextInstruction:
        r0 = opStack[opStackOfs];
        r1 = opStack[(uint8_t) (opStackOfs - 1)];
nextInstruction2:
#ifdef DEBUG_VM
        if ( (unsigned)programCounter >= vm->codeLength ) {
            Com_Error(-1, "VM pc out of range" );
            return 0;
        }

        if ( programStack <= vm->stackBottom ) {
            Com_Error(-1, "VM stack overflow" );
            return 0;
        }

        if ( programStack & 3 ) {
            Com_Error(-1, "VM program stack misaligned" );
            return 0;
        }

        if ( vm_debugLevel > 1 ) {
            Com_Printf( "%s %s\n", DEBUGSTR, opnames[opcode] );
        }
        profileSymbol->profileCount++;
#endif
        opcode = codeImage[ programCounter++ ];

        switch ( opcode ) {
#ifdef DEBUG_VM
        default:
            Com_Error(-1, "Bad VM instruction" );  // this should be scanned on load!
            return 0;
#endif
        case OP_BREAK:
            vm->breakCount++;
            goto nextInstruction2;
        case OP_CONST:
            opStackOfs++;
            r1 = r0;
            r0 = opStack[opStackOfs] = r2;

            programCounter += 1;
            goto nextInstruction2;
        case OP_LOCAL:
            opStackOfs++;
            r1 = r0;
            r0 = opStack[opStackOfs] = r2+programStack;

            programCounter += 1;
            goto nextInstruction2;

        case OP_LOAD4:
#ifdef DEBUG_VM
            if(opStack[opStackOfs] & 3)
            {
                Com_Error(-1, "OP_LOAD4 misaligned" );
                return 0;
            }
#endif
            r0 = opStack[opStackOfs] = *(int *) &image[ r0 & dataMask ];
            goto nextInstruction2;
        case OP_LOAD2:
            r0 = opStack[opStackOfs] = *(unsigned short *)&image[ r0 & dataMask ];
            goto nextInstruction2;
        case OP_LOAD1:
            r0 = opStack[opStackOfs] = image[ r0 & dataMask ];
            goto nextInstruction2;

        case OP_STORE4:
            *(int *)&image[ r1 & dataMask ] = r0;
            opStackOfs -= 2;
            goto nextInstruction;
        case OP_STORE2:
            *(short *)&image[ r1 & dataMask ] = r0;
            opStackOfs -= 2;
            goto nextInstruction;
        case OP_STORE1:
            image[ r1 & dataMask ] = r0;
            opStackOfs -= 2;
            goto nextInstruction;

        case OP_ARG:
            // single byte offset from programStack
            *(int *)&image[ (codeImage[programCounter] + programStack) & dataMask ] = r0;
            opStackOfs--;
            programCounter += 1;
            goto nextInstruction;

        case OP_BLOCK_COPY:
            VM_BlockCopy(r1, r0, r2);
            programCounter += 1;
            opStackOfs -= 2;
            goto nextInstruction;

        case OP_CALL:
            // save current program counter
            *(int *)&image[ programStack ] = programCounter;

            // jump to the location on the stack
            programCounter = r0;
            opStackOfs--;
            if ( programCounter < 0 ) {
                // system call
                int     r;
//              int     temp;
#ifdef DEBUG_VM
                int     stomped;

                if ( vm_debugLevel ) {
                    Com_Printf( "%s---> systemcall(%i)\n", DEBUGSTR, -1 - programCounter );
                }
#endif
                // save the stack to allow recursive VM entry
//              temp = vm->callLevel;
                vm->programStack = programStack - 4;
#ifdef DEBUG_VM
                stomped = *(int *)&image[ programStack + 4 ];
#endif
                *(int *)&image[ programStack + 4 ] = -1 - programCounter;

//VM_LogSyscalls( (int *)&image[ programStack + 4 ] );
                {
                    // the vm has ints on the stack, we expect
                    // pointers so we might have to convert it
                    if (sizeof(intptr_t) != sizeof(int)) {
                        intptr_t argarr[ MAX_VMSYSCALL_ARGS ];
                        int *imagePtr = (int *)&image[ programStack ];
                        int i;
                        for (i = 0; i < ARRAY_LEN(argarr); ++i) {
                            argarr[i] = *(++imagePtr);
                        }
                        r = vm->systemCall( argarr );
                    } else {
                        intptr_t* argptr = (intptr_t *)&image[ programStack + 4 ];
                        r = vm->systemCall( argptr );
                    }
                }

#ifdef DEBUG_VM
                // this is just our stack frame pointer, only needed
                // for debugging
                *(int *)&image[ programStack + 4 ] = stomped;
#endif

                // save return value
                opStackOfs++;
                opStack[opStackOfs] = r;
                programCounter = *(int *)&image[ programStack ];
//              vm->callLevel = temp;
#ifdef DEBUG_VM
                if ( vm_debugLevel ) {
                    Com_Printf( "%s<--- %s\n", DEBUGSTR, VM_ValueToSymbol( vm, programCounter ) );
                }
#endif
            } else if ( (unsigned)programCounter >= vm->instructionCount ) {
                Com_Error(-1, "VM program counter out of range in OP_CALL" );
                return 0;
            } else {
                programCounter = vm->instructionPointers[ programCounter ];
            }
            goto nextInstruction;

        // push and pop are only needed for discarded or bad function return values
        case OP_PUSH:
            opStackOfs++;
            goto nextInstruction;
        case OP_POP:
            opStackOfs--;
            goto nextInstruction;

        case OP_ENTER:
#ifdef DEBUG_VM
            profileSymbol = VM_ValueToFunctionSymbol( vm, programCounter );
#endif
            // get size of stack frame
            v1 = r2;

            programCounter += 1;
            programStack -= v1;
#ifdef DEBUG_VM
            // save old stack frame for debugging traces
            *(int *)&image[programStack+4] = programStack + v1;
            if ( vm_debugLevel ) {
                Com_Printf( "%s---> %s\n", DEBUGSTR, VM_ValueToSymbol( vm, programCounter - 5 ) );
                if ( vm->breakFunction && programCounter - 5 == vm->breakFunction ) {
                    // this is to allow setting breakpoints here in the debugger
                    vm->breakCount++;
//                  vm_debugLevel = 2;
//                  VM_StackTrace( vm, programCounter, programStack );
                }
//              vm->callLevel++;
            }
#endif
            goto nextInstruction;
        case OP_LEAVE:
            // remove our stack frame
            v1 = r2;

            programStack += v1;

            // grab the saved program counter
            programCounter = *(int *)&image[ programStack ];
#ifdef DEBUG_VM
            profileSymbol = VM_ValueToFunctionSymbol( vm, programCounter );
            if ( vm_debugLevel ) {
//              vm->callLevel--;
                Com_Printf( "%s<--- %s\n", DEBUGSTR, VM_ValueToSymbol( vm, programCounter ) );
            }
#endif
            // check for leaving the VM
            if ( programCounter == -1 ) {
                goto done;
            } else if ( (unsigned)programCounter >= vm->codeLength ) {
                Com_Error(-1, "VM program counter out of range in OP_LEAVE" );
                return 0;
            }
            goto nextInstruction;

        /*
        ===================================================================
        BRANCHES
        ===================================================================
        */

        case OP_JUMP:
            if ( (unsigned)r0 >= vm->instructionCount )
            {
                Com_Error(-1, "VM program counter out of range in OP_JUMP" );
                return 0;
            }

            programCounter = vm->instructionPointers[ r0 ];

            opStackOfs--;
            goto nextInstruction;

        case OP_EQ:
            opStackOfs -= 2;
            if ( r1 == r0 ) {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_NE:
            opStackOfs -= 2;
            if ( r1 != r0 ) {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_LTI:
            opStackOfs -= 2;
            if ( r1 < r0 ) {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_LEI:
            opStackOfs -= 2;
            if ( r1 <= r0 ) {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_GTI:
            opStackOfs -= 2;
            if ( r1 > r0 ) {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_GEI:
            opStackOfs -= 2;
            if ( r1 >= r0 ) {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_LTU:
            opStackOfs -= 2;
            if ( ((unsigned)r1) < ((unsigned)r0) ) {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_LEU:
            opStackOfs -= 2;
            if ( ((unsigned)r1) <= ((unsigned)r0) ) {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_GTU:
            opStackOfs -= 2;
            if ( ((unsigned)r1) > ((unsigned)r0) ) {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_GEU:
            opStackOfs -= 2;
            if ( ((unsigned)r1) >= ((unsigned)r0) ) {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_EQF:
            opStackOfs -= 2;

            if(((float *) opStack)[(uint8_t) (opStackOfs + 1)] == ((float *) opStack)[(uint8_t) (opStackOfs + 2)])
            {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_NEF:
            opStackOfs -= 2;

            if(((float *) opStack)[(uint8_t) (opStackOfs + 1)] != ((float *) opStack)[(uint8_t) (opStackOfs + 2)])
            {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_LTF:
            opStackOfs -= 2;

            if(((float *) opStack)[(uint8_t) (opStackOfs + 1)] < ((float *) opStack)[(uint8_t) (opStackOfs + 2)])
            {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_LEF:
            opStackOfs -= 2;

            if(((float *) opStack)[(uint8_t) ((uint8_t) (opStackOfs + 1))] <= ((float *) opStack)[(uint8_t) ((uint8_t) (opStackOfs + 2))])
            {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_GTF:
            opStackOfs -= 2;

            if(((float *) opStack)[(uint8_t) (opStackOfs + 1)] > ((float *) opStack)[(uint8_t) (opStackOfs + 2)])
            {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }

        case OP_GEF:
            opStackOfs -= 2;

            if(((float *) opStack)[(uint8_t) (opStackOfs + 1)] >= ((float *) opStack)[(uint8_t) (opStackOfs + 2)])
            {
                programCounter = r2;    //vm->instructionPointers[r2];
                goto nextInstruction;
            } else {
                programCounter += 1;
                goto nextInstruction;
            }


        //===================================================================

        case OP_NEGI:
            opStack[opStackOfs] = -r0;
            goto nextInstruction;
        case OP_ADD:
            opStackOfs--;
            opStack[opStackOfs] = r1 + r0;
            goto nextInstruction;
        case OP_SUB:
            opStackOfs--;
            opStack[opStackOfs] = r1 - r0;
            goto nextInstruction;
        case OP_DIVI:
            opStackOfs--;
            opStack[opStackOfs] = r1 / r0;
            goto nextInstruction;
        case OP_DIVU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned) r1) / ((unsigned) r0);
            goto nextInstruction;
        case OP_MODI:
            opStackOfs--;
            opStack[opStackOfs] = r1 % r0;
            goto nextInstruction;
        case OP_MODU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned) r1) % ((unsigned) r0);
            goto nextInstruction;
        case OP_MULI:
            opStackOfs--;
            opStack[opStackOfs] = r1 * r0;
            goto nextInstruction;
        case OP_MULU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned) r1) * ((unsigned) r0);
            goto nextInstruction;

        case OP_BAND:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned) r1) & ((unsigned) r0);
            goto nextInstruction;
        case OP_BOR:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned) r1) | ((unsigned) r0);
            goto nextInstruction;
        case OP_BXOR:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned) r1) ^ ((unsigned) r0);
            goto nextInstruction;
        case OP_BCOM:
            opStack[opStackOfs] = ~((unsigned) r0);
            goto nextInstruction;

        case OP_LSH:
            opStackOfs--;
            opStack[opStackOfs] = r1 << r0;
            goto nextInstruction;
        case OP_RSHI:
            opStackOfs--;
            opStack[opStackOfs] = r1 >> r0;
            goto nextInstruction;
        case OP_RSHU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned) r1) >> r0;
            goto nextInstruction;

        case OP_NEGF:
            ((float *) opStack)[opStackOfs] =  -((float *) opStack)[opStackOfs];
            goto nextInstruction;
        case OP_ADDF:
            opStackOfs--;
            ((float *) opStack)[opStackOfs] = ((float *) opStack)[opStackOfs] + ((float *) opStack)[(uint8_t) (opStackOfs + 1)];
            goto nextInstruction;
        case OP_SUBF:
            opStackOfs--;
            ((float *) opStack)[opStackOfs] = ((float *) opStack)[opStackOfs] - ((float *) opStack)[(uint8_t) (opStackOfs + 1)];
            goto nextInstruction;
        case OP_DIVF:
            opStackOfs--;
            ((float *) opStack)[opStackOfs] = ((float *) opStack)[opStackOfs] / ((float *) opStack)[(uint8_t) (opStackOfs + 1)];
            goto nextInstruction;
        case OP_MULF:
            opStackOfs--;
            ((float *) opStack)[opStackOfs] = ((float *) opStack)[opStackOfs] * ((float *) opStack)[(uint8_t) (opStackOfs + 1)];
            goto nextInstruction;

        case OP_CVIF:
            ((float *) opStack)[opStackOfs] = (float) opStack[opStackOfs];
            goto nextInstruction;
        case OP_CVFI:
            opStack[opStackOfs] = Q_ftol(((float *) opStack)[opStackOfs]);
            goto nextInstruction;
        case OP_SEX8:
            opStack[opStackOfs] = (signed char) opStack[opStackOfs];
            goto nextInstruction;
        case OP_SEX16:
            opStack[opStackOfs] = (short) opStack[opStackOfs];
            goto nextInstruction;
        }
    }

done:
    vm->currentlyInterpreting = qfalse;

	if (opStackOfs != 1 || *opStack != 0xDEADBEEF)
	{
		Com_Error(-1, "Interpreter stack error");
	}

    vm->programStack = stackOnEntry;

    // return the result
    return opStack[opStackOfs];
}

