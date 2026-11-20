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
{ "BREAK", OP_BREAK },

{ "CNSTF4", OP_CONST },
{ "CNSTI4", OP_CONST },
{ "CNSTP4", OP_CONST },
{ "CNSTU4", OP_CONST },

{ "CNSTI2", OP_CONST },
{ "CNSTU2", OP_CONST },

{ "CNSTI1", OP_CONST },
{ "CNSTU1", OP_CONST },

//{ "ARGB", OP_ARG },
//{ "ARGF", OP_ARG },
//{ "ARGI", OP_ARG },
//{ "ARGP", OP_ARG },
//{ "ARGU", OP_ARG },

{ "ASGNB",  OP_BLOCK_COPY },
{ "ASGNF4", OP_STORE4 },
{ "ASGNI4", OP_STORE4 },
{ "ASGNP4", OP_STORE4 },
{ "ASGNU4", OP_STORE4 },

{ "ASGNI2", OP_STORE2 },
{ "ASGNU2", OP_STORE2 },

{ "ASGNI1", OP_STORE1 },
{ "ASGNU1", OP_STORE1 },

{ "INDIRB", OP_IGNORE },    // block copy deals with this
{ "INDIRF4", OP_LOAD4 },
{ "INDIRI4", OP_LOAD4 },
{ "INDIRP4", OP_LOAD4 },
{ "INDIRU4", OP_LOAD4 },

{ "INDIRI2", OP_LOAD2 },
{ "INDIRU2", OP_LOAD2 },

{ "INDIRI1", OP_LOAD1 },
{ "INDIRU1", OP_LOAD1 },

// conversions (CV...) depend on the size operand, see AssembleConversion

{ "NEGF4", OP_NEGF },
{ "NEGI4", OP_NEGI },

//{ "CALLB", OP_UNDEF },
//{ "CALLF", OP_UNDEF },
//{ "CALLI", OP_UNDEF },
//{ "CALLP", OP_UNDEF },
//{ "CALLU", OP_UNDEF },
//{ "CALLV", OP_CALL },

//{ "RETF", OP_UNDEF },
//{ "RETI", OP_UNDEF },
//{ "RETP", OP_UNDEF },
//{ "RETU", OP_UNDEF },
//{ "RETV", OP_UNDEF },

{ "ADDRGP4", OP_CONST },

//{ "ADDRFP", OP_PARM },
//{ "ADDRLP", OP_LOCAL },

{ "ADDF4", OP_ADDF },
{ "ADDI4", OP_ADD },
{ "ADDP4", OP_ADD },
{ "ADDP", OP_ADD },
{ "ADDU4", OP_ADD },

{ "SUBF4", OP_SUBF },
{ "SUBI4", OP_SUB },
{ "SUBP4", OP_SUB },
{ "SUBU4", OP_SUB },

{ "LSHI4", OP_LSH },
{ "LSHU4", OP_LSH },

{ "MODI4", OP_MODI },
{ "MODU4", OP_MODU },

{ "RSHI4", OP_RSHI },
{ "RSHU4", OP_RSHU },

{ "BANDI4", OP_BAND },
{ "BANDU4", OP_BAND },

{ "BCOMI4", OP_BCOM },
{ "BCOMU4", OP_BCOM },

{ "BORI4", OP_BOR },
{ "BORU4", OP_BOR },

{ "BXORI4", OP_BXOR },
{ "BXORU4", OP_BXOR },

{ "DIVF4", OP_DIVF },
{ "DIVI4", OP_DIVI },
{ "DIVU4", OP_DIVU },

{ "MULF4", OP_MULF },
{ "MULI4", OP_MULI },
{ "MULU4", OP_MULU },

{ "EQF4", OP_EQF },
{ "EQI4", OP_EQ },
{ "EQU4", OP_EQ },

{ "GEF4", OP_GEF },
{ "GEI4", OP_GEI },
{ "GEU4", OP_GEU },

{ "GTF4", OP_GTF },
{ "GTI4", OP_GTI },
{ "GTU4", OP_GTU },

{ "LEF4", OP_LEF },
{ "LEI4", OP_LEI },
{ "LEU4", OP_LEU },

{ "LTF4", OP_LTF },
{ "LTI4", OP_LTI },
{ "LTU4", OP_LTU },

{ "NEF4", OP_NEF },
{ "NEI4", OP_NE },
{ "NEU4", OP_NE },

{ "CNSTI8", OP_CONST8 },
{ "CNSTU8", OP_CONST8 },
{ "CNSTF8", OP_CONST8 },

{ "ASGNI8", OP_STORE8 },
{ "ASGNU8", OP_STORE8 },
{ "ASGNF8", OP_STORE8 },

{ "INDIRI8", OP_LOAD8 },
{ "INDIRU8", OP_LOAD8 },
{ "INDIRF8", OP_LOAD8 },

{ "ADDI8", OP_ADD8 },
{ "ADDU8", OP_ADD8 },
{ "SUBI8", OP_SUB8 },
{ "SUBU8", OP_SUB8 },
{ "MULI8", OP_MUL8 },
{ "MULU8", OP_MUL8 },
{ "DIVI8", OP_DIVI8 },
{ "DIVU8", OP_DIVU8 },
{ "MODI8", OP_MODI8 },
{ "MODU8", OP_MODU8 },

{ "BANDI8", OP_BAND8 },
{ "BANDU8", OP_BAND8 },
{ "BORI8", OP_BOR8 },
{ "BORU8", OP_BOR8 },
{ "BXORI8", OP_BXOR8 },
{ "BXORU8", OP_BXOR8 },
{ "BCOMI8", OP_BCOM8 },
{ "BCOMU8", OP_BCOM8 },
{ "LSHI8", OP_LSH8 },
{ "LSHU8", OP_LSH8 },
{ "RSHI8", OP_RSHI8 },
{ "RSHU8", OP_RSHU8 },
{ "NEGI8", OP_NEGI8 },
{ "NEGU8", OP_NEGI8 },

{ "ADDF8", OP_ADDD },
{ "SUBF8", OP_SUBD },
{ "MULF8", OP_MULD },
{ "DIVF8", OP_DIVD },
{ "NEGF8", OP_NEGD },

{ "EQI8", OP_EQ8 },
{ "EQU8", OP_EQ8 },
{ "NEI8", OP_NE8 },
{ "NEU8", OP_NE8 },
{ "LTI8", OP_LTI8 },
{ "LEI8", OP_LEI8 },
{ "GTI8", OP_GTI8 },
{ "GEI8", OP_GEI8 },
{ "LTU8", OP_LTU8 },
{ "LEU8", OP_LEU8 },
{ "GTU8", OP_GTU8 },
{ "GEU8", OP_GEU8 },

{ "EQF8", OP_EQD },
{ "NEF8", OP_NED },
{ "LTF8", OP_LTD },
{ "LEF8", OP_LED },
{ "GTF8", OP_GTD },
{ "GEF8", OP_GED },

{ "JUMPV", OP_JUMP },

{ "LOADB4", OP_UNDEF },
{ "LOADF4", OP_UNDEF },
{ "LOADI4", OP_UNDEF },
{ "LOADP4", OP_UNDEF },
{ "LOADU4", OP_UNDEF },


