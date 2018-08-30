#include "c.h"
typedef Node NODEPTR_TYPE;
#define OP_LABEL(p)     (specific((p)->op))
#define LEFT_CHILD(p)   ((p)->kids[0])
#define RIGHT_CHILD(p)  ((p)->kids[1])
#define STATE_LABEL(p)  ((p)->x.state)
#define PANIC	   error
/*
generated at Thu Aug 30 01:00:52 2018
by lburg.c - faked rcsid
*/
static void _kids(NODEPTR_TYPE, int, NODEPTR_TYPE[]);
static void _label(NODEPTR_TYPE);
static int _rule(void*, int);

#define _stmt_NT 1
#define _P_NT 2
#define _V_NT 3
#define _bogus_NT 4
#define _I_NT 5
#define _U_NT 6
#define _F_NT 7
#define _B_NT 8

#if 0
static char *_ntname[] = {
	0,
	"stmt",
	"P",
	"V",
	"bogus",
	"I",
	"U",
	"F",
	"B",
	0
};
#endif

struct _state {
	short cost[9];
	struct {
		unsigned int _stmt:4;
		unsigned int _P:4;
		unsigned int _V:6;
		unsigned int _bogus:3;
		unsigned int _I:5;
		unsigned int _U:5;
		unsigned int _F:4;
		unsigned int _B:2;
	} rule;
};

static short _nts_0[] = { _P_NT, 0 };
static short _nts_1[] = { _V_NT, 0 };
static short _nts_2[] = { _I_NT, 0 };
static short _nts_3[] = { _U_NT, 0 };
static short _nts_4[] = { _F_NT, 0 };
static short _nts_5[] = { _B_NT, 0 };
static short _nts_6[] = { _bogus_NT, 0 };
static short _nts_7[] = { 0 };
static short _nts_8[] = { _P_NT, _B_NT, 0 };
static short _nts_9[] = { _P_NT, _F_NT, 0 };
static short _nts_10[] = { _P_NT, _I_NT, 0 };
static short _nts_11[] = { _P_NT, _U_NT, 0 };
static short _nts_12[] = { _P_NT, _P_NT, 0 };
static short _nts_13[] = { _F_NT, _F_NT, 0 };
static short _nts_14[] = { _I_NT, _I_NT, 0 };
static short _nts_15[] = { _I_NT, _P_NT, 0 };
static short _nts_16[] = { _U_NT, _P_NT, 0 };
static short _nts_17[] = { _U_NT, _U_NT, 0 };
static short _nts_18[] = { _U_NT, _I_NT, 0 };

static short *_nts[] = {
	0,	/* 0 */
	_nts_0,	/* 1 */
	_nts_0,	/* 2 */
	_nts_0,	/* 3 */
	_nts_0,	/* 4 */
	_nts_0,	/* 5 */
	_nts_0,	/* 6 */
	_nts_0,	/* 7 */
	_nts_0,	/* 8 */
	_nts_0,	/* 9 */
	_nts_1,	/* 10 */
	_nts_2,	/* 11 */
	_nts_3,	/* 12 */
	_nts_0,	/* 13 */
	_nts_4,	/* 14 */
	_nts_5,	/* 15 */
	_nts_1,	/* 16 */
	_nts_6,	/* 17 */
	_nts_6,	/* 18 */
	_nts_6,	/* 19 */
	_nts_6,	/* 20 */
	_nts_6,	/* 21 */
	_nts_6,	/* 22 */
	_nts_7,	/* 23 */
	_nts_7,	/* 24 */
	_nts_7,	/* 25 */
	_nts_7,	/* 26 */
	_nts_5,	/* 27 */
	_nts_4,	/* 28 */
	_nts_2,	/* 29 */
	_nts_3,	/* 30 */
	_nts_0,	/* 31 */
	_nts_8,	/* 32 */
	_nts_9,	/* 33 */
	_nts_10,	/* 34 */
	_nts_11,	/* 35 */
	_nts_12,	/* 36 */
	_nts_0,	/* 37 */
	_nts_0,	/* 38 */
	_nts_0,	/* 39 */
	_nts_0,	/* 40 */
	_nts_0,	/* 41 */
	_nts_2,	/* 42 */
	_nts_3,	/* 43 */
	_nts_4,	/* 44 */
	_nts_2,	/* 45 */
	_nts_3,	/* 46 */
	_nts_0,	/* 47 */
	_nts_2,	/* 48 */
	_nts_4,	/* 49 */
	_nts_3,	/* 50 */
	_nts_0,	/* 51 */
	_nts_4,	/* 52 */
	_nts_2,	/* 53 */
	_nts_12,	/* 54 */
	_nts_0,	/* 55 */
	_nts_0,	/* 56 */
	_nts_0,	/* 57 */
	_nts_0,	/* 58 */
	_nts_0,	/* 59 */
	_nts_4,	/* 60 */
	_nts_2,	/* 61 */
	_nts_3,	/* 62 */
	_nts_0,	/* 63 */
	_nts_7,	/* 64 */
	_nts_7,	/* 65 */
	_nts_7,	/* 66 */
	_nts_7,	/* 67 */
	_nts_13,	/* 68 */
	_nts_14,	/* 69 */
	_nts_10,	/* 70 */
	_nts_15,	/* 71 */
	_nts_16,	/* 72 */
	_nts_11,	/* 73 */
	_nts_17,	/* 74 */
	_nts_13,	/* 75 */
	_nts_14,	/* 76 */
	_nts_10,	/* 77 */
	_nts_11,	/* 78 */
	_nts_17,	/* 79 */
	_nts_14,	/* 80 */
	_nts_18,	/* 81 */
	_nts_14,	/* 82 */
	_nts_17,	/* 83 */
	_nts_14,	/* 84 */
	_nts_18,	/* 85 */
	_nts_17,	/* 86 */
	_nts_14,	/* 87 */
	_nts_3,	/* 88 */
	_nts_2,	/* 89 */
	_nts_14,	/* 90 */
	_nts_17,	/* 91 */
	_nts_17,	/* 92 */
	_nts_14,	/* 93 */
	_nts_13,	/* 94 */
	_nts_14,	/* 95 */
	_nts_17,	/* 96 */
	_nts_13,	/* 97 */
	_nts_14,	/* 98 */
	_nts_17,	/* 99 */
	_nts_13,	/* 100 */
	_nts_14,	/* 101 */
	_nts_17,	/* 102 */
	_nts_13,	/* 103 */
	_nts_14,	/* 104 */
	_nts_17,	/* 105 */
	_nts_13,	/* 106 */
	_nts_14,	/* 107 */
	_nts_17,	/* 108 */
	_nts_13,	/* 109 */
	_nts_14,	/* 110 */
	_nts_17,	/* 111 */
	_nts_13,	/* 112 */
	_nts_14,	/* 113 */
	_nts_17,	/* 114 */
	_nts_13,	/* 115 */
	_nts_14,	/* 116 */
	_nts_17,	/* 117 */
	_nts_0,	/* 118 */
	_nts_7,	/* 119 */
};

#if 0
static char *_templates[] = {
/* 0 */	0,
/* 1 */	"",	/* stmt: INDIRB(P) */
/* 2 */	"",	/* stmt: INDIRF(P) */
/* 3 */	"",	/* stmt: INDIRI(P) */
/* 4 */	"",	/* stmt: INDIRU(P) */
/* 5 */	"",	/* stmt: INDIRP(P) */
/* 6 */	"",	/* stmt: CALLF(P) */
/* 7 */	"",	/* stmt: CALLI(P) */
/* 8 */	"",	/* stmt: CALLU(P) */
/* 9 */	"",	/* stmt: CALLP(P) */
/* 10 */	"",	/* stmt: V */
/* 11 */	"",	/* bogus: I */
/* 12 */	"",	/* bogus: U */
/* 13 */	"",	/* bogus: P */
/* 14 */	"",	/* bogus: F */
/* 15 */	"",	/* bogus: B */
/* 16 */	"",	/* bogus: V */
/* 17 */	"",	/* I: bogus */
/* 18 */	"",	/* U: bogus */
/* 19 */	"",	/* P: bogus */
/* 20 */	"",	/* F: bogus */
/* 21 */	"",	/* B: bogus */
/* 22 */	"",	/* V: bogus */
/* 23 */	"",	/* F: CNSTF */
/* 24 */	"",	/* I: CNSTI */
/* 25 */	"",	/* P: CNSTP */
/* 26 */	"",	/* U: CNSTU */
/* 27 */	"",	/* V: ARGB(B) */
/* 28 */	"",	/* V: ARGF(F) */
/* 29 */	"",	/* V: ARGI(I) */
/* 30 */	"",	/* V: ARGU(U) */
/* 31 */	"",	/* V: ARGP(P) */
/* 32 */	"",	/* V: ASGNB(P,B) */
/* 33 */	"",	/* V: ASGNF(P,F) */
/* 34 */	"",	/* V: ASGNI(P,I) */
/* 35 */	"",	/* V: ASGNU(P,U) */
/* 36 */	"",	/* V: ASGNP(P,P) */
/* 37 */	"",	/* B: INDIRB(P) */
/* 38 */	"",	/* F: INDIRF(P) */
/* 39 */	"",	/* I: INDIRI(P) */
/* 40 */	"",	/* U: INDIRU(P) */
/* 41 */	"",	/* P: INDIRP(P) */
/* 42 */	"",	/* I: CVII(I) */
/* 43 */	"",	/* I: CVUI(U) */
/* 44 */	"",	/* I: CVFI(F) */
/* 45 */	"",	/* U: CVIU(I) */
/* 46 */	"",	/* U: CVUU(U) */
/* 47 */	"",	/* U: CVPU(P) */
/* 48 */	"",	/* F: CVIF(I) */
/* 49 */	"",	/* F: CVFF(F) */
/* 50 */	"",	/* P: CVUP(U) */
/* 51 */	"",	/* P: CVPP(P) */
/* 52 */	"",	/* F: NEGF(F) */
/* 53 */	"",	/* I: NEGI(I) */
/* 54 */	"",	/* V: CALLB(P,P) */
/* 55 */	"",	/* F: CALLF(P) */
/* 56 */	"",	/* I: CALLI(P) */
/* 57 */	"",	/* U: CALLU(P) */
/* 58 */	"",	/* P: CALLP(P) */
/* 59 */	"",	/* V: CALLV(P) */
/* 60 */	"",	/* V: RETF(F) */
/* 61 */	"",	/* V: RETI(I) */
/* 62 */	"",	/* V: RETU(U) */
/* 63 */	"",	/* V: RETP(P) */
/* 64 */	"",	/* V: RETV */
/* 65 */	"",	/* P: ADDRGP */
/* 66 */	"",	/* P: ADDRFP */
/* 67 */	"",	/* P: ADDRLP */
/* 68 */	"",	/* F: ADDF(F,F) */
/* 69 */	"",	/* I: ADDI(I,I) */
/* 70 */	"",	/* P: ADDP(P,I) */
/* 71 */	"",	/* P: ADDP(I,P) */
/* 72 */	"",	/* P: ADDP(U,P) */
/* 73 */	"",	/* P: ADDP(P,U) */
/* 74 */	"",	/* U: ADDU(U,U) */
/* 75 */	"",	/* F: SUBF(F,F) */
/* 76 */	"",	/* I: SUBI(I,I) */
/* 77 */	"",	/* P: SUBP(P,I) */
/* 78 */	"",	/* P: SUBP(P,U) */
/* 79 */	"",	/* U: SUBU(U,U) */
/* 80 */	"",	/* I: LSHI(I,I) */
/* 81 */	"",	/* U: LSHU(U,I) */
/* 82 */	"",	/* I: MODI(I,I) */
/* 83 */	"",	/* U: MODU(U,U) */
/* 84 */	"",	/* I: RSHI(I,I) */
/* 85 */	"",	/* U: RSHU(U,I) */
/* 86 */	"",	/* U: BANDU(U,U) */
/* 87 */	"",	/* I: BANDI(I,I) */
/* 88 */	"",	/* U: BCOMU(U) */
/* 89 */	"",	/* I: BCOMI(I) */
/* 90 */	"",	/* I: BORI(I,I) */
/* 91 */	"",	/* U: BORU(U,U) */
/* 92 */	"",	/* U: BXORU(U,U) */
/* 93 */	"",	/* I: BXORI(I,I) */
/* 94 */	"",	/* F: DIVF(F,F) */
/* 95 */	"",	/* I: DIVI(I,I) */
/* 96 */	"",	/* U: DIVU(U,U) */
/* 97 */	"",	/* F: MULF(F,F) */
/* 98 */	"",	/* I: MULI(I,I) */
/* 99 */	"",	/* U: MULU(U,U) */
/* 100 */	"",	/* V: EQF(F,F) */
/* 101 */	"",	/* V: EQI(I,I) */
/* 102 */	"",	/* V: EQU(U,U) */
/* 103 */	"",	/* V: GEF(F,F) */
/* 104 */	"",	/* V: GEI(I,I) */
/* 105 */	"",	/* V: GEU(U,U) */
/* 106 */	"",	/* V: GTF(F,F) */
/* 107 */	"",	/* V: GTI(I,I) */
/* 108 */	"",	/* V: GTU(U,U) */
/* 109 */	"",	/* V: LEF(F,F) */
/* 110 */	"",	/* V: LEI(I,I) */
/* 111 */	"",	/* V: LEU(U,U) */
/* 112 */	"",	/* V: LTF(F,F) */
/* 113 */	"",	/* V: LTI(I,I) */
/* 114 */	"",	/* V: LTU(U,U) */
/* 115 */	"",	/* V: NEF(F,F) */
/* 116 */	"",	/* V: NEI(I,I) */
/* 117 */	"",	/* V: NEU(U,U) */
/* 118 */	"",	/* V: JUMPV(P) */
/* 119 */	"",	/* V: LABELV */
};

static char _isinstruction[] = {
/* 0 */	0,
/* 1 */	0,	/*  */
/* 2 */	0,	/*  */
/* 3 */	0,	/*  */
/* 4 */	0,	/*  */
/* 5 */	0,	/*  */
/* 6 */	0,	/*  */
/* 7 */	0,	/*  */
/* 8 */	0,	/*  */
/* 9 */	0,	/*  */
/* 10 */	0,	/*  */
/* 11 */	0,	/*  */
/* 12 */	0,	/*  */
/* 13 */	0,	/*  */
/* 14 */	0,	/*  */
/* 15 */	0,	/*  */
/* 16 */	0,	/*  */
/* 17 */	0,	/*  */
/* 18 */	0,	/*  */
/* 19 */	0,	/*  */
/* 20 */	0,	/*  */
/* 21 */	0,	/*  */
/* 22 */	0,	/*  */
/* 23 */	0,	/*  */
/* 24 */	0,	/*  */
/* 25 */	0,	/*  */
/* 26 */	0,	/*  */
/* 27 */	0,	/*  */
/* 28 */	0,	/*  */
/* 29 */	0,	/*  */
/* 30 */	0,	/*  */
/* 31 */	0,	/*  */
/* 32 */	0,	/*  */
/* 33 */	0,	/*  */
/* 34 */	0,	/*  */
/* 35 */	0,	/*  */
/* 36 */	0,	/*  */
/* 37 */	0,	/*  */
/* 38 */	0,	/*  */
/* 39 */	0,	/*  */
/* 40 */	0,	/*  */
/* 41 */	0,	/*  */
/* 42 */	0,	/*  */
/* 43 */	0,	/*  */
/* 44 */	0,	/*  */
/* 45 */	0,	/*  */
/* 46 */	0,	/*  */
/* 47 */	0,	/*  */
/* 48 */	0,	/*  */
/* 49 */	0,	/*  */
/* 50 */	0,	/*  */
/* 51 */	0,	/*  */
/* 52 */	0,	/*  */
/* 53 */	0,	/*  */
/* 54 */	0,	/*  */
/* 55 */	0,	/*  */
/* 56 */	0,	/*  */
/* 57 */	0,	/*  */
/* 58 */	0,	/*  */
/* 59 */	0,	/*  */
/* 60 */	0,	/*  */
/* 61 */	0,	/*  */
/* 62 */	0,	/*  */
/* 63 */	0,	/*  */
/* 64 */	0,	/*  */
/* 65 */	0,	/*  */
/* 66 */	0,	/*  */
/* 67 */	0,	/*  */
/* 68 */	0,	/*  */
/* 69 */	0,	/*  */
/* 70 */	0,	/*  */
/* 71 */	0,	/*  */
/* 72 */	0,	/*  */
/* 73 */	0,	/*  */
/* 74 */	0,	/*  */
/* 75 */	0,	/*  */
/* 76 */	0,	/*  */
/* 77 */	0,	/*  */
/* 78 */	0,	/*  */
/* 79 */	0,	/*  */
/* 80 */	0,	/*  */
/* 81 */	0,	/*  */
/* 82 */	0,	/*  */
/* 83 */	0,	/*  */
/* 84 */	0,	/*  */
/* 85 */	0,	/*  */
/* 86 */	0,	/*  */
/* 87 */	0,	/*  */
/* 88 */	0,	/*  */
/* 89 */	0,	/*  */
/* 90 */	0,	/*  */
/* 91 */	0,	/*  */
/* 92 */	0,	/*  */
/* 93 */	0,	/*  */
/* 94 */	0,	/*  */
/* 95 */	0,	/*  */
/* 96 */	0,	/*  */
/* 97 */	0,	/*  */
/* 98 */	0,	/*  */
/* 99 */	0,	/*  */
/* 100 */	0,	/*  */
/* 101 */	0,	/*  */
/* 102 */	0,	/*  */
/* 103 */	0,	/*  */
/* 104 */	0,	/*  */
/* 105 */	0,	/*  */
/* 106 */	0,	/*  */
/* 107 */	0,	/*  */
/* 108 */	0,	/*  */
/* 109 */	0,	/*  */
/* 110 */	0,	/*  */
/* 111 */	0,	/*  */
/* 112 */	0,	/*  */
/* 113 */	0,	/*  */
/* 114 */	0,	/*  */
/* 115 */	0,	/*  */
/* 116 */	0,	/*  */
/* 117 */	0,	/*  */
/* 118 */	0,	/*  */
/* 119 */	0,	/*  */
};

static char *_string[] = {
/* 0 */	0,
/* 1 */	"stmt: INDIRB(P)",
/* 2 */	"stmt: INDIRF(P)",
/* 3 */	"stmt: INDIRI(P)",
/* 4 */	"stmt: INDIRU(P)",
/* 5 */	"stmt: INDIRP(P)",
/* 6 */	"stmt: CALLF(P)",
/* 7 */	"stmt: CALLI(P)",
/* 8 */	"stmt: CALLU(P)",
/* 9 */	"stmt: CALLP(P)",
/* 10 */	"stmt: V",
/* 11 */	"bogus: I",
/* 12 */	"bogus: U",
/* 13 */	"bogus: P",
/* 14 */	"bogus: F",
/* 15 */	"bogus: B",
/* 16 */	"bogus: V",
/* 17 */	"I: bogus",
/* 18 */	"U: bogus",
/* 19 */	"P: bogus",
/* 20 */	"F: bogus",
/* 21 */	"B: bogus",
/* 22 */	"V: bogus",
/* 23 */	"F: CNSTF",
/* 24 */	"I: CNSTI",
/* 25 */	"P: CNSTP",
/* 26 */	"U: CNSTU",
/* 27 */	"V: ARGB(B)",
/* 28 */	"V: ARGF(F)",
/* 29 */	"V: ARGI(I)",
/* 30 */	"V: ARGU(U)",
/* 31 */	"V: ARGP(P)",
/* 32 */	"V: ASGNB(P,B)",
/* 33 */	"V: ASGNF(P,F)",
/* 34 */	"V: ASGNI(P,I)",
/* 35 */	"V: ASGNU(P,U)",
/* 36 */	"V: ASGNP(P,P)",
/* 37 */	"B: INDIRB(P)",
/* 38 */	"F: INDIRF(P)",
/* 39 */	"I: INDIRI(P)",
/* 40 */	"U: INDIRU(P)",
/* 41 */	"P: INDIRP(P)",
/* 42 */	"I: CVII(I)",
/* 43 */	"I: CVUI(U)",
/* 44 */	"I: CVFI(F)",
/* 45 */	"U: CVIU(I)",
/* 46 */	"U: CVUU(U)",
/* 47 */	"U: CVPU(P)",
/* 48 */	"F: CVIF(I)",
/* 49 */	"F: CVFF(F)",
/* 50 */	"P: CVUP(U)",
/* 51 */	"P: CVPP(P)",
/* 52 */	"F: NEGF(F)",
/* 53 */	"I: NEGI(I)",
/* 54 */	"V: CALLB(P,P)",
/* 55 */	"F: CALLF(P)",
/* 56 */	"I: CALLI(P)",
/* 57 */	"U: CALLU(P)",
/* 58 */	"P: CALLP(P)",
/* 59 */	"V: CALLV(P)",
/* 60 */	"V: RETF(F)",
/* 61 */	"V: RETI(I)",
/* 62 */	"V: RETU(U)",
/* 63 */	"V: RETP(P)",
/* 64 */	"V: RETV",
/* 65 */	"P: ADDRGP",
/* 66 */	"P: ADDRFP",
/* 67 */	"P: ADDRLP",
/* 68 */	"F: ADDF(F,F)",
/* 69 */	"I: ADDI(I,I)",
/* 70 */	"P: ADDP(P,I)",
/* 71 */	"P: ADDP(I,P)",
/* 72 */	"P: ADDP(U,P)",
/* 73 */	"P: ADDP(P,U)",
/* 74 */	"U: ADDU(U,U)",
/* 75 */	"F: SUBF(F,F)",
/* 76 */	"I: SUBI(I,I)",
/* 77 */	"P: SUBP(P,I)",
/* 78 */	"P: SUBP(P,U)",
/* 79 */	"U: SUBU(U,U)",
/* 80 */	"I: LSHI(I,I)",
/* 81 */	"U: LSHU(U,I)",
/* 82 */	"I: MODI(I,I)",
/* 83 */	"U: MODU(U,U)",
/* 84 */	"I: RSHI(I,I)",
/* 85 */	"U: RSHU(U,I)",
/* 86 */	"U: BANDU(U,U)",
/* 87 */	"I: BANDI(I,I)",
/* 88 */	"U: BCOMU(U)",
/* 89 */	"I: BCOMI(I)",
/* 90 */	"I: BORI(I,I)",
/* 91 */	"U: BORU(U,U)",
/* 92 */	"U: BXORU(U,U)",
/* 93 */	"I: BXORI(I,I)",
/* 94 */	"F: DIVF(F,F)",
/* 95 */	"I: DIVI(I,I)",
/* 96 */	"U: DIVU(U,U)",
/* 97 */	"F: MULF(F,F)",
/* 98 */	"I: MULI(I,I)",
/* 99 */	"U: MULU(U,U)",
/* 100 */	"V: EQF(F,F)",
/* 101 */	"V: EQI(I,I)",
/* 102 */	"V: EQU(U,U)",
/* 103 */	"V: GEF(F,F)",
/* 104 */	"V: GEI(I,I)",
/* 105 */	"V: GEU(U,U)",
/* 106 */	"V: GTF(F,F)",
/* 107 */	"V: GTI(I,I)",
/* 108 */	"V: GTU(U,U)",
/* 109 */	"V: LEF(F,F)",
/* 110 */	"V: LEI(I,I)",
/* 111 */	"V: LEU(U,U)",
/* 112 */	"V: LTF(F,F)",
/* 113 */	"V: LTI(I,I)",
/* 114 */	"V: LTU(U,U)",
/* 115 */	"V: NEF(F,F)",
/* 116 */	"V: NEI(I,I)",
/* 117 */	"V: NEU(U,U)",
/* 118 */	"V: JUMPV(P)",
/* 119 */	"V: LABELV",
};
#endif

static short _decode_stmt[] = {
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
};

static short _decode_P[] = {
	0,
	19,
	25,
	41,
	50,
	51,
	58,
	65,
	66,
	67,
	70,
	71,
	72,
	73,
	77,
	78,
};

static short _decode_V[] = {
	0,
	22,
	27,
	28,
	29,
	30,
	31,
	32,
	33,
	34,
	35,
	36,
	54,
	59,
	60,
	61,
	62,
	63,
	64,
	100,
	101,
	102,
	103,
	104,
	105,
	106,
	107,
	108,
	109,
	110,
	111,
	112,
	113,
	114,
	115,
	116,
	117,
	118,
	119,
};

static short _decode_bogus[] = {
	0,
	11,
	12,
	13,
	14,
	15,
	16,
};

static short _decode_I[] = {
	0,
	17,
	24,
	39,
	42,
	43,
	44,
	53,
	56,
	69,
	76,
	80,
	82,
	84,
	87,
	89,
	90,
	93,
	95,
	98,
};

static short _decode_U[] = {
	0,
	18,
	26,
	40,
	45,
	46,
	47,
	57,
	74,
	79,
	81,
	83,
	85,
	86,
	88,
	91,
	92,
	96,
	99,
};

static short _decode_F[] = {
	0,
	20,
	23,
	38,
	48,
	49,
	52,
	55,
	68,
	75,
	94,
	97,
};

static short _decode_B[] = {
	0,
	21,
	37,
};

static int _rule(void *state, int goalnt) {
	if (goalnt < 1 || goalnt > 8)
		fatal("_rule", "Bad goal nonterminal %d\n", goalnt);
	if (!state)
		return 0;
	switch (goalnt) {
	case _stmt_NT:	return _decode_stmt[((struct _state *)state)->rule._stmt];
	case _P_NT:	return _decode_P[((struct _state *)state)->rule._P];
	case _V_NT:	return _decode_V[((struct _state *)state)->rule._V];
	case _bogus_NT:	return _decode_bogus[((struct _state *)state)->rule._bogus];
	case _I_NT:	return _decode_I[((struct _state *)state)->rule._I];
	case _U_NT:	return _decode_U[((struct _state *)state)->rule._U];
	case _F_NT:	return _decode_F[((struct _state *)state)->rule._F];
	case _B_NT:	return _decode_B[((struct _state *)state)->rule._B];
	default:
		fatal("_rule", "Bad goal nonterminal %d\n", goalnt);
		return 0;
	}
}

static void _closure_P(NODEPTR_TYPE, int);
static void _closure_V(NODEPTR_TYPE, int);
static void _closure_bogus(NODEPTR_TYPE, int);
static void _closure_I(NODEPTR_TYPE, int);
static void _closure_U(NODEPTR_TYPE, int);
static void _closure_F(NODEPTR_TYPE, int);
static void _closure_B(NODEPTR_TYPE, int);

static void _closure_P(NODEPTR_TYPE a, int c) {
	struct _state *p = STATE_LABEL(a);
	if (c + 1 < p->cost[_bogus_NT]) {
		p->cost[_bogus_NT] = c + 1;
		p->rule._bogus = 3;
		_closure_bogus(a, c + 1);
	}
}

static void _closure_V(NODEPTR_TYPE a, int c) {
	struct _state *p = STATE_LABEL(a);
	if (c + 1 < p->cost[_bogus_NT]) {
		p->cost[_bogus_NT] = c + 1;
		p->rule._bogus = 6;
		_closure_bogus(a, c + 1);
	}
	if (c + 0 < p->cost[_stmt_NT]) {
		p->cost[_stmt_NT] = c + 0;
		p->rule._stmt = 10;
	}
}

static void _closure_bogus(NODEPTR_TYPE a, int c) {
	struct _state *p = STATE_LABEL(a);
	if (c + 1 < p->cost[_V_NT]) {
		p->cost[_V_NT] = c + 1;
		p->rule._V = 1;
		_closure_V(a, c + 1);
	}
	if (c + 1 < p->cost[_B_NT]) {
		p->cost[_B_NT] = c + 1;
		p->rule._B = 1;
		_closure_B(a, c + 1);
	}
	if (c + 1 < p->cost[_F_NT]) {
		p->cost[_F_NT] = c + 1;
		p->rule._F = 1;
		_closure_F(a, c + 1);
	}
	if (c + 1 < p->cost[_P_NT]) {
		p->cost[_P_NT] = c + 1;
		p->rule._P = 1;
		_closure_P(a, c + 1);
	}
	if (c + 1 < p->cost[_U_NT]) {
		p->cost[_U_NT] = c + 1;
		p->rule._U = 1;
		_closure_U(a, c + 1);
	}
	if (c + 1 < p->cost[_I_NT]) {
		p->cost[_I_NT] = c + 1;
		p->rule._I = 1;
		_closure_I(a, c + 1);
	}
}

static void _closure_I(NODEPTR_TYPE a, int c) {
	struct _state *p = STATE_LABEL(a);
	if (c + 1 < p->cost[_bogus_NT]) {
		p->cost[_bogus_NT] = c + 1;
		p->rule._bogus = 1;
		_closure_bogus(a, c + 1);
	}
}

static void _closure_U(NODEPTR_TYPE a, int c) {
	struct _state *p = STATE_LABEL(a);
	if (c + 1 < p->cost[_bogus_NT]) {
		p->cost[_bogus_NT] = c + 1;
		p->rule._bogus = 2;
		_closure_bogus(a, c + 1);
	}
}

static void _closure_F(NODEPTR_TYPE a, int c) {
	struct _state *p = STATE_LABEL(a);
	if (c + 1 < p->cost[_bogus_NT]) {
		p->cost[_bogus_NT] = c + 1;
		p->rule._bogus = 4;
		_closure_bogus(a, c + 1);
	}
}

static void _closure_B(NODEPTR_TYPE a, int c) {
	struct _state *p = STATE_LABEL(a);
	if (c + 1 < p->cost[_bogus_NT]) {
		p->cost[_bogus_NT] = c + 1;
		p->rule._bogus = 5;
		_closure_bogus(a, c + 1);
	}
}

static void _label(NODEPTR_TYPE a) {
	int c;
	struct _state *p;

	if (!a)
		fatal("_label", "Null tree\n", 0);
	STATE_LABEL(a) = p = allocate(sizeof *p, FUNC);
	p->rule._stmt = 0;
	p->cost[1] =
	p->cost[2] =
	p->cost[3] =
	p->cost[4] =
	p->cost[5] =
	p->cost[6] =
	p->cost[7] =
	p->cost[8] =
		0x7fff;
	switch (OP_LABEL(a)) {
	case 17: /* CNSTF */
		/* F: CNSTF */
		if (0 + 0 < p->cost[_F_NT]) {
			p->cost[_F_NT] = 0 + 0;
			p->rule._F = 2;
			_closure_F(a, 0 + 0);
		}
		break;
	case 21: /* CNSTI */
		/* I: CNSTI */
		if (0 + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = 0 + 0;
			p->rule._I = 2;
			_closure_I(a, 0 + 0);
		}
		break;
	case 22: /* CNSTU */
		/* U: CNSTU */
		if (0 + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = 0 + 0;
			p->rule._U = 2;
			_closure_U(a, 0 + 0);
		}
		break;
	case 23: /* CNSTP */
		/* P: CNSTP */
		if (0 + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = 0 + 0;
			p->rule._P = 2;
			_closure_P(a, 0 + 0);
		}
		break;
	case 33: /* ARGF */
		_label(LEFT_CHILD(a));
		/* V: ARGF(F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 3;
			_closure_V(a, c + 0);
		}
		break;
	case 37: /* ARGI */
		_label(LEFT_CHILD(a));
		/* V: ARGI(I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 4;
			_closure_V(a, c + 0);
		}
		break;
	case 38: /* ARGU */
		_label(LEFT_CHILD(a));
		/* V: ARGU(U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 5;
			_closure_V(a, c + 0);
		}
		break;
	case 39: /* ARGP */
		_label(LEFT_CHILD(a));
		/* V: ARGP(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 6;
			_closure_V(a, c + 0);
		}
		break;
	case 41: /* ARGB */
		_label(LEFT_CHILD(a));
		/* V: ARGB(B) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_B_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 2;
			_closure_V(a, c + 0);
		}
		break;
	case 49: /* ASGNF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: ASGNF(P,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 8;
			_closure_V(a, c + 0);
		}
		break;
	case 53: /* ASGNI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: ASGNI(P,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 9;
			_closure_V(a, c + 0);
		}
		break;
	case 54: /* ASGNU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: ASGNU(P,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 10;
			_closure_V(a, c + 0);
		}
		break;
	case 55: /* ASGNP */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: ASGNP(P,P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 11;
			_closure_V(a, c + 0);
		}
		break;
	case 57: /* ASGNB */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: ASGNB(P,B) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_B_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 7;
			_closure_V(a, c + 0);
		}
		break;
	case 65: /* INDIRF */
		_label(LEFT_CHILD(a));
		/* stmt: INDIRF(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 2;
		}
		/* F: INDIRF(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_F_NT]) {
			p->cost[_F_NT] = c + 0;
			p->rule._F = 3;
			_closure_F(a, c + 0);
		}
		break;
	case 69: /* INDIRI */
		_label(LEFT_CHILD(a));
		/* stmt: INDIRI(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 3;
		}
		/* I: INDIRI(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 3;
			_closure_I(a, c + 0);
		}
		break;
	case 70: /* INDIRU */
		_label(LEFT_CHILD(a));
		/* stmt: INDIRU(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 4;
		}
		/* U: INDIRU(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 3;
			_closure_U(a, c + 0);
		}
		break;
	case 71: /* INDIRP */
		_label(LEFT_CHILD(a));
		/* stmt: INDIRP(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 5;
		}
		/* P: INDIRP(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = c + 0;
			p->rule._P = 3;
			_closure_P(a, c + 0);
		}
		break;
	case 73: /* INDIRB */
		_label(LEFT_CHILD(a));
		/* stmt: INDIRB(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 1;
		}
		/* B: INDIRB(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_B_NT]) {
			p->cost[_B_NT] = c + 0;
			p->rule._B = 2;
			_closure_B(a, c + 0);
		}
		break;
	case 113: /* CVFF */
		_label(LEFT_CHILD(a));
		/* F: CVFF(F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_F_NT]) {
			p->cost[_F_NT] = c + 0;
			p->rule._F = 5;
			_closure_F(a, c + 0);
		}
		break;
	case 117: /* CVFI */
		_label(LEFT_CHILD(a));
		/* I: CVFI(F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 6;
			_closure_I(a, c + 0);
		}
		break;
	case 129: /* CVIF */
		_label(LEFT_CHILD(a));
		/* F: CVIF(I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_F_NT]) {
			p->cost[_F_NT] = c + 0;
			p->rule._F = 4;
			_closure_F(a, c + 0);
		}
		break;
	case 133: /* CVII */
		_label(LEFT_CHILD(a));
		/* I: CVII(I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 4;
			_closure_I(a, c + 0);
		}
		break;
	case 134: /* CVIU */
		_label(LEFT_CHILD(a));
		/* U: CVIU(I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 4;
			_closure_U(a, c + 0);
		}
		break;
	case 150: /* CVPU */
		_label(LEFT_CHILD(a));
		/* U: CVPU(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 6;
			_closure_U(a, c + 0);
		}
		break;
	case 151: /* CVPP */
		_label(LEFT_CHILD(a));
		/* P: CVPP(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = c + 0;
			p->rule._P = 5;
			_closure_P(a, c + 0);
		}
		break;
	case 181: /* CVUI */
		_label(LEFT_CHILD(a));
		/* I: CVUI(U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 5;
			_closure_I(a, c + 0);
		}
		break;
	case 182: /* CVUU */
		_label(LEFT_CHILD(a));
		/* U: CVUU(U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 5;
			_closure_U(a, c + 0);
		}
		break;
	case 183: /* CVUP */
		_label(LEFT_CHILD(a));
		/* P: CVUP(U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = c + 0;
			p->rule._P = 4;
			_closure_P(a, c + 0);
		}
		break;
	case 193: /* NEGF */
		_label(LEFT_CHILD(a));
		/* F: NEGF(F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_F_NT]) {
			p->cost[_F_NT] = c + 0;
			p->rule._F = 6;
			_closure_F(a, c + 0);
		}
		break;
	case 197: /* NEGI */
		_label(LEFT_CHILD(a));
		/* I: NEGI(I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 7;
			_closure_I(a, c + 0);
		}
		break;
	case 209: /* CALLF */
		_label(LEFT_CHILD(a));
		/* stmt: CALLF(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 6;
		}
		/* F: CALLF(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_F_NT]) {
			p->cost[_F_NT] = c + 0;
			p->rule._F = 7;
			_closure_F(a, c + 0);
		}
		break;
	case 213: /* CALLI */
		_label(LEFT_CHILD(a));
		/* stmt: CALLI(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 7;
		}
		/* I: CALLI(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 8;
			_closure_I(a, c + 0);
		}
		break;
	case 214: /* CALLU */
		_label(LEFT_CHILD(a));
		/* stmt: CALLU(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 8;
		}
		/* U: CALLU(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 7;
			_closure_U(a, c + 0);
		}
		break;
	case 215: /* CALLP */
		_label(LEFT_CHILD(a));
		/* stmt: CALLP(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 9;
		}
		/* P: CALLP(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = c + 0;
			p->rule._P = 6;
			_closure_P(a, c + 0);
		}
		break;
	case 216: /* CALLV */
		_label(LEFT_CHILD(a));
		/* V: CALLV(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 13;
			_closure_V(a, c + 0);
		}
		break;
	case 217: /* CALLB */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: CALLB(P,P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 12;
			_closure_V(a, c + 0);
		}
		break;
	case 241: /* RETF */
		_label(LEFT_CHILD(a));
		/* V: RETF(F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 14;
			_closure_V(a, c + 0);
		}
		break;
	case 245: /* RETI */
		_label(LEFT_CHILD(a));
		/* V: RETI(I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 15;
			_closure_V(a, c + 0);
		}
		break;
	case 246: /* RETU */
		_label(LEFT_CHILD(a));
		/* V: RETU(U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 16;
			_closure_V(a, c + 0);
		}
		break;
	case 247: /* RETP */
		_label(LEFT_CHILD(a));
		/* V: RETP(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 17;
			_closure_V(a, c + 0);
		}
		break;
	case 248: /* RETV */
		/* V: RETV */
		if (0 + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = 0 + 0;
			p->rule._V = 18;
			_closure_V(a, 0 + 0);
		}
		break;
	case 263: /* ADDRGP */
		/* P: ADDRGP */
		if (0 + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = 0 + 0;
			p->rule._P = 7;
			_closure_P(a, 0 + 0);
		}
		break;
	case 279: /* ADDRFP */
		/* P: ADDRFP */
		if (0 + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = 0 + 0;
			p->rule._P = 8;
			_closure_P(a, 0 + 0);
		}
		break;
	case 295: /* ADDRLP */
		/* P: ADDRLP */
		if (0 + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = 0 + 0;
			p->rule._P = 9;
			_closure_P(a, 0 + 0);
		}
		break;
	case 305: /* ADDF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* F: ADDF(F,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_F_NT]) {
			p->cost[_F_NT] = c + 0;
			p->rule._F = 8;
			_closure_F(a, c + 0);
		}
		break;
	case 309: /* ADDI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* I: ADDI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 9;
			_closure_I(a, c + 0);
		}
		break;
	case 310: /* ADDU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* U: ADDU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 8;
			_closure_U(a, c + 0);
		}
		break;
	case 311: /* ADDP */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* P: ADDP(P,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = c + 0;
			p->rule._P = 10;
			_closure_P(a, c + 0);
		}
		/* P: ADDP(I,P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = c + 0;
			p->rule._P = 11;
			_closure_P(a, c + 0);
		}
		/* P: ADDP(U,P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = c + 0;
			p->rule._P = 12;
			_closure_P(a, c + 0);
		}
		/* P: ADDP(P,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = c + 0;
			p->rule._P = 13;
			_closure_P(a, c + 0);
		}
		break;
	case 321: /* SUBF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* F: SUBF(F,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_F_NT]) {
			p->cost[_F_NT] = c + 0;
			p->rule._F = 9;
			_closure_F(a, c + 0);
		}
		break;
	case 325: /* SUBI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* I: SUBI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 10;
			_closure_I(a, c + 0);
		}
		break;
	case 326: /* SUBU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* U: SUBU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 9;
			_closure_U(a, c + 0);
		}
		break;
	case 327: /* SUBP */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* P: SUBP(P,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = c + 0;
			p->rule._P = 14;
			_closure_P(a, c + 0);
		}
		/* P: SUBP(P,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_P_NT]) {
			p->cost[_P_NT] = c + 0;
			p->rule._P = 15;
			_closure_P(a, c + 0);
		}
		break;
	case 341: /* LSHI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* I: LSHI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 11;
			_closure_I(a, c + 0);
		}
		break;
	case 342: /* LSHU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* U: LSHU(U,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 10;
			_closure_U(a, c + 0);
		}
		break;
	case 357: /* MODI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* I: MODI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 12;
			_closure_I(a, c + 0);
		}
		break;
	case 358: /* MODU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* U: MODU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 11;
			_closure_U(a, c + 0);
		}
		break;
	case 373: /* RSHI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* I: RSHI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 13;
			_closure_I(a, c + 0);
		}
		break;
	case 374: /* RSHU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* U: RSHU(U,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 12;
			_closure_U(a, c + 0);
		}
		break;
	case 389: /* BANDI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* I: BANDI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 14;
			_closure_I(a, c + 0);
		}
		break;
	case 390: /* BANDU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* U: BANDU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 13;
			_closure_U(a, c + 0);
		}
		break;
	case 405: /* BCOMI */
		_label(LEFT_CHILD(a));
		/* I: BCOMI(I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 15;
			_closure_I(a, c + 0);
		}
		break;
	case 406: /* BCOMU */
		_label(LEFT_CHILD(a));
		/* U: BCOMU(U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 14;
			_closure_U(a, c + 0);
		}
		break;
	case 421: /* BORI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* I: BORI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 16;
			_closure_I(a, c + 0);
		}
		break;
	case 422: /* BORU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* U: BORU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 15;
			_closure_U(a, c + 0);
		}
		break;
	case 437: /* BXORI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* I: BXORI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 17;
			_closure_I(a, c + 0);
		}
		break;
	case 438: /* BXORU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* U: BXORU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 16;
			_closure_U(a, c + 0);
		}
		break;
	case 449: /* DIVF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* F: DIVF(F,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_F_NT]) {
			p->cost[_F_NT] = c + 0;
			p->rule._F = 10;
			_closure_F(a, c + 0);
		}
		break;
	case 453: /* DIVI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* I: DIVI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 18;
			_closure_I(a, c + 0);
		}
		break;
	case 454: /* DIVU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* U: DIVU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 17;
			_closure_U(a, c + 0);
		}
		break;
	case 465: /* MULF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* F: MULF(F,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_F_NT]) {
			p->cost[_F_NT] = c + 0;
			p->rule._F = 11;
			_closure_F(a, c + 0);
		}
		break;
	case 469: /* MULI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* I: MULI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_I_NT]) {
			p->cost[_I_NT] = c + 0;
			p->rule._I = 19;
			_closure_I(a, c + 0);
		}
		break;
	case 470: /* MULU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* U: MULU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_U_NT]) {
			p->cost[_U_NT] = c + 0;
			p->rule._U = 18;
			_closure_U(a, c + 0);
		}
		break;
	case 481: /* EQF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: EQF(F,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 19;
			_closure_V(a, c + 0);
		}
		break;
	case 485: /* EQI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: EQI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 20;
			_closure_V(a, c + 0);
		}
		break;
	case 486: /* EQU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: EQU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 21;
			_closure_V(a, c + 0);
		}
		break;
	case 497: /* GEF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: GEF(F,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 22;
			_closure_V(a, c + 0);
		}
		break;
	case 501: /* GEI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: GEI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 23;
			_closure_V(a, c + 0);
		}
		break;
	case 502: /* GEU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: GEU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 24;
			_closure_V(a, c + 0);
		}
		break;
	case 513: /* GTF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: GTF(F,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 25;
			_closure_V(a, c + 0);
		}
		break;
	case 517: /* GTI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: GTI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 26;
			_closure_V(a, c + 0);
		}
		break;
	case 518: /* GTU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: GTU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 27;
			_closure_V(a, c + 0);
		}
		break;
	case 529: /* LEF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: LEF(F,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 28;
			_closure_V(a, c + 0);
		}
		break;
	case 533: /* LEI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: LEI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 29;
			_closure_V(a, c + 0);
		}
		break;
	case 534: /* LEU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: LEU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 30;
			_closure_V(a, c + 0);
		}
		break;
	case 545: /* LTF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: LTF(F,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 31;
			_closure_V(a, c + 0);
		}
		break;
	case 549: /* LTI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: LTI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 32;
			_closure_V(a, c + 0);
		}
		break;
	case 550: /* LTU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: LTU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 33;
			_closure_V(a, c + 0);
		}
		break;
	case 561: /* NEF */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: NEF(F,F) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_F_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_F_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 34;
			_closure_V(a, c + 0);
		}
		break;
	case 565: /* NEI */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: NEI(I,I) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_I_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_I_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 35;
			_closure_V(a, c + 0);
		}
		break;
	case 566: /* NEU */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* V: NEU(U,U) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_U_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_U_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 36;
			_closure_V(a, c + 0);
		}
		break;
	case 584: /* JUMPV */
		_label(LEFT_CHILD(a));
		/* V: JUMPV(P) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_P_NT] + 0;
		if (c + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = c + 0;
			p->rule._V = 37;
			_closure_V(a, c + 0);
		}
		break;
	case 600: /* LABELV */
		/* V: LABELV */
		if (0 + 0 < p->cost[_V_NT]) {
			p->cost[_V_NT] = 0 + 0;
			p->rule._V = 38;
			_closure_V(a, 0 + 0);
		}
		break;
	default:
		fatal("_label", "Bad terminal %d\n", OP_LABEL(a));
	}
}

static void _kids(NODEPTR_TYPE p, int eruleno, NODEPTR_TYPE kids[]) {
	if (!p)
		fatal("_kids", "Null tree\n", 0);
	if (!kids)
		fatal("_kids", "Null kids\n", 0);
	switch (eruleno) {
	case 118: /* V: JUMPV(P) */
	case 89: /* I: BCOMI(I) */
	case 88: /* U: BCOMU(U) */
	case 63: /* V: RETP(P) */
	case 62: /* V: RETU(U) */
	case 61: /* V: RETI(I) */
	case 60: /* V: RETF(F) */
	case 59: /* V: CALLV(P) */
	case 58: /* P: CALLP(P) */
	case 57: /* U: CALLU(P) */
	case 56: /* I: CALLI(P) */
	case 55: /* F: CALLF(P) */
	case 53: /* I: NEGI(I) */
	case 52: /* F: NEGF(F) */
	case 51: /* P: CVPP(P) */
	case 50: /* P: CVUP(U) */
	case 49: /* F: CVFF(F) */
	case 48: /* F: CVIF(I) */
	case 47: /* U: CVPU(P) */
	case 46: /* U: CVUU(U) */
	case 45: /* U: CVIU(I) */
	case 44: /* I: CVFI(F) */
	case 43: /* I: CVUI(U) */
	case 42: /* I: CVII(I) */
	case 41: /* P: INDIRP(P) */
	case 40: /* U: INDIRU(P) */
	case 39: /* I: INDIRI(P) */
	case 38: /* F: INDIRF(P) */
	case 37: /* B: INDIRB(P) */
	case 31: /* V: ARGP(P) */
	case 30: /* V: ARGU(U) */
	case 29: /* V: ARGI(I) */
	case 28: /* V: ARGF(F) */
	case 27: /* V: ARGB(B) */
	case 9: /* stmt: CALLP(P) */
	case 8: /* stmt: CALLU(P) */
	case 7: /* stmt: CALLI(P) */
	case 6: /* stmt: CALLF(P) */
	case 5: /* stmt: INDIRP(P) */
	case 4: /* stmt: INDIRU(P) */
	case 3: /* stmt: INDIRI(P) */
	case 2: /* stmt: INDIRF(P) */
	case 1: /* stmt: INDIRB(P) */
		kids[0] = LEFT_CHILD(p);
		break;
	case 22: /* V: bogus */
	case 21: /* B: bogus */
	case 20: /* F: bogus */
	case 19: /* P: bogus */
	case 18: /* U: bogus */
	case 17: /* I: bogus */
	case 16: /* bogus: V */
	case 15: /* bogus: B */
	case 14: /* bogus: F */
	case 13: /* bogus: P */
	case 12: /* bogus: U */
	case 11: /* bogus: I */
	case 10: /* stmt: V */
		kids[0] = p;
		break;
	case 119: /* V: LABELV */
	case 67: /* P: ADDRLP */
	case 66: /* P: ADDRFP */
	case 65: /* P: ADDRGP */
	case 64: /* V: RETV */
	case 26: /* U: CNSTU */
	case 25: /* P: CNSTP */
	case 24: /* I: CNSTI */
	case 23: /* F: CNSTF */
		break;
	case 117: /* V: NEU(U,U) */
	case 116: /* V: NEI(I,I) */
	case 115: /* V: NEF(F,F) */
	case 114: /* V: LTU(U,U) */
	case 113: /* V: LTI(I,I) */
	case 112: /* V: LTF(F,F) */
	case 111: /* V: LEU(U,U) */
	case 110: /* V: LEI(I,I) */
	case 109: /* V: LEF(F,F) */
	case 108: /* V: GTU(U,U) */
	case 107: /* V: GTI(I,I) */
	case 106: /* V: GTF(F,F) */
	case 105: /* V: GEU(U,U) */
	case 104: /* V: GEI(I,I) */
	case 103: /* V: GEF(F,F) */
	case 102: /* V: EQU(U,U) */
	case 101: /* V: EQI(I,I) */
	case 100: /* V: EQF(F,F) */
	case 99: /* U: MULU(U,U) */
	case 98: /* I: MULI(I,I) */
	case 97: /* F: MULF(F,F) */
	case 96: /* U: DIVU(U,U) */
	case 95: /* I: DIVI(I,I) */
	case 94: /* F: DIVF(F,F) */
	case 93: /* I: BXORI(I,I) */
	case 92: /* U: BXORU(U,U) */
	case 91: /* U: BORU(U,U) */
	case 90: /* I: BORI(I,I) */
	case 87: /* I: BANDI(I,I) */
	case 86: /* U: BANDU(U,U) */
	case 85: /* U: RSHU(U,I) */
	case 84: /* I: RSHI(I,I) */
	case 83: /* U: MODU(U,U) */
	case 82: /* I: MODI(I,I) */
	case 81: /* U: LSHU(U,I) */
	case 80: /* I: LSHI(I,I) */
	case 79: /* U: SUBU(U,U) */
	case 78: /* P: SUBP(P,U) */
	case 77: /* P: SUBP(P,I) */
	case 76: /* I: SUBI(I,I) */
	case 75: /* F: SUBF(F,F) */
	case 74: /* U: ADDU(U,U) */
	case 73: /* P: ADDP(P,U) */
	case 72: /* P: ADDP(U,P) */
	case 71: /* P: ADDP(I,P) */
	case 70: /* P: ADDP(P,I) */
	case 69: /* I: ADDI(I,I) */
	case 68: /* F: ADDF(F,F) */
	case 54: /* V: CALLB(P,P) */
	case 36: /* V: ASGNP(P,P) */
	case 35: /* V: ASGNU(P,U) */
	case 34: /* V: ASGNI(P,I) */
	case 33: /* V: ASGNF(P,F) */
	case 32: /* V: ASGNB(P,B) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = RIGHT_CHILD(p);
		break;
	default:
		fatal("_kids", "Bad rule number %d\n", eruleno);
	}
}


static void reduce(NODEPTR_TYPE p, int goalnt) {
	int i, sz = opsize(p->op), rulenumber = _rule(p->x.state, goalnt);
	short *nts = _nts[rulenumber];
	NODEPTR_TYPE kids[10];

	assert(rulenumber);
	_kids(p, rulenumber, kids);
	for (i = 0; nts[i]; i++)
		reduce(kids[i], nts[i]);
	switch (optype(p->op)) {
#define xx(ty) if (sz == ty->size) return
	case I:
	case U:
		xx(chartype);
		xx(shorttype);
		xx(inttype);
		xx(longtype);
		xx(longlong);
		break;
	case F:
		xx(floattype);
		xx(doubletype);
		xx(longdouble);
		break;
	case P:
		xx(voidptype);
		xx(funcptype);
		break;
	case V:
	case B: if (sz == 0) return;
#undef xx
	}
	printdag(p, 2);
	assert(0);
}

void check(Node p) {
	struct _state { short cost[1]; };

	_label(p);
	if (((struct _state *)p->x.state)->cost[1] > 0) {
		printdag(p, 2);
		assert(0);
	}
	reduce(p, 1);
}
