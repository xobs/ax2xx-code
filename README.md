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
