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

#Use this CFLAGS line on solaris, etc
#CFLAGS=-O

#Try these for HPUX
#CFLAGS=-O2
#CC=gcc

OBJ=run.o lexer.o parser.o symbol.o emitter.o

all: as31 TestBoot.bin ax211

as31: $(OBJ) as31.o
	$(CC) $(CFLAGS) -o as31 $(OBJ) as31.o
	chmod a+rx as31
	strip as31

as31_gtk: $(OBJ) as31_gtk.o
	$(CC) $(CFLAGS) -o as31_gtk $(OBJ) as31_gtk.o `gtk-config --libs`
	chmod a+rx as31
	strip as31

ax211:
	make -C ax211-util

TestBoot.bin: TestBoot.asm as31
	./as31 -Fbin TestBoot.asm
	# The ROM has an origin of 0x2900.  Pull it out.
	dd if=TestBoot.bin of=tmp.bin bs=10496 skip=1 2> /dev/null
	# Pad it with 0xff and bring it up to 512 bytes
	perl -e 'print chr(0xff) for(0..510); print chr(0x00);' > TestBoot.bin
	dd if=tmp.bin of=TestBoot.bin conv=notrunc 2> /dev/null
	rm -f tmp.bin

as31_gtk.o: as31_gtk.c as31.h
	$(CC) $(CFLAGS) `gtk-config --cflags` -c as31_gtk.c 

as31.o: as31.c as31.h
emitter.o: emitter.c as31.h
symbol.o: symbol.c as31.h parser.h
lexer.o: lexer.c as31.h parser.h
parser.o: parser.c as31.h
run.o: run.c as31.h

clean:
	rm -f as31 as31.1 as31_gtk *~ *.o core
	rm -f *.lst *.hex *.tdr *.byte *.od *.srec
	rm -f TestBoot.bin
	make -C ax211-util clean
