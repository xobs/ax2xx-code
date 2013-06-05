/* ----------------------------------------------------------------------
 * FILE: symbol.c
 * PACKAGE: as31 - 8031/8051 Assembler.
 *
 * DESCRIPTION:
 *	This file contains the symbol table search/insertion routines
 *	associated with user defined symbols.
 *
 *	The reserved keyword (instructions/directives) look up routine
 *	is defined here.
 *
 *	The opcode table for all of the instructions is located in this
 *	file.
 *
 * REVISION HISTORY:
 *	Jan. 19, 1990 - Created. (Ken Stauffer)
 *
 * AUTHOR:
 *	All code in this file written by Ken Stauffer (University of Calgary)
 *	January, 1990.
 *
 */

#include <ctype.h>
#include <string.h>

/* normally in stdlib.h, but that include file also defines */
/* "div" which conflicts with the div in this file */
extern void *malloc(size_t size);
extern void free(void *ptr);


#include "as31.h"
#include "parser.h"

#define B(a)		(0xF0+(a))
#define ACC(a)		(0xE0+(a))
#define PSW(a)		(0xD0+(a))
#define T2CON(a)	(0xC8+(a))
#define IP(a)		(0xB8+(a))
#define P3(a)		(0xB0+(a))
#define IE(a)		(0xA8+(a))
#define P2(a)		(0xA0+(a))
#define SCON(a)		(0x98+(a))
#define P1(a)		(0x90+(a))
#define TCON(a)		(0x88+(a))
#define P0(a)		(0x80+(a))

/* ---------------------------------------------------------------------- 
 * sinit[]
 *	These symbols are not reserved keywords.
 *	This array contains the initial symbol table entries
 *	for the user symbol table. The symbols are
 *	basically convienient names that make writing
 *	in 8031/8051 bearable.
 *
 *	The function syminit() inserts these entries into the
 *	symbol table.
 *
 */

static struct symbol sinit[] = {
	{ "AC",		1, LABEL,		PSW(6),	NULL },
	{ "ACC",	1, LABEL,		ACC(0),	NULL },
	{ "B",		1, LABEL,		B(0),	NULL },
	{ "CY",		1, LABEL,		PSW(7),	NULL },
	{ "DPH",	1, LABEL,		0x83,	NULL },
	{ "DPL",	1, LABEL,		0x82,	NULL },
	{ "EA",		1, LABEL,		IE(7),	NULL },
	{ "ES",		1, LABEL,		IE(4),	NULL },
	{ "ET0",	1, LABEL,		IE(1),	NULL },
	{ "ET1",	1, LABEL,		IE(3),	NULL },
	{ "ET2",	1, LABEL,		IE(5),	NULL },
	{ "EX0",	1, LABEL,		IE(0),	NULL },
	{ "EX1",	1, LABEL,		IE(2),	NULL },
	{ "EXEN2",	1, LABEL,		T2CON(3),NULL },
	{ "EXF2",	1, LABEL,		T2CON(6),NULL },
	{ "F0",		1, LABEL,		PSW(5),	NULL },
	{ "IE",		1, LABEL,		IE(0),	NULL },
	{ "IE0",	1, LABEL,		TCON(1),NULL },
	{ "IE1",	1, LABEL,		TCON(3),NULL },
	{ "INT0",	1, LABEL,		P3(2),  NULL },
	{ "INT1",	1, LABEL,		P3(3),  NULL },
	{ "IP",		1, LABEL,		IP(0),	NULL },
	{ "IT0",	1, LABEL,		TCON(0),NULL },
	{ "IT1",	1, LABEL,		TCON(2),NULL },
	{ "OV",		1, LABEL,		PSW(2),	NULL },
	{ "P",		1, LABEL,		PSW(0),	NULL },
	{ "P0",		1, LABEL,		P0(0),	NULL },
	{ "P1",		1, LABEL,		P1(0),	NULL },
	{ "P2",		1, LABEL,		P2(0),	NULL },
	{ "P3",		1, LABEL,		P3(0),	NULL },
	{ "PCON",	1, LABEL,		0x87,	NULL },
	{ "PS",		1, LABEL,		IP(4),	NULL },
	{ "PSW",	1, LABEL,		PSW(0),	NULL },
	{ "PT0",	1, LABEL,		IP(1),	NULL },
	{ "PT1",	1, LABEL,		IP(3),	NULL },
	{ "PT2",	1, LABEL,		IP(5),	NULL },
	{ "PX0",	1, LABEL,		IP(0),	NULL },
	{ "PX1",	1, LABEL,		IP(2),	NULL },
	{ "RB8",	1, LABEL,		SCON(2),NULL },
	{ "RCAP2H",	1, LABEL,		0xCB,	NULL },
	{ "RCAP2L",	1, LABEL,		0xCA,	NULL },
	{ "RCLK",	1, LABEL,		T2CON(5),NULL },
	{ "REN",	1, LABEL,		SCON(4),NULL },
	{ "RD",		1, LABEL,		P3(7),  NULL },
	{ "RI",		1, LABEL,		SCON(0),NULL },
	{ "RL2",	1, LABEL,		T2CON(0),NULL },
	{ "RS0",	1, LABEL,		PSW(3),	NULL },
	{ "RS1",	1, LABEL,		PSW(4),	NULL },
	{ "RXD",	1, LABEL,		P3(0),  NULL },
	{ "SBUF",	1, LABEL,		0x99,	NULL },
	{ "SCON",	1, LABEL,		SCON(0),NULL },
	{ "SM0",	1, LABEL,		SCON(7),NULL },
	{ "SM1",	1, LABEL,		SCON(6),NULL },
	{ "SM2",	1, LABEL,		SCON(5),NULL },
	{ "SP",		1, LABEL,		0x81,	NULL },
	{ "T0",		1, LABEL,		P3(4),  NULL },
	{ "T1",		1, LABEL,		P3(5),  NULL },
	{ "T2",		1, LABEL,		P0(0),  NULL },
	{ "T2CON",	1, LABEL,		T2CON(0),NULL },
	{ "T2EX",	1, LABEL,		P0(1),  NULL },
	{ "TB8",	1, LABEL,		SCON(3),NULL },
	{ "TCLK",	1, LABEL,		T2CON(4),NULL },
	{ "TCON",	1, LABEL,		TCON(0),NULL },
	{ "TF0",	1, LABEL,		TCON(5),NULL },
	{ "TF1",	1, LABEL,		TCON(7),NULL },
	{ "TF2",	1, LABEL,		T2CON(7),NULL },
	{ "TH0",	1, LABEL,		0x8C,	NULL },
	{ "TH1",	1, LABEL,		0x8D,	NULL },
	{ "TH2",	1, LABEL,		0xCD,	NULL },
	{ "TI",		1, LABEL,		SCON(1),NULL },
	{ "TL0",	1, LABEL,		0x8A,	NULL },
	{ "TL1",	1, LABEL,		0x8B,	NULL },
	{ "TL2",	1, LABEL,		0xCC,	NULL },
	{ "TMOD",	1, LABEL,		0x89,	NULL },
	{ "TR0",	1, LABEL,		TCON(4),NULL },
	{ "TR1",	1, LABEL,		TCON(6),NULL },
	{ "TR2",	1, LABEL,		T2CON(2),NULL },
	{ "TXD",	1, LABEL,		P3(1),  NULL },
	{ "WR",		1, LABEL,		P3(6),	NULL },
};

#define SINITSIZE	(sizeof(sinit)/sizeof(sinit[0]))

/* ----------------------------------------------------------------------
 * opcode vectors:
 *	These arrays contain the various opcodes for the
 *	various forms an instruction may take.
 *
 *	The ordering of these opcodes is very critical to the
 *	proper fuctioning of the assembler.
 *
 *	When a given form of an instruction is parsed, the parser
 *	indexes one of these arrays by the correct amount and thus
 *	obtains the correct opcode for the particular form.
 *
 */

static unsigned char acall[]=	{ 0x11 };
static unsigned char add[]=	{ 0x28, 0x25, 0x26, 0x24 };
static unsigned char addc[]=	{ 0x38, 0x35, 0x36, 0x34 };
static unsigned char ajmp[]=	{ 0x01 };
static unsigned char anl[]=	{ 0x58, 0x55, 0x56, 0x54, 0x52, 0x53, 0x82,
				  0xb0 };
static unsigned char cjne[]=	{ 0xb5, 0xb4, 0xb8, 0xb6 };
static unsigned char clr[]=	{ 0xe4, 0xc3, 0xc2 };
static unsigned char cpl[]=	{ 0xf4, 0xb3, 0xb2 };
static unsigned char da[]=	{ 0xd4 };
static unsigned char dec[]=	{ 0x14, 0x18, 0x15, 0x16 };
static unsigned char div[]=	{ 0x84 };
static unsigned char djnz[]=	{ 0xd8, 0xd5 };
static unsigned char inc[]=	{ 0x04, 0x08, 0x05, 0x06, 0xa3 };
static unsigned char jb[]=	{ 0x20 };
static unsigned char jbc[]=	{ 0x10 };
static unsigned char jc[]=	{ 0x40 };
static unsigned char jmp[]=	{ 0x73 };
static unsigned char jnb[]=	{ 0x30 };
static unsigned char jnc[]=	{ 0x50 };
static unsigned char jnz[]=	{ 0x70 };
static unsigned char jz[]=	{ 0x60 };
static unsigned char lcall[]=	{ 0x12 };
static unsigned char ljmp[]=	{ 0x02 };
static unsigned char mov[]=	{ 0xe8, 0xe5, 0xe6, 0x74, 0xf5, 0x75, 0xf8,
				  0xa8, 0x78, 0x88, 0x85, 0x86, 0xf6, 0xa6,
				  0x76, 0x90, 0xa2, 0x92 };
static unsigned char movc[]=	{ 0x93, 0x83 };
static unsigned char movx[]=	{ 0xe2, 0xe3, 0xe0, 0xf2, 0xf3, 0xf0 };
static unsigned char mul[]=	{ 0xa4 };
static unsigned char nop[]=	{ 0x00 };
static unsigned char orl[]=	{ 0x48, 0x45, 0x46, 0x44, 0x42, 0x43, 0x72,
				  0xa0 };
static unsigned char pop[]=	{ 0xd0 };
static unsigned char push[]=	{ 0xc0 };
static unsigned char ret[]=	{ 0x22 };
static unsigned char reti[]=	{ 0x32 };
static unsigned char rl[]=	{ 0x23 };
static unsigned char rlc[]=	{ 0x33 };
static unsigned char rr[]=	{ 0x03 };
static unsigned char rrc[]=	{ 0x13 };
static unsigned char setb[]=	{ 0xd3, 0xd2 };
static unsigned char sjmp[]=	{ 0x80 };
static unsigned char subb[]=	{ 0x98, 0x95, 0x96, 0x94 };
static unsigned char swap[]=	{ 0xc4 };
static unsigned char xch[]=	{ 0xc8, 0xc5, 0xc6 };
static unsigned char xchd[]=	{ 0xd6 };
static unsigned char xrl[]=	{ 0x68, 0x65, 0x66, 0x64, 0x62, 0x63 };

/* ----------------------------------------------------------------------
 * optable[]
 *	This table contains opcodes, directives and a few reserved
 *	symbols.
 *
 *	The second field is the keywords token value.
 *
 *	Unless the symbol is an opcode, the third field will
 *	be NULL.
 * 
 *	The third field is a pointer to an array of opcode bytes.
 *
 *    --> This list must be in order by the first field
 */

static struct opcode optable[] = {
	{"a",		A,	NULL		},
	{"ab",		AB,	NULL		},
	{"acall",	ACALL,	acall		},
	{"add",		ADD,	add		},
	{"addc",	ADDC,	addc		},
	{"ajmp",	AJMP,	ajmp		},
	{"anl",		ANL,	anl		},
	{"byte",	D_BYTE,	NULL		},
	{"c",		C,	NULL		},
	{"cjne",	CJNE,	cjne		},
	{"clr",		CLR,	clr		},
	{"cpl",		CPL,	cpl		},
	{"da",		DA,	da		},
	{"db",		D_BYTE,	NULL		},
	{"dec",		DEC,	dec		},
	{"div",		DIV,	div		},
	{"djnz",	DJNZ,	djnz		},
	{"dptr",	DPTR,	NULL		},
	{"dw",		D_WORD, NULL		},
	{"end",		D_END,	NULL		},
	{"equ",		D_EQU,	NULL		},
	{"flag",	D_FLAG,	NULL		},
	{"inc",		INC,	inc		},
	{"jb",		JB,	jb		},
	{"jbc",		JBC,	jbc		},
	{"jc",		JC,	jc		},
	{"jmp",		JMP,	jmp		},
	{"jnb",		JNB,	jnb		},
	{"jnc",		JNC,	jnc		},
	{"jnz",		JNZ,	jnz		},
	{"jz",		JZ,	jz		},
	{"lcall",	LCALL,	lcall		},
	{"ljmp",	LJMP,	ljmp		},
	{"mov",		MOV,	mov		},
	{"movc",	MOVC,	movc		},
	{"movx",	MOVX,	movx		},
	{"mul",		MUL,	mul		},
	{"nop",		NOP,	nop		},
	{"org",		D_ORG,	NULL		},
	{"orl",		ORL,	orl		},
	{"pc",		PC,	NULL		},
	{"pop",		POP,	pop		},
	{"push",	PUSH,	push		},
	{"r0",		R0,	NULL		},
	{"r1",		R1,	NULL		},
	{"r2",		R2,	NULL		},
	{"r3",		R3,	NULL		},
	{"r4",		R4,	NULL		},
	{"r5",		R5,	NULL		},
	{"r6",		R6,	NULL		},
	{"r7",		R7,	NULL		},
	{"ret",		RET,	ret		},
	{"reti",	RETI,	reti		},
	{"rl",		RL,	rl		},
	{"rlc",		RLC,	rlc		},
	{"rr",		RR,	rr		},
	{"rrc",		RRC,	rrc		},
	{"setb",	SETB,	setb		},
	{"sjmp",	SJMP,	sjmp		},
	{"skip",	D_SKIP,	NULL		},
	{"subb",	SUBB,	subb		},
	{"swap",	SWAP,	swap		},
	{"word",	D_WORD,	NULL		},
	{"xch",		XCH,	xch		},
	{"xchd",	XCHD,	xchd		},
	{"xrl",		XRL,	xrl		}
};

#define OPTABSIZE	(sizeof(optable)/sizeof(struct opcode))


/* ----------------------------------------------------------------------
 * lookop:
 *	Do a binary search through optable[], for a matching
 *	symbol. Return the symbol found or NULL.
 *
 */

struct opcode *lookop(const char *s)
{
	register int low,high,mid,cond;

	low = 0;
	high = OPTABSIZE-1;
	while( low<=high ) {
		mid = (low+high)/2;
		if( (cond = strcasecmp(s,optable[mid].name)) < 0 )
			high = mid-1;
		else if(cond > 0 )
			low = mid+1;
		else
			return(&optable[mid]);
	}
	return(NULL);
}

/* ----------------------------------------------------------------------
 * symtab, hash, looksym:
 *	User symbol table routines.
 *	symtab is the hash table for the user symbols.
 *	(chaining is used for collision resolution).
 *
 */

static struct symbol *symtab[HASHTABSIZE];

int hash(const char *s)
{
	register const char *p;
	register unsigned h=0, g;

	for (p=s; *p; p++) {
		h = (h<<4) + toupper(*p);
		if ( (g = h&0xf0000000) ) {
			h = h ^ (g >> 24);
			h = h ^ g;
		}
	}
	return( h % HASHTABSIZE );
}

struct symbol *looksym(const char *s)
{
	register struct symbol *ptr, *prev;
	char *p;
	register int hv;

	hv = hash(s);

	prev = NULL;
	for(ptr=symtab[hv]; ptr; ptr = ptr->next) {
		if( !strcasecmp(ptr->name,s) ) {
			if( prev != NULL ) {
				prev->next = ptr->next;
				ptr->next = symtab[hv];
				symtab[hv] = ptr;
			}
			return(ptr);
		}
		prev = ptr;
	}

	if ( (p = malloc(strlen(s) + 1)) ) {
		strcpy(p,s);
	} else {
		error("Cannot allocate %d bytes",strlen(s)+1);
	}

	ptr = (struct symbol *) malloc( sizeof(struct symbol) );
	if( ptr == NULL )
		error("Cannot allocate %d bytes",sizeof(struct symbol));
	ptr->name = p;
	ptr->predefined = 0;
	ptr->type = UNDEF;
	ptr->next = symtab[hv];
	symtab[hv] = ptr;
	return(ptr);
}

/* ----------------------------------------------------------------------
 * syminit:
 *	Initializes the hash table, with the initial symbols from
 *	sinit[]
 *
 */

// static struct symbol *symtab[HASHTABSIZE];

void syminit(void)
{
	register int i,hv;

	/* clear all entries in the symbol table */
	memset(symtab, 0, sizeof(struct symbol *) * HASHTABSIZE);

	/* load all the pre-defined symbols */
	for(i=0; i<SINITSIZE; i++ ) {
		hv = hash(sinit[i].name);
		if( symtab[hv] )
			sinit[i].next = symtab[hv];
		symtab[hv] = &sinit[i];
		sinit[i].next = NULL;
	}
}

/* free all the memory allocated for the symbols */

void freesym(void)
{
	struct symbol *sym, *next;
	int i;

	for (i=0; i<HASHTABSIZE; i++) {
		sym = symtab[i];
		while (sym != NULL) {
			if (sym->predefined) {
				sym = sym->next;
			} else {
				if (sym->name) free(sym->name);
				next = sym->next;
				free(sym);
				sym = next;
			}
		}
	}
}



