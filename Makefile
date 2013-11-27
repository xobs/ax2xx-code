# as31 - 8031/8051 Assembler
#
# Maintained at:  http://www.pjrc.com/tech/8051/
# Please report problems to:  paul@pjrc.com
#
#
# REVISION HISTORY:
#	Jan. 19, 1990	Created. (Ken Stauffer).
#	Jan. 30, 1990	Theo played here.
#	Late 1994	Paul Stoffregen updates the code
#	Sept 1995	bug fixes, add obsure option for cgi
#	April 2000	fix filename handling
#			convert everything to ANSI C and bison
#			properly handle 0b0h as hex 0xB0, not binary 0
#			Make most errors into warning, and modify parser
#			  to recover and continue to the end of the file
#			  so that all warning are shown.
#	March 2001	fix crash with srecord format
#			command line version returns non-zero on error


#Use this CFLAGS line with gcc on linux
CFLAGS=-O3 -Wall
CC=gcc

PROGRAM_SOURCES=TestBoot.bin dump-rom.bin blink-led.bin debugger.bin fuzzer.bin blink-led-ax215.bin
PROGRAMS=$(PROGRAM_SOURCES:.asm=.bin)

DBG_OFFSET=31488

all: as31 ax215 $(PROGRAMS)

as31: src/as31
	+make -C src/as31

ax215: src/ax215
	+make -C src/ax215

%.bin: progs/%.asm as31
	./as31 -Fbin $<
	@mv progs/$@ tmp.bin
	@dd if=tmp.bin of=$@ bs=$(DBG_OFFSET) skip=1 2> /dev/null
	@rm -f tmp.bin

clean:
	make -C src/ax215 clean
	make -C src/as31 clean
	rm -f TestBoot.bin dump-rom.bin blink-led.bin
