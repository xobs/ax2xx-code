/* pass2.c
 *
 * Functions for pass 2 of the disassembler.
 *
 * Copyright 2001 - 2003 by David Sullins
 *
 * This file is part of Dis51.
 * 
 * Dis51 is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2 of the License.
 * 
 * Dis51 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Dis51; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * You may contact the author at davesullins@earthlink.net.
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "disasm-mnem.h"

static void pass1(uint8_t *bfr, int size, int offset);

/* nextbyte: read a byte from memory
 *
 */
uint8_t nextbyte(uint8_t *bfr, int *addr) {
    uint8_t c = bfr[*addr];
    (*addr)++;
    return c;
}

/* longaddr
 *
 */
static int longaddr_print(uint8_t op1, uint8_t op2, char *label)
{
	int addr;
	
	/* calculate address */
	addr = ((((int)op1)<<8) | op2);
	
	/* form label string */
	sprintf(label, "label_%d", lbl[addr]);
	
	return lbl[addr];
}

/* absaddr
 *
 */
static int absaddr_print(uint8_t opcode, uint8_t operand, int memPtr, char *label)
{
	int addr;
	
	/* calculate address */
	addr = ((memPtr & 0xf800) | (((int)opcode & 0xe0)<<3) | operand);
	
	/* form label string */
	sprintf(label, "label_%d", lbl[addr]);
	
	return lbl[addr];
}

/* reladdr
 *
 */
static int reladdr_print(uint8_t operand, int memPtr, char *label)
{
	int addr;
	
	/* calculate address */
	addr = memPtr + (signed char)operand;
	
	/* form label string */
	sprintf(label, "label_%d", lbl[addr]);
	
	return lbl[addr];
}


/* printhex
 *
 * Pretty format a hexadecimal number in a string.
 */
static void printhex(uint8_t num, char *name)
{
	if ((num >= 0xa0) || ((num <= 0xf) && (num >= 0xa)))
		snprintf(name, 5, "0x0%x", num);
	else
		snprintf(name, 5, "0x%x", num);
}

/* sfrdecode
 *
 * Convert a direct memory value to a SFR name if appropriate.
 * Otherwise output the formatted number.
 */
static void sfrdecode(uint8_t sfr, char *name)
{
	if (sfr & 0x80)
		strncpy(name, sfrname[sfr&0x7f], 5);
	else
		printhex(sfr, name);
}

/* sfbitdecode
 *
 * Convert a bit memory value to a SF bit name if appropriate.
 * Otherwise output the formatted number.
 */
static void sfbitdecode(uint8_t sfbit, char *name)
{
	if (sfbit & 0x80)
		strncpy(name, sfbitname[sfbit&0x7f], 6);
	else
		printhex(sfbit, name);
}

/* _listhex, listhex
 *
 * Output list-format address and data for the -l command line switch.
 *
 * Contributed by Peter Peres.
 */
#define listhex(len,addr,ofile) { if(Lst) _listhex(ofile,bfr,addr,len); }
static void _listhex(FILE *file, uint8_t *bfr, int addr, int len)
{
	fprintf( file, "  %04X %02X", addr-len, bfr[addr-len] );
	switch(len) {
	  case 3:
	    fprintf( file, "%02X", bfr[addr-2] );
	  case 2:
	  	fprintf( file, "%02X", bfr[addr-1] );
		if(len == 2)
			fprintf( file, "  ");
		break;
	  default:
	    fprintf( file, "    ");
	}
	fprintf( file, "\t\t" );
}

/* dis_inst2: Disassemble one instruction to ofile
 * 
 * Returns address immediately after instruction.
 */
static int dis_inst2(FILE *ofile, uint8_t *bfr, int addr)
{
	uint8_t opcode;
	uint8_t op1, op2;
	char label[6];
	char name[6];
	char name2[5];
	int bytes = 1;

	/* Fetch opcode */
	opcode = nextbyte(bfr, &addr);
	
	/* Fetch second and third byte, if appropriate */
	if (op_format[opcode] > 1) {
		op1 = nextbyte(bfr, &addr);
		bytes = 2;
	}
	if (op_format[opcode] > 6) {
		op2 = nextbyte(bfr, &addr);
		bytes = 3;
	}
	
	/* Output decoded instruction */
	if(!Lst)
		fprintf(ofile, "\t");
	else
		listhex(bytes, addr, ofile);
	switch(op_format[opcode]) {
		case 0:
				/* A5 is an illegal opcode */
			fprintf(ofile, "DB 85h  ; illegal opcode");
		case 1:
				/* no operands */
			fprintf(ofile, mnemonic[opcode]);
			break;
		case 2:
				/* one immediate operand */
			printhex(op1, name);
			fprintf(ofile, mnemonic[opcode], name);
			break;
		case 3:
				/* one direct operand */
			sfrdecode(op1, name);
			fprintf(ofile, mnemonic[opcode], name);
			break;
		case 4:
				/* one bit-addressed operand */
			sfbitdecode(op1, name);
			fprintf(ofile, mnemonic[opcode], name);
			break;
		case 5:
				/* one operand, relative address */
			/* relative addr calculation */
			reladdr_print(op1, addr, label);
			fprintf(ofile, mnemonic[opcode], label);
			break;
		case 6:
				/* one operand, absolute address */
			/* absolute addr calculation */
			absaddr_print(opcode, op1, addr, label);
			fprintf(ofile, mnemonic[opcode], label);
			break;
		case 7:
				/* two-byte immediate operand */
			/* MOV DPTR, #immediate16 */
			fprintf(ofile, mnemonic[opcode], op1, op2);
			break;
		case 8:
				/* two operands: direct, immediate */
			sfrdecode(op1, name);
			printhex(op2, name2);
			fprintf(ofile, mnemonic[opcode], name, name2);
			break;
		case 9:
				/* two operands: direct, direct */
			/* (operands in reverse order) */
			sfrdecode(op1, name);
			sfrdecode(op2, name2);
			fprintf(ofile, mnemonic[opcode], name2, name);
			break;
		case 10:
				/* two operands: immediate, relative */
			printhex(op1, name);
			reladdr_print(op2, addr, label);
			fprintf(ofile, mnemonic[opcode], name, label);
			break;
		case 11:
				/* two operands: direct, relative */
			sfrdecode(op1, name);
			reladdr_print(op2, addr, label);
			fprintf(ofile, mnemonic[opcode], name, label);
			break;
		case 12:
				/* two operands: bit, relative */
			sfbitdecode(op1, name);
			reladdr_print(op2, addr, label);
			fprintf(ofile, mnemonic[opcode], name, label);
			break;
		case 13:
				/* long address */
			/* long addr calculation */
			longaddr_print(op1, op2, label);
			fprintf(ofile, mnemonic[opcode], label);
			break;
		default:
				/* error in op_format table */
			fprintf(ofile, "DB 0%Xh  ; error in op_format table",
			        opcode);
	}
	fprintf(ofile, "\n");
	
	return addr;
}

/* pass2: Disassemble memory to given output file (pass 2)
 *
 */
int disasm_8051(FILE *ofile, uint8_t *src_bfr, int size, int offset) {
	int addr = offset;
    uint8_t bfr[size+offset];
    memset(bfr, 0, sizeof(bfr));
    memcpy(bfr+offset, src_bfr, size);

    pass1(bfr, size, offset);
	
    fprintf(ofile, ".org 0x%04X\n", addr);
	while (addr < offset+size)
	{
        /* Step 2: Output a label if one exists */
        if (lbl[addr])
            fprintf(ofile, "label_%d:\n", lbl[addr]);

        /* Disassemble next instruction */
        addr = dis_inst2(ofile, bfr, addr);
	}
    return 0;
}


/* newlbl is used by longaddr, absaddr, and reladdr 
 * Keep track of next available label
 */
static unsigned short newlbl = 0;

/* longaddr
 *
 */
static int longaddr(uint8_t op1, uint8_t op2)
{
	int addr;
	
	/* calculate address */
	addr = ((((int)op1)<<8) | op2);
	
	/* check if this address has already been labelled */
	if (lbl[addr] == 0)
		lbl[addr] = ++newlbl;
	
	return addr;
}

/* absaddr
 *
 */
static int absaddr(uint8_t opcode, uint8_t operand, int memPtr)
{
	int addr;
	
	/* calculate address */
	addr = ((memPtr & 0xf800) | (((int)opcode & 0xe0)<<3) | operand);
	
	/* check if this address has already been labelled */
	if (lbl[addr] == 0)
		lbl[addr] = ++newlbl;
	
	return addr;
}

/* reladdr
 *
 */
static int reladdr(uint8_t operand, int memPtr)
{
	int addr;
	
	/* calculate address */
	addr = memPtr + (signed char)operand;
	
	/* check if this address has already been labelled */
	if (lbl[addr] == 0)
		lbl[addr] = ++newlbl;
	
	return addr;
}

/* dis_inst: Disassemble one instruction (pass 1)
 * 
 * Also increments memPtr to point to the next instruction address.
 *
 * Return -1 on error.
 * Otherwise, return opcode byte.
 *
 * CAVEAT: Indirect jumps not handled (JMP @A+DPTR)
 */
static int dis_inst1(uint8_t *bfr, int size, int addr)
{
	uint8_t opcode;
	uint8_t op1, op2;
	int newaddr = -1;
	
	opcode = nextbyte(bfr, &addr);
	
	switch(op_format[opcode]) {
		case 0:
				/* A5 is an illegal opcode */
			fprintf(stderr,
			   "Illegal opcode A5 at address %04X\n", addr-1);
			newaddr = -1;
			break;
		case 1:
				/* no operands */
			newaddr = addr;
			/* if this is a return, stop disassembly */
			if ((opcode & 0xef) == 0x22)
				newaddr = -1;
			/* we don't handle indirect jumps */
			else if (opcode == 0x73)
				newaddr = -1;
			break;
		case 2:
		case 3:
		case 4:
				/* one operand */
			nextbyte(bfr, &addr);
			newaddr = addr;
			break;
		case 5:
				/* one operand, relative address */
			op1 = nextbyte(bfr, &addr);
			/* relative addr calculation */
			newaddr = reladdr(op1, addr);
			/* if this is a branch, continue disassembly */
//			if (opcode != 0x80)
//				pass1(bfr, size, addr);
			break;
		case 6:
				/* one operand, absolute address */
			op1 = nextbyte(bfr, &addr);
			/* absolute addr calculation */
			newaddr = absaddr(opcode, op1, addr);
			/* if this is a call, continue disassembly */
//			if (opcode & 0x10)
//				pass1(bfr, size, addr);
			break;
		case 7:
		case 8:
		case 9:
				/* two operands */
			nextbyte(bfr, &addr);
			nextbyte(bfr, &addr);
			newaddr = addr;
			break;
		case 10:
		case 11:
		case 12:
				/* two operands, relative address */
			nextbyte(bfr, &addr);
			op2 = nextbyte(bfr, &addr);
			/* relative addr calculation */
			newaddr = reladdr(op2, addr);
			/* this is always a branch instruction */
//			pass1(bfr, size, addr);
			break;
		case 13:
				/* long address */
			op1 = nextbyte(bfr, &addr);
			op2 = nextbyte(bfr, &addr);
			/* long addr calculation */
			newaddr = longaddr(op1, op2);
			/* if this is a call, continue disassembly */
//			if (opcode == 0x12)
//				pass1(bfr, size, addr);
			break;
		default:
				/* error in op_format table */
			fprintf(stderr, 
			     "Invalid opcode format, error in format table\n");
			newaddr = -1;
			break;
	}
	
	return addr;
}

/* pass1: Disassemble instructions starting at given entry point (pass 1)
 *
 */
static void pass1(uint8_t *bfr, int size, int offset) {
    int base = offset;
	while ((offset != -1) && (offset >= base) && (offset < base+size)) {
		/* no error, we haven't been here before, and non-empty */
		/* disassemble next instruction */
		offset = dis_inst1(bfr, size, offset);
    }
}
