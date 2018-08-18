/*
Triseism - Standlone interpreter of Quake III Virtual Machine
Copyright 2003  PhaethonH <phaethon@linux.ucla.edu>

Permission granted to copy, modify, distribute, or otherwise use this code,
provided this copyright notice remains intact
*/

/*
  vm mantle: Q3VM system traps (syscalls)
*/


/*
TODO: These are g-syscalls.  Need some mechanism to support cg-syscalls, ui-syscalls, and randomfoo-syscalls.
*/


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "q3vmtrap.h"


#define TRAP(name) trap_##name
#define PARM (q3vm_t *self)

#define ARG(n) (q3vm_get_I4(self, self->RP + ((2 + n) * sizeof(vmword))))


TRAP(Print) PARM
{
  char *text;

//crumb("SYSCALL Print [%d]\n", ARG(0)); 
  text = (char*)(self->ram) + ARG(0);
//crumb("PRINTING [%s]\n", text);
  printf("%s", text);
  return 0;
}


TRAP(Error) PARM
{
  char *msg;

  msg = (char*)(self->ram) + ARG(0);
  printf("%s", msg);
  self->PC = self->romlen + 1;
  return 0;
}


/* int trap_Milliseconds( void ) */
/* Number of milliseconds since Quake 3 started running ... um ... */
TRAP(Milliseconds) PARM
{
  return 0;
}


/* void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) */
/* Notify engine that the VM memory contains a vmCvar_t struct with which the engine updates and synchronises. */
TRAP(Cvar_Register) PARM
{
  return 0;
}


/* void    trap_Cvar_Update( vmCvar_t *cvar ) */
/* Forceful synchronization of VM vmCvar_t with engine cvar. */
TRAP(Cvar_Update) PARM
{
  return 0;
}


/* void trap_Cvar_Set( const char *var_name, const char *value ) */
/* Set the value of a cvar. */
TRAP(Cvar_Set) PARM
{
  return 0;
}


/* int trap_Cvar_VariableIntegerValue( const char *var_name ) */
/* Get cvar value as an integer. */
TRAP(Cvar_VariableIntegerValue) PARM
{
  return 0;
}


/* void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) */
/* Get cvar value as a string. */
TRAP(Cvar_VariableStringBuffer) PARM
{
  return 0;
}

/* int     trap_Argc( void ) */
/* Number of arguments to a ConsoleCommand or ConsoleCommand. */
TRAP(Argc) PARM
{
  return 0;
}


/* void    trap_Argv( int n, char *buffer, int bufferLength ) */
/* Get an argument from a ConsoleCommand or ConsoleCommand. */
/* 0 is the command itself */
TRAP(Argv) PARM
{
  return 0;
}


/* int     trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) */
/* Open a file for I/O. */
TRAP(FS_FOpenFile) PARM
{
  return 0;
}


/* void    trap_FS_Read( void *buffer, int len, fileHandle_t f ) */
/* Read from file. */
TRAP(FS_Read) PARM
{
  return 0;
}


/* void    trap_FS_Write( const void *buffer, int len, fileHandle_t f ) */
/* Write to file. */
TRAP(FS_Write) PARM
{
  return 0;
}


/* void    trap_FS_FCloseFile( fileHandle_t f ) */
/* Close a file. */
TRAP(FS_FCloseFile) PARM
{
  return 0;
}


/* int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) */
/* Console command (admin/rcon command) */
TRAP(SendConsoleCommand) PARM
{
  return 0;
}


/* void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_ t, playerState_t *clients, int sizeofGClient ) */
/* This syscall notifies the engine where in VM memory the data for g_entities and g_clients exists. */
/* Because the engine does not have specific detailed knowledge of the VM
memory layout, it cannot determine every field of a certain block of VM memory.
The VM and the engine shares the struct layout (offset and sequence of fields)
of engine-specific structs (e.g. entityState_t and playerState_t) through q_*
files, and the engine expects the VM to place these shared structs layouts at
the beginnings of gentity_t and gclient_t, for simplification. */
TRAP(LocateGameData) PARM
{
  void *entities, *clients;

  entities = (void*)((char*)(self->ram) + ARG(0));
  clients = (void*)((char*)(self->ram) + ARG(3));

  return 0;
}


/* void trap_DropClient( int clientNum, const char *reason ) */
/* Remove client from game.  This syscall automagically triggers a call to
vmMain(GAME_CLIENT_DISCONNECT, ...) */
TRAP(DropClient) PARM
{
  return 0;
}


/* void trap_SendServerCommand( int clientNum, const char *text ) */
/* Send a "server command" to client (usually as a way of notifying the client
of data that otherwise doesn't fit in network packets or configstrings, e.g.
chat texts, scores, player locations, event messages) */
TRAP(SendServerCommand) PARM
{
  return 0;
}


/* void trap_SetConfigstring( int num, const char *string ) */
/* Set the value of a configstring (max `num' of 1023) */
TRAP(SetConfigstring) PARM
{
  return 0;
}


/* void trap_GetConfigstring( int num, char *buffer, int bufferSize ) */
/* Get value of a configstring. */
TRAP(GetConfigstring) PARM
{
  return 0;
}


/* void trap_GetUserinfo( int num, char *buffer, int bufferSize ) */
/* Get userinfo for a client.  This is data sent to server from client about
stuff like player name, selected model, railtrail color. */
TRAP(GetUserinfo) PARM
{
  return 0;
}


/* void trap_SetUserinfo( int num, const char *buffer ) */
/* Sets userinfo string.  This gets sent back to the client, and can be treated
as a sort of confirmation messsage. */
TRAP(SetUserinfo) PARM
{
  return 0;
}


/* void trap_GetServerinfo( char *buffer, int bufferSize ) */
/* Get server info.  This is also configstring 1. */
TRAP(GetServerinfo) PARM
{
  return 0;
}


/* void trap_SetBrushModel( gentity_t *ent, const char *name ) */
/* huh? */
TRAP(SetBrushModel) PARM
{
  return 0;
}


/* void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) */
/* Trace/collision-detection. */
TRAP(Trace) PARM
{
  return 0;
}


/* int trap_PointContents( const vec3_t point, int passEntityNum ) */
/* Contents flags for this particular point in the world, ignoring entity number `passEntityNum'. */
TRAP(PointContents) PARM
{
  return 0;
}


/* qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 ) */
/* Check if point `p2' exists in the PVS (Potential Visibility Set) of point `p1'. */
/* this can interpreted as "if point `p2' is, or might be, seen from `p1'" */
TRAP(InPVS) PARM
{
  return 0;
}


/* qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) */
/* Check if point `p2' exists in the PVS of `p1', ignore portals (mirrors, camers). */
TRAP(InPVSIgnorePortals) PARM
{
  return 0;
}


/* void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open )  */
/* Change the state of a portal? */
TRAP(AdjustAreaPortalState) PARM
{
  return 0;
}


/* qboolean trap_AreasConnected( int area1, int area2 )  */
/* Check if areas `area1' and `area2' are connected. */
TRAP(AreasConnected) PARM
{
  return 0;
}


/* void trap_LinkEntity( gentity_t *ent ) */
/*
Link an entity into the world.
Linking an entity:
 * enables collision detection for and with the entity (interacts with others)
 * activates PVS data for entity (it sees others, others see it)
*/
TRAP(LinkEntity) PARM
{
  return 0;
}


/* void trap_UnlinkEntity( gentity_t *ent ) */
/*
Unlink entity from world.
Unlinking an entity:
  * disables collision detection wrt to the entity.
  * disables any PVS data (invisible, and blind).
*/
TRAP(UnlinkEntity) PARM
{
  return 0;
}


/* int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) */
/* Get list of entities that have origins within the specified box, number of entities constrained by `maxcount' */
TRAP(EntitiesInBox) PARM
{
  return 0;
}


/* qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) */
/* Check if two entities touch (collision detection) */
TRAP(EntityContact) PARM
{
  return 0;
}


/* int trap_BotAllocateClient( void ) */
/* allocate space for a Q3 bot. */
TRAP(BotAllocateClient) PARM
{
  return 0;
}


/* void trap_BotFreeClient( int clientNum ) */
/* remove space allocated for Q3 bot. */
TRAP(BotFreeClient) PARM
{
  return 0;
}


/* void trap_GetUsercmd( int clientNum, usercmd_t *cmd ) */
/* Retrieve usercmd from client (movement, weapon, aim) */
TRAP(GetUsercmd) PARM
{
  return 0;
}


/* qboolean trap_GetEntityToken( char *buffer, int bufferSize ) */
/* Read all entity key/value pairs of a map bsp. */
/* returns... ? */
TRAP(GetEntityToken) PARM
{
  return 0;
}


/* int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) */
/* List of files fitting the constraints of `path' and `extension'.  List is
NULL-terminated list of NULL-terminated strings: "foo\0bar\0baz\0\0".  Returns number of matching files. */
TRAP(FS_GetFileList) PARM
{
  return 0;
}


/* int trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points) */
/* Create a debug polygon.  (wtf?) */
/* Returns handle number of debug polygon. */
TRAP(DebugPolygonCreate) PARM
{
  return 0;
}


/* void trap_DebugPolygonDelete(int id) */
/* Delete debug polygon by handle. */
TRAP(DebugPolygonDelete) PARM
{
  return 0;
}


/* int trap_RealTime( qtime_t *qtime ) */
/* Get time_t struct. */
TRAP(RealTime) PARM
{
  return 0;
}


/* void trap_SnapVector( float *v ) */
/* Round each compoennt of `v' to the nearest integer (i.e. make fractional part 0). */
TRAP(SnapVector) PARM
{
  return 0;
}


TRAP(TraceCapsule) PARM
{
  return 0;
}


TRAP(EntityContactCapsule) PARM
{
  return 0;
}


TRAP(FS_Seek) PARM
{
  return 0;
}



/*
 speed-purposes syscalls.
*/

/* void *memset (void *dest, int c, size_t count) */
TRAP(memset) PARM
{
  int arg[3];
  char *dest;
  int c;
  size_t count;

  dest = (char*)(self->ram) + ARG(0);
  c = ARG(1);
  count = ARG(2);
  return 0;
}


/* void *memcpy (void *dest, const void *src, size_t count) */
TRAP(memcpy) PARM
{
  return 0;
}


/* char *strncpy (char *strDest, const char *strSource, size_t count) */
TRAP(strncpy) PARM
{
  char *dst, *src;
  int count;
  int retval;

//crumb("SYSCALL strncpy [%d] <- [%d] + %d\n", ARG(0), ARG(1), ARG(2));
  dst = (char*)(self->ram) + ARG(0);
  src = (char*)(self->ram) + ARG(1);
  count = ARG(2);
  strncpy(dst, src, count);
  retval = dst - (char*)self->ram;
  return retval;
}


/* double sin (double x) */
TRAP(sin) PARM
{
  return 0;
}


/* double cos (double x) */
TRAP(cos) PARM
{
  return 0;
}


/* double atan2 (double x) */
TRAP(atan2) PARM
{
  return 0;
}


/* double sqrt (double x) */
TRAP(sqrt) PARM
{
  return 0;
}


/* double floor (double x) */
TRAP(floor) PARM
{
  return 0;
}


/* double ceil (double x) */
TRAP(ceil) PARM
{
  return 0;
}


/* void testPrintInt (int n) */
TRAP(testPrintInt) PARM
{
  return 0;
}


/* double testPrintFloat (double x) */
TRAP(testPrintFloat) PARM
{
  return 0;
}


/* Screw the bot functions. */




int
q3vm_syscall (q3vm_t *self, int trap)
{
  int retval;
  vmword w;

  retval = 0;
  switch (trap)
    {
#define MAPTRAP(n, f) case n: retval = trap_##f (self); break;
      MAPTRAP(-1, Print)
      MAPTRAP(-2, Error)
      MAPTRAP(-3, Milliseconds)
      MAPTRAP(-4, Cvar_Register)
      MAPTRAP(-5, Cvar_Update)
      MAPTRAP(-6, Cvar_Set)
      MAPTRAP(-7, Cvar_VariableIntegerValue)
      MAPTRAP(-8, Cvar_VariableStringBuffer)
      MAPTRAP(-9, Argc)
      MAPTRAP(-10, Argv)
      MAPTRAP(-11, FS_FOpenFile)
      MAPTRAP(-12, FS_Read)
      MAPTRAP(-13, FS_Write)
      MAPTRAP(-14, FS_FCloseFile)
      MAPTRAP(-15, SendConsoleCommand)
      MAPTRAP(-16, LocateGameData)
      MAPTRAP(-17, DropClient)
      MAPTRAP(-18, SendServerCommand)
      MAPTRAP(-19, SetConfigstring)
      MAPTRAP(-20, GetConfigstring)
      MAPTRAP(-21, GetUserinfo)
      MAPTRAP(-22, SetUserinfo)
      MAPTRAP(-23, GetServerinfo)
      MAPTRAP(-24, SetBrushModel)
      MAPTRAP(-25, Trace)
      MAPTRAP(-26, PointContents)
      MAPTRAP(-27, InPVS)
      MAPTRAP(-28, InPVSIgnorePortals)
      MAPTRAP(-29, AdjustAreaPortalState)
      MAPTRAP(-30, AreasConnected)
      MAPTRAP(-31, LinkEntity)
      MAPTRAP(-32, UnlinkEntity)
      MAPTRAP(-33, EntitiesInBox)
      MAPTRAP(-34, EntityContact)
      MAPTRAP(-35, BotAllocateClient)
      MAPTRAP(-36, BotFreeClient)
      MAPTRAP(-37, GetUsercmd)
      MAPTRAP(-38, GetEntityToken)
      MAPTRAP(-39, FS_GetFileList)
      MAPTRAP(-40, DebugPolygonCreate)
      MAPTRAP(-41, DebugPolygonDelete)
      MAPTRAP(-42, RealTime)
      MAPTRAP(-43, SnapVector)
      MAPTRAP(-44, TraceCapsule)
      MAPTRAP(-45, EntityContactCapsule)
      MAPTRAP(-46, FS_Seek)

      MAPTRAP(-101, memset)
      MAPTRAP(-102, memcpy)
      MAPTRAP(-103, strncpy)
      MAPTRAP(-104, sin)
      MAPTRAP(-105, cos)
      MAPTRAP(-106, atan2)
      MAPTRAP(-107, sqrt)
      MAPTRAP(-111, floor)
      MAPTRAP(-112, ceil)
      MAPTRAP(-113, testPrintInt)
      MAPTRAP(-114, testPrintFloat)
    }

  w = q3vm_drop(self);
  self->PC = w.I4;
  w.I4 = retval;
  q3vm_push(self, w);
  return 1;
}
