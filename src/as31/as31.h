/* ----------------------------------------------------------------------
 * FILE: as31.h
 * PACKAGE: as31 - 8031/8051 Assembler.
 *
 * DESCRIPTION:
 *	The sole header file for the 8031/8051 assembler package.
 *	It defines several structures used by the yacc stack.
 *	It defines several macros for testing the bitsize of numeric
 *	quantities.
 *
 *	Some macros to extract information from the mode structure.
 *
 * REVISION HISTORY:
 *	Jan. 19, 1990 - Created. (Ken Stauffer)
 *
 * AUTHOR:
 *	All code in this file written by Ken Stauffer (University of Calgary).
 *	January, 1990.
 *
 */

#include <stdio.h>

/* ----------------------------------------------------------------------
 * user / keyword symbol structures:
 */

struct opcode {
	char *name;
	int type;
	unsigned char *bytes;
};

struct symbol {
	char *name;
	char predefined;
	char type;
	long value;
	struct symbol *next;
};

#define UNDEF	0
#define LABEL	1

/* these are unused... maybe someday the can be used and the */
/* parser can check that each addressing mode is using the */
/* correct type of data, or a generic LABEL type */
#define BIT	2
#define IMEM	3
#define EMEM	3
#define CONST	4
#define REG	5

/* ----------------------------------------------------------------------
 * addressing mode stuff:
 */

struct mode {
	unsigned char mode;		/* value to index with */
	unsigned char size;		/* # of bytes used */
	unsigned char orval;		/* value OR'd to obcode */
	unsigned char byte1;		/* extra byte 1 */
	unsigned char byte2;		/* extra byte 2 */
};

#define set_md(m,a)	((m).mode=(a))
#define set_sz(m,a)	((m).size=(a))
#define set_ov(m,a)	((m).orval=(a))
#define set_b1(m,a)	((m).byte1=(a))
#define set_b2(m,a)	((m).byte2=(a))

#define get_md(m)	((m).mode)
#define get_sz(m)	((m).size)
#define get_ov(m)	((m).orval)
#define get_b1(m)	((m).byte1)
#define get_b2(m)	((m).byte2)

/* ----------------------------------------------------------------------
 * yacc stack stuff:
 */

struct value {
	long v;
	unsigned char d;		/* expression defined flag */
};

union ystack {
	long value;
	struct value val;
	struct opcode *op;
	struct symbol *sym;
	struct mode mode;
	char *str;
};

#define YYSTYPE union ystack

/* ----------------------------------------------------------------------
 * IS_BIT_MAPPED_RAM:
 *	True is the byte 'a' is the byte address of a bit mapped
 *	RAM location.
 */
#define isbmram(a)	(((a)&0xf0)==0x20)

/* ----------------------------------------------------------------------
 * IS_BIT_MAPPED_SFR:
 *	True is the byte 'a' is the byte address of a bit mapped
 *	SPECIAL FUCTION REGISTER.
 */
#define isbmsfr(a)	(((a)&0x80) && !((a) & 0x07))

/* ----------------------------------------------------------------------
 * isbit8, ...
 *	Macros to check the sizes of values and to convert
 *	a value to a certain, size.
 *
 */
#define size8		(~0x00ff)
#define size11		(~0x07ff)
#define size13		(~0x1fff)
#define size16		(~0xffff)

#define size10		(~0x03ff)
#define size12		(~0x0fff)
#define size15		(~0x7fff)

#define isbit8(v)	( !( ((v)>=0) ? (v)&size8 : -(v)>=128) )
#define isbit11(v)	( !( ((v)>=0) ? (v)&size11 : (-(v))&size10 ) )
#define isbit13(v)	( !( ((v)>=0) ? (v)&size13 : (-(v))&size12 ) )
#define isbit16(v)	( !( ((v)>=0) ? (v)&size16 : (-(v))&size15 ) )

/* ----------------------------------------------------------------------
 * Size of user hash table.  Use a prime number for best performance
 */
#define HASHTABSIZE		4999

/* ----------------------------------------------------------------------
 * Macros to nicely test which pass we are in.
 */
#define pass1			(!pass)
#define pass2			(pass)



/* from lexer.c */
extern int yylex(void);
extern int seek_eol(void);
extern const char *get_last_token(void);
extern int lineno;


/* from parser.y */
extern int yyparse(void);
extern void clear_location_counter(void);


/* from emitter.c */
extern void emitusage(void);
extern const char *emit_extension(const char *ftype);
extern const char *emit_desc_lookup(int num);
extern const char *emit_desc_to_name_lookup(const char *desc);
extern int emitopen(const char *file, const char *ftype, const char *arg);
extern void emitclose(void);
extern void emitaddr(unsigned long a);
extern void emitbyte(int b);


/* from symbol.c */
extern struct opcode *lookop(const char *s);
extern struct symbol *looksym(const char *s);
extern void syminit(void);
extern void freesym(void);


/* from run.c */
extern int run_as31(const char *infile, int lst, int use_stdout,
        const char *fmt, const char *arg);
extern void error(const char *fmt, ...);
extern void warn(const char *fmt, ...);
extern void mesg_f(const char *fmt, ...);
extern int dashl;
extern int pass;
extern int abort_asap;
extern unsigned long lc;
extern FILE *listing;


/* from as31.c or as31_gtk.c */
extern void mesg(const char *str);


