/* ----------------------------------------------------------------------
 * FILE: as31.y
 * PACKAGE: as31 - 8031/8051 Assembler.
 *
 * DESCRIPTION:
 *	This file contains the yacc parser for the assembler.
 *	Related to this are the following:
 *		error(), warn(), yyerror()
 *		genbyte(), genword(), genstr(), makeop()
 *
 *
 * REVISION HISTORY:
 *	Jan. 19, 1990 - Created. (Ken Stauffer)
 *
 * AUTHOR:
 *	All code in this file written by Ken Stauffer (University of Calgary).
 *	January 1990.
 *
 */

%{

#include <stdio.h>
#include <stdlib.h>

#define NOPE
#include "as31.h"
#undef NOPE

#define YYSTYPE union ystack

static unsigned char bytebuf[1024];		/* used by dumplist() */
static int bytecount;


void yyerror(const char *s);
int makeop(struct opcode * op, struct mode *m, int add);
void inclc(int i);
char *padline(char *line);
void dumplist(char *txt, int show);
void genbyte(int b);
void genstr(const char *s);
void genword(unsigned long w);

/* ------------------------ G R A M M E R ----------------------------- */

%}

%token STRING
%token D_ORG
%token D_BYTE
%token D_WORD
%token D_SKIP
%token D_EQU
%token D_FLAG
%token D_END
%token ACALL
%token ADD
%token ADDC
%token AJMP
%token ANL
%token CJNE
%token CLR
%token CPL
%token DA
%token DEC
%token DIV
%token DJNZ
%token INC
%token JB
%token JBC
%token JC
%token JMP
%token JNB
%token JNC
%token JNZ
%token JZ
%token LCALL
%token LJMP
%token MOV
%token MOVC
%token MOVX
%token NOP
%token MUL
%token ORL
%token POP
%token PUSH
%token RET
%token RETI
%token RL
%token RLC
%token RR
%token RRC
%token SETB
%token SJMP
%token SUBB
%token SWAP
%token XCH
%token XCHD
%token XRL
%token AB
%token A
%token C
%token PC
%token DPTR
%token BITPOS
%token R0
%token R1
%token R2
%token R3
%token R4
%token R5
%token R6
%token R7
%token VALUE
%token SYMBOL

%left '+' '-'
%left '*' '/' '%'
%left '|' '&'
%left '>' '<'

%start program

%%
program		:	linelist
{
}
		;

linelist	: linelist line
		| line
		;

line		: undefsym ':' linerest
{
	if (abort_asap) {YYABORT;}
	if( pass1 ) {
		$1.sym->type = LABEL;
		$1.sym->value = lc;
	}
	inclc($3.value);
	bytecount = 0;
}
		| linerest		{ inclc($1.value); bytecount = 0; }
		;

linerest	: directive '\n'	{
						$$.value = $1.value;
						if( dashl && pass2 )
							dumplist($2.str,1);
					}
		| instr '\n'		{
						$$.value = $1.value;
						if( dashl && pass2 )
							dumplist($2.str,1);

					}
		| '\n'			{
						$$.value = 0;
						if( dashl && pass2 )
							dumplist($1.str,0);
					}
		| error			{
						seek_eol();
					}
		   '\n'			{
						$$.value = 0;
						if( dashl && pass2 )
							dumplist($1.str,0);
					}
		;





/* --------------------
 * DIRECTIVES:
 *
 */

directive	: '.' D_ORG defexpr
{
	lc = $3.val.v;
	if( pass2 ) emitaddr(lc);
	bytecount = 0;
	$$.value = 0;
}
		| '.' D_BYTE blist	{ $$.value = $3.value; }
		| '.' D_WORD wlist	{ $$.value = $3.value; }
		| '.' D_SKIP defexpr	{ $$.value = $3.val.v;
					  if( pass2 )
						emitaddr(lc+$$.value); }
		| '.' D_EQU undefsym ',' expr
{
	if( $5.val.d == 0 )
		warn("Expression is undefined in pass 1");
	$3.sym->type = LABEL;
	$3.sym->value = $5.val.v;
	$$.value = 0;
}
	
		| '.' D_FLAG SYMBOL ',' flag
{
	$3.sym->type = LABEL;
	$3.sym->value = $5.value;
	$$.value = 0;
}
		| '.' D_END			{ $$.value = 0; }
		;

defexpr		: expr
{
		if( $1.val.d == 0 )
			warn("Expression is undefined in pass 1");
		if( !(isbit16($1.val.v)) )
			warn("Value greater than 16-bits");
		$$.value = $1.val.v;
}
		;

flag		: flagv BITPOS
{
	if( !isbit8($1.value) )
		warn("Bit address exceeds 8-bits");
	if( isbmram($1.value) )
		$$.value = ($1.value-0x20)*8+ $2.value;
	else if( isbmsfr($1.value) )
		$$.value = $1.value + $2.value;
	else
		warn("Invalid bit addressable RAM location");
}
		;

flagv		: SYMBOL
{
	if( $1.sym->type == UNDEF )
		warn("Symbol %s must be defined in pass 1",$1.sym->name);
	$$.value = $1.sym->value;
}
		| VALUE			{ $$.value = $1.value; }
		;


undefsym	: SYMBOL
{
	if( $1.sym->type != UNDEF && pass1)
		warn("Attempt to redefine symbol: %s",$1.sym->name);
	$$.sym = $1.sym;
}
		;

blist		: blist ',' data8
{
	if( pass2 ) genbyte($3.value);
	$$.value = $1.value + 1;
}
		| blist ',' STRING
{
	if( pass1 )
		$$.value = $1.value + $3.value;
	else {
		$$.value = $1.value + strlen($3.str);
		genstr($3.str);
		
		free($3.str);
	}
}
		| data8
{
	if( pass2 ) genbyte($1.value);
	$$.value = 1;
}
		| STRING
{
	if( pass1 )
		$$.value = $1.value;
	else {
		$$.value = strlen($1.str);
		genstr($1.str);
		free($1.str);
	}
}
		;

wlist		: wlist ',' data16
{
	if( pass2 ) genword($3.value);
	$$.value = $1.value + 2;
}
		| data16
{
	if( pass2 ) genword($1.value);
	$$.value = 2;
}
		;



/* --------------------
 * EXPRESSIONS:
 *
 */

expr		: '*'			{ $$.val.v = lc;
					  $$.val.d = 1; }

		| '(' expr ')'		{ $$.val.v = $2.val.v;
					  $$.val.d = $2.val.d; }

		| '-' expr %prec '*'	{ $$.val.v = -$2.val.v;
					  $$.val.d = $2.val.d;  }

		| expr '|' expr		{ $$.val.v = $1.val.v | $3.val.v;
					  $$.val.d = $1.val.d && $3.val.d; }

		| expr '&' expr		{ $$.val.v = $1.val.v & $3.val.v;
					  $$.val.d = $1.val.d && $3.val.d; }

		| expr '*' expr		{ $$.val.v = $1.val.v * $3.val.v;
					  $$.val.d = $1.val.d && $3.val.d; }

		| expr '/' expr		{ $$.val.v = $1.val.v / $3.val.v;
					  $$.val.d = $1.val.d && $3.val.d; }

		| expr '%' expr		{ $$.val.v = $1.val.v % $3.val.v;
					  $$.val.d = $1.val.d && $3.val.d; }

		| expr '-' expr		{ $$.val.v = $1.val.v - $3.val.v;
					  $$.val.d = $1.val.d && $3.val.d; }

		| expr '+' expr		{ $$.val.v = $1.val.v + $3.val.v;
					  $$.val.d = $1.val.d && $3.val.d; }

		| expr '>' '>' expr	{ $$.val.v = $1.val.v >> $4.val.v;
					  $$.val.d = $1.val.d && $4.val.d; }

		| expr '<' '<' expr	{ $$.val.v = $1.val.v << $4.val.v;
					  $$.val.d = $1.val.d && $4.val.d; }
		| SYMBOL
{
	if( pass1 ) {
		$$.val.v = $1.sym->value;
		$$.val.d = ($1.sym->type != UNDEF);
	}
	else {
		if( $1.sym->type == UNDEF )
			warn("Undefined symbol %s",$1.sym->name);
		$$.val.v = $1.sym->value;
		$$.val.d = 1;
	}
}
		| VALUE		{ $$.val.v = $1.val.v; $$.val.d=1; }
		;





/* --------------------
 * INSTRUCTIONS:
 *
 */

instr		: NOP
				{ $$.value = makeop($1.op,NULL,0); }
		| ACALL addr11
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| AJMP addr11
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| ADD two_op1
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| ADDC two_op1
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| SUBB two_op1
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| XRL two_op1
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| XRL two_op2
				{ $$.value = makeop($1.op,&$2.mode,4); }
		| ANL two_op1
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| ANL two_op2
				{ $$.value = makeop($1.op,&$2.mode,4); }
		| ANL two_op3
				{ $$.value = makeop($1.op,&$2.mode,6); }
		| ORL two_op1
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| ORL two_op2
				{ $$.value = makeop($1.op,&$2.mode,4); }
		| ORL two_op3
				{ $$.value = makeop($1.op,&$2.mode,6); }
		| XCH two_op1
				{ if( get_md($2.mode) == 3 )
					warn("Immediate mode is illegal");
				  $$.value = makeop($1.op,&$2.mode,0);
				}
		| INC single_op1
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| INC DPTR
				{ $$.value = makeop($1.op,NULL,4); }
		| DEC single_op1
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| DA A
				{ $$.value = makeop($1.op,NULL,0); }
		| DIV AB
				{ $$.value = makeop($1.op,NULL,0); }
		| JMP '@' A '+' DPTR
				{ $$.value = makeop($1.op,NULL,0); }
		| JMP '@' DPTR '+' A
				{ $$.value = makeop($1.op,NULL,0); }
		| MUL AB
				{ $$.value = makeop($1.op,NULL,0); }
		| RET
				{ $$.value = makeop($1.op,NULL,0); }
		| RETI
				{ $$.value = makeop($1.op,NULL,0); }
		| RL A
				{ $$.value = makeop($1.op,NULL,0); }
		| RLC A
				{ $$.value = makeop($1.op,NULL,0); }
		| RR A
				{ $$.value = makeop($1.op,NULL,0); }
		| RRC A
				{ $$.value = makeop($1.op,NULL,0); }
		| SWAP A
				{ $$.value = makeop($1.op,NULL,0); }
		| XCHD two_op1
				{ if( get_md($2.mode) != 2 )
					warn("Invalid addressing mode");
				  $$.value = makeop($1.op,&$2.mode,-2); }
		| CLR single_op2
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| CPL single_op2
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| SETB single_op2
				{ if( get_md($2.mode) == 0 )
					warn("Invalid addressing mode");
				  $$.value = makeop($1.op,&$2.mode,-1); }
		| PUSH data8
				{
				   struct mode tmp;
					set_md(tmp,0);
					set_ov(tmp,0);
					set_sz(tmp,1);
					set_b1(tmp,$2.value);
					$$.value = makeop($1.op,&tmp,0);
				}
		| POP data8
				{
				   struct mode tmp;
					set_md(tmp,0);
					set_ov(tmp,0);
					set_sz(tmp,1);
					set_b1(tmp,$2.value);
					$$.value = makeop($1.op,&tmp,0);
				}
		| LJMP addr16
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| LCALL addr16
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| JC relative
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| JNC relative
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| JNZ relative
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| JZ relative
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| SJMP relative
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| CJNE three_op1
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| JB two_op4
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| JNB two_op4
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| JBC two_op4
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| DJNZ two_op5
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| MOV two_op1
				{ $$.value = makeop($1.op,&$2.mode,0); }
		| MOV two_op2
				{ $$.value = makeop($1.op,&$2.mode,4); }
		| MOV two_op6
				{ $$.value = makeop($1.op,&$2.mode,6); }


		| MOVC A ',' '@' A '+' DPTR
				{ $$.value = makeop($1.op,NULL,0); }
		| MOVC A ',' '@' DPTR '+' A
				{ $$.value = makeop($1.op,NULL,0); }
		| MOVC A ',' '@' A '+' PC
				{ $$.value = makeop($1.op,NULL,1); }
		| MOVC A ',' '@' PC '+' A
				{ $$.value = makeop($1.op,NULL,1); }

		| MOVX A ',' '@' regi
				{ $$.value = makeop($1.op,NULL,$5.value); }
		| MOVX A ',' '@' DPTR
				{ $$.value = makeop($1.op,NULL,2); }
		| MOVX '@' regi ',' A
				{ $$.value = makeop($1.op,NULL,$3.value+3); }
		| MOVX '@' DPTR ',' A
				{ $$.value = makeop($1.op,NULL,5); }
		;




/* --------------------
 * ADDRESSING MODES:
 *
 */

two_op1		: A ',' reg
				{
					set_md($$.mode,0);
					set_ov($$.mode, $3.value);
					set_sz($$.mode, 0);
				}
		| A ',' data8
				{
					set_md($$.mode,1);
					set_ov($$.mode,0);
					set_sz($$.mode,1);
					set_b1($$.mode,$3.value);
				}
		| A ',' '@' regi
				{
					set_md($$.mode,2);
					set_ov($$.mode,$4.value);
					set_sz($$.mode,0);
				}
		| A ',' '#' data8
				{
					set_md($$.mode,3);
					set_ov($$.mode,0);
					set_sz($$.mode,1);
					set_b1($$.mode,$4.value);
				}
		;

two_op2		: data8 ',' A
				{
					set_md($$.mode,0);
					set_ov($$.mode,0);
					set_sz($$.mode,1);
					set_b1($$.mode,$1.value);
				}
		| data8 ',' '#' data8
				{
					set_md($$.mode,1);
					set_ov($$.mode,0);
					set_sz($$.mode,2);
					set_b1($$.mode,$1.value);
					set_b2($$.mode,$4.value);
				}
		;

two_op3		: C ',' bit
				{
					set_md($$.mode,0);
					set_ov($$.mode,0);
					set_sz($$.mode,1);
					set_b1($$.mode,$3.value);
				}
		| C ',' '/' bit
				{
					set_md($$.mode,1);
					set_ov($$.mode,0);
					set_sz($$.mode,1);
					set_b1($$.mode,$4.value);
				}
		| C ',' '!' bit
				{
					set_md($$.mode,1);
					set_ov($$.mode,0);
					set_sz($$.mode,1);
					set_b1($$.mode,$4.value);
				}
		;

two_op4		: bit ',' rel
				{
					set_md($$.mode,0);
					set_ov($$.mode,0);
					set_sz($$.mode,2);
					set_b1($$.mode,$1.value);
					set_b2($$.mode,$3.value);
				}
		;

two_op5		: reg ',' rel2
				{
					set_md($$.mode,0);
					set_ov($$.mode,$1.value);
					set_sz($$.mode,1);
					set_b1($$.mode,$3.value);
				}
		| data8 ',' rel
				{
					set_md($$.mode,1);
					set_ov($$.mode,0);
					set_sz($$.mode,2);
					set_b1($$.mode,$1.value);
					set_b2($$.mode,$3.value);
				}
		;

two_op6		: reg ',' A
				{
					set_md($$.mode,0);
					set_ov($$.mode,$1.value);
					set_sz($$.mode,0);
				}
		| reg ',' data8
				{
					set_md($$.mode,1);
					set_ov($$.mode,$1.value);
					set_sz($$.mode,1);
					set_b1($$.mode,$3.value);
				}
		| reg ',' '#' data8
				{
					set_md($$.mode,2);
					set_ov($$.mode,$1.value);
					set_sz($$.mode,1);
					set_b1($$.mode,$4.value);
				}
		| data8 ',' reg
				{
					set_md($$.mode,3);
					set_ov($$.mode,$3.value);
					set_sz($$.mode,1);
					set_b1($$.mode,$1.value);
				}
		| data8 ',' data8
				{
					set_md($$.mode,4);
					set_ov($$.mode,0);
					set_sz($$.mode,2);
					set_b1($$.mode,$3.value);
					set_b2($$.mode,$1.value);
				}
		| data8 ',' '@' regi
				{
					set_md($$.mode,5);
					set_ov($$.mode,$4.value);
					set_sz($$.mode,1);
					set_b1($$.mode,$1.value);
				}
		| '@' regi ',' A
				{
					set_md($$.mode,6);
					set_ov($$.mode,$2.value);
					set_sz($$.mode,0);
				}
		| '@' regi ',' data8
				{
					set_md($$.mode,7);
					set_ov($$.mode,$2.value);
					set_sz($$.mode,1);
					set_b1($$.mode,$4.value);
				}
		| '@' regi ',' '#' data8
				{
					set_md($$.mode,8);
					set_ov($$.mode,$2.value);
					set_sz($$.mode,1);
					set_b1($$.mode,$5.value);
				}
		| DPTR ',' '#' data16
			{
				set_md($$.mode,9);
				set_ov($$.mode,0);
				set_sz($$.mode,2);
				set_b1($$.mode, ($4.value & 0xff00) >> 8 );
				set_b2($$.mode, ($4.value & 0x00ff) );
			}
		| C ',' bit
				{
					set_md($$.mode,10);
					set_ov($$.mode,0);
					set_sz($$.mode,1);
					set_b1($$.mode,$3.value);
				}
	/*
	 * Following two productions cannot be represented by:
	 *
	 *	bit ',' C
	 *
	 * Because yacc gives tons of reduce/reduce errors if
 	 * that is attempted.
	 *
	 */
		| data8 ',' C
				{
					set_md($$.mode,11);
					set_ov($$.mode,0);
					set_sz($$.mode,1);
					set_b1($$.mode,$1.value);
				}
		| data8 BITPOS ',' C
{
	if( pass2 ) {
		if( !isbit8($1.value) )
			warn("Bit address exceeds 8-bits");
		if( isbmram($1.value) )
			set_b1($$.mode, ($1.value-0x20)*8+ $2.value );
		else if( isbmsfr($1.value) )
			set_b1($$.mode, $1.value + $2.value );
		else
			warn("Invalid bit addressable RAM location");
	}
	set_md($$.mode,11);
	set_ov($$.mode,0);
	set_sz($$.mode,1);
}
		;


single_op1	: A
				{
					set_md($$.mode,0);
					set_ov($$.mode,0);
					set_sz($$.mode,0);
				}

		| reg
				{
					set_md($$.mode,1);
					set_ov($$.mode,$1.value);
					set_sz($$.mode,0);
				}
		| data8
				{
					set_md($$.mode,2);
					set_ov($$.mode,0);
					set_sz($$.mode,1);
					set_b1($$.mode,$1.value);
				}
		| '@' regi
				{
					set_md($$.mode,3);
					set_ov($$.mode,$2.value);
					set_sz($$.mode,0);
				}
		;

single_op2	: A
				{
					set_md($$.mode,0);
					set_ov($$.mode,0);
					set_sz($$.mode,0);
				}
		| C
				{
					set_md($$.mode,1);
					set_ov($$.mode,0);
					set_sz($$.mode,0);
				}
		| bit
				{
					set_md($$.mode,2);
					set_ov($$.mode,0);
					set_sz($$.mode,1);
					set_b1($$.mode,$1.value);
				}
		;

three_op1	: A ',' data8 ',' rel
				{
					set_md($$.mode,0);
					set_ov($$.mode,0);
					set_sz($$.mode,2);
					set_b1($$.mode,$3.value);
					set_b2($$.mode,$5.value);
				}
		| A ',' '#' data8 ',' rel
				{
					set_md($$.mode,1);
					set_ov($$.mode,0);
					set_sz($$.mode,2);
					set_b1($$.mode,$4.value);
					set_b2($$.mode,$6.value);
				}
		| reg ',' '#' data8 ',' rel
				{
					set_md($$.mode,2);
					set_ov($$.mode,$1.value);
					set_sz($$.mode,2);
					set_b1($$.mode,$4.value);
					set_b2($$.mode,$6.value);
				}
		| '@' regi ',' '#' data8 ',' rel
				{
					set_md($$.mode,3);
					set_ov($$.mode,$2.value);
					set_sz($$.mode,2);
					set_b1($$.mode,$5.value);
					set_b2($$.mode,$7.value);
				}
		;

rel		: expr
{
		long offset;
		if( pass2 ) {
			offset = $1.val.v - (lc+3);
			if( offset > 127 || offset < -128 )
			   warn("Relative offset exceeds -128 / +127");
			$$.value = offset;
		}
}
		;

/*
 * This production differs from the above, by 1 number!
 *
 */

rel2		: expr
{
		long offset;
		if( pass2 ) {
			offset = $1.val.v - (lc+2); /* different! */
			if( offset > 127 || offset < -128 )
			   warn("Relative offset exceeds -128 / +127");
			$$.value = offset;
		}
}
		;


bit		: bitv BITPOS
{
	if( pass2 ) {
		if( !isbit8($1.value) )
			warn("Bit address exceeds 8-bits");
		if( isbmram($1.value) )
			$$.value = ($1.value-0x20)*8+$2.value;
		else if( isbmsfr($1.value) )
			$$.value = $1.value + $2.value;
		else
			warn("Invalid bit addressable RAM location");
	}
}
		| bitv
{
	if( pass2 ) {
		if( !isbit8($1.value) )
			warn("Bit address exceeds 8-bits");
		$$.value = $1.value;
	}
}
		;

bitv		: SYMBOL
{
	if( $1.sym->type == UNDEF && pass2 )
		warn("Symbol %s undefined",$1.sym->name);
	$$.value = $1.sym->value;
}
		| VALUE		{ $$.value = $1.value; }
		;

reg		: R0		{ $$.value = 0; }
		| R1		{ $$.value = 1; }
		| R2		{ $$.value = 2; }
		| R3		{ $$.value = 3; }
		| R4		{ $$.value = 4; }
		| R5		{ $$.value = 5; }
		| R6		{ $$.value = 6; }
		| R7		{ $$.value = 7; }
		;

regi		: R0		{ $$.value = 0; }
		| R1		{ $$.value = 1; }
		| R2
				{ $$.value = 0;
				  warn("Illegal indirect register: @r2"); }
		| R3
				{ $$.value = 0;
				  warn("Illegal indirect register: @r3"); }
		| R4
				{ $$.value = 0;
				  warn("Illegal indirect register: @r4"); }
		| R5
				{ $$.value = 0;
				  warn("Illegal indirect register: @r5"); }
		| R6
				{ $$.value = 0;
				  warn("Illegal indirect register: @r6"); }
		| R7
				{ $$.value = 0;
				  warn("Illegal indirect register: @r7"); }
		;

data8		: expr
{
	if( pass2 ) {
		if( !isbit8($1.val.v) )
			warn("Expression greater than 8-bits");
	}
	$$.value = $1.val.v;
}
		;

data16		: expr
{
	if( pass2 ) {
		if( !isbit16($1.val.v) )
			warn("Expression greater than 16-bits");
	}
	$$.value = $1.val.v;
}
		;

addr11		: expr
{
		if( pass2 ) {
			if( !isbit16($1.val.v)  )
				warn("Address greater than 16-bits");
			if( ($1.val.v & size11) != ((lc+2) & size11) )
				warn("Address outside current 2K page");
		}
		set_md($$.mode,0);
		set_ov($$.mode, ($1.val.v&0x0700)>>3 );
		set_sz($$.mode,1);
		set_b1($$.mode,$1.val.v&0x00ff);
}
		;

addr16		: expr
{
		if( pass2 ) {
			if( !isbit16($1.val.v)  )
				warn("Address greater than 16-bits");
		}
		set_md($$.mode,0);
		set_ov($$.mode, 0 );
		set_sz($$.mode,2);
		set_b1($$.mode, ($1.val.v & 0xff00 ) >> 8 );
		set_b2($$.mode, ($1.val.v & 0x00ff ) );
}
		;

relative	: expr
{
		long offset=0;
		if( pass2 ) {
			offset = $1.val.v - (lc+2);
			if( offset>127 || offset<-128 )
			   warn("Relative offset exceeds -128 / +127");
		}
		set_md($$.mode,0);
		set_ov($$.mode,0);
		set_sz($$.mode,1);
		set_b1($$.mode,offset);

}
		;

%%

/* ---------------------------------------------------------------------- */

void yyerror(const char *s)
{
	warn("%s near \"%s\"", s, get_last_token());
}


/* ----------------------------------------------------------------------
 * makeop:
 *	This function makes an opcode based on the instruction symbol table
 *	entry, and an addressing mode structure.
 *	This function is called from both passes, but
 *	only generates code in pass 2.
 *
 *	Resultant opcode bytes are passed to genbyte().
 *
 *	Returns the nuumber of bytes that the instruction
 *	occupies.
 *
 */

int makeop(struct opcode * op, struct mode *m, int add)
{
	register unsigned int newop;

	if( m == NULL ) {
		if(pass2) genbyte(op->bytes[0+add]);
		return(1);
	}

	if( pass2 ) {
		newop = op->bytes[ get_md(*m)+add ] | get_ov(*m);
		genbyte(newop);
		if( get_sz(*m) > 0 ) genbyte( get_b1(*m) );
		if( get_sz(*m) > 1 ) genbyte( get_b2(*m) );
	}
	return( get_sz(*m)+1 );
}


/* ----------------------------------------------------------------------
 * inclc:
 *	Increments the Location Counter by 'i' amount.
 *	Check to see if 'i' overflows 64K.
 *	Checks to see if assembler is overlapping previous sections
 *	of code. (using a large bit field).
 *
 */

#define indx(a) ( (a)/(sizeof(long)*8) )
#define bit(a)	( 1 << ((a)%(sizeof(long)*8)) )

#define getloc(a) (regions[indx(a)] & bit(a))
#define setloc(a) (regions[indx(a)] |= bit(a))

static unsigned long regions[ 0x10000/(sizeof(long)*8) ];

void inclc(int i)
{

	while (i-- > 0) {
		if( pass2 && getloc(lc) )
			error("Location counter overlaps");
		if( pass2 ) setloc(lc);
		lc += 1;
	}
		
	if( lc > 0xffff )
		error("Location counter has exceeded 16-bits");
}

void clear_location_counter(void)
{
	memset(regions, 0, sizeof(regions));
}


/* ----------------------------------------------------------------------
 * padline:
 *	This routine returns a new string, which is equivilant to
 *	'line' except that all tabs have been expanded to spaces, and
 *	the total length has been truncated to 60 chars.
 */

char *padline(char *line)
{
	static char newline[61];
	char *p1;
	int pos=0,nxtpos;

	for(p1=line; pos<sizeof(newline)-1 && *p1; p1++ ) {
		if( *p1 == '\t' ) {
			nxtpos = pos+8-pos%8;
			while(pos<sizeof(newline)-1 && pos <= nxtpos)
				newline[pos++] = ' ';
		} else if( *p1 != '\n' )
			newline[pos++]= *p1;
	}
	newline[pos] = '\0';
	return(newline);
}


/* ----------------------------------------------------------------------
 * dumplist:
 *	Outputs the current location counter, bytebuf[] array, and
 *	the string 'txt' to the listing file.
 *	This routine is called for every source line encountered in the
 *	source file. (Only in pass 2, and if listing is turned on).
 *
 */

void dumplist(char *txt, int show)
{
	int i,j;

	fprintf(listing,show?"%04X: ":"      ",lc);

	j=0;
	for(i=0; i<bytecount; i++ ) {
		fprintf(listing,"%02X ",bytebuf[i]);
		if( ++j >= 4 ) {
			j = 0;
			fprintf(listing,"\n      ");
		}
	}
	while(++j <= 4)
		fprintf(listing,"   ");

	fprintf(listing," %s\n",padline(txt));
}

/* ----------------------------------------------------------------------
 * gen* routines:
 *	Place information into the bytebuf[] array, and also
 *	call emitbyte with the byte.
 *
 */

void genbyte(int b)
{
	if( bytecount < sizeof(bytebuf) )
		bytebuf[bytecount++] = b;
	emitbyte(b);
}

void genstr(const char *s)
{
	while( *s )
		genbyte(*s++);
}

void genword(unsigned long w)
{
	genbyte( (w & 0xff00) >> 8 );
	genbyte( (w & 0x00ff) );
}

