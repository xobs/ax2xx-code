/* global.c
 *
 * Data shared by all modules
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
 * WArrANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Dis51; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * You may contact the author at davesullins@earthlink.net.
 */

int Lst;
short lbl[65536];

const char mnemonic[256][20] = {
/* 0 */
	"nop",
	"ajmp %s\n",
	"ljmp %s\n",
	"rr A",
	"inc A",
	"inc %s",
	"inc @R0",
	"inc @R1",
	"inc R0",
	"inc R1",
	"inc R2",
	"inc R3",
	"inc R4",
	"inc R5",
	"inc R6",
	"inc R7",
/* 1 */
	"jbc %s, %s",
	"acall %s",
	"lcall %s",
	"rrc A",
	"dec A",
	"dec %s",
	"dec @R0",
	"dec @R1",
	"dec R0",
	"dec R1",
	"dec R2",
	"dec R3",
	"dec R4",
	"dec R5",
	"dec R6",
	"dec R7",
/* 2 */
	"jb %s, %s",
	"ajmp %s\n",
	"ret\n",
	"rl A",
	"add A, #%s",
	"add A, %s",
	"add A, @R0",
	"add A, @R1",
	"add A, R0",
	"add A, R1",
	"add A, R2",
	"add A, R3",
	"add A, R4",
	"add A, R5",
	"add A, R6",
	"add A, R7",
/* 3 */
	"jnb %s, %s",
	"acall %s",
	"reti\n",
	"rlc A",
	"addc A, #%s",
	"addc A, %s",
	"addc A, @R0",
	"addc A, @R1",
	"addc A, R0",
	"addc A, R1",
	"addc A, R2",
	"addc A, R3",
	"addc A, R4",
	"addc A, R5",
	"addc A, R6",
	"addc A, R7",
/* 4 */
	"jc %s",
	"ajmp %s\n",
	"orl %s, A",
	"orl %s, #%s",
	"orl A, #%s",
	"orl A, %s",
	"orl A, @R0",
	"orl A, @R1",
	"orl A, R0",
	"orl A, R1",
	"orl A, R2",
	"orl A, R3",
	"orl A, R4",
	"orl A, R5",
	"orl A, R6",
	"orl A, R7",
/* 5 */
	"jnc %s",
	"acall %s",
	"anl %s, A",
	"anl %s, #%s",
	"anl A, #%s",
	"anl A, %s",
	"anl A, @R0",
	"anl A, @R1",
	"anl A, R0",
	"anl A, R1",
	"anl A, R2",
	"anl A, R3",
	"anl A, R4",
	"anl A, R5",
	"anl A, R6",
	"anl A, R7",
/* 6 */
	"jz %s",
	"ajmp %s\n",
	"xrl %s, A",
	"xrl %s, #%s",
	"xrl A, #%s",
	"xrl A, %s",
	"xrl A, @R0",
	"xrl A, @R1",
	"xrl A, R0",
	"xrl A, R1",
	"xrl A, R2",
	"xrl A, R3",
	"xrl A, R4",
	"xrl A, R5",
	"xrl A, R6",
	"xrl A, R7",
/* 7 */
	"jnz %s",
	"acall %s",
	"orl C, %s",
	"jmp @A+DPTR\n",
	"mov A, #%s",
	"mov %s, #%s",
	"mov @R0, #%s",
	"mov @R1, #%s",
	"mov R0, #%s",
	"mov R1, #%s",
	"mov R2, #%s",
	"mov R3, #%s",
	"mov R4, #%s",
	"mov R5, #%s",
	"mov R6, #%s",
	"mov R7, #%s",
/* 8 */
	"sjmp %s\n",
	"ajmp %s\n",
	"anl C, %s",
	"movc A, @A+PC",
	"div AB",
	"mov %s, %s",
	"mov %s, @R0",
	"mov %s, @R1",
	"mov %s, R0",
	"mov %s, R1",
	"mov %s, R2",
	"mov %s, R3",
	"mov %s, R4",
	"mov %s, R5",
	"mov %s, R6",
	"mov %s, R7",
/* 9 */
	"mov DPTR, #0%X%02X",
	"acall %s",
	"mov %s, C",
	"movc A, @A+DPTR",
	"subb A, #%s",
	"subb A, %s",
	"subb A, @R0",
	"subb A, @R1",
	"subb A, R0",
	"subb A, R1",
	"subb A, R2",
	"subb A, R3",
	"subb A, R4",
	"subb A, R5",
	"subb A, R6",
	"subb A, R7",
/* A */
	"orl C, /%s",
	"ajmp %s\n",
	"mov C, %s",
	"inc DPTR",
	"mul AB",
	"",               /* undefined opcode */
	"mov @R0, %s",
	"mov @R1, %s",
	"mov R0, %s",
	"mov R1, %s",
	"mov R2, %s",
	"mov R3, %s",
	"mov R4, %s",
	"mov R5, %s",
	"mov R6, %s",
	"mov R7, %s",
/* B */
	"anl C, /%s",
	"acall %s",
	"cpl %s",
	"cpl C",
	"cjne A, #%s, %s",
	"cjne A, %s, %s",
	"cjne @R0, #%s, %s",
	"cjne @R1, #%s, %s",
	"cjne R0, #%s, %s",
	"cjne R1, #%s, %s",
	"cjne R2, #%s, %s",
	"cjne R3, #%s, %s",
	"cjne R4, #%s, %s",
	"cjne R5, #%s, %s",
	"cjne R6, #%s, %s",
	"cjne R7, #%s, %s",
/* C */
	"push %s",
	"ajmp %s\n",
	"clr %s",
	"clr C",
	"swap A",
	"xch A, %s",
	"xch A, @R0",
	"xch A, @R1",
	"xch A, R0",
	"xch A, R1",
	"xch A, R2",
	"xch A, R3",
	"xch A, R4",
	"xch A, R5",
	"xch A, R6",
	"xch A, R7",
/* D */
	"pop %s",
	"acall %s",
	"setb %s",
	"setb C",
	"da A",
	"djnz %s, %s",
	"xchd A, @R0",
	"xchd A, @R1",
	"djnz R0, %s",
	"djnz R1, %s",
	"djnz R2, %s",
	"djnz R3, %s",
	"djnz R4, %s",
	"djnz R5, %s",
	"djnz R6, %s",
	"djnz R7, %s",
/* E */
	"movx A, @DPTR",
	"ajmp %s\n",
	"movx A, @R0",
	"movx A, @R1",
	"clr A",
	"mov A, %s",
	"mov A, @R0",
	"mov A, @R1",
	"mov A, R0",
	"mov A, R1",
	"mov A, R2",
	"mov A, R3",
	"mov A, R4",
	"mov A, R5",
	"mov A, R6",
	"mov A, R7",
/* F */
	"movx @DPTR, A",
	"acall %s",
	"movx @R0, A",
	"movx @R1, A",
	"cpl A",
	"mov %s, A",
	"mov @R0, A",
	"mov @R1, A",
	"mov R0, A",
	"mov R1, A",
	"mov R2, A",
	"mov R3, A",
	"mov R4, A",
	"mov R5, A",
	"mov R6, A",
	"mov R7, A"
};


/* op_format table
 *
 *  0 = illegal opcode
 *  1 = no operands
 *  2 = one immediate operand
 *  3 = one direct operand
 *  4 = one bit-addressed operand
 *  5 = one relative address operand
 *  6 = one absolute address operand
 *  7 = two-byte immediate operand
 *  8 = two operands: direct, immediate
 *  9 = two operands: direct, direct
 * 10 = two operands: immediate, relative address
 * 11 = two operands: direct, relative address
 * 12 = two operands: bit address, relative address
 * 13 = two-byte long address operand
 */
const char op_format[256] = {
	 1, 6, 13, 1, 1,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0 */
	12, 6, 13, 1, 1,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 1 */
	12, 6,  1, 1, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 2 */
	12, 6,  1, 1, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 3 */
	 5, 6,  3, 8, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 4 */
	 5, 6,  3, 8, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 5 */
	 5, 6,  3, 8, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 6 */
	 5, 6,  4, 1, 2,  8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* 7 */
	 5, 6,  4, 1, 1,  9, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 8 */
	 7, 6,  4, 1, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 9 */
	 4, 6,  4, 1, 1,  0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* A */
	 4, 6,  4, 1, 10, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, /* B */
	 3, 6,  4, 1, 1,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* C */
	 3, 6,  4, 1, 1, 11, 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, /* D */
	 1, 6,  1, 1, 1,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* E */
	 1, 6,  1, 1, 1,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  /* F */
};

const char sfbitname[128][6] = {
/* 80 */
	"P0.0", "P0.1", "P0.2", "P0.3", "P0.4", "P0.5", "P0.6", "P0.7",
/* 88 */
	"IT0", "IE0", "IT1", "IE1", "TR0", "TF0", "TR1", "TF1",
/* 90 */
	"P1.0", "P1.1", "P1.2", "P1.3", "P1.4", "P1.5", "P1.6", "P1.7",
/* 98 */
	"RI", "TI", "RB8", "TB8", "REN", "SM2", "SM1", "SM0",
/* A0 */
	"P2.0", "P2.1", "P2.2", "P2.3", "P2.4", "P2.5", "P2.6", "P2.7",
/* A8 */
	"EX0", "ET0", "EX1", "ET1", "ES", "0xAD", "0xAE", "EA",
/* B0 */
	"P3.0", "P3.1", "P3.2", "P3.3", "P3.4", "P3.5", "P3.6", "P3.7",
/* B8 */
	"PX0", "PT0", "PX1", "PT1", "PS", "0xBD", "0xBE", "0xBF",
/* C0 */
	"0xC0", "0xC1", "0xC2", "0xC3", "0xC4", "0xC5", "0xC6", "0xC7",
/* C8 */
	"0xC8", "0xC9", "0xCA", "0xCB", "0xCC", "0xCD", "0xCE", "0xCF",
/* D0 */
	"P", "0xD1", "OV", "RS0", "RS1", "F0", "AC", "CY",
/* D8 */
	"0xD8", "0xD9", "0xDA", "0xDB", "0xDC", "0xDD", "0xDE", "0xDF",
/* E0 */
	"ACC.0", "ACC.1", "ACC.2", "ACC.3", "ACC.4", "ACC.5", "ACC.6", "ACC.7",
/* E8 */
	"0xE8", "0xE9", "0xEA", "0xEB", "0xEC", "0xED", "0xEE", "0xEF",
/* F0 */
	"B.0", "B.1", "B.2", "B.3", "B.4", "B.5", "B.6", "B.7",
/* F8 */
	"0xF8", "0xF9", "0xFA", "0xFB", "0xFC", "0xFD", "0xFE", "0xFF"
};

const char sfrname[128][5] = {
/* 80 */
	"P0", "SP", "DPL", "DPH", "0x84", "0x85", "0x86", "PCON",
/* 88 */
	"TCON", "TMOD", "TL0", "TL1", "TH0", "TH1", "0x8E", "0x8F",
/* 90 */
	"P1", "0x91", "0x92", "0x93", "0x94", "0x95", "0x96", "0x97",
/* 98 */
	"SCON", "SBUF", "0x9A", "0x9B", "0x9C", "0x9D", "0x9E", "0x9F",
/* A0 */
	"P2", "0xA1", "0xA2", "0xA3", "0xA4", "0xA5", "0xA6", "0xA7",
/* A8 */
	"IE", "0xA9", "0xAA", "0xAB", "0xAC", "0xAD", "0xAE", "0xAF",
/* B0 */
	"P3", "0xB1", "0xB2", "0xB3", "0xB4", "0xB5", "0xB6", "0xB7",
/* B8 */
	"IP", "0xB9", "0xBA", "0xBB", "0xBC", "0xBD", "0xBE", "0xBF",
/* C0 */
	"0xC0", "0xC1", "0xC2", "0xC3", "0xC4", "0xC5", "0xC6", "0xC7",
/* C8 */
	"0xC8", "0xC9", "0xCA", "0xCB", "0xCC", "0xCD", "0xCE", "0xCF",
/* D0 */
	"PSW", "0xD1", "0xD2", "0xD3", "0xD4", "0xD5", "0xD6", "0xD7",
/* D8 */
	"0xD8", "0xD9", "0xDA", "0xDB", "0xDC", "0xDD", "0xDE", "0xDF",
/* E0 */
	"ACC", "0xE1", "0xE2", "0xE3", "0xE4", "0xE5", "0xE6", "0xE7",
/* E8 */
	"0xE8", "0xE9", "0xEA", "0xEB", "0xEC", "0xED", "0xEE", "0xEF",
/* F0 */
	"B", "0F1", "0xF2", "0xF3", "0xF4", "0xF5", "0xF6", "0xF7",
/* F8 */
	"0xF8", "0xF9", "0xFA", "0xFB", "0xFC", "0xFD", "0xFE", "0xFFh"
};
