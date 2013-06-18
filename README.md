Overview
========

This is a collection of utilities and source to work with assembly for the
AX211 SD processor.  It consists of an assembler, a utility to kick the
processor into code-load mode, and a source file to upload once it's in
code-load mode.

Assembler
---------
The assembler originally comes from https://www.pjrc.com/tech/8051/.  It is
a beta test release.  The parser.y file is pre-processed due to Bison not
functioning on the current Novena OpenEmbedded distribution.


Utility
-------

The ax211-utility is used for testing the chip.  It is also used to load and
execute code, and for working with the EIM interface.

For more information, run "./ax211 -h"


ROM File
--------

The ROM file expects to be loaded at offset 0x2900.  You're limited to 512
bytes, and you shouldn't assume anything is set up such as a stack.


Notes on the AX211
==================

The AX211 isn't well documented.  This aims to fix that.


Memory Areas
------------

The 8051 has two different kinds of areas, each of which is further
subdivided:

* IRAM - located on-chip.  256 bytes in total.  Accessed using "mov"
instruction.
    * IRAM address 0-7 corresponds to CPU registers R0-R7
    * IRAM address 8-0x7f are on-chip RAM
    * IRAM address 0x80-0xff are Special Function Registers (SFRs).  See
    below.

* XRAM - 16 kilobytes of "External" RAM.  Accessed by storing an address in
DPTR (which is really made up of SFR 0x82 and 0x83, called DPL and DPH
respectively) and either loading to the accumulator using "movx A, @DPTR" or
storing from the accumulator using "movx @DPTR, A".
    * 0x0000 - 0x0006 is reserved somehow.  Contains 0x51 0x00 0x00 0x00 ...
    * 0x0007 - 0x01ff is protected and returns 0xff
    * 0x0200 - 0x2002 is interrupt vector 0 (SPI)
    * 0x0203 - 0x2005 is interrupt vector 1 (other SPI)
    * 0x0206 - 0x0208 is interrupt vector 2 (NAND)
    * 0x0209 - 0x020b is interrupt vector 3 (unknown)
    * 0x020c - 0x02aff is general-purpose RAM
        * Code execution for APPO factory mode begins at offset 0x2900
    * 0x2ba0 - 0x2bff contains something interesting; I'm not sure what
    * 0x2c00 - 0x3fff is read-only and contains zeroes
    * 0x0000 - 0x3fff is mirrored at 0x4000, 0x8000, and 0xc000


Extended Opcodes
----------------

The AX211 is likely similar to the AX208, which is a digital picture frame
processor from AppoTech.  A limited reference manual is available that
describes some 16-bit opcodes provided by this processor.

Unfortunately the opcodes themselves aren't described, only the mnemonics.
Fortunately, it appears as though the 8051 instruction set is listed in
opcode order (i.e. "NOP" [opcode 0x00] is listed first, followed by AJMP
[opcode 0x01] and LJMP [opcode 0x02] and so-on), so we shall assume that
the extended mnemonics are also listed in the same order.

For information on standard 8051 opcodes, see:
http://www.win.tue.nl/~aeb/comp/8051/set8051.html

Extended opcodes begin with the "Undefined" opcode 0xa5.  This opcode was
undefined in the original 8051, and is sometimes used by manufacturers to
provide additional instructions while retaining compatibility.

0xa5 0x90 -> register 0xc0c1 is xored with 0xc8c9 and stored in 0xc0c1

0xa5 0x91 -> clear register ER2

         |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |
         |   8   |   9   |   A   |   B   |   C   |   D   |   E   |   F   |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     00  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     08  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     10  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     18  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     20  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     28  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     30  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     38  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     40  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     48  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     50  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     58  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     60  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     68  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     70  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     78  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     80  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     88  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     90  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     98  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     A0  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     A8  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     B0  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     B8  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     C0  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     C8  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     D0  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     D8  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     E0  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     E8  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     F0  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     F8  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+


Special Function Registers
--------------------------
    
         |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |
         |   8   |   9   |   A   |   B   |   C   |   D   |   E   |   F   |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     80  |       |  SP   |  DPL  |  DPH  |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     88  | SDOS  | SDI4  | SDI3  | SDI2  | SDI1  | SDCMD |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     90  | SDSM  |       |       | SDBL  | SDBH  |       | SDDL  | SDDH  |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     98  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     A0  | NSTAT | NCMD  | NSRCL | NSRCH |       |       | ER00  | ER01  |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     A8  |  IE   | NTYP1 | NTYP2 | NADD0 | NADD1 | NADD2 | NADD3 | NADD4 |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     B0  |       | RAND  |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     B8  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     C0  | ER10  | ER11  |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     C8  | ER20  | ER21  |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     D0  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     D8  | ER30  | ER31  |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     E0  | ACC   |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     E8  |       |       | N???? | SDDIR |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     F0  | B     |       |       |       |       |       | PORT1 |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+
     F8  |       |       |       |       |       |       |       |       |
    -----+-------+-------+-------+-------+-------+-------+-------+-------+

SP:     Stack Pointer

DPL:    Data Pointer (low)

DPH:    Data Pointer (high)

IE:     Interrupt enable

        | Axxx 4321 |
            A = all interrupts (0 - all interrupts disabled)
            4 - enable/disable interrupt 4
            3 - enable/disable interrupt 3
            2 - enable/disable interrupt 2
            1 - enable/disable interrupt 1

SDOS:   SD output state.  Write a "1" here to kick off a transfer.  There appear to be extra bits that determine what sort of transfer it is.

        | ?ABC ???S |
            S = "start transfer" bit
            ABC = ? (maybe indicates 'CMD' bit?)

N????:  Does weird things.

        | ???? b??? |
            b - When 0, NAND transfer doesn't work

SDI1..4: Register values R1..R4 from the SD command

SDCMD:  The number of the command that was sent (without start bit).  E.g. if the first byte was 0x42, then SDCMD would equal 0x02.

SDSM:   SD state machine state.

        | ???? AA?X |
            A - If 0, then the state machine is idle

SDBL:   SD transfer bytes (low byte), minus one

SDBH:   SD transfer bytes (high byte), minus one

        | ???? ???h |
            h = "high bit of address"

SDDL:   SD transfer source address (low byte), divided by 4

SDDH:   SD transfer source address (high byte), divided by 4

        | xxxx xDDD | (Only lower three bits are used)

SDDIR:  SD pin direction registers

PORT1:  GPIO for the NAND port.  When set to 0xff, drives pins high.k

NSTAT:  Defines the type of NAND

        | ???? S??? |
            S = SLC size, with 0=512 bytes and 1=256 bytes

NCMD:   NAND command.  The command to send comes from this table:

        | WRA2 1CCC |
            W - command is a write
            R - command is a read
            A - If set, sends a four-byte "Address" field
            2 - Add a fifth "Address" byte and add a post-address command
            1 - Send an initial command byte
            C - Command, from the table below

        Note: Setting R and W together will crash the card.  If neither is
        set, then neither a read nor a write is performed, but the command
        will still be sent.

        Note: If "W" or "R" are set, then an initial "CLE" is set, and "2"
        has no effect.

        Note: If sending "A" and "2" but not "W" or "R", then the engine
        will send one command and five address bytes, but no second command
        byte.

     CMD | Result
     ----+-------------------------
       0 | nop
       1 | nop
       2 | Read ID (CLE 0x90 / ALE 0x00 / read)
       3 | Read 528 bytes
       4 | CMD 0x60 / [4 addrs] / 0xd0
       5 | CMD 0x80 / [4 addrs] / write 528 bytes
       6 | CMD 0x70
       7 | nop
    0x47 | Read 527 bytes without any CMDs
    0x87 | Write 527 bytes without any CMDs
    0x6f | CMD [NTYP1] / [4 addrs] / read 527 bytes
    0xaf | CMD [NTYP1] / [4 addrs] / read 527 bytes

NSRCL:  Source address for NAND transfers.  Actual address is calculated as (SFR_A3<<8+SFR_A2)*8.

NSRCH:  Source address for NAND transfers, high byte.

NTYP1:  Used in some "read" operations as the initial command type.

NTYP2:  Used in some "read" operations as the final command type.

NADD0..4: NAND address registers.  These define, in order, which address to specify when sending a NAND command.  These registers are reset after each NAND command that uses addresses.  For example, if you set NADD0..4 and then call NCMD6 (read status), this will not change the values of NADD0..4.
As a special case, NCMD2 (read ID) seems to store some sort of data in NCMD0..4.  The data it stores is not the actual NAND ID.


ACC:    Accumulator

B:      Scratch register.  Used for multiply/divide operations.

RAND:   Appears to be a random number (but may be a counter of some sort)


The DPL+DPH combination form the address that's read by the DPTR register.

The contents of the SFRs when the program is first loaded is:

    0c 80 02 00 00 00 00 00  00 22 5b 27 c0 00 00 00
    02 00 00 00 00 fe 00 00  10 00 00 50 00 00 01 28
    34 00 00 00 00 00 ec 00  8f 00 00 f1 05 8d e9 fd
    f0 1f c3 00 00 00 00 00  44 00 00 00 00 00 00 00
    ff ff ff ef 00 00 00 00  ff ff dd eb 00 00 00 00
    80 ec 00 00 00 00 00 00  e8 bd bd df 00 00 00 00
    3f 00 00 00 78 07 00 00  80 00 87 ff 00 00 00 00
    48 ff 00 2c 00 3f 00 00  14 fe ff ff 00 00 00 00


NAND Storage
------------

The initial boot code is protected with a very robust error correction
algorithm.  Every 512-bytes is protected by both a 16-bit CRC16 and a
108-bit ECC of some sort (likely BCH).

At the end of a 512-byte block there will be 16 bytes of ECC data.  For
example, the following pattern was observed after a 512-byte block
consisting entirely of 0xff:

    00000200  7f a1 8e be 74 9d 5f 07  d7 f6 d1 81 12 59 5e f9 |....t._......Y^.|

Bytes 0 and 1 are the result of a CRC16 across the entire field.  The CRC16
of 512 bytes of 0xff is 0x7fa1.

Byte 2 serves dual-purpose.  The lower nybble is fixed to the value "0xe"
for reasons that are not yet clear.

The upper nybble of byte 2, along with the remaining 13 bytes are all ECC
data.



Using the Interactive Debugger
==============================

The ax211 binary now includes an interactive debugger.  To run it, you must
supply an executable binary.  Commands on the host will get transmitted to
the card, and the responses will get read back.

The wire protocol is simply a re-purposed SD interface running over two
wires.  Commands go out the CMD line, and responses come back on the CMD
line.  You must know how many bytes the response will take, and stop
wiggling the clock line once you have reached that many bytes.  Be sure to
add two bytes, one for the start byte and one for the CRC7.

For information on available commands, run "help".

Debugger Internals
------------------

When the debugger starts up, it dumps a small portion of the AX211's RAM
and searches for some magical fixup sequences.  These three-byte sequences
begin with the preamble "0xa5", and are used to overcome some limitations
of the 8051, namely with respect to reading from internal RAM.

After it performs fixups, the debugger enters into its main loop.  It acts
as a shell, and gets its commands from the debug_commands[] array.  Each
element of the debug_commands array is made up of a struct debug_command:

    struct debug_command {
        char    *name;      // Name of the command
        char    *desc;      // Brief, one-line description
        char    *help;      // Multi-line "help" description
        int (*func)(struct dbg *dbg, int argc, char **argv);
    };

The func() gets called with a C-style argc and argv, with argv[0] set to
the command name.  You can call getopt() to process arguments.

Return a negative value upon error.  If possible, return a negative errno
value, for example -EINVAL or -ENOSPC.
