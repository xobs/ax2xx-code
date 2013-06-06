; PAULMON2, a user-friendly 8051 monitor, by Paul Stoffregen
; Please email comments, suggestions, bugs to paul@pjrc.com

; It's free.  PAULMON2 is in the public domain.  You may copy
; sections of code from PAULMON2 into your own programs, even
; for commercial purposes.  PAULMON2 should only be distributed
; free of charge, but may be bundled as 'value-added' with other
; products, such as development boards, CDROMs, etc.  Please
; distribute the PAULMON2.DOC file and other files, not just
; the object code!

; The PAULMON2.EQU and PAULMON2.HDR files contain valuable
; information that could help you to write programs for use
; with PAULMON2.

; PAULMON2 is in the public domain. PAULMON2 is distributed in
; the hope that it will be useful, but without any warranty;
; without even the implied warranty of merchantability or fitness
; for a particular purpose. 


; You are probably reading this code to see what it looks like
; and possibly learn something, or to modify it for some reason.
; Either is ok, but please remember that this code uses a number
; of tricks to cram all the functionality into just 4k.  As a
; result, the code can be difficult to read, and adding new
; features can be very difficult without growing beyond 4k.  To
; add or modify commands in PAULMON2, please consider using the
; "external command" functionality.  It is easier to develop
; new commands this way, and you can distribute them to other
; users.  Email paul@pjrc.com if you have new PAULMON2
; commands to contribute to others.  Details about adding new
; commands to PAULMON2 (with examples) can be found at:

; http://www.pjrc.com/tech/8051/pm2_docs/addons.html


;---------------------------------------------------------;
;							  ;
;	    PAULMON2's default configuration		  ;
;							  ;
;---------------------------------------------------------;

; PAULMON2 should be assembled using the modified AS31 assembler,
; originally written by Ken Stauffer, many small changes by Paul
; Stoffregen.  This free assembler is available on the web at
; http://www.pjrc.com/tech/8051/index.html
; As well, these web pages have a fill-out form which makes it
; very easy to custom configure PAULMON2.  Using this form will
; edit the code for you, run the AS31 assmebler, and send you the
; object code to program into your chip.


; These two parameters control where PAULMON2 will be assembled,
; and where it will attempt to LJMP at the interrupt vector locations.

.equ	base, 0x0000		;location for PAULMON2
.equ	vector, 0x2000		;location to LJMP interrupt vectors

; These three parameters tell PAULMON2 where the user's memory is
; installed.  "bmem" and "emem" define the space that will be searched
; for program headers, user installed commands, start-up programs, etc.
; "bmem" and "emem" should be use so they exclude memory areas where
; perphreal devices may be mapped, as reading memory from an io chip
; may reconfigure it unexpectedly.  If flash rom is used, "bmem" and "emem"
; should also include the space where the flash rom is mapped.

.equ	pgm, 0x2000		;default location for the user program
.equ	bmem, 0x1000		;where is the beginning of memory
.equ	emem, 0xFFFF		;end of the memory

; Flash ROM parameters.	 If "has_flash" is set to zero, all flash rom
; features are turned off, otherwise "bflash" and "eflash" should specify
; the memory range which is flash rom.	Though AMD doesn't suggest it,
; you may be able to map only a part of the flash rom with your address
; decoder logic (and not use the rest), but you should be careful that
; "bflash" and "eflash" don't include and memory which is NOT flash rom
; so that the erase algorithm won't keep applying erase pulses until it
; finally gives up (which will stress the thin oxide and degrade the
; flash rom's life and reliability).  "erase_pin" allows you to specify
; the bit address for a pin which (if held low) will tell PAULMON2 to
; erase the flash rom chip when it starts up.  This is useful if you
; download programs with the "start-up" headers on them and the code you've
; put in the flash rom crashes!

.equ	has_flash, 0		;set to non-zero value if flash installed
.equ	bflash, 0x8000		;first memory location of Flash ROM
.equ	eflash, 0xFFFF		;last memory location of Flash ROM
.equ	erase_pin, 0		;00 = disable erase pin feature
;.equ	erase_pin, 0xB5		;B5 = pin 15, P3.5 (T1)

; Please note... much of the memory management code only looks at the
; upper 8 bits of an address, so it's not a good idea to somehow map
; your memory chips (with complex address decoding logic) into chunks
; less than 256 bytes.	In other words, only using a piece of a flash
; rom chip and mapping it between C43A to F91B would confuse PAULMON2
; (as well as require quit a bit of address decoding logic circuitry)


; To set the baud rate, use this formula or set to 0 for auto detection
; baud_const = 256 - (crystal / (12 * 16 * baud))

.equ	baud_const, 0		;automatic baud rate detection
;.equ	baud_const, 255		;57600 baud w/ 11.0592 MHz
;.equ	baud_const, 253		;19200 baud w/ 11.0592 MHz
;.equ	baud_const, 252		;19200 baud w/ 14.7456 MHz
;.equ	baud_const, 243		;4808 baud w/ 12 MHz

.equ	line_delay, 6		;num of char times to pause during uploads

; About download speed: when writing to ram, PAULMON2 can accept data
; at the maximum baud rate (baud_const=255 or 57600 baud w/ 11.0592 MHz).
; Most terminal emulation programs introduce intentional delays when
; sending ascii data, which you would want to turn off for downloading
; larger programs into ram.  For Flash ROM, the maximum speed is set by
; the time it takes to program each location... 9600 baud seems to work
; nicely for the AMD 28F256 chip.  The "character pacing" delay in a
; terminal emulation program should be sufficient to download to flash
; rom and any baud rate.  Some flash rom chips can write very quickly,
; allowing high speed baud rates, but other chips can not.  You milage
; will vary...


; Several people didn't like the key definations in PAULMON1.
; Actually, I didn't like 'em either, but I never took the time
; to change it.	 Eventually I got used to them, but now it's
; really easy to change which keys do what in PAULMON2.	 You
; can guess what to do below, but don't use lowercase.

.equ	help_key, '?'		;help screen
.equ	dir_key,  'M'		;directory
.equ	run_key,  'R'		;run program
.equ	dnld_key, 'D'		;download
.equ	upld_key, 'U'		;upload
.equ	nloc_key, 'N'		;new memory location
.equ	jump_key, 'J'		;jump to memory location
.equ	dump_key, 'H'		;hex dump memory
.equ	intm_key, 'I'		;hex dump internal memory
.equ	edit_key, 'E'		;edit memory
.equ	clrm_key, 'C'		;clear memory
.equ	erfr_key, 'Z'		;erase flash rom

; timing parameters for AMD Flash ROM 28F256.  These parameters
; and pretty conservative and they seem to work with crystals
; between 6 MHz to 24 MHz... (tested with AMD 28F256 chips only)
; unless you know this is a problem, it is probably not a good
; idea to fiddle with these.

;.equ	pgmwait, 10		;22.1184 MHz crystal assumed
.equ	pgmwait, 19		;11.0592 MHz
.equ	verwait, 5
;.equ	erwait1, 40		;fourty delays @22.1184
.equ	erwait1, 20		;twenty delays for 11.0592 MHz
.equ	erwait2, 229		;each delay .5 ms @22.1184MHz



; These symbols configure paulmon2's internal memory usage.
; It is usually not a good idea to change these unless you
; know that you really have to.

.equ	psw_init, 0		;value for psw (which reg bank to use)
.equ	dnld_parm, 0x10		;block of 16 bytes for download
.equ	stack, 0x30		;location of the stack
.equ	baud_save, 0x78		;save baud for warm boot, 4 bytes

;---------------------------------------------------------;
;							  ;
;		     Interrupt Vectors			  ;
;  (and little bits of code crammed in the empty spaces)  ;
;							  ;
;---------------------------------------------------------;

	.org	base
	ljmp	poweron		;reset vector

	.org	base+3
	ljmp	vector+3	;ext int0 vector

r6r7todptr:
	mov	dpl, r6
	mov	dph, r7
	ret

	.org	base+11
	ljmp	vector+11	;timer0 vector

dptrtor6r7:
	mov	r6, dpl
	mov	r7, dph
	ret

	.org	base+19
	ljmp	vector+19	;ext int1 vector

dash:	mov	a, #'-'		;seems kinda trivial, but each time
	ajmp	cout		;this appears in code, it takes 4
	nop			;bytes, but an acall takes only 2

	.org	base+27
	ljmp	vector+27	;timer1 vector

cout_sp:acall	cout
	ajmp	space
	nop

	.org	base+35
	ljmp	vector+35	;uart vector

dash_sp:acall	dash
	ajmp	space
	nop

	.org	base+43
	ljmp	vector+43	;timer2 vector (8052)


;---------------------------------------------------------;
;							  ;
;	The jump table for user programs to call	  ;
;	      subroutines within PAULMON		  ;
;							  ;
;---------------------------------------------------------;

.org	base+46		;never change this line!!  Other
			;programs depend on these locations
			;to access paulmon2 functions

	ajmp	phex1		;2E
	ajmp	cout		;30
	ajmp	cin		;32
	ajmp	phex		;34
	ajmp	phex16		;36
	ajmp	pstr		;38
	ajmp	ghex		;3A
	ajmp	ghex16		;3C
	ajmp	esc		;4E
	ajmp	upper		;40
	ljmp	autobaud	;42
pcstr_h:ljmp	pcstr		;45
	ajmp	newline		;48
	ljmp	lenstr		;4A
	ljmp	pint8u		;4D
	ljmp	pint8		;50
	ljmp	pint16u		;53
	ljmp	smart_wr	;56
	ljmp	prgm		;59
	ljmp	erall		;5C
	ljmp	find		;5F
cin_filter_h:
	ljmp	cin_filter	;62
	ajmp	asc2hex		;64


;---------------------------------------------------------;
;							  ;
;	       Subroutines for serial I/O		  ;
;							  ;
;---------------------------------------------------------;


cin:	jnb	ri, cin
	clr	ri
	mov	a, sbuf
	ret

dspace: acall	space
space:	mov	a, #' '
cout:	jnb	ti, cout
	clr	ti		;clr ti before the mov to sbuf!
	mov	sbuf, a
	ret

;clearing ti before reading sbuf takes care of the case where
;interrupts may be enabled... if an interrupt were to happen
;between those two instructions, the serial port will just
;wait a while, but in the other order and the character could
;finish transmitting (during the interrupt routine) and then
;ti would be cleared and never set again by the hardware, causing
;the next call to cout to hang forever!

newline2:			;print two newlines
	acall	newline
newline:push	acc		;print one newline
	mov	a, #13
	acall	cout
	mov	a, #10
	acall	cout
	pop	acc
	ret

	;get 2 digit hex number from serial port
	; c = set if ESC pressed, clear otherwise
	; psw.5 = set if return w/ no input, clear otherwise
ghex:
ghex8:	clr	psw.5
ghex8c:
	acall	cin_filter_h	;get first digit
	acall	upper
	cjne	a, #27, ghex8f
ghex8d: setb	c
	clr	a
	ret
ghex8f: cjne	a, #13, ghex8h
	setb	psw.5
	clr	c
	clr	a
	ret
ghex8h: mov	r2, a
	acall	asc2hex
	jc	ghex8c
	xch	a, r2		;r2 will hold hex value of 1st digit
	acall	cout
ghex8j:
	acall	cin_filter_h	;get second digit
	acall	upper
	cjne	a, #27, ghex8k
	sjmp	ghex8d
ghex8k: cjne	a, #13, ghex8m
	mov	a, r2
	clr	c
	ret
ghex8m: cjne	a, #8, ghex8p
ghex8n: acall	cout
	sjmp	ghex8c
ghex8p: cjne	a, #21, ghex8q
	sjmp	ghex8n
ghex8q: mov	r3, a
	acall	asc2hex
	jc	ghex8j
	xch	a, r3
	acall	cout
	mov	a, r2
	swap	a
	orl	a, r3
	clr	c
	ret




	;carry set if esc pressed
	;psw.5 set if return pressed w/ no input
ghex16:
	mov	r2, #0		;start out with 0
	mov	r3, #0
	mov	r4, #4		;number of digits left
	clr	psw.5

ghex16c:
	acall	cin_filter_h
	acall	upper
	cjne	a, #27, ghex16d
	setb	c		;handle esc key
	clr	a
	mov	dph, a
	mov	dpl, a
	ret
ghex16d:cjne	a, #8, ghex16f
	sjmp	ghex16k
ghex16f:cjne	a, #127, ghex16g  ;handle backspace
ghex16k:cjne	r4, #4, ghex16e	  ;have they entered anything yet?
	sjmp	ghex16c
ghex16e:acall	cout
	acall	ghex16y
	inc	r4
	sjmp	ghex16c
ghex16g:cjne	a, #13, ghex16i	  ;return key
	mov	dph, r3
	mov	dpl, r2
	cjne	r4, #4, ghex16h
	clr	a
	mov	dph, a
	mov	dpl, a
	setb	psw.5
ghex16h:clr	c
	ret
ghex16i:mov	r5, a		  ;keep copy of original keystroke
	acall	asc2hex
	jc	ghex16c
	xch	a, r5
	lcall	cout
	mov	a, r5
	push	acc
	acall	ghex16x
	pop	acc
	add	a, r2
	mov	r2, a
	clr	a
	addc	a, r3
	mov	r3, a
	djnz	r4, ghex16c
	clr	c
	mov	dpl, r2
	mov	dph, r3
	ret

ghex16x:  ;multiply r3-r2 by 16 (shift left by 4)
	mov	a, r3
	swap	a
	anl	a, #11110000b
	mov	r3, a
	mov	a, r2
	swap	a
	anl	a, #00001111b
	orl	a, r3
	mov	r3, a
	mov	a, r2
	swap	a
	anl	a, #11110000b
	mov	r2, a
	ret

ghex16y:  ;divide r3-r2 by 16 (shift right by 4)
	mov	a, r2
	swap	a
	anl	a, #00001111b
	mov	r2, a
	mov	a, r3
	swap	a
	anl	a, #11110000b
	orl	a, r2
	mov	r2, a
	mov	a, r3
	swap	a
	anl	a, #00001111b
	mov	r3, a
	ret


	;carry set if invalid input
asc2hex:
	clr	c
	add	a, #208
	jnc	hex_not
	add	a, #246
	jc	hex_maybe
	add	a, #10
	clr	c
	ret
hex_maybe:
	add	a, #249
	jnc	hex_not
	add	a, #250
	jc	hex_not
	add	a, #16
	clr	c
	ret
hex_not:setb	c
	ret



phex:
phex8:
	push	acc
	swap	a
	anl	a, #15
	add	a, #246
	jnc	phex_b
	add	a, #7
phex_b: add	a, #58
	acall	cout
	pop	acc
phex1:	push	acc
	anl	a, #15
	add	a, #246
	jnc	phex_c
	add	a, #7
phex_c: add	a, #58
	acall	cout
	pop	acc
	ret


phex16:
	push	acc
	mov	a, dph
	acall	phex
	mov	a, dpl
	acall	phex
	pop	acc
	ret


;a not so well documented feature of pstr is that you can print
;multiple consecutive strings without needing to reload dptr
;(which takes 3 bytes of code!)... this is useful for inserting
;numbers or spaces between strings.

pstr:	push	acc
pstr1:	clr	a
	movc	a, @a+dptr
	inc	dptr
	jz	pstr2
	mov	c, acc.7
	anl	a, #0x7F
	acall	cout
	jc	pstr2
	sjmp	pstr1
pstr2:	pop	acc
	ret


upper:	;converts the ascii code in Acc to uppercase, if it is lowercase
	push	acc
	clr	c
	subb	a, #97
	jc	upper2		;is it a lowercase character
	subb	a, #26
	jnc	upper2
	pop	acc
	add	a, #224		;convert to uppercase
	ret
upper2: pop	acc		;don't change anything
	ret


lenstr: mov	r0, #0	  ;returns length of a string in r0
	push	acc
lenstr1:clr	a
	movc	a,@a+dptr
	jz	lenstr2
	mov	c,acc.7
	inc	r0
	Jc	lenstr2
	inc	dptr
	sjmp	lenstr1
lenstr2:pop	acc
	ret


esc:  ;checks to see if <ESC> is waiting on serial port
      ;C=clear if no <ESC>, C=set if <ESC> pressed
      ;buffer is flushed
	push	acc
	clr	c
	jnb	ri,esc2
	mov	a,sbuf
	cjne	a,#27,esc1
	setb	c
esc1:	clr	ri
esc2:	pop	acc
	ret


;---------------------------------------------------------;
;							  ;
;    The 'high-level' stuff to interact with the user	  ;
;							  ;
;---------------------------------------------------------;


menu: ;first we print out the prompt, which isn't as simple
      ;as it may seem, since external code can add to the
      ;prompt, so we've got to find and execute all of 'em.
	mov	dptr, #prompt1	  ;give 'em the first part of prompt
	acall	pcstr_h
	mov	a, r7
	acall	phex
	mov	a, r6
	acall	phex
	;mov	 dptr, #prompt2
	acall	pstr

;now we're finally past the prompt, so let's get some input
	acall	cin_filter_h	;get the input, finally
	acall	upper

;push return address onto stack so we can just jump to the program
	mov	b, #(menu & 255)  ;we push the return address now,
	push	b		  ;to save code later...
	mov	b, #(menu >> 8)	  ;if bogus input, just ret for
	push	b		  ;another prompt.



;first we'll look through memory for a program header that says
;it's a user installed command which matches what the user pressed

;user installed commands need to avoid changing R6/R7, which holds
;the memory pointer.  The stack pointer can't be changed obviously.
;all the other general purpose registers should be available for
;user commands to alter as they wish.

menux:	mov	b, a		;now search for external commands...
	mov	dptr, #bmem
menux1: acall	find
	jnc	menuxend	   ;searched all the commands?
	mov	dpl, #4
	clr	a
	movc	a,@a+dptr
	cjne	a, #254, menux2	 ;only FE is an ext command
	inc	dpl
	clr	a
	movc	a,@a+dptr
	cjne	a, b, menux2	  ;only run if they want it
	acall	space
	mov	dpl, #32
	acall	pstr		   ;print command name
	acall	newline
	mov	dpl, #64
	clr	a
	jmp	@a+dptr		;take a leap of faith and jump to it!
menux2: inc	dph
	mov	a, dph
	cjne	a, #((emem+1) >> 8) & 255, menux1
menuxend:
	mov	a, b


;since we didn't find a user installed command, use the builtin ones

menu1a: cjne	a, #help_key, menu1b
	mov	dptr, #help_cmd2
	acall	pcstr_h
	ajmp	help
menu1b: cjne	a, #dir_key, menu1c
	mov	dptr, #dir_cmd
	acall	pcstr_h
	ajmp	dir
menu1c: cjne	a, #run_key, menu1d
	mov	dptr, #run_cmd
	acall	pcstr_h
	ajmp	run
menu1d: cjne	a, #dnld_key, menu1e
	mov	dptr, #dnld_cmd
	acall	pcstr_h
	ajmp	dnld
menu1e: cjne	a, #upld_key, menu1f
	mov	dptr, #upld_cmd
	acall	pcstr_h
	ajmp	upld
menu1f: cjne	a, #nloc_key, menu1g
	mov	dptr, #nloc_cmd
	acall	pcstr_h
	ajmp	nloc
menu1g: cjne	a, #jump_key, menu1h
	mov	dptr, #jump_cmd
	acall	pcstr_h
	ajmp	jump
menu1h: cjne	a, #dump_key, menu1i
	mov	dptr, #dump_cmd
	acall	pcstr_h
	ajmp	dump
menu1i: cjne	a, #edit_key, menu1j
	mov	dptr, #edit_cmd
	acall	pcstr_h
	ajmp	edit
menu1j: cjne	a, #clrm_key, menu1k
	mov	dptr, #clrm_cmd
	acall	pcstr_h
	ajmp	clrm
menu1k: cjne	a, #erfr_key, menu1l
	mov	a, #has_flash
	jz	menu_end
	mov	dptr, #erfr_cmd
	acall	pcstr_h
	ajmp	erfr
menu1l: cjne	a, #intm_key, menu1m
	mov	dptr, #intm_cmd
	acall	pcstr_h
	ljmp	intm
menu1m:

    ;invalid input, no commands to run...
menu_end:			;at this point, we have not found
	ajmp	newline		;anything to run, so we give up.
				;remember, we pushed menu, so newline
				;will just return to menu.

;..........................................................

;---------------------------------------------------------;

;dnlds1 = "Begin sending Intel HEX format file <ESC> to abort"
;dnlds2 = "Download aborted"
;dnlds3 = "Download completed"


;16 byte parameter table: (eight 16 bit values)
;  *   0 = lines received
;  *   1 = bytes received
;  *   2 = bytes written
;  *   3 = bytes unable to write
;  *   4 = incorrect checksums
;  *   5 = unexpected begin of line
;  *   6 = unexpected hex digits (while waiting for bol)
;  *   7 = unexpected non-hex digits (in middle of a line)

dnld:
	mov	dptr, #dnlds1		 
	acall	pcstr_h		   ;"begin sending file <ESC> to abort"
	mov	r0, #dnld_parm
	mov	r2, #16
dnld0:	mov	@r0, #0		;initialize all parameters to 0
	inc	r0
	djnz	r2, dnld0

	  ;look for begining of line marker ':'
dnld1:	acall	cin
	cjne	a, #27, dnld2	;Test for escape
	sjmp	dnld_esc
dnld2:	cjne	a, #':', dnld2b
	mov	r1, #0
	acall	dnld_inc
	sjmp	dnld3
dnld2b:	  ;check to see if it's a hex digit, error if it is
	acall	asc2hex
	jc	dnld1
	mov	r1, #6
	acall	dnld_inc
	sjmp	dnld1
	  ;begin taking in the line of data
dnld3:	mov	a, #'.'
	acall	cout
	mov	r4, #0		;r4 will count up checksum
	acall	dnld_ghex
	mov	r0, a		;R0 = # of data bytes
	acall	dnld_ghex
	mov	dph, a		;High byte of load address
	acall	dnld_ghex
	mov	dpl, a		;Low byte of load address
	acall	dnld_ghex	;Record type
	cjne	a, #1, dnld4	;End record?
	sjmp	dnld_end
dnld4:	jnz	dnld_unknown	;is it a unknown record type???
dnld5:	mov	a, r0
	jz	dnld_get_cksum
	acall	dnld_ghex	;Get data byte
	mov	r2, a
	mov	r1, #1
	acall	dnld_inc	;count total data bytes received
	mov	a, r2
	lcall	smart_wr	;c=1 if an error writing
	clr	a
	addc	a, #2
	mov	r1, a
;     2 = bytes written
;     3 = bytes unable to write
	acall	dnld_inc
	inc	dptr
	djnz	r0, dnld5
dnld_get_cksum:
	acall	dnld_ghex	;get checksum
	mov	a, r4
	jz	dnld1		;should always add to zero
dnld_sumerr:
	mov	r1, #4
	acall	dnld_inc	;all we can do it count # of cksum errors
	sjmp	dnld1

dnld_unknown:	;handle unknown line type
	mov	a, r0
	jz	dnld_get_cksum	;skip data if size is zero
dnld_ukn2:
	acall	dnld_ghex	;consume all of unknown data
	djnz	r0, dnld_ukn2
	sjmp	dnld_get_cksum

dnld_end:   ;handles the proper end-of-download marker
	mov	a, r0
	jz	dnld_end_3	;should usually be zero
dnld_end_2:
	acall	dnld_ghex	;consume all of useless data
	djnz	r0, dnld_ukn2
dnld_end_3:
	acall	dnld_ghex	;get the last checksum
	mov	a, r4
	jnz	dnld_sumerr
	acall	dnld_dly
	mov	dptr, #dnlds3
	acall	pcstr_h		   ;"download went ok..."
	;consume any cr or lf character that may have been
	;on the end of the last line
	jnb	ri, dnld_sum
	acall	cin
	sjmp	dnld_sum



dnld_esc:   ;handle esc received in the download stream
	acall	dnld_dly
	mov	dptr, #dnlds2	 
	acall	pcstr_h		   ;"download aborted."
	sjmp	dnld_sum

dnld_dly:   ;a short delay since most terminal emulation programs
	    ;won't be ready to receive anything immediately after
	    ;they've transmitted a file... even on a fast Pentium(tm)
	    ;machine with 16550 uarts!
	mov	r0, #0
dnlddly2:mov	r1, #0
	djnz	r1, *		;roughly 128k cycles, appox 0.1 sec
	djnz	r0, dnlddly2
	ret

dnld_inc:     ;increment parameter specified by R1
	      ;note, values in Acc and R1 are destroyed
	mov	a, r1
	anl	a, #00000111b	;just in case
	rl	a
	add	a, #dnld_parm
	mov	r1, a		;now r1 points to lsb
	inc	@r1
	mov	a, @r1
	jnz	dnldin2
	inc	r1
	inc	@r1
dnldin2:ret

dnld_gp:     ;get parameter, and inc to next one (@r1)
	     ;carry clear if parameter is zero.
	     ;16 bit value returned in dptr
	setb	c
	mov	dpl, @r1
	inc	r1
	mov	dph, @r1
	inc	r1
	mov	a, dpl
	jnz	dnldgp2
	mov	a, dph
	jnz	dnldgp2
	clr	c
dnldgp2:ret



;a spacial version of ghex just for the download.  Does not
;look for carriage return or backspace.	 Handles ESC key by
;poping the return address (I know, nasty, but it saves many
;bytes of code in this 4k ROM) and then jumps to the esc
;key handling.	This ghex doesn't echo characters, and if it
;sees ':', it pops the return and jumps to an error handler
;for ':' in the middle of a line.  Non-hex digits also jump
;to error handlers, depending on which digit.
	  
dnld_ghex:
dnldgh1:acall	cin
	acall	upper
	cjne	a, #27, dnldgh3
dnldgh2:pop	acc
	pop	acc
	sjmp	dnld_esc
dnldgh3:cjne	a, #':', dnldgh5
dnldgh4:mov	r1, #5		;handle unexpected beginning of line
	acall	dnld_inc
	pop	acc
	pop	acc
	ajmp	dnld3		;and now we're on a new line!
dnldgh5:acall	asc2hex
	jnc	dnldgh6
	mov	r1, #7
	acall	dnld_inc
	sjmp	dnldgh1
dnldgh6:mov	r2, a		;keep first digit in r2
dnldgh7:acall	cin
	acall	upper
	cjne	a, #27, dnldgh8
	sjmp	dnldgh2
dnldgh8:cjne	a, #':', dnldgh9
	sjmp	dnldgh4
dnldgh9:acall	asc2hex
	jnc	dnldghA
	mov	r1, #7
	acall	dnld_inc
	sjmp	dnldgh7
dnldghA:xch	a, r2
	swap	a
	orl	a, r2
	mov	r2, a
	add	a, r4		;add into checksum
	mov	r4, a
	mov	a, r2		;return value in acc
	ret

;dnlds4 =  "Summary:"
;dnlds5 =  " lines received"
;dnlds6a = " bytes received"
;dnlds6b = " bytes written"

dnld_sum:    ;print out download summary
	mov	a, r6
	push	acc
	mov	a, r7
	push	acc
	mov	dptr, #dnlds4
	acall	pcstr_h
	mov	r1, #dnld_parm
	mov	r6, #dnlds5 & 255
	mov	r7, #dnlds5 >> 8
	acall	dnld_i0
	mov	r6, #dnlds6a & 255
	mov	r7, #dnlds6a >> 8
	acall	dnld_i0
	mov	r6, #dnlds6b & 255
	mov	r7, #dnlds6b >> 8
	acall	dnld_i0

dnld_err:    ;now print out error summary
	mov	r2, #5
dnlder2:acall	dnld_gp
	jc	dnlder3		;any errors?
	djnz	r2, dnlder2
	 ;no errors, so we print the nice message
	mov	dptr, #dnlds13
	acall	pcstr_h
	sjmp	dlnd_sum_done

dnlder3:  ;there were errors, so now we print 'em
	mov	dptr, #dnlds7
	acall	pcstr_h
	  ;but let's not be nasty... only print if necessary
	mov	r1, #(dnld_parm+6)
	mov	r6, #dnlds8 & 255
	mov	r7, #dnlds8 >> 8
	acall	dnld_item
	mov	r6, #dnlds9 & 255
	mov	r7, #dnlds9 >> 8
	acall	dnld_item
	mov	r6, #dnlds10 & 255
	mov	r7, #dnlds10 >> 8
	acall	dnld_item
	mov	r6, #dnlds11 & 255
	mov	r7, #dnlds11 >> 8
	acall	dnld_item
	mov	r6, #dnlds12 & 255
	mov	r7, #dnlds12 >> 8
	acall	dnld_item
dlnd_sum_done:
	pop	acc
	mov	r7, a
	pop	acc
	mov	r6, a
	ajmp	newline

dnld_item:
	acall	dnld_gp		;error conditions
	jnc	dnld_i3
dnld_i2:acall	space
	lcall	pint16u
	acall	r6r7todptr
	acall	pcstr_h
dnld_i3:ret

dnld_i0:acall	dnld_gp		;non-error conditions
	sjmp	dnld_i2


;dnlds7:  = "Errors:"
;dnlds8:  = " bytes unable to write"
;dnlds9:  = " incorrect checksums"
;dnlds10: = " unexpected begin of line"
;dnlds11: = " unexpected hex digits"
;dnlds12: = " unexpected non-hex digits"
;dnlds13: = "No errors detected"



;---------------------------------------------------------;


jump:
	mov	dptr, #prompt8
	acall	pcstr_h
	acall	r6r7todptr
	acall	phex16
	mov	dptr, #prompt4
	acall	pcstr_h
	acall	ghex16
	jb	psw.5, jump3
	jnc	jump2
	ajmp	abort2
jump2:
	acall	dptrtor6r7
jump3:	acall	newline
	mov	dptr, #runs1
	acall	pcstr_h
	acall	r6r7todptr

jump_doit:  ;jump to user code @dptr (this used by run command also)
	clr	a
	mov	psw, a
	mov	b, a
	mov	r0, a
	mov	r1, a
	mov	r2, a
	mov	r3, a
	mov	r4, a
	mov	r5, a
	mov	r6, a
	mov	r7, a
	mov	sp, #8		;start w/ sp=7, like a real reset
	push	acc		;unlike a real reset, push 0000
	push	acc		;in case they end with a RET
	jmp	@a+dptr


;---------------------------------------------------------;

dump:	
	mov	r2, #16		;number of lines to print
	acall	newline2
dump1:	acall	r6r7todptr
	acall	phex16		;tell 'em the memory location
	mov	a,#':'
	acall	cout_sp
	mov	r3, #16		;r3 counts # of bytes to print
	acall	r6r7todptr
dump2:	clr	a
	movc	a, @a+dptr
	inc	dptr
	acall	phex		;print each byte in hex
	acall	space
	djnz	r3, dump2
	acall	dspace		;print a couple extra space
	mov	r3, #16
	acall	r6r7todptr
dump3:	clr	a
	movc	a, @a+dptr
	inc	dptr
	anl	a, #01111111b	;avoid unprintable characters
	cjne	a, #127, dump3b
	clr	a		;avoid 127/255 (delete/rubout) char
dump3b: add	a, #224
	jc	dump4
	clr	a		;avoid control characters
dump4:	add	a, #32
	acall	cout
	djnz	r3, dump3
	acall	newline
	acall	line_dly
	acall	dptrtor6r7
	acall	esc
	jc	dump5
	djnz	r2, dump1	;loop back up to print next line
dump5:	ajmp	newline

;---------------------------------------------------------;

edit:	   ;edit external ram...
	mov	dptr, #edits1
	acall	pcstr_h
	acall	r6r7todptr
edit1:	acall	phex16
	mov	a,#':'
	acall	cout_sp
	mov	a,#'('
	acall	cout
	acall	dptrtor6r7
	clr	a
	movc	a, @a+dptr
	acall	phex
	mov	dptr,#prompt10
	acall	pcstr_h
	acall	ghex
	jb	psw.5,edit2
	jc	edit2
	acall	r6r7todptr
	lcall	smart_wr
	acall	newline
	acall	r6r7todptr
	inc	dptr
	acall	dptrtor6r7
	ajmp	edit1
edit2:	mov	dptr,#edits2
	ajmp	pcstr_h

;---------------------------------------------------------;

dir:
	mov	dptr, #prompt9
	acall	pcstr_h
	mov	r0, #21
dir0a:	acall	space
	djnz	r0, dir0a
	;mov	dptr, #prompt9b
	acall	pcstr_h

	mov	dph, #(bmem >> 8)
dir1:	acall	find		;find the next program in memory
	jc	dir2
dir_end:ajmp	newline		;we're done if no more found
dir2:
	acall	dspace
	mov	dpl, #32	;print its name
	acall	pstr
	mov	dpl, #32	;how long is the name
	acall	lenstr
	mov	a, #33
	clr	c
	subb	a, r0
	mov	r0, a
	mov	a, #' '		;print the right # of spaces
dir3:	acall	cout
	djnz	r0, dir3
	mov	dpl, #0
	acall	phex16		;print the memory location
	mov	r0, #6
	mov	a, #' '
dir4:	acall	cout
	djnz	r0, dir4
	mov	dpl, #4		;now figure out what type it is
	clr	a
	movc	a, @a+dptr
	mov	r2, dph		;save this, we're inside a search

dir5:	cjne	a, #254, dir5b
	mov	dptr, #type1	;it's an external command
	sjmp	dir7
dir5b:	cjne	a, #253, dir5c
dir5bb: mov	dptr, #type4	;it's a startup routine
	sjmp	dir7
dir5c:	cjne	a, #35, dir5d
	mov	dptr, #type2	;it's an ordinary program
	sjmp	dir7
dir5d:	cjne	a, #249, dir5e
	sjmp	dir5bb
dir5e:
dir6:	mov	dptr, #type5	;who knows what the hell it is

dir7:	acall	pcstr_h		   ;print out the type
	mov	dph, r2		;go back and find the next one
	acall	newline
	mov	a, #(emem >> 8)
	cjne	a, dph, dir8	;did we just print the last one?
	ajmp	dir_end
dir8:	inc	dph
	mov	a, dph
	cjne	a, #((emem+1) >> 8) & 255, dir1
	ajmp	dir_end


;type1=Ext Command
;type4=Startup
;type2=Program
;type5=???

;---------------------------------------------------------;


run:   
	acall	newline2
	mov	r2, #255	;first print the menu, count items
	mov	dptr, #bmem
	dec	dph
run2:	inc	dph
	mov	a, dph
	cjne	a, #((emem+1) >> 8) & 255, run2b
	sjmp	run3
run2b:	acall	find
	jnc	run3		;have we found 'em all??
	mov	dpl, #4
	clr	a
	movc	a, @a+dptr
	orl	a, #00000011b
	cpl	a
	jz	run2		;this one doesn't run... find next
	acall	dspace
	inc	r2
	mov	a, #'A'		;print the key to press
	add	a, r2
	acall	cout_sp
	acall	dash_sp
	mov	dpl, #32
	acall	pstr		;and the command name
	acall	newline
	ajmp	run2		;and continue doing this
run3:	cjne	r2, #255, run4	;are there any to run??
	mov	dptr, #prompt5
	ajmp	pcstr_h
run4:	mov	dptr, #prompt3	;ask the big question!
	acall	pcstr_h
	mov	a, #'A'
	acall	cout
	acall	dash
	mov	a, #'A'		;such user friendliness...
	add	a, r2		;even tell 'em the choices
	acall	cout
	mov	dptr, #prompt4
	acall	pcstr_h
	acall	cin_filter_h
	cjne	a, #27, run4aa	;they they hit <ESC>
	ajmp	newline
run4aa: mov	r3, a
	mov	a, #31
	clr	c
	subb	a, r2
	mov	a, r3
	jc	run4a
	acall	upper
run4a:	acall	cout
	mov	r3, a
	acall	newline
	;check to see if it's under 32, if so convert to uppercase
	mov	a, r3
	clr	c
	subb	a, #'A'
	jc	run4		;if they typed less than 'A'
	mov	r3, a		;R3 has the number they typed
	mov	a, r2		;A=R2 has the maximum number
	clr	c
	subb	a, r3
	jc	run4		;if they typed over the max
	inc	r3
	mov	dptr, #bmem
	dec	dph
run5:	inc	dph
	mov	a, dph
	cjne	a, #((emem+1) >> 8) & 255, run5b
	sjmp	run8
run5b:	acall	find
	jnc	run8		;Shouldn't ever do this jump!
	mov	dpl, #4
	clr	a
	movc	a, @a+dptr
	orl	a, #00000011b
	cpl	a
	jz	run5		;this one doesn't run... find next
	djnz	r3, run5	;count til we find the one they want
	acall	newline
	mov	dpl, #64
	ajmp	jump_doit
run8:	ret

;---------------------------------------------------------;

help:
	mov	dptr, #help1txt
	acall	pcstr_h
	mov	r4, #help_key
	mov	dptr, #help_cmd
	acall	help2
	mov	r4, #dir_key
	;mov	 dptr, #dir_cmd
	acall	help2
	mov	r4, #run_key
	;mov	 dptr, #run_cmd
	acall	help2
	mov	r4, #dnld_key
	;mov	 dptr, #dnld_cmd
	acall	help2
	mov	r4, #upld_key
	;mov	 dptr, #upld_cmd
	acall	help2
	mov	r4, #nloc_key
	;mov	 dptr, #nloc_cmd
	acall	help2
	mov	r4, #jump_key
	;mov	 dptr, #jump_cmd
	acall	help2
	mov	r4, #dump_key
	;mov	 dptr, #dump_cmd
	acall	help2
	mov	r4, #intm_key
	;mov	dptr, #intm_cmd
	acall	help2
	mov	r4, #edit_key
	;mov	 dptr, #edit_cmd
	acall	help2
	mov	r4, #clrm_key
	;mov	 dptr, #clrm_cmd
	acall	help2
	mov	a, #has_flash
	jz	help_skerfm
	mov	r4, #erfr_key
	;mov	 dptr, #erfr_cmd
	acall	help2
help_skerfm:
	mov	dptr, #help2txt
	acall	pcstr_h
	mov	dptr, #bmem
help3:	acall	find
	jnc	help4
	mov	dpl, #4
	clr	a
	movc	a,@a+dptr
	cjne	a, #254, help3a	   ;only FE is an ext command
	acall	dspace
	inc	dpl
	clr	a
	movc	a,@a+dptr
	acall	cout
	acall	dash_sp
	mov	dpl, #32
	acall	pstr
	acall	newline
help3a: inc	dph
	mov	a, dph
	cjne	a, #((emem+1) >> 8) & 255, help3
help4:	
	ajmp	newline

help2:				;print 11 standard lines
	acall	dspace		;given key in R4 and name in dptr
	mov	a, r4
	acall	cout
	acall	dash_sp
	acall	pcstr_h
	ajmp	newline

;---------------------------------------------------------;

upld:

	acall	get_mem
	;assume we've got the beginning address in r3/r2
	;and the final address in r5/r4 (r4=lsb)...

	;print out what we'll be doing
	mov	dptr, #uplds3
	acall	pcstr_h
	mov	a, r3
	acall	phex
	mov	a, r2
	acall	phex
	;mov	 dptr, #uplds4
	acall	pcstr_h
	mov	a, r5
	acall	phex
	mov	a, r4
	acall	phex
	acall	newline

	;need to adjust end location by 1...
	mov	dph, r5
	mov	dpl, r4
	inc	dptr
	mov	r4, dpl
	mov	r5, dph

	mov	dptr, #prompt7
	acall	pcstr_h
	acall	cin
	cjne	a, #27, upld2e
	ajmp	abort_it
upld2e: acall	newline
	mov	dpl, r2
	mov	dph, r3

upld3:	mov	a, r4		;how many more bytes to output??
	clr	c
	subb	a, dpl
	mov	r2, a
	mov	a, r5
	subb	a, dph
	jnz	upld4		;if >256 left, then do next 16
	mov	a, r2
	jz	upld7		;if we're all done
	anl	a, #11110000b
	jnz	upld4		;if >= 16 left, then do next 16
	sjmp	upld5		;otherwise just finish it off
upld4:	mov	r2, #16
upld5:	mov	a, #':'		;begin the line
	acall	cout
	mov	a, r2
	acall	phex		;output # of data bytes
	acall	phex16		;output memory location
	mov	a, dph
	add	a, dpl
	add	a, r2
	mov	r3, a		;r3 will become checksum
	clr	a
	acall	phex		;output 00 code for data
upld6:	clr	a
	movc	a, @a+dptr
	acall	phex		;output each byte
	add	a, r3
	mov	r3, a
	inc	dptr
	djnz	r2, upld6	;do however many bytes we need
	mov	a, r3
	cpl	a
	inc	a
	acall	phex		;and finally the checksum
	acall	newline
	acall	line_dly
	acall	esc
	jnc	upld3		;keep working if no esc pressed
	sjmp	abort_it
upld7:	mov	a, #':'
	acall	cout
	clr	a
	acall	phex
	acall	phex
	acall	phex
	inc	a
	acall	phex
	mov	a, #255
	acall	phex
upld8:	ajmp	newline2


line_dly: ;a brief delay between line while uploading, so the
	;receiving host can be slow (i.e. most windows software)
	mov	a, r0
	push	acc
	mov	r0, #line_delay*2
line_d2:mov	a, th0		;get baud rate const
line_d3:inc	a
	nop
	nop
	jnz	line_d3
	djnz	r0, line_d2
	pop	acc
	mov	r0, a
	ret

;---------------------------------------------------------;

get_mem:     ;this thing gets the begin and end locations for
	     ;a few commands.  If an esc or enter w/ no input,
	     ;it pops it's own return and returns to the menu
	     ;(nasty programming, but we need tight code for 4k rom)
	acall	newline2
	mov	dptr, #beg_str
	acall	pcstr_h
	acall	ghex16
	jc	pop_it
	jb	psw.5, pop_it
	push	dph
	push	dpl
	acall	newline
	mov	dptr, #end_str
	acall	pcstr_h
	acall	ghex16
	mov	r5, dph
	mov	r4, dpl
	pop	acc
	mov	r2, a
	pop	acc
	mov	r3, a
	jc	pop_it
	jb	psw.5, pop_it
	ajmp	newline

pop_it: pop	acc
	pop	acc
abort_it:
	acall	newline
abort2: mov	dptr, #abort
	ajmp	pcstr_h


clrm:
	acall	get_mem
	mov	dptr, #sure
	acall	pcstr_h
	acall	cin_filter_h
	acall	upper
	cjne	a, #'Y', abort_it
	acall	newline2
     ;now we actually do it

clrm2:	mov	dph, r3
	mov	dpl, r2
clrm3:	clr	a
	lcall	smart_wr
	mov	a, r5
	cjne	a, dph, clrm4
	mov	a, r4
	cjne	a, dpl, clrm4
	ret
clrm4:	inc	dptr
	sjmp	clrm3

;---------------------------------------------------------;

nloc:
	mov	dptr, #prompt6
	acall	pcstr_h
	acall	ghex16
	jc	abort2
	jb	psw.5, abort2
	acall	dptrtor6r7
	ajmp	newline2

;---------------------------------------------------------;

erfr:
	acall	newline2
	mov	dptr, #erfr_cmd
	acall	pcstr_h
	mov	a, #','
	acall	cout_sp
	mov	dptr, #sure
	acall	pcstr_h
	acall	cin_filter_h
	acall	upper
	cjne	a, #'Y', abort_it
	acall	newline2
	lcall	erall
	mov	dptr, #erfr_ok
	jnc	erfr_end
	mov	dptr, #erfr_err
erfr_end:
	ajmp	pcstr_h



;---------------------------------------------------------;

intm:	acall	newline
	mov	r0, #0
intm2:	acall	newline
	cjne	r0, #0x80, intm3 
	ajmp	newline
intm3:	mov	a, r0
	acall	phex
	mov	a, #':'
	acall	cout
intm4:	acall	space
	mov	a, @r0
	acall	phex
	inc	r0
	mov	a, r0
	anl	a, #00001111b
	jnz	intm4
	sjmp	intm2





;**************************************************************
;**************************************************************
;*****							  *****
;*****	     2k page boundry is somewhere near here	  *****
;*****	       (no ajmp or acall past this point)	  *****
;*****							  *****
;**************************************************************
;**************************************************************



;---------------------------------------------------------;
;							  ;
;   Subroutines for memory managment and non-serial I/O	  ;
;							  ;
;---------------------------------------------------------;


;finds the next header in the external memory.
;  Input DPTR=point to start search (only MSB used)
;  Output DPTR=location of next module
;    C=set if a header found, C=clear if no more headers
find:	mov	dpl, #0
	clr	a
	movc	a, @a+dptr
	cjne	a, #0xA5, find3
	inc	dptr
	clr	a
	movc	a, @a+dptr
	cjne	a, #0xE5, find3
	inc	dptr
	clr	a
	movc	a, @a+dptr
	cjne	a, #0xE0, find3
	inc	dptr
	clr	a
	movc	a, @a+dptr
	cjne	a, #0xA5, find3
	mov	dpl, #0			;found one here!
	setb	c
	ret
find3:	mov	a, #(emem >> 8)
	cjne	a, dph, find4		;did we just check the end
	clr	c
	ret
find4:	inc	dph			;keep on searching
	sjmp	find



;routine that erases the whole flash rom!  C=1 if failure, C=0 if ok

erall:	mov	a, #has_flash
	jz	erallno
	mov	dptr, #bflash		;is it already erased ??
erall0: clr	a
	movc	a, @a+dptr
	cpl	a
	jnz	erall_b			;do actual erase if any byte not 255
	inc	dptr
	mov	a, #((eflash+1) & 255)
	cjne	a, dpl, erall0
	mov	a, #(((eflash+1) >> 8) & 255)
	cjne	a, dph, erall0
	;if we get here, the entire chip was already erased,
	;so there is no need to do anything
	clr	c
	ret
erall_b:
	mov	dptr, #bflash		;first program to all 00's
erall1: clr	a
	movc	a, @a+dptr
	jz	erall2			;don't waste time!
	clr	a
	lcall	prgm			;ok, program this byte
	;if the program operation failed... we should abort because
	;they are all likely to fail and it will take a long time...
	;which give the appearance that the program has crashed,
	;when it's really following the flash rom algorithm
	;correctly and getting timeouts.
	jc	erallno
	;mov	a, #'.'
	;lcall	cout
erall2: inc	dptr
	mov	a, #((eflash+1) & 255)
	cjne	a, dpl, erall1
	mov	a, #(((eflash+1) >> 8) & 255)
	cjne	a, dph, erall1		;after this it's all 00's
	mov	dptr, #bflash		;beginning address
	mov	r4, #232		;max # of trials, lsb
	mov	r5, #4			;max # of trials, msb-1
erall3: 
	;mov	a, #'#'
	;lcall	cout
	djnz	r4, erall3a
	djnz	r5, erall3a
erallno:setb	c
	ret				;if it didn't work!
erall3a:mov	a, #0x20
	mov	c, ea		 ;-	;turn off all interrupts!!
	mov	psw.1, c
	clr	ea
	movx	@dptr, a		;send the erase setup
	movx	@dptr, a		;and begin the erase
	mov	r3, #erwait1
erwt:	mov	r2, #erwait2		;now wait 10ms...
	djnz	r2, *
	djnz	r3, erwt
erall4: mov	a, #0xA0
	movx	@dptr, a		;send erase verify
	mov	r2, #verwait		;wait for 6us
	djnz	r2, *
	clr	a
	movc	a, @a+dptr
	mov	c, psw.1
	mov	ea, c		;-     ;turn interrupts back on
	cpl	a
	jnz	erall3			;erase again if not FF
	inc	dptr
	mov	a, #(((eflash+1) >> 8) & 255)  ;verify whole array
	cjne	a, dph, erall4
	mov	a, #((eflash+1) & 255)
	cjne	a, dpl, erall4
	mov	a, #255
	mov	dptr, #bflash
	movx	@dptr, a		;reset the flash rom
	clr	a
	movx	@dptr, a		;and go back to read mode
	clr	c
	ret




;a routine that writes ACC to into flash memory at DPTR
; assumes that Vpp is active and stable already.
; C is set if error occurs, C is clear if it worked

prgm:	mov	b, a
	mov	a, r2
	push	acc
	mov	a, r3
	push	acc
	mov	r2, #25	       ;try to program 25 times if needed
prgm2:	mov	a, #40h
	mov	c, ea		 ;-	;turn off all interrupts!!
	mov	psw.1, c
	clr	ea
	movx	@dptr, a	;send setup programming command
	mov	a, b
	movx	@dptr, a	;write to the cell
	mov	r3, #pgmwait	;now wait for 10us
	djnz	r3, *
	mov	a, #0xC0
	movx	@dptr, a	;send program verify command
	mov	r3, #verwait	;wait 6us while it adds margin
	djnz	r3, *
	clr	a
	movc	a, @a+dptr
	mov	c, psw.1
	mov	ea, c		;-     ;turn interrupts back on
	clr	c
	subb	a, b
	jz	prgmok		;note, C is still clear is ACC=0
	djnz	r2, prgm2
prgmbad:setb	c		;it gets here if programming failure
prgmok: clr	a
	movx	@dptr, a	;and go back into read mode
	pop	acc
	mov	r3, a
	pop	acc
	mov	r2, a
	mov	a, b		;restore ACC to original value
	ret



;************************************
;To make PAULMON2 able to write to other
;types of memory than RAM and flash rom,
;modify this "smart_wr" routine.  This
;code doesn't accept any inputs other
;that the address (dptr) and value (acc),
;so this routine must know which types
;of memory are in what address ranges
;************************************


;Write to Flash ROM or ordinary RAM.  Carry bit will indicate
;if the value was successfully written, C=1 if not written.


smart_wr:
	push	acc
	push	b
	mov	b, a
	;do we even have a flash rom?
	mov	a, #has_flash
	jz	wr_ram
	;there is a flash rom, but is this address in it?
	mov	a, dph
	cjne	a, #(eflash >> 8), isfl3
	sjmp	wr_flash
isfl3:	jnc	wr_ram
	cjne	a, #(bflash >> 8), isfl4
	sjmp	wr_flash
isfl4:	jnc	wr_flash
	sjmp	wr_ram

wr_flash:
	mov	a, b
	acall	prgm
	pop	b
	pop	acc
	ret

wr_ram: mov	a, b
	movx	@dptr, a	;write the value to memory
	clr	a
	movc	a, @a+dptr	;read it back from code memory
	clr	c
	subb	a, b
	jz	smwrok
	movx	a, @dptr	;read it back from data memory
	clr	c
	subb	a, b
	jz	smwrok
smwrbad:setb	c
	sjmp	smwrxit
smwrok: clr	c
smwrxit:pop	b
	pop	acc
	ret




;---------------------------------------------------------;
;							  ;
;	Power-On initialization code and such...	  ;
;							  ;
;---------------------------------------------------------;

;first the hardware has to get initialized.

intr_return:
	reti

poweron:
	clr	a
	mov	ie, a		;all interrupts off
	mov	ip, a
	mov	psw, #psw_init
	;clear any interrupt status, just in case the user put
	;"ljmp 0" inside their interrupt service code.
	acall	intr_return
	acall	intr_return
	cpl	a
	mov	p0, a
	mov	p1, a
	mov	p2, a
	mov	p3, a
	mov	sp, #stack

;Before we start doing any I/O, a short delay is required so
;that any external hardware which may be in "reset mode" can
;initialize.  This is typically a problem when a 82C55 chip
;is used and its reset line is driven from the R-C reset
;circuit used for the 8051.  Because the 82C55 reset pin
;switches from zero to one at a higher voltage than the 8051,
;any 82C55 chips would still be in reset mode right now...

rst_dly:
	mov	r1, #200	;approx 100000 cycles
rdly2:	mov	r2, #249	;500 cycles
	djnz	r2, *
	djnz	r1, rdly2

;Check for the Erase-on-startup signal and erase Flash ROM 
;if it's there.

	mov	a, #has_flash
	jz	skip_erase
	mov	a, #erase_pin
	jz	skip_erase
	mov	r0, #250	;check it 250 times, just to be sure
chk_erase:
	mov	c, erase_pin
	mov	r1, #200
	djnz	r1, *		;short delay
	jc	skip_erase	;skip erase if this bit is not low
	djnz	r0, chk_erase
	lcall	erall		;and this'll delete the flash rom
skip_erase:

;run any user initialization programs in external memory
	mov	b, #249
	acall	stcode

;initialize the serial port, auto baud detect if necessary
	acall	autobaud	;set up the serial port
	;mov	a, th1
	;lcall	phex

;run the start-up programs in external memory.
	mov	b, #253
	acall	stcode

;now print out the nice welcome message

welcome:
	mov	r0, #24
welcm2: lcall	newline
	djnz	r0, welcm2
	mov	r0, #15
	mov	a, #' '
welcm4: lcall	cout
	djnz	r0, welcm4
	mov	dptr, #logon1
	lcall	pcstr
	mov	dptr, #logon2
	lcall	pcstr
	lcall	dir
	mov	r6, #(pgm & 255)
	mov	r7, #(pgm >> 8)
	ljmp	menu


stcode: mov	dptr, #bmem	 ;search for startup routines
stcode2:lcall	find
	jnc	stcode5
	mov	dpl, #4
	clr	a
	movc	a, @a+dptr
	cjne	a, b, stcode4	;only startup code if matches B
	push	b
	push	dph
	mov	a, #(stcode3 & 255)
	push	acc
	mov	a, #(stcode3 >> 8)
	push	acc
	mov	dpl, #64
	clr	a
	jmp	@a+dptr		;jump to the startup code
stcode3:pop	dph		;hopefully it'll return to here
	pop	b
stcode4:inc	dph
	mov	a, dph
	cjne	a, #((emem+1) >> 8) & 255, stcode2
stcode5:ret			;now we've executed all of 'em


;to do automatic baud rate detection, we assume the user will
;press the carriage return, which will cause this bit pattern
;to appear on port 3 pin 0 (CR = ascii code 13, assume 8N1 format)
;
;	       0 1 0 1 1 0 0 0 0 1
;	       | |	       | |
; start bit----+ +--lsb	  msb--+ +----stop bit
;
;we'll start timer #1 in 16 bit mode at the transition between the
;start bit and the LSB and stop it between the MBS and stop bit.
;That will give approx the number of cpu cycles for 8 bits.  Divide
;by 8 for one bit and by 16 since the built-in UART takes 16 timer
;overflows for each bit.  We need to be careful about roundoff during
;division and the result has to be inverted since timer #1 counts up.  Of
;course, timer #1 gets used in 8-bit auto reload mode for generating the
;built-in UART's baud rate once we know what the reload value should be.


autobaud:
	mov	a, #baud_const	;skip if user supplied baud rate constant
	jnz	autoend_jmp
	mov	a, baud_save+3	;is there a value from a previous boot?
	xrl	baud_save+2, #01010101b
	xrl	baud_save+1, #11001100b
	xrl	baud_save+0, #00011101b
	cjne	a, baud_save+2, autob1
	cjne	a, baud_save+1, autob1
	cjne	a, baud_save+0, autob1
autoend_jmp:
	ajmp	autoend

autob1: ;wait for inactivity

	mov	pcon, #0x80	;configure uart, fast baud
	mov	scon, #0x42	;configure uart, but receive disabled
	mov	tmod, #0x11	;get timers ready for action (16 bit mode)
	clr	a
	mov	tcon, a
	mov	tl0, a
	mov	th0, a
	mov	tl1, a
	mov	th1, a

	;make sure there is no activity on the line
	;before we actually begin looking for the carriage return
	mov	r0, #200
autob1b:mov	r1, #30
autob1c:jnb	p3.0, autob1
	djnz	r1, autob1c
	djnz	r0, autob1b

autob2: ;look for the bits of the carriage return
	jb	p3.0, autob2	;wait for start bit
	jb	p3.0, autob2
	jb	p3.0, autob2	;  check it a few more times to make
	jb	p3.0, autob2	;  sure we don't trigger on some noise
	jb	p3.0, autob2
autob2b:jnb	p3.0, autob2b	;wait for bit #0 to begin
	setb	tr1		;and now we're timing it
autob2c:jb	tf1, autob1	;check for timeout while waiting
	jb	p3.0, autob2c	;wait for bit #1 to begin
autob2d:jb	tf1, autob1	;check for timeout while waiting
	jnb	p3.0, autob2d	;wait for bit #2 to begin
autob2e:jb	tf1, autob1	;check for timeout while waiting
	jb	p3.0, autob2e	;wait for bit #4 to begin
	setb	tr0		;start timing last 4 bits
autob2f:jb	tf1, autob1	;check for timeout while waiting
	jnb	p3.0, autob2f	;wait for stop bit to begin
	clr	tr1		;stop timing (both timers)
	clr	tr0

	jb	tf1, autob1	;check for timeout one last time

	;compute the baud rate based on timer1
	mov	a, tl1
	rlc	a
	mov	b, a
	mov	a, th1
	rlc	a
	jc	autob1		;error if timer0 > 32767
	mov	c, b.7
	addc	a, #0
	cpl	a
	inc	a		;now a has the value to load into th1
	jz	autob1		;error if baud rate too fast

	;after we get the carriage return, we need to make sure there
	;isn't any "crap" on the serial line, as there is in the case
	;were we get the letter E (and conclude the wrong baud rate).
	;unfortunately the simple approach of just looking at the line
	;for silence doesn't work, because we have to accept the case
	;where the user's terminal emulation is configured to send a
	;line feed after the carriage return.  The best thing to do is
	;use the uart and look see if it receives anything

autob3: mov	th1, a		;config timer1
	mov	tl1, #255	;start asap!
	mov	tmod, #0x21	;autoreload mode
	setb	ren		;turn on the uart
	setb	tr1		;turn on timer1 for its clock

	mov	a, th1
	cpl	a
	inc	a
	mov	r1, a
autob3b:mov	r0, #255
autob3c:djnz	r0, autob3c
	djnz	r1, autob3b

	jnb	ri, autob4
	;if we got here, there was some stuff after the carriage
	;return, so we'll read it and see if it was the line feed
	clr	ri
	mov	a, sbuf
	anl	a, #01111111b
	add	a, #246
	jz	autob4		;ok if 0A, the line feed character
	add	a, #5
	jz	autob4		;of if 05, since we may have missed start bit
autob1_jmp:
	ljmp	autob1
autob4:
	;compute the baud rate based on timer0, check against timer1 value
	mov	a, tl0
	rlc	a
	mov	r0, a
	mov	a, th0
	rlc	a
	mov	r1, a
	jc	autob1_jmp	;error if timer0 > 32767
	mov	a, r0
	rlc	a
	mov	b, a
	mov	a, r1
	rlc	a
	mov	c, b.7
	addc	a, #0
	jz	autob1_jmp	;error if baud too fast!
	cpl	a
	inc	a
	cjne	a, th1, autob1_jmp
	;acc has th1 value at this point

autoend:mov	baud_save+3, a
	mov	baud_save+2, a	;store the baud rate for next warm boot.
	mov	baud_save+1, a
	mov	baud_save+0, a
	xrl	baud_save+2, #01010101b
	xrl	baud_save+1, #11001100b 
	xrl	baud_save+0, #00011101b 
	mov	th1, a
	mov	tl1, a
	mov	tmod, #0x21	;set timer #1 for 8 bit auto-reload
	mov	pcon, #0x80	;configure built-in uart
	mov	scon, #0x52
	setb	tr1		;start the baud rate timer
	ret



;---------------------------------------------------------;
;							  ;
;     More subroutines, but less frequent used, so	  ;
;     they're down here in the second 2k page.		  ;
;							  ;
;---------------------------------------------------------;



;this twisted bit of code looks for escape sequences for
;up, down, left, right, pageup, and pagedown, as well
;as ordinary escape and ordinary characters.  Escape
;sequences are required to arrive with each character
;nearly back-to-back to the others, otherwise the characters
;are treated as ordinary user keystroaks.  cin_filter
;returns a single byte when it sees the multi-byte escape
;sequence, as shown here.

; return value	 key	      escape sequence
;   11 (^K)	 up	      1B 5B 41
;   10 (^J)	 down	      1B 5B 42
;   21 (^U)	 right	      1B 5B 43
;    8 (^H)	 left	      1B 5B 44
;   25 (^Y)	 page up      1B 5B 35 7E
;   26 (^Z)	 page down    1B 5B 36 7E

.equ	esc_char, 27

cin_filter:
	jnb	ri, cinf1
	lcall	cin
	cjne	a, #esc_char, cinf_end
	;if esc was already in sbuf, just ignore it
cinf1:	lcall	cin
	cjne	a, #esc_char, cinf_end
cinf2:	acall	cinf_wait
	jb	ri, cinf4
	mov	a, #esc_char
	ret			;an ordinary ESC

cinf4:	;if we get here, it's a control code, since a character
	;was received shortly after receiving an ESC character
	lcall	cin
	cjne	a, #'[', cinf_consume
	acall	cinf_wait
	jnb	ri, cin_filter
	lcall	cin
cinf5a: cjne	a, #'A', cinf5b
	mov	a, #11
	ret
cinf5b: cjne	a, #'B', cinf5c
	mov	a, #10
	ret
cinf5c: cjne	a, #'C', cinf5d
	mov	a, #21
	ret
cinf5d: cjne	a, #'D', cinf5e
	mov	a, #8
	ret
cinf5e: cjne	a, #0x35, cinf5f
	sjmp	cinf8
cinf5f: cjne	a, #0x36, cinf5g
	sjmp	cinf8
cinf5g: sjmp	cinf_consume		;unknown escape sequence

cinf8:	;when we get here, we've got the sequence for pageup/pagedown
	;but there's one more incoming byte to check...
	push	acc
	acall	cinf_wait
	jnb	ri, cinf_restart
	lcall	cin
	cjne	a, #0x7E, cinf_notpg
	pop	acc
	add	a, #228
cinf_end: ret
cinf_restart:
	pop	acc
	sjmp	cin_filter
cinf_notpg:
	pop	acc
;unrecognized escape... eat up everything that's left coming in
;quickly, then begin looking again
cinf_consume:
	acall	cinf_wait
	jnb	ri, cin_filter
	lcall	cin
	cjne	a, #esc_char, cinf_consume
	sjmp	cinf2

;this thing waits for a character to be received for approx
;4 character transmit time periods.  It returns immedately
;or after the entire wait time.	 It does not remove the character
;from the buffer, so ri should be checked to see if something
;actually did show up while it was waiting
	.equ	char_delay, 4		;number of char xmit times to wait
cinf_wait:
	mov	a, r2
	push	acc
	mov	r2, #char_delay*5
cinfw2: mov	a, th0
cinfw3: jb	ri, cinfw4
	inc	a
	jnz	cinfw3
	djnz	r2, cinfw2
cinfw4: pop	acc
	mov	r2, a
	ret




pint8u: ;prints the unsigned 8 bit value in Acc in base 10
	push	b
	push	acc
	sjmp	pint8b

pint8:	;prints the signed 8 bit value in Acc in base 10
	push	b
	push	acc
	jnb	acc.7, pint8b
	mov	a, #'-'
	lcall	cout
	pop	acc
	push	acc
	cpl	a
	add	a, #1
pint8b: mov	b, #100
	div	ab
	setb	f0
	jz	pint8c
	clr	f0
	add	a, #'0'
	lcall	cout
pint8c: mov	a, b
	mov	b, #10
	div	ab
	jnb	f0, pint8d
	jz	pint8e
pint8d: add	a, #'0'
	lcall	cout
pint8e: mov	a, b
	add	a, #'0'
	lcall	cout
	pop	acc
	pop	b
	ret



	;print 16 bit unsigned integer in DPTR, using base 10.
pint16u:	;warning, destroys r2, r3, r4, r5, psw.5
	push	acc
	mov	a, r0
	push	acc
	clr	psw.5
	mov	r2, dpl
	mov	r3, dph

pint16a:mov	r4, #16		;ten-thousands digit
	mov	r5, #39
	acall	pint16x
	jz	pint16b
	add	a, #'0'
	lcall	cout
	setb	psw.5

pint16b:mov	r4, #232	;thousands digit
	mov	r5, #3
	acall	pint16x
	jnz	pint16c
	jnb	psw.5, pint16d
pint16c:add	a, #'0'
	lcall	cout
	setb	psw.5

pint16d:mov	r4, #100	;hundreds digit
	mov	r5, #0
	acall	pint16x
	jnz	pint16e
	jnb	psw.5, pint16f
pint16e:add	a, #'0'
	lcall	cout
	setb	psw.5

pint16f:mov	a, r2		;tens digit
	mov	r3, b
	mov	b, #10
	div	ab
	jnz	pint16g
	jnb	psw.5, pint16h
pint16g:add	a, #'0'
	lcall	cout

pint16h:mov	a, b		;and finally the ones digit
	mov	b, r3
	add	a, #'0'
	lcall	cout

	pop	acc
	mov	r0, a
	pop	acc
	ret

;ok, it's a cpu hog and a nasty way to divide, but this code
;requires only 21 bytes!  Divides r2-r3 by r4-r5 and leaves
;quotient in r2-r3 and returns remainder in acc.  If Intel
;had made a proper divide, then this would be much easier.

pint16x:mov	r0, #0
pint16y:inc	r0
	clr	c
	mov	a, r2
	subb	a, r4
	mov	r2, a
	mov	a, r3
	subb	a, r5
	mov	r3, a
	jnc	pint16y
	dec	r0
	mov	a, r2
	add	a, r4
	mov	r2, a
	mov	a, r3
	addc	a, r5
	mov	r3, a
	mov	a, r0
	ret



;pcstr prints the compressed strings.  A dictionary of 128 words is
;stored in 4 bit packed binary format.	When pcstr finds a byte in
;a string with the high bit set, it prints the word from the dictionary.
;A few bytes have special functions and everything else prints as if
;it were an ordinary string.

; special codes for pcstr:
;    0 = end of string
;   13 = CR/LF
;   14 = CR/LF and end of string
;   31 = next word code should be capitalized

pcstr:	push	acc
	mov	a, r0
	push	acc
	mov	a, r1
	push	acc
	mov	a, r4
	push	acc
	setb	psw.1
	setb	psw.5
pcstr1: clr	a
	movc	a, @a+dptr
	inc	dptr
	jz	pcstr2
	jb	acc.7, decomp
	anl	a, #0x7F
pcstrs1:cjne	a, #13, pcstrs2
	lcall	newline
	setb	psw.1
	sjmp	pcstr1
pcstrs2:cjne	a, #31, pcstrs3
	clr	psw.5
	sjmp	pcstr1
pcstrs3:cjne	a, #14, pcstrs4
	lcall	newline
	sjmp	pcstr2
pcstrs4:
	clr	psw.1
	lcall	cout
	sjmp	pcstr1
pcstr2: pop	acc
	mov	r4, a
	pop	acc
	mov	r1, a
	pop	acc
	mov	r0, a
	pop	acc
	ret

;dcomp actually takes care of printing a word from the dictionary

; dptr = position in packed words table
; r4=0 if next nibble is low, r4=255 if next nibble is high

decomp: anl	a, #0x7F
	mov	r0, a		;r0 counts which word
	jb	psw.1, decomp1	;avoid leading space if first word
	lcall	space
decomp1:clr	psw.1
	push	dpl
	push	dph
	mov	dptr, #words
	mov	r4, #0
	mov	a, r0
	jz	dcomp3
	;here we must seek past all the words in the table
	;that come before the one we're supposed to print
	mov	r1, a
dcomp2: acall	get_next_nibble
	jnz	dcomp2
	;when we get here, a word has been skipped... keep doing
	;this until we're pointing to the correct one
	djnz	r1, dcomp2
dcomp3: ;now we're pointing to the correct word, so all we have
	;to do is print it out
	acall	get_next_nibble
	jz	dcomp_end
	cjne	a, #15, dcomp4
	;the character is one of the 12 least commonly used
	acall	get_next_nibble
	inc	a
	movc	a, @a+pc
	sjmp	dcomp5
	.db	"hfwgybxvkqjz"
dcomp4: ;the character is one of the 14 most commonly used
	inc	a
	movc	a, @a+pc
	sjmp	dcomp5
	.db	"etarnisolumpdc"
dcomp5: ;decide if it should be uppercase or lowercase
	mov	c, psw.5
	mov	acc.5, c
	setb	psw.5
	cjne	r0, #20, dcomp6
	clr	acc.5
dcomp6: cjne	r0, #12, dcomp7
	clr	acc.5
dcomp7: lcall	cout
	sjmp	dcomp3
dcomp_end:
	pop	dph
	pop	dpl
	ajmp	pcstr1

get_next_nibble:	;...and update dptr and r4, of course
	clr	a
	movc	a, @a+dptr
	cjne	r4, #0, gnn2
	mov	r4, #255
	anl	a, #00001111b
	ret
gnn2:	mov	r4, #0
	inc	dptr
	swap	a
	anl	a, #00001111b
	ret


;---------------------------------------------------------;
;							  ;
;	 Here begins the data tables and strings	  ;
;							  ;
;---------------------------------------------------------;

;this is the dictionary of 128 words used by pcstr.

words:
	.db	0x82, 0x90, 0xE8, 0x23, 0x86, 0x05, 0x4C, 0xF8
	.db	0x44, 0xB3, 0xB0, 0xB1, 0x48, 0x5F, 0xF0, 0x11
	.db	0x7F, 0xA0, 0x15, 0x7F, 0x1C, 0x2E, 0xD1, 0x40
	.db	0x5A, 0x50, 0xF1, 0x03, 0xBF, 0xBA, 0x0C, 0x2F
	.db	0x96, 0x01, 0x8D, 0x3F, 0x95, 0x38, 0x0D, 0x6F
	.db	0x5F, 0x12, 0x07, 0x71, 0x0E, 0x56, 0x2F, 0x48
	.db	0x3B, 0x62, 0x58, 0x20, 0x1F, 0x76, 0x70, 0x32
	.db	0x24, 0x40, 0xB8, 0x40, 0xE1, 0x61, 0x8F, 0x01
	.db	0x34, 0x0B, 0xCA, 0x89, 0xD3, 0xC0, 0xA3, 0xB9
	.db	0x58, 0x80, 0x04, 0xF8, 0x02, 0x85, 0x60, 0x25
	.db	0x91, 0xF0, 0x92, 0x73, 0x1F, 0x10, 0x7F, 0x12
	.db	0x54, 0x93, 0x10, 0x44, 0x48, 0x07, 0xD1, 0x26
	.db	0x56, 0x4F, 0xD0, 0xF6, 0x64, 0x72, 0xE0, 0xB8
	.db	0x3B, 0xD5, 0xF0, 0x16, 0x4F, 0x56, 0x30, 0x6F
	.db	0x48, 0x02, 0x5F, 0xA8, 0x20, 0x1F, 0x01, 0x76
	.db	0x30, 0xD5, 0x60, 0x25, 0x41, 0xA4, 0x2C, 0x60
	.db	0x05, 0x6F, 0x01, 0x3F, 0x26, 0x1F, 0x30, 0x07
	.db	0x8E, 0x1D, 0xF0, 0x63, 0x99, 0xF0, 0x42, 0xB8
	.db	0x20, 0x1F, 0x23, 0x30, 0x02, 0x7A, 0xD1, 0x60
	.db	0x2F, 0xF0, 0xF6, 0x05, 0x8F, 0x93, 0x1A, 0x50
	.db	0x28, 0xF0, 0x82, 0x04, 0x6F, 0xA3, 0x0D, 0x3F
	.db	0x1F, 0x51, 0x40, 0x23, 0x01, 0x3E, 0x05, 0x43
	.db	0x01, 0x7A, 0x01, 0x17, 0x64, 0x93, 0x30, 0x2A
	.db	0x08, 0x8C, 0x24, 0x30, 0x99, 0xB0, 0xF3, 0x19
	.db	0x60, 0x25, 0x41, 0x35, 0x09, 0x8E, 0xCB, 0x19
	.db	0x12, 0x30, 0x05, 0x1F, 0x31, 0x1D, 0x04, 0x14
	.db	0x4F, 0x76, 0x12, 0x04, 0xAB, 0x27, 0x90, 0x56
	.db	0x01, 0x2F, 0xA8, 0xD5, 0xF0, 0xAA, 0x26, 0x20
	.db	0x5F, 0x1C, 0xF0, 0xF3, 0x61, 0xFE, 0x01, 0x41
	.db	0x73, 0x01, 0x27, 0xC1, 0xC0, 0x84, 0x8F, 0xD6
	.db	0x01, 0x87, 0x70, 0x56, 0x4F, 0x19, 0x70, 0x1F
	.db	0xA8, 0xD9, 0x90, 0x76, 0x02, 0x17, 0x43, 0xFE
	.db	0x01, 0xC1, 0x84, 0x0B, 0x15, 0x7F, 0x02, 0x8B
	.db	0x14, 0x30, 0x8F, 0x63, 0x39, 0x6F, 0x19, 0xF0
	.db	0x11, 0xC9, 0x10, 0x6D, 0x02, 0x3F, 0x91, 0x09
	.db	0x7A, 0x41, 0xD0, 0xBA, 0x0C, 0x1D, 0x39, 0x5F
	.db	0x07, 0xF2, 0x11, 0x17, 0x20, 0x41, 0x6B, 0x35
	.db	0x09, 0xF7, 0x75, 0x12, 0x0B, 0xA7, 0xCC, 0x48
	.db	0x02, 0x3F, 0x64, 0x12, 0xA0, 0x0C, 0x27, 0xE3
	.db	0x9F, 0xC0, 0x14, 0x77, 0x70, 0x11, 0x40, 0x71
	.db	0x21, 0xC0, 0x68, 0x25, 0x41, 0xF0, 0x62, 0x7F
	.db	0xD1, 0xD0, 0x21, 0xE1, 0x62, 0x58, 0xB0, 0xF3
	.db	0x05, 0x1F, 0x73, 0x30, 0x77, 0xB1, 0x6F, 0x19
	.db	0xE0, 0x19, 0x43, 0xE0, 0x58, 0x2F, 0xF6, 0xA4
	.db	0x14, 0xD0, 0x23, 0x03, 0xFE, 0x31, 0xF5, 0x14
	.db	0x30, 0x99, 0xF8, 0x03, 0x3F, 0x64, 0x22, 0x51
	.db	0x60, 0x25, 0x41, 0x2F, 0xE3, 0x01, 0x56, 0x27
	.db	0x93, 0x09, 0xFE, 0x11, 0xFE, 0x79, 0xBA, 0x60
	.db	0x75, 0x42, 0xEA, 0x62, 0x58, 0xA0, 0xE5, 0x1F
	.db	0x53, 0x4F, 0xD1, 0xC0, 0xA3, 0x09, 0x42, 0x53
	.db	0xF7, 0x12, 0x04, 0x62, 0x1B, 0x30, 0xF5, 0x05
	.db	0xF7, 0x69, 0x0C, 0x35, 0x1B, 0x70, 0x82, 0x2F
	.db	0x2F, 0x14, 0x4F, 0x51, 0xC0, 0x64, 0x25, 0x00

;STR

logon1: .db	"Welcome",128,148,"2, by",31,248,31,254,13,14
logon2: .db	32,32,"See",148,"2.DOC,",148,"2.EQU",164
	.db	148,"2.HDR",180,213,141,".",14
abort:	.db	" ",31,158,31,160,"!",13,14
prompt1:.db	148,"2 Loc:",0
prompt2:.db	" >", 160	;must follow after prompt1
prompt3:.db	134,202,130,'(',0
prompt4:.db	"),",149,140,128,200,": ",0
prompt5:.db	31,151,130,195,"s",199,166,131,","
	.db	186," JUMP",128,134,161,"r",130,13,14
prompt6:.db	13,13,31,135,131,129,": ",0
prompt7:.db	31,228,251," key: ",0
prompt8:.db	13,13,31,136,128,131,129," (",0
prompt9:.db	13,13,31,130,31,253,0
prompt9b:.db	 31,129,32,32,32,32,32,31,201,14	;must follow prompt9
prompt10:.db	") ",31,135,31,178,": ",0
beg_str:.db	"First",31,129,": ",0
end_str:.db	"Last",31,129,":",32,32,0
sure:	.db	31,185,161," sure?",0
edits1: .db	13,13,31,156,154,146,",",140,128,200,14
edits2: .db	"  ",31,156,193,",",142,129,247,13,14
dnlds1: .db	13,13,31,159," ascii",249,150,31,152,132,137
	.db	",",149,140,128,160,13,14
dnlds2: .db	13,31,138,160,"ed",13,14
dnlds3: .db	13,31,138,193,"d",13,14
dnlds4: .db	"Summary:",14
dnlds5: .db	" ",198,"s",145,"d",14
dnlds6a:.db	" ",139,145,"d",14
dnlds6b:.db	" ",139," written",14
dnlds7: .db	31,155,":",14
dnlds8: .db	" ",139," unable",128," write",14
dnlds9: .db	32,32,"bad",245,"s",14
dnlds10:.db	" ",133,159,150,198,14
dnlds11:.db	" ",133,132,157,14
dnlds12:.db	" ",133," non",132,157,14
dnlds13:.db	31,151,155," detected",13,14
runs1:	.db	13,134,"ning",130,":",13,14
uplds3: .db	13,13,"Sending",31,152,132,137,172,32,32,0
uplds4: .db	" ",128,32,32,0		;must follow uplds3
help1txt:.db	13,13,"Standard",31,158,"s",14
help2txt:.db	31,218,31,244,"ed",31,158,"s",14
type1:	.db	31,154,158,0
type2:	.db	31,130,0
type4:	.db	31,143,31,226,31,170,0
type5:	.db	"???",0
help_cmd2:.db	31,215,0
help_cmd: .db	31,142,215,209,0	;these 11 _cmd string must be in order
dir_cmd:  .db	31,209,130,"s",0
run_cmd:  .db	31,134,130,0
dnld_cmd: .db	31,138,0
upld_cmd: .db	31,147,0
nloc_cmd: .db	31,135,129,0
jump_cmd: .db	31,136,128,131,129,0
dump_cmd: .db	31,132,219,154,131,0
intm_cmd: .db	31,132,219,192,131,0
edit_cmd: .db	31,156,154,146,0
clrm_cmd: .db	31,237,131,0
erfr_cmd: .db	31,203,153,144,0
erfr_ok:  .db	31,153,144,203,'d',13,14
erfr_err: .db	31,133,155,13,14


