Overview
========

This is a collection of utilities and source to work with assembly for the
AX211 SD processor.  It consists of an assembler, a utility to kick the
processor into code-load mode, and a source file to upload once it''s in
code-load mode.

Assembler
---------
The assembler originally comes from https://www.pjrc.com/tech/8051/.  It is
a beta test release.  The parser.y file is pre-processed due to Bison not
functioning on the current Novena OpenEmbedded distribution.

Please report any problems to

paul@pjrc.com


Utility
-------

The ax211-utility is used for testing the chip.  It is also used to load and
execute code, and for working with the EIM interface.

For more information, run "./ax211 -h"


ROM File
--------

The ROM file expects to be loaded at offset 0x2900.  You''re limited to 512
bytes, and you shouldn''t assume anything is set up such as a stack.


Notes on the AX211
==================

The AX211 isn''t well documented.  This aims to fix that.


Special Function Registers
--------------------------
    
     |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |
-----+-------+-------+-------+-------+-------+-------+-------+-------+
 80  |       |  SP   |  DPL  |  DPH  |       |       |       |       |
-----+-------+-------+-------+-------+-------+-------+-------+-------+
 88  |       |       |       |       |       |       |       |       |
-----+-------+-------+-------+-------+-------+-------+-------+-------+
 90  |       |       |       |       |       |       |       |       |
-----+-------+-------+-------+-------+-------+-------+-------+-------+
 98  |       |       |       |       |       |       |       |       |
-----+-------+-------+-------+-------+-------+-------+-------+-------+
 A0  |       |       |       |       |       |       |       |       |
-----+-------+-------+-------+-------+-------+-------+-------+-------+
 A8  |  IE   |       |       |       |       |       |       |       |
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
 E0  |  ACC  |       |       |       |       |       |       |       |
-----+-------+-------+-------+-------+-------+-------+-------+-------+
 E8  |       |       |       |       |       |       |       |       |
-----+-------+-------+-------+-------+-------+-------+-------+-------+
 F0  |   B   |       |       |       |       |       | PORT1 |       |
-----+-------+-------+-------+-------+-------+-------+-------+-------+
 F8  |       |       |       |       |       |       |       |       |
-----+-------+-------+-------+-------+-------+-------+-------+-------+

SP:     Stack Pointer
DPL:    Data Pointer (low)
DPH:    Data Pointer (high)
IE:     Interrupt enable
PORT1:  GPIO for the NAND port.
ACC:    Accumulator
B:      Scratch register.  Used for multiply/divide operations.


The DPL+DPH combination form the address that''s read by the DPTR register.

