/* ----------------------------------------------------------------------
 * FILE: emitter.c
 * PACKAGE: as31 - 8031/8051 Assembler.
 *
 * DESCRIPTION:
 *	This file contains the code to generate various
 *	object code formats. Provisions exist to
 *	support many types of output formats within the
 *	same executable.
 *
 * REVISION HISTORY:
 *	Jan. 19, 1990 - Created. (Ken Stauffer)
 *	Jan. 29, 1990 - Added S-records (Theo Deraadt)
 *
 *
 * AUTHOR:
 *	All code in this file written by Ken Stauffer (University of Calgary).
 *	January, 1990.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "as31.h"

/* ----------------------------------------------------------------------
 * DECLARE your own open(), close(), addr(), and byte() routines here.
 *
 */

static int open_tdr(const char *file, const char *ftype, const char *arg);
static void close_tdr(void);
static void addr_tdr(unsigned long a);
static void byte_tdr(unsigned char b);

static int open_byte(const char *file, const char *ftype, const char *arg);
static void close_byte(void);
static void addr_byte(unsigned long a);
static void byte_byte(unsigned char b);

static int open_od(const char *file, const char *ftype, const char *arg);
static void close_od(void);
static void addr_od(unsigned long a);
static void byte_od(unsigned char b);

static int open_srec(const char *file, const char *ftype, const char *arg);
static void close_srec(void);
static void addr_srec(unsigned long a);
static void byte_srec(unsigned char b);

static int open_hex(const char *file, const char *ftype, const char *arg);
static void close_hex(void);
static void addr_hex(unsigned long a);
static void byte_hex(unsigned char b);

static int open_bin(const char *file, const char *ftype, const char *arg);
static void close_bin(void);
static void addr_bin(unsigned long a);
static void byte_bin(unsigned char b);

/* ----------------------------------------------------------------------
 * ADD an entry to this table to register your
 * output format routines. Give your object format
 * a name to be specified with the -F option.
 *
 */

static int format;

static struct {
	char *name;
	char *extension;
	char *description;
	int (*e_open)(const char *file, const char *ftype, const char *arg);
	void (*e_close)(void);
	void (*e_addr)(unsigned long a);
	void (*e_byte)(unsigned char b);
} formtab[] = {
	{ "hex",   "hex",  "Intel-Hex (.hex)",  open_hex,  close_hex,  addr_hex,  byte_hex },
	{ "bin",   "bin",  "Binary (.bin)",  open_bin,  close_bin,  addr_bin,  byte_bin },
	{ "tdr",   "tdr",  "TDR Format (.tdr)", open_tdr,  close_tdr,  addr_tdr,  byte_tdr },
	{ "byte",  "byte", "BYTE list (.byte)",  open_byte, close_byte, addr_byte, byte_byte },
	{ "od",    "od",   "Octal Dump (.od)", open_od,   close_od,   addr_od,   byte_od },
	{ "srec2", "srec", "S Record, 16 bit Address (.srec)", open_srec, close_srec, addr_srec, byte_srec },
	{ "srec3", "srec", "S Record, 24 bit Address (.srec)", open_srec, close_srec, addr_srec, byte_srec },
	{ "srec4", "srec", "S Record, 32 bit Address (.srec)", open_srec, close_srec, addr_srec, byte_srec }
};

#define FORMTABSIZE	(sizeof(formtab)/sizeof(formtab[0]))

void emitusage(void)
{
	int i;

	fprintf(stderr, "\tfmt is one of:");
	for(i=0; i<FORMTABSIZE; ) {
		fprintf(stderr, "%s", formtab[i].name);
		if( ++i < FORMTABSIZE)
			fprintf(stderr, ", ");
	}
	fprintf(stderr, ".\n");
}

const char *emit_extension(const char *ftype)
{
	int i;

	if (ftype) {
		for(i=0; i<FORMTABSIZE; i++ ) {
			if (!strcmp(formtab[i].name, ftype))
				return formtab[i].extension;
		}
	}
	return formtab[0].extension;
}


const char *emit_desc_lookup(int num)
{
	if (num >= 0 && num < FORMTABSIZE)
		return formtab[num].description;
	return NULL;
}

const char *emit_desc_to_name_lookup(const char *desc)
{
	int i;

	if (desc) {
		for(i=0; i<FORMTABSIZE; i++ ) {
			if (!strcmp(formtab[i].description, desc))
				return formtab[i].name;
		}
	}
	return NULL;
}


int emitopen(const char *file, const char *ftype, const char *arg)
{
	int i;
	if( ftype ) {
		for(i=0; i<FORMTABSIZE; i++ ) {
			if( !strcmp(formtab[i].name,ftype) ) {
				format = i;
				return (*formtab[format].e_open)
					(file,ftype,arg);
			}
		}
		mesg_f("no format \"%s\", using \"%s\"\n",
			ftype, formtab[0].name);
	}
	/*
	 * 0th entry is the default format type
	 */
	format = 0;
	return (*formtab[format].e_open)(file, ftype, arg);
}

void emitclose(void)
{
	(*formtab[format].e_close)();
}

void emitaddr(unsigned long a)
{
	(*formtab[format].e_addr)(a);
}

void emitbyte(int b)
{
	(*formtab[format].e_byte)(b);
}

/* ----------------------------------------------------------------------
 * Individual file format routines appear here:
 *	Each file format must define the following routines:
 *		open()	- Called ONCE before any of the others.
 *			It is passed with a filename and a format
 *			specific argument.
 *
 *		close() - Called ONCE when no more emit_byte()
 *			function calls will be made.
 *
 *		addr() - Called when ever a new address has been set
 *			in the assembler (ie. .org, .skip).
 *			This routine is also called once when the
 *			location counter is set to 0 at the very start of
 *			assembling.
 * 
 *		byte() - Called with each byte to be outputed.
 *
 */


static unsigned long addr;
static FILE *fout=NULL;
static long int offset;
static int newaddr;
static int pos=-666;
static unsigned char bytes[16];


/*-----------------------------------------------------------------------
 * "hex" format.  Intel HEX format expected by many EPROM programmers
 */


void hexdump(void)     /* dumps one line into file */
{
	int i, sum;

	if (fout == NULL) return;
	fprintf(fout,":%02X%04lX00", pos, addr & 0xFFFF);
	sum = pos + ((addr>>8)&0xff) + (addr&0xff) ;
	for (i=0; i < pos; i++) {
		fprintf(fout,"%02X", bytes[i] & 0xFF );
		sum += bytes[i]&0xff;
	}
	fprintf(fout, "%02X\n", (-sum)&0xff);
	addr += pos;
	pos = 0;
}

static int open_hex(const char *file, const char *ftype, const char *arg)
{
	if (file == NULL) fout = stdout;
	else fout = fopen(file, "w");

	if( fout == NULL ) {
		mesg_f("Cannot open %s for writing.\n", file);
		return -1;
	}
	pos = 0;
	return 0;
}

static void close_hex(void)
{
	if (fout == NULL) return;
	if ( pos > 0 ) hexdump();
	fprintf(fout, ":00000001FF\n");  /* end of file marker */
	fclose(fout);
}

static void addr_hex(unsigned long a)
{
	if ( pos > 0 ) hexdump();
	addr = a;
}

static void byte_hex(unsigned char b)
{
	bytes[pos] = b;
	pos += 1;
	if ( pos == 16) hexdump();
}


/*-----------------------------------------------------------------------
 * "bin" format.  Raw binary data
 */

static int open_bin(const char *file, const char *ftype, const char *arg)
{
	if (file == NULL) fout = stdout;
	else fout = fopen(file, "w");

	if( fout == NULL ) {
		mesg_f("Cannot open %s for writing.\n", file);
		return -1;
	}
	addr = 0;
	return 0;
}

static void close_bin(void)
{
	if (fout == NULL) return;
	fclose(fout);
}

static void addr_bin(unsigned long a)
{
	long i;

	if (fout == NULL) return;
	if (a > addr) {
		for (i=0; i < a - addr; i++) {
			fprintf(fout, "%c", 0);
		}
		addr = a;
		return;
	}
	if (a < addr) {
		error("address changed to %lX, from %lX", a, addr);
		error("binary output format can't write backwards");
		addr = a;
		return;
	}
}

static void byte_bin(unsigned char b)
{
	if (fout == NULL) return;
	fprintf(fout, "%c", b & 0xFF);
	addr++;
}


/* ----------------------------------------------------------------------
 * "tdr" format. For tdr's 68008 system. Generates a
 * script file readable by a debugger.
 *      [addr] : [byte] [byte] ..
 *
 * arg: This is a number in decimal which specifies
 *      the offset, -Ftdr -A0000
 *
 *      These options specifies the tdr format, with an argument
 *      of 0. This becomes the offset used in generating the
 *      script file. The default if no A is present is 0x10000.
 * 
 */

static int open_tdr(const char *file, const char *ftype, const char *arg)
{
	if (file == NULL) fout = stdout;
	else fout = fopen(file,"w");

	if( fout == NULL ) {
		mesg_f("Cannot open %s for writing.\n",file);
		return -1;
	}
	if (arg) {
		offset = atoi(arg);
	} else {
		offset = 0x10000;
	}
	return 0;
}

static void close_tdr(void)
{
	if (fout == NULL) return;
	if( pos != 15 ) fprintf(fout,"\n");
	fclose(fout);
}

static void addr_tdr(unsigned long a)
{
	addr = a;
	newaddr = 1;
}

static void byte_tdr(unsigned char b)
{
	if (fout == NULL) return;
	if( newaddr ) {
		if( pos != -666 ) fprintf(fout,"\n");
		newaddr = 0;
		pos = 15;
		fprintf(fout,"%06lx: ", (addr + offset) & 0xFFFFFF);
	} else if( pos == 15 ) {
		fprintf(fout,"%06lx: ", (addr + offset) & 0xFFFFFF);
	}

	fprintf(fout,"%02x ", b & 0xFF );

	if( pos-- == 0 ) {
		fprintf(fout, "\n");
		pos = 15;
	}
	addr += 1;
}


/* ----------------------------------------------------------------------
 * "byte" format.
 *	Like "tdr" but each byte is on a line by itself.
 *	This is nice for debugging. No -A is used.
 */

static int open_byte(const char *file, const char *ftype, const char *arg)
{
	if (file == NULL) fout = stdout;
	else fout = fopen(file,"w");

	if( fout == NULL ) {
		mesg_f("Cannot open %s for writing.\n", file);
		return -1;
	}
	return 0;
}

static void close_byte(void)
{
	if (fout == NULL) return;
	fclose(fout);
}

static void addr_byte(unsigned long a)
{
	addr = a;
}

static void byte_byte(unsigned char b)
{
	if (fout == NULL) return;
	fprintf(fout,"%04lX: %02X\n", addr & 0xFFFF, b & 0xFF );
	addr += 1;
}


/* ----------------------------------------------------------------------
 * "od", this format shows 16 bytes per line, with address.
 *	It also includes ascii on one side.
 *
 * The format is similar to the od(1) program under Unix.
 *
 */

static int od_pos;
static unsigned char od_buf[16];
static unsigned long saveaddr;

void dumpline(unsigned long a, unsigned char *b, int len)
{
	int i;

	if (fout == NULL) return;
	if(len <= 0 ) return;

	fprintf(fout,"%04lx: ", a & 0xFFFF);

	for(i=0; i<8; i++ ) {
		if( i <= len )
			fprintf(fout,"%02x ",b[i]);
		else
			fprintf(fout,"   ");
	}

	fprintf(fout,"- ");

	for(i=8; i<16; i++ ) {
		if( i <= len )
			fprintf(fout,"%02x ",b[i]);
		else
			fprintf(fout,"   ");
	}
	fprintf(fout,"   ");

	for(i=0; i<16; i++ ) {
		if( i <= len )
			fprintf(fout,"%c",
				(b[i]>=' ' && b[i]<='~') ? b[i] : '.' );
		else
			break;
	}
	fprintf(fout,"\n");
}

static int open_od(const char *file, const char *ftype, const char *arg)
{
	if (file == NULL) fout = stdout;
	else fout = fopen(file,"w");

	if( fout == NULL ) {
		mesg_f("Cannot open %s for writing.\n",file);
		return -1;
	}
	return 0;
}

static void close_od(void)
{
	if (fout == NULL) return;
	dumpline(saveaddr, od_buf, od_pos-1);
	fclose(fout);
}

static void addr_od(unsigned long a)
{
	newaddr = 1;
	addr = a;
}

static void byte_od(unsigned char b)
{
	if( newaddr ) {
		dumpline(saveaddr, od_buf, od_pos-1);
		od_pos = 0;
		newaddr = 0;
		saveaddr = addr;
	} else if( od_pos == 16 ) {
		dumpline(saveaddr, od_buf, od_pos-1);
		od_pos = 0;
		saveaddr = addr;
	}
	od_buf[od_pos++] = b & 0x00ff;

	addr += 1;
}


/* ----------------------------------------------------------------------
 * srecord format. This is called with "-Fsrec2", "-Fsrec3", or
 * "-Fsrec4"...
 *
 * arg: This is a number in decimal which specifies
 *	the offset, -Fsrec3 -A0000
 *
 *	These options specifies the tdr format, with an argument
 *	of 0. This becomes the offset used in generating the
 *	script file. The default if no A is present is 0x0000.
 * 
 */

#define SREC_BYTESPERLINE 32

static int srec_format;
static int srec_check, srec_index;
static char srec_buf[SREC_BYTESPERLINE];
static long srec_address;

void srec_finishline(void)
{
	int i;

	if (fout == NULL) return;
	srec_check = srec_index + (srec_address & 0xff) + ((srec_address>>8) & 0xff) + 4;

	switch(srec_format) {
	case 2:
		fprintf(fout, "S1%02X%04lX", srec_index + 4,
			srec_address & 0xFFFF);
		break;
	case 3:
		fprintf(fout, "S2%02X%06lX", srec_index + 6,
			srec_address & 0xFFFFFF);
		srec_check += ((srec_address>>16) & 0xff) + 2;
		break;
	case 4:
		fprintf(fout, "S3%02X%08lX", srec_index + 8, srec_address);
		srec_check += ((srec_address>>16) & 0xff) +
			((srec_address>>24) & 0xff) + 4;
		break;
	}

	for(i=0; i<srec_index; i++) {
		fprintf(fout, "%02X", srec_buf[i] & 0xff);
		srec_check += srec_buf[i];
	}

	fprintf(fout, "%02X\n", (~srec_check & 0xff) );
	srec_index = 0;
}

static int open_srec(const char *file, const char *ftype, const char *arg)
{
	if (ftype == NULL) {
		mesg_f("No S Record Format\n");
		return -1;
	}

	if (sscanf(ftype, "srec%d", &srec_format) != 1) {
		mesg_f("Illegal S Record format \"%s\", must be \"srec2\", \"srec3\", or \"srec4\"\n", ftype);
		return -1;
	}

	if (srec_format < 2 || srec_format > 4) {
		mesg_f("Illegal S Record format %d %s (must be 2, 3, or 4)\n",
			srec_format, ftype);
		return -1;
	}

	if (file == NULL) fout = stdout;
	else fout = fopen(file,"w");

	if( fout == NULL ) {
		mesg_f("Cannot open %s for writing.\n",file);
		return -1;
	}

	if(arg)	offset = atoi(arg);
	else	offset = 0;

	fprintf(fout, "S0030000%02X\n", (~3 & 0xff) );
	return 0;
}

static void close_srec(void)
{
	if (fout == NULL) return;
	if(srec_index)
		srec_finishline();
	switch(srec_format) {
	case 2:
		fprintf(fout, "S9030000%02X\n", ~3 & 0xff);
		break;
	case 3:
		fprintf(fout, "S804000000%02X\n", ~4 & 0xff);
		break;
	case 4:
		fprintf(fout, "S70500000000%02X\n", ~5 & 0xff);
		break;
	}
	fclose(fout);
}

static void addr_srec(unsigned long a)
{
	if(srec_index>0)
		srec_finishline();
	srec_address = a + offset;
}

static void byte_srec(unsigned char b)
{
	srec_buf[srec_index++] = b;
	if(srec_index==SREC_BYTESPERLINE) {
		srec_finishline();
		srec_address += SREC_BYTESPERLINE;
	}
}


