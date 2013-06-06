; User installable disassembler and single-step run for paulmon2

; Please email comments, suggestions, bugs to paul@pjrc.com

; This code is in the public domain. It is distributed in
; the hope that it will be useful, but without any warranty;
; without even the implied warranty of merchantability or fitness
; for a particular purpose.

; For more information, please see

; http://www.pjrc.com/tech/8051/pm2_docs/index.html

.equ	locat, 0x1000		;location for these commands (usually 1000)
.equ	paulmon2, 0x0000	;location where paulmon2 is at (usually 0000)

.equ    phex1, 0x2E+paulmon2
.equ    cout, 0x30+paulmon2		;send acc to uart
.equ    phex, 0x34+paulmon2		;print acc in hex
.equ    phex16, 0x36+paulmon2		;print dptr in hex
.equ    pstr, 0x38+paulmon2		;print string @dptr
.equ    ghex, 0x3A+paulmon2		;get two-digit hex (acc)
.equ    ghex16, 0x3C+paulmon2		;get four-digit hex (dptr)
.equ	upper, 0x40+paulmon2		;convert acc to uppercase
.equ	newline, 0x48+paulmon2
.equ	pcstr, 0x45+paulmon2
.equ	pint, 0x50+paulmon2
.equ	smart_wr, 0x56+paulmon2
.equ	cin_filter, 0x62+paulmon2
.equ	asc2hex, 0x65+paulmon2


.equ    list_key, 'L'		;list (disassemble)
.equ    step_key, 'S'		;single step run
.equ    vtedit_key, 'E'           ;memory editor

;for testing in ram
;.equ    list_key, 'K'           ;list (disassemble)
;.equ    step_key, 'W'           ;single step run
;.equ    vtedit_key, 'E'           ;memory editor



;location of two bytes used by single-step in internal ram which we
;hope the user's program won't write over while it's running.  These
;two bytes hold the address of the previous instruction, so we can
;show the last executed instruction with the current value of the
;registers, which is much more intuitive than displaying the next
;instruction by grabbing the program counter from the stack as was
;done in PAULMON1's single step.

.equ    lastpc, 0x7C		;don't forget to update the docs below



;DIS
;---------------------------------------------------------;
;                                                         ;
;                      list command                       ;
;                                                         ;
;---------------------------------------------------------;


.org	locat
.db	0xA5,0xE5,0xE0,0xA5	;signiture
.db	254,list_key,0,0		;id (254=user installed command)
.db	0,0,0,0			;prompt code vector
.dB	0,0,0,0			;reserved
.db	0,0,0,0			;reserved
.db	0,0,0,0			;reserved
.db	0,0,0,0			;user defined
.db	255,255,255,255		;length and checksum (255=unused)
.db	"List",0

newline_h:ljmp	newline

.org	locat+64                ;executable code begins here

;	disassembler register usage
;	r0 = temporary storage
;	r1 = temporart storage
;	r2 = first instruction byte
;	r3 = second instruction byte
;	r4 = third instruction byte
;	r5 = line count
;	r6 = program counter (lsb)
;	r7 = program counter (msb)

list:	acall	newline_h
	mov	r5, #20
	clr	psw.1		;use ordinary long format
list2:	acall	disasm
	djnz	r5, list2
	ajmp	newline_h

disasm:
;print out the memory location and fetch the next three bytes
	mov	a, r7
	mov	dph, a
	acall	phex_h
	mov	a, r6
	mov	dpl, a
	acall	phex_h
        clr     a
        movc    a, @a+dptr
	mov	r2, a
	inc	dptr
	clr	a
	movc	a, @a+dptr
	mov	r3, a
	inc	dptr
	clr	a
	movc	a, @a+dptr
	mov	r4, a
        mov     a, r2
	anl	a, #00001000b
	jnz	lookup2

;fetch constants for instructions not using R0-R7
lookup1:
	mov	a, r2
	rr	a
	anl	a, #01111000b	;grab upper 4 bits
	mov	r0, a		;keep in r0 for a moment
	mov	a, r2
	anl	a, #00000111b	;get lower 3 bits
	orl	a, r0		;combine in upper 4
        mov     dptr, #opot1	;opot=operand offset table
        movc    a, @a+dptr
	sjmp    unpack

;fetch constants for R0-R7 instructions
lookup2:
        mov     a, r2
        swap    a
        anl     a, #00001111b
        mov     dptr, #opot2
        movc    a, @a+dptr

;now we'll unpack the operand code (# bytes and addr mode)
unpack:	anl     a, #00000011b
	mov	r0, a

;increment the r7/r6 pointer
        add     a, r6
        mov     r6, a
        mov     a, r7
        addc    a, #0
        mov     r7, a

;now print the bytes and spaces (r0 has # of bytes to print)
pbytes:	mov     a, #':'
        acall   cout_h
        acall   space_h
	jb	psw.1, pmnu		;skip bytes if running single-step
        mov     a, r2
        acall   phex_h
        acall   space_h
        cjne    r0, #1, pbytes2
        mov     r1, #11
        sjmp    pbytes4
pbytes2:mov     a, r3
        acall   phex_h
        acall   space_h
        cjne    r0, #2, pbytes3
        mov     r1, #8
        sjmp    pbytes4
pbytes3:mov     a, r4
        acall   phex_h
        mov     r1, #6
pbytes4:acall   space_h
        djnz    r1, pbytes4


;prints the mnunonic name and spaces
pmnu:   mov     a, r2
        anl     a, #00001000b
        jnz     pmnu_lookup2
pmnu_lookup1:
        mov     dptr, #mnot1    ;mnot=mnunonic offset table
        mov     a, r2
        rr      a
        anl     a, #01111000b   ;grab upper 4 bits
        mov     r0, a           ;keep in r0 for a moment
        mov     a, r2
        anl     a, #00000111b   ;get lower 3 bits
        orl     a, r0           ;combine in upper 4
        movc    a, @a+dptr
        mov     r1, a
	sjmp	pmnu0
pmnu_lookup2:
        mov     dptr, #mnot2    ;16 byte table for r0-r7 instructions
        mov     a, r2
        swap    a
        anl     a, #00001111b
        movc    a, @a+dptr
        mov     r1, a
pmnu0:	mov	dptr, #mnu_tbl
	mov     r0, #8
	clr	c
pmnu1:	mov	a, #' '
	jc	pmnu2
	mov	a, r1
	movc	a, @a+dptr
	inc	r1
	mov	c, acc.7
	anl	a, #0x7F
pmnu2:	acall	cout_h
	djnz	r0, pmnu1



;print the operands

        mov     a, #dasm2 & 255  ;(low)
        push    acc
        mov     a, #dasm2 >> 8   ;(high)
        push    acc

am_lookup0:
        mov     a, r2
        anl     a, #00001000b
        jnz     am_lookup2

;fetch constants for instructions not using R0-R7
am_lookup1:
        mov     a, r2
        rr      a
        anl     a, #01111000b   ;grab upper 4 bits
        mov     r0, a           ;keep in r0 for a moment
        mov     a, r2
        anl     a, #00000111b   ;get lower 3 bits
        orl     a, r0           ;combine in upper 4
        mov     dptr, #opot1    ;opot=operand offset table
        movc    a, @a+dptr
        sjmp    am_unpack

;fetch constants for R0-R7 instructions
am_lookup2:
        mov     a, r2
        swap    a
        anl     a, #00001111b
        mov     dptr, #opot2
        movc    a, @a+dptr

am_unpack:
        anl     a, #11111100b
        rr      a
        rr      a
        dec     a

	mov     dptr, #oprt      ;oprt=operand routine table
	rl	a
	add	a, dpl
	mov	dpl, a
	clr	a
	addc	a, dph
	mov	dph, a
	clr	a
        jmp     @a+dptr
dasm2:  
	ajmp	newline_h


oprt:   ajmp   opcd1           ;addr11
	ajmp   opcd2           ;A,Rn
	ajmp   opcd3           ;A,direct
	ajmp   opcd4           ;A,@Ri
	ajmp   opcd5           ;A,#data
	ajmp   opcd6           ;direct,A
	ajmp   opcd7           ;direct,#data
	ajmp   opcd8           ;C,bit
	ajmp   opcd9           ;C,/bit
	ajmp   opcd10          ;A,direct,rel
	ajmp   opcd11          ;A,#data,rel
	ajmp   opcd12          ;Rn,#data,rel
	ajmp   opcd13          ;@Ri,#data,rel
	ajmp   pa              ;A
	ajmp   prc             ;C
	ajmp   pbit            ;bit
	ajmp   pdirect         ;direct
	ajmp   p_reg_i         ;@Ri
	ajmp   opcd19          ;AB
	ajmp   opcd20          ;Rn,rel
	ajmp   opcd21          ;direct,rel
	ajmp   p_reg_n         ;Rn
	ajmp   pdptr           ;DPTR
	ajmp   opcd24          ;bit,rel
	ajmp   prel            ;rel
	ajmp   opcd26          ;@A+DPTR
	ajmp   opcd27          ;addr16
	ajmp   opcd28          ;Rn,A
	ajmp   opcd29          ;Rn,direct
	ajmp   opcd30          ;Rn,#data
	ajmp   opcd31          ;direct,Rn
	ajmp   opcd32          ;direct,direct
	ajmp   opcd33          ;direct,@Ri
	ajmp   opcd34          ;@Ri,A
	ajmp   opcd35          ;@Ri,direct
	ajmp   opcd36          ;@Ri,#data
	ajmp   opcd37          ;bit,C
	ajmp   opcd38          ;DPTR,#data16
	ajmp   opcd39          ;A,@A+DPTR
	ajmp   opcd40          ;A,@A+PC
	ajmp   opcd41          ;A,@DPTR
	ajmp   opcd42          ;@DPTR,A
	ret                    ; <nothing>



opcd4:                      ;A,@Ri              done
        acall   pac
p_reg_i:mov     a,#'@'
        acall   cout_h
        mov     a,#'R'
        acall   cout_h
        mov     a, r2
        anl     a,#00000001b
        ajmp    phex1_h

opcd3:                      ;A,direct           done
        acall   pac
pdirect:
        mov     a, r3
        jb      acc.7,pdir1
pdir0:  mov     a, r3
        ajmp    phex_h
pdir1:  mov     dptr,#sfrmnu
pdir2:  clr     a
        movc    a,@a+dptr
        inc     dptr
        jz      pdir0
        mov     r0,a
        clr     c
        subb    a, r3
        jnz     pdir3
pstr_h:
	ljmp	pstr

pdir3:  clr     a
        movc    a,@a+dptr
        inc     dptr
        jnb     acc.7,pdir3
        sjmp    pdir2

               
opcd9:                      ;C,/bit             done
        acall   prc
        acall   pcomma
        mov     a, #'/'
        acall   cout_h
pbit: 
        mov     a, r3
        anl     a,#01111000b
        rl      a
        swap    a
        mov     r0,a
        mov     a, r3
        anl     a,#10000000b
        jz      pbit1
        mov     dptr,#bitptr        ;it's a Special Function Reg.
        mov     a,r0
        movc    a,@a+dptr
        mov     dptr,#bitmnu
        addc    a,dpl
        mov     dpl,a
        jnc     pbit0
        inc     dph
pbit0:  acall   pstr_h
        sjmp    pbit2
pbit1:  mov     a,r0            ;it's between 20h and 2Fh
        add     a,#20h
        acall   phex_h
pbit2:  mov     a,#'.'
        acall   cout_h
        mov     a, r3
        anl     a,#00000111b
        ajmp    phex1_h


opcd10:                     ;A,direct,rel       done
        acall   pac
        acall   pdirect
opcd10a:acall   pcomma
        mov     a, r4
        mov     r3, a
prel:
        mov     a, r3
        add     a, r6
	mov	dpl, a
	mov	a, r3
	jb	acc.7, prel2
	clr	a
	sjmp	prel3
prel2:	clr	a
	cpl	a
prel3:	addc	a, r7
	mov	dph, a
	ljmp	phex16


pat:            ;prints the '@' symbol
        mov     a,#'@'
        ajmp    cout_h

pac:            ;print "A,"
        acall   pa
pcomma:         ;prints a comma
        mov     a,#','
        acall   cout_h
pspace: mov	a, #' '
	ajmp	cout_h

plb:            ;prints the '#' symbol
        mov     a,#'#'
        ajmp    cout_h


opcd6:                      ;direct,A           done
        acall   pdirect
        acall   pcomma
pa:             ;prints 'A'
        mov     a,#'A'
        ajmp    cout_h

opcd37:                     ;bit,C              done
        acall   pbit
        acall   pcomma
prc:             ;prints 'C'
        mov     a,#'C'
        ajmp    cout_h

opcd26:                     ;@A+DPTR            done
        acall   pat
        acall   pa
        mov     a,#'+'
        acall   cout_h
pdptr:          ;prints DPTR
	mov	a, #'D'
	acall	cout_h
	mov	a, #'P'
	acall	cout_h
	mov	a, #'T'
	acall	cout_h
	mov	a, #'R'
cout_h:
	ljmp	cout

opcd1:  mov     a, r7       ;addr11             done
        anl     a, #11111000b
        mov     r0, a
        mov     a, r2
        swap    a
        rr      a
        anl     a, #00000111b
        orl     a, r0
        acall   phex_h
        mov     a, r3
        ajmp    phex_h

opcd2:                      ;A,Rn               done
        acall   pac
p_reg_n:mov     a,#'R'
        acall   cout_h
        mov     a, r2
        anl     a,#00000111b
phex1_h:
	ljmp	phex1



opcd5:                      ;A,#data            done
        acall   pa
pdata:  acall   pcomma
        acall   plb
        mov     a, r3
phex_h:
	ljmp	phex

opcd7:                      ;direct,#data       done
        acall   pdirect
        mov     a, r4
        mov     r3, a
        ajmp    pdata
opcd8:                      ;C,bit              done
        acall   prc
        acall   pcomma
        ajmp    pbit

opcd11:                     ;A,#data,rel        done
        acall   pa
opcd11a:acall   pcomma
        acall   plb
        mov     a, r3
        acall   phex_h
        ajmp    opcd10a
opcd12:                     ;Rn,#data,rel       done
        acall   p_reg_n
        ajmp    opcd11a
opcd13:                     ;@Ri,#data,rel      done
        acall   p_reg_i
        ajmp    opcd11a
opcd19:                     ;AB                 done
        acall   pa
        mov     a, #'B'
        ajmp    cout_h
opcd20:                     ;Rn,rel             done
        acall   p_reg_n
        acall   pcomma
        ajmp    prel
opcd21:                     ;direct,rel         done
        acall   pdirect
        ajmp    opcd10a
opcd24:                     ;bit,rel            done
        acall   pbit
        ajmp    opcd10a
opcd28:                     ;Rn,A               done
        acall   p_reg_n
        acall   pcomma
        ajmp    pa
opcd29:                     ;Rn,direct          done
        acall   p_reg_n
        acall   pcomma          
        ajmp    pdirect
opcd30:                     ;Rn,#data           done
        acall   p_reg_n
        ajmp    pdata
opcd31:                     ;direct,Rn          done
        acall   pdirect
        acall   pcomma
        ajmp    p_reg_n
opcd32:                     ;direct,direct      done
        mov     a, r3
        push    acc
        mov     a, r4
        mov     r3, a
        acall   pdirect
        acall   pcomma
        pop     acc
        mov     r3, a
        ajmp    pdirect
opcd33:                     ;direct,@Ri         done
        acall   pdirect
        acall   pcomma
        ajmp    p_reg_i
opcd34:                     ;@Ri,A              done
        acall   p_reg_i
        acall   pcomma
        ajmp    pa
opcd35:                     ;@Ri,direct         done
        acall   p_reg_i
        acall   pcomma
        ajmp    pdirect
opcd36:                     ;@Ri,#data          done
        acall   p_reg_i
        ajmp    pdata
opcd38:                     ;DPTR,#data16       done
        acall   pdptr
        acall   pcomma
        acall   plb
opcd27: mov     a, r3  ;addr16          done
        acall   phex_h
        mov     a, r4
        ajmp    phex_h
opcd39:                     ;A,@A+DPTR          done
        acall   pac
        acall   pat
        acall   pa
        mov     a,#'+'
        acall   cout_h
        ajmp    pdptr
opcd40:                     ;A,@A+PC            done
        acall   pac
        acall   pat
        acall   pa
        mov     a,#'+'
        acall   cout_h
        mov     a,#'P'
        acall   cout_h
        ajmp    prc
opcd41:                     ;A,@DPTR            done
        acall   pac
        acall   pat
        ajmp    pdptr
opcd42:                     ;@DPTR,A            done
        acall   pat
        acall   pdptr
        acall   pcomma
        ajmp    pa





sfrmnu: .db     0xE0,"AC",'C'+128
        .db     0x81,'S','P'+128
        .db     0x82,"DP",'L'+128
        .db     0x83,"DP",'H'+128
        .db     0x80,'P','0'+128
        .db     0x90,'P','1'+128
        .db     0xA0,'P','2'+128
        .db     0xB0,'P','3'+128
        .db     0x99,"SBU",'F'+128
        .db     0xCD,"TH",'2'+128
        .db     0xC8,"T2CO",'N'+128
        .db     0xCC,"TL",'2'+128
        .db     0xCB,"RCAP2",'H'+128
        .db     0xCA,"RCAP2",'L'+128
        .db     0x8C,"TH",'0'+128
        .db     0x8A,"TL",'0'+128
        .db     0x8D,"TH",'1'+128
        .db     0x8B,"TL",'1'+128
sfr1:   .db     0xF0,'B'+128               ;5
sfr2:   .db     0xD0,"PS",'W'+128          ;7
sfr3:   .db     0xA8,'I','E'+128
sfr4:   .db     0xB8,'I','P'+128
sfr5:   .db     0x89,"TMO",'D'+128         ;8
sfr6:   .db     0x88,"TCO",'N'+128         ;8
sfr7:   .db     0x98,"SCO",'N'+128         ;8
sfr8:   .db     0x87,"PCO",'N'+128         ;8
        .db     0                       ;just in case


opot2:  .db     0x59, 0x59, 0x09, 0x09  ;inc, dec, add, addc
        .db     0x09, 0x09, 0x09, 0x7A  ;orl, anl, xrl, mov
        .db     0x7E, 0x09, 0x76, 0x33  ;mov, subb, mov, cjne
        .db     0x09, 0x52, 0x09, 0x71  ;xch, djnz, mov, mov

bitptr: .db     0x00, 0x02, 0x06, 0x08, 0x0C, 0x0E, 0x10, 0x12
        .db     0x14, 0x16, 0x1B, 0x1E, 0x20, 0x23, 0x24, 0x25


;some stuff used by single step... it's here to fill up some of
;the unused space from the end of the disassembler code and the
;beginning of the single-step header (which must begin on a 256
;byte page boundry)


wr_check:   ;write to memory and check that it worked.
	    ;acc=0 if it worked, nonzero if it didn't write
	mov	r0, a		;keep a copy of the data in r0
	movx	@dptr, a
	clr	a
	movc	a, @a+dptr
	clr	c
	subb	a, r0
	ret

;delay for approx 1 character transmit time
chardly:mov     r1, #80     
chdly2:	mov     a, th1 
        cpl     a     
        inc     a
        mov     r0, a
        djnz    r0, *
        djnz    r1, chdly2
	ret

prcolon:acall	phex_h
	mov	a, #':'
	ajmp	cout_h

phexsp:	acall	phex_h
space_h:
	mov	a, #' '
	ajmp	cout_h


;SINGLE
;---------------------------------------------------------;
;                                                         ;
;                 single step command                     ;
;                                                         ;
;---------------------------------------------------------;

.org    locat+0x400
.db     0xA5,0xE5,0xE0,0xA5     ;signiture
.db     254,step_key,0,0             ;id (254=user installed command)
.db     0,0,0,0                 ;prompt code vector
.dB     0,0,0,0                 ;reserved
.db     0,0,0,0                 ;reserved
.db     0,0,0,0                 ;reserved
.db     0,0,0,0                 ;user defined
.db     255,255,255,255         ;length and checksum (255=unused)
.db     "Single-Step",0
.org    locat+0x440             ;executable code begins here


        
ssrun:
	;first check to make sure they connect int1 low
	jnb	p3.3, ssrun2
        mov     dptr, #sserr1	;give error msg if int1 not grounded
pcstr_h:
	ljmp	pcstr

ssrun2:	;make sure there's a ljmp at the int1 vector location
	mov	dptr, #0x0013
	clr	a
	movc	a, @a+dptr
	add	a, #254
	jz	ssrun3
	mov	dptr, #sserr2	;give error that paulmon2 was not found.
	ajmp	pcstr_h
ssrun3:	;now write an ljmp to "step" in the ram and check it.
	inc	dptr
	movc	a, @a+dptr
	mov	r0, a
	clr	a
	inc	dptr
	movc	a, @a+dptr
	mov	dpl, a
	mov	dph, r0		;now data pointer points to int1 target
	mov	a, #2
	acall	wr_check
	jnz	ssrun4
	inc	dptr
	mov	a, #(step >> 8)
	acall	wr_check
	jnz	ssrun4
	inc	dptr
	mov	a, #(step & 255)
	acall	wr_check
	jz	ssrun5
ssrun4:	mov	r0, dpl
	mov	r1, dph
	mov	dptr, #sserr3	;error: couldn't write to memory @xxxx
	acall	pcstr_h
	mov	a, r1
	acall	phex_h
	mov	a, r0
	acall	phex_h
	ajmp	newline_h
ssrun5:	mov	a, ip		;set to high priority interrupt
	anl	a, #00000100b
	mov	ip, a
	;let's not beat around the bush (like paulmon1), all 
	;we need to know is where to jump into memory.
	mov	dptr, #prompt8
	acall	pcstr_h
	mov	a, r7
	acall	phex_h
	mov	a, r6
	acall	phex_h
        mov     dptr,#prompt4
        acall   pcstr_h
        lcall   ghex16		;ask for the jump location
	jb	psw.5, ssrun7
        jnc     ssrun6
        mov     dptr,#abort
        acall   pstr_h
        ajmp    newline_h
ssrun6:	mov	r6, dpl		;where we'll begin executing
	mov	r7, dph
ssrun7:	clr	tcon.2		;need low-level triggered int1
	mov     dptr,#ssmsg     ;tell 'em we're starting
	acall	pcstr_h
	mov	dptr,#ssnames
	acall	pstr_h
	clr	a
	mov	sp, #8		;just like after a reset
	push	acc		;unlike a 8051 start-up, push return addr
	push	acc		;of 0000, just in case they end w/ ret
	mov	dpl, r6		;load the program's address into dptr
	mov	dph, r7
	mov	psw, a		;and clear everything to zero
	mov	r0, a
	mov	r1, a
	mov	r2, a
	mov	r3, a
	mov	r4, a
	mov	r5, a
	mov	r6, a
	mov	r7, a
	mov	b, a
	mov	lastpc, #ssstart & 255
	mov	(lastpc+1), #ssstart >> 8
	setb	ie.2
	setb	ea		;turn on the interrupt
ssstart:jmp	@a+dptr


done:	acall	chardly
	pop	acc
	mov	r1, a
	pop	acc
	mov	r0, a
	pop	dpl
	pop	dph
	pop	psw
	pop	acc
	reti

step:    ;this is the single step interrupt service code...
	push	acc
	push	psw		;Stack Contents: (in this order)
	push	dph		;PC_L PC_H ACC PSW DPH DPL R0 R1
	push	dpl
	mov	a, r0
	push	acc
	mov	a, r1
	push	acc
	;in case the previous instruction was "clr ti", we
	;must wait for a character transmit time "in case it
	;was a move to SBUF) and then set ti so that our cout
	;doesn't hang when we transmit the first character!
	acall	chardly
	setb	ti

	;now print out a line that looks like this:
	;ACC B C DPTR  R0 R1 R2 R3 R4 R5 R6 R7  SP    PC  Instruction
	;00 00 0 3F00  00:00:00:00:00:00:00:00  00 - 0000: LJMP    0825

	acall	space_h
	acall	space_h
        mov     a, sp
        add     a, #251
        mov     r0, a           ;r0 points to user's acc on stack
        mov     a, @r0
        acall   phexsp          ;print acc
        mov     a, b
        acall   phexsp          ;print b register
	inc	r0
        mov     a, @r0
        rl      a
        anl     a, #1
        acall   phex1_h         ;print carry bit
        acall   space_h
	inc	r0
	mov	a, @r0
	acall	phex_h		;print dptr (msb)
	inc	r0
	mov	a, @r0
	acall	phexsp		;print dptr (lsb)
	acall	space_h
	inc	r0
	mov	a, @r0
	acall	prcolon		;print r0
	inc	r0
	mov	a, @r0
	acall	prcolon		;print r1
	mov	a, r2
	acall	prcolon		;print r2
	mov	a, r3
	acall	prcolon		;print r3
	mov	a, r4
	acall	prcolon		;print r4
	mov	a, r5
	acall	prcolon		;print r5
	mov	a, r6
	acall	prcolon		;print r6
	mov	a, r7
	acall	phexsp		;print r7
	acall	space_h
	mov	a, r0
	add	a, #248
	acall	phexsp		;print stack pointer
	acall	space_h
	acall	space_h
	;now the trick is to disassemble the instruction... this isn't
	;easy, since the user wants to see the last instruction that
	;just executed, but program counter on the stack points to the
	;next instruction to be executed.  The dirty trick is to grab
	;the program counter from last time where we stashed it in some
	;memory that hopefully the user's program hasn't overwritten.
	mov	a, lastpc
	mov	lastpc, r6
	mov	r6, a
	mov	a, (lastpc+1)
	mov	(lastpc+1), r7
	mov	r7, a
	mov	a, r2
	push	acc
	mov	a, r3
	push	acc
	mov	a, r4
	push	acc
	setb	psw.1		;tell it to use a compact format
	;the disassembler uses quite a bit of stack space... if the
	;user didn't leave enough room for the stack to grow with
	;all this overhead, it will likely crash somewhere in the
	;disassembler... oh well, not much I can do about it.  The
	;worst case stack usage for disasm is 9 bytes.  We just
	;pushed 5 and 6 at the beginning of step.  With the two
	;bytes for the interrupt, a total of 22 bytes of free stack
	;space must be available to use the single-step feature.
	acall	disasm
	pop	acc
	mov	r4, a
	pop	acc
	mov	r3, a
	pop	acc
	mov	r2, a
	mov	r7, (lastpc+1)
	mov	r6, lastpc
	;now grab the user's PC value to keep it for next time
        mov     a, sp
        add     a, #249
        mov     r0, a           ;r0 points to user's acc on stack
	mov	a, @r0
	mov	lastpc, a
	inc	r0
	mov	a, @r0
	mov	(lastpc+1), a

;SINGLE STEP

step1:  lcall   cin_filter
        lcall   upper
step2:  cjne    a, #13, step7
        ajmp    done
step7:  cjne    a, #' ', step8    ;check space
        ajmp    done
step8:  cjne    a,#'?',step10  ;check '?'
        mov     dptr,#help5txt
        acall   pcstr_h
        ajmp    step1
step10: cjne    a,#'Q',step11  ;check 'Q'=quit and run normal
        mov     dptr, #squit
        acall   pstr_h
        clr     ie.2
	acall	chardly
	mov	8, #0		;force return to 0000
	mov	9, #0
	mov	sp, #9
	reti
step11:
	cjne    a,#'H',step12  ;check 'H'=hex dump internal ram
        ajmp    ssdmp
step12: cjne    a,#'R',step13  ;check 'R'=print out registers
        ajmp    ssreg
step13: cjne    a,#'S',step14  ;check 'S'=skip next inst
        ajmp    ssskip
step14: cjne    a,#'A',step15  ;check 'A'=change acc value
        ajmp    sschacc
step15: cjne	a,#'.',step20
	mov	dptr, #ssnames
	acall	pstr_h
	ajmp	step1

step20: ajmp    step1
 
  
pequal:        ; prints '='
        mov     a,#'='
        ajmp    cout_h

ssdmp:
        mov     dptr, #ssdmps1
        acall   pstr_h
        clr     a
        acall   prcolon
        acall   space_h
	mov	r0, sp
	dec	r0
        mov     a, @r0
        acall   phexsp
	inc	r0
	mov	a, @r0
        acall   phex_h
        mov     r0, #2
        mov     r1, #14
        ajmp    ssdmp2
ssdmp1: mov     a, r0
	acall	prcolon
        mov     r1, #16
ssdmp2: acall   space_h
        mov     a, @r0
        acall   phex_h
        inc     r0
        djnz    r1, ssdmp2
        acall   newline_h
        cjne    r0, #0x80, ssdmp1
        acall   newline_h
	ajmp	step1


ssreg:
	mov	dptr, #sfr2+1
	acall	pstr_h
        acall   pequal
        mov     a, sp
	add	a, #252
	mov	r0, a
        mov     a, @r0
        acall   phexsp		;print psw
        mov     dptr,#sfr3+1
        mov     r0, 0xA8
        acall   psfr		;print ie
        mov     dptr,#sfr4+1
        mov     r0, 0xB8
        acall   psfr		;print ip
        mov     dptr,#sfr5+1
        mov     r0, 0x89
        acall   psfr		;print tmod
        mov     dptr,#sfr6+1
        mov     r0, 0x88
        acall   psfr		;print tcon
        mov     dptr,#sfr7+1
        mov     r0, 0x98
        acall   psfr		;print smod
        mov     dptr,#sfr8+1
        mov     r0, 0x87
        acall   psfr		;print pcon
        mov     a, #'T'
        acall   cout_h
        mov     a, #'0'
        acall   cout_h
        acall   pequal
        mov     a, 8Ch
        acall   phex_h		;print Timer 0
        mov     a, 8Ah
        acall   phex_h
        acall   space_h
        mov     a, #'T'
        acall   cout_h
        mov     a, #'1'
        acall   cout_h
        acall   pequal
        mov     a, 8Dh		;print Timer 1
        acall   phex_h
        mov     a, 8Bh
        acall   phex_h
        acall   newline_h
        ajmp    step1

psfr:   acall   pstr_h
        acall   pequal
        mov     a, r0
        ajmp    phexsp



;skip the next instruction
ssskip:
	mov	r0, #23
ssskip2:acall	space_h
	djnz	r0, ssskip2
        mov     dptr,#sskip1
        acall   pstr_h
	mov	a, sp
	add	a, #249
	mov	r0, a		;set r0 to point to pc on stack
	mov	a, @r0
	mov	lastpc, r6	;keep r6/r7 safe in lastpc
	mov	r6, a		;put user's pc into r6/r7
	inc	r0
	mov	a, @r0
	mov	(lastpc+1), r7
	mov	r7, a
	mov	a, r2
	push	acc
	mov	a, r3
	push	acc
	mov	a, r4
	push	acc
        setb    psw.1           ;tell it to use a compact format
        acall   disasm		;run disasm to show 'em what was skipped
	pop	acc
	mov	r4, a
	pop	acc
	mov	r3, a
	pop	acc
	mov	r2, a
        mov     a, sp
        add     a, #249
        mov     r0, a           ;set r0 to point to pc on stack
	mov	a, r6
        mov     r6, lastpc	;restore r6/r7
	mov	lastpc, a	;update lastpc with next inst addr
	mov	@r0, a		;also update user's pc!!
	inc	r0
	mov	a, r7
	mov	r7, (lastpc+1)
	mov	(lastpc+1), a
	mov	@r0, a
	ajmp	step1

sschacc:
        mov     a, sp
	add	a, #251
	mov	r0, a		;r0 points to acc on stack
        mov     dptr, #chaccs1
        acall   pstr_h
        lcall   ghex
        jc      chacc2
	jb	psw.5, chacc2
        mov     @r0, a
        acall   newline_h
        ajmp    step1
chacc2: mov     dptr, #abort
        acall   pstr_h
        ajmp    step1




;stuff some of the disassembler tables, strings, etc since we have a
;bit of space before the beginning of the editor command code


       ;opcode offset table (gives #bytes for the instruction
       ;and the number of the routine to print the operands)

opot1:  .db     0xAD, 0x06, 0x6F, 0x39, 0x39, 0x46, 0x49, 0x49 ;0
        .db     0x63, 0x06, 0x6F, 0x39, 0x39, 0x46, 0x49, 0x49 ;1
        .db     0x63, 0x06, 0xAD, 0x39, 0x16, 0x0E, 0x11, 0x11 ;2
        .db     0x63, 0x06, 0xAD, 0x39, 0x16, 0x0E, 0x11, 0x11 ;3
        .db     0x66, 0x06, 0x1A, 0x1F, 0x16, 0x0E, 0x11, 0x11 ;4
        .db     0x66, 0x06, 0x1A, 0x1F, 0x16, 0x0E, 0x11, 0x11 ;5
        .db     0x66, 0x06, 0x1A, 0x1F, 0x16, 0x0E, 0x11, 0x11 ;6
        .db     0x66, 0x06, 0x22, 0x69, 0x16, 0x1F, 0x92, 0x92 ;7
        .db     0x66, 0x06, 0x22, 0xA1, 0x4D, 0x83, 0x86, 0x86 ;8
        .db     0x9B, 0x06, 0x96, 0x9D, 0x16, 0x0E, 0x11, 0x11 ;9
        .db     0x26, 0x06, 0x22, 0x5D, 0x4D, 0xAD, 0x8E, 0x8E ;A
        .db     0x26, 0x06, 0x42, 0x3D, 0x2F, 0x2B, 0x37, 0x37 ;B
        .db     0x46, 0x06, 0x42, 0x3D, 0x39, 0x0E, 0x11, 0x11 ;C
        .db     0x46, 0x06, 0x42, 0x3D, 0x39, 0x57, 0x11, 0x11 ;D
        .db     0xA5, 0x06, 0x11, 0x11, 0x39, 0x0E, 0x11, 0x11 ;E
        .db     0xA9, 0x06, 0x89, 0x89, 0x39, 0x1A, 0x89, 0x89 ;F

mnot1:  ;mnunonic offset table (gives offset into above table)

        .db     0x5A, 0x0E, 0x48, 0x73  ;nop, ajmp, ljmp, rr
        .db     0x2B, 0x2B, 0x2B, 0x2B  ;inc, inc, inc, inc
        .db     0x30, 0x00, 0x43, 0x75  ;jbc, acall, lcall rrc
        .db     0x21, 0x21, 0x21, 0x21  ;

        .db     0x2E, 0x0E, 0x67, 0x6E  ; etc...
        .db     0x06, 0x06, 0x06, 0x06  ;
        .db     0x38, 0x00, 0x6A, 0x70  ;
        .db     0x0A, 0x0A, 0x0A, 0x0A  ;

        .db     0x33, 0x0E, 0x5D, 0x5D  ;
        .db     0x5D, 0x5D, 0x5D, 0x5D  ;
        .db     0x3B, 0x00, 0x12, 0x12  ;
        .db     0x12, 0x12, 0x12, 0x12  ;

        .db     0x41, 0x0E, 0x8F, 0x8F  ;
        .db     0x8F, 0x8F, 0x8F, 0x8F  ;
        .db     0x3E, 0x00, 0x5D, 0x35  ;
        .db     0x4C, 0x4C, 0x4C, 0x4C  ;

        .db     0x7C, 0x0E, 0x12, 0x4F  ;
        .db     0x24, 0x4C, 0x4C, 0x4C  ;
        .db     0x4C, 0x00, 0x4C, 0x4F  ;
        .db     0x80, 0x80, 0x80, 0x80  ;

        .db     0x5D, 0x0E, 0x4C, 0x2B  ;
        .db     0x57, 0x92, 0x4C, 0x4C  ;
        .db     0x12, 0x00, 0x1C, 0x1C  ;
        .db     0x15, 0x15, 0x15, 0x15  ;

        .db     0x63, 0x0E, 0x19, 0x19  ;
        .db     0x84, 0x88, 0x88, 0x88  ;
        .db     0x60, 0x00, 0x78, 0x78  ;
        .db     0x1F, 0x27, 0x8B, 0x8B  ;

        .db     0x53, 0x0E, 0x53, 0x53  ;
        .db     0x19, 0x4C, 0x4C, 0x4C  ;
        .db     0x53, 0x00, 0x53, 0x53  ;
        .db     0x1C, 0x4C, 0x4C, 0x4C  ;


mnot2:  .db     0x2B, 0x21, 0x06, 0x0A  ;inc, dec, add, addc
        .db     0x5D, 0x12, 0x8F, 0x4C  ;orl, anl, xlr, mov
        .db     0x4C, 0x80, 0x4C, 0x15  ;mov, subb, mov, cjne
        .db     0x88, 0x27, 0x4C, 0x4C  ;xch, djnz, mov, mov


;---------------------------------------------------------;
;                                                         ;
;                  External Memory Editor                 ;
;                                                         ;
;---------------------------------------------------------;

;register usage:
; R4,    Flags:
;         bit0: 0=display CODE memory, 1=display DATA memory
;         bit1: 0=editing disabled, 1=editing enabled
;         bit2: 0=editing in hex, 1=editing in ascii
;         bit3: 0=normal, 1=in middle of hex entry (value in r5)
; R6/R7, current memory location
;

.org	locat+0x800
.db	0xA5,0xE5,0xE0,0xA5	;signiture
.db	254,vtedit_key,0,0	;id (254=user installed command)
.db	0,0,0,0			;prompt code vector
.dB	0,0,0,0			;reserved
.db	0,0,0,0			;reserved
.db	0,0,0,0			;reserved
.db	0,0,0,0			;user defined
.db	255,255,255,255		;length and checksum (255=unused)
.db	"Memory Editor (VT100)",0

.org	locat+0x0840            ;executable code begins here


	mov	r4, #0
	acall	redraw
main:
	mov	a, r4
	clr	acc.3
	mov	r4, a
main2:	lcall   cin_filter
	acall	input_ck_2nd
cmd1:	cjne	a, #27, cmd2		;quit
	ajmp	quit
cmd2:	cjne	a, #11, cmd3		;up
	ajmp	cmd_up
cmd3:	cjne	a, #10, cmd4		;down
	ajmp	cmd_down
cmd4:	cjne	a, #8, cmd5		;left
	ajmp	cmd_left
cmd5:	cjne	a, #21, cmd6		;right
	ajmp	cmd_right
cmd6:	cjne	a, #12, cmd7		;redraw
	acall	redraw
	ajmp	main
cmd7:	cjne	a, #17, cmd8		;quit
	ajmp	quit
cmd8:	cjne	a, #3, cmd9		;code memory
	mov	a, r4
	anl	a, #11111110b
	mov	r4, a
	acall	cursor_home
	mov	dptr, #str_code
	acall	pstr_hh
	acall	redraw_data
	ajmp	main
cmd9:	cjne	a, #4, cmd10		;data memory
	mov	a, r4
	orl	a, #00000001b
	mov	r4, a
	acall	cursor_home
	mov	dptr, #str_data
	acall	pstr_hh
	acall	redraw_data
	ajmp	main
cmd10:	cjne	a, #7, cmd11		;goto memory loc
	ajmp	cmd_goto
cmd11:	cjne	a, #5, cmd12		;toggle editing
	ajmp	cmd_edit
cmd12:	cjne	a, #6, cmd13		;fill memory
	ajmp	cmd_fill
cmd13:	cjne	a, #1, cmd14		;edit in ascii
	mov	a, r4
	jnb	acc.1, main
	setb	acc.2
	mov	r4, a
	acall	erase_commands
	acall	print_commands
	acall	redraw_cursor
	ajmp	main
cmd14:	cjne	a, #24, cmd15		;edit in hex
        mov     a, r4
        jnb     acc.1, main
        clr     acc.2
        mov     r4, a
        acall   erase_commands
        acall   print_commands
        acall   redraw_cursor
        ajmp    main
cmd15:	cjne    a, #25, cmd16           ;page up
	ajmp	cmd_pgup
cmd16:	cjne    a, #26, cmd17           ;page down
	ajmp	cmd_pgdown
cmd17:


cmd_data:	;the input character wasn't any particular command, so
		;maybe it's some input data being typed for edit mode
	mov	b, a			;keep a copy of user data in b
	mov	a, r4
	jb	acc.1, cmd_data2
cmd_abort:
	ajmp	main			;ignore if not in edit mode
cmd_data2:
	jnb	acc.2, input_hex
input_ascii:
	mov	a, b
	acall	ascii_only
	cjne	a, b, cmd_abort		;check that input is an ascii char
	mov	dph, r7
	mov	dpl, r6
	lcall	smart_wr		;write the char to memory
	ajmp	cmd_right

input_hex:
	mov	a, b
	lcall	upper
	lcall	asc2hex
	jc	cmd_abort		;ignore if not hex
	mov	r0, a			;keep hex value of input in r0
        mov     dph, r7			;load dptr with address
        mov     dpl, r6
	mov	a, r4
	jb	acc.3, input_hex_2nd
input_hex_1st:
	mov	a, r0
	mov	r5, a
	mov	a, r4
	setb	acc.3		;remember that we're waiting for 2nd char
	mov	r4, a
	acall	redraw_cursor
	ajmp	main2
input_hex_2nd:
	mov	a, r5			;get data from memory
	swap	a			;shift nibbles
	anl	a, #11110000b		;just in case
	add	a, r0			;add in this input to lower part
	lcall	smart_wr		;write back to memory
	mov	a, r4
	clr	acc.3
	mov	r4, a
	ajmp	cmd_right

input_ck_2nd:
	;the main input routine will always call here when a new
	;byte is entered... so we can do something special if we
	;were waiting for the second character and it is not a
	;legal hex character
	push	acc
	mov	a, r4
	jb	acc.1, inck2d
	;if editing is disabled, don't do anything
	clr	acc.3
inck2b:	mov	r4, a
inck2c:	pop	acc
	ret
inck2d:	jnb	acc.3, inck2b
	;if we get here, we were actually waiting for the 2nd char
	pop	acc
	push	acc
	lcall	upper
	lcall	asc2hex
	jnc	inck2c		;proceed normally if it is valid
	;if we get here, we did not get a hex legal char
	pop	acc
	push	acc
	cjne	a, #esc_char, inck2e
        mov     a, r4
        clr     acc.3
        mov     r4, a
        acall   redraw_cursor
	pop	acc
	pop	acc		;don't return and do the quit cmd
	pop	acc		;just quit this entry and wait for next cmd
	ajmp	main
inck2e: mov     dph, r7                 ;load dptr with address
        mov     dpl, r6
	mov	a, r5
        lcall   smart_wr                ;write to memory
	mov	a, r4
	clr	acc.3
	mov	r4, a
	acall	redraw_cursor
	sjmp	inck2c

; R4,    Flags:
;         bit0: 0=display CODE memory, 1=display DATA memory
;         bit1: 0=editing disabled, 1=editing enabled
;         bit2: 0=editing in hex, 1=editing in ascii
;         bit3: 0=normal, 1=in middle of hex entry (value in r5)


cmd_fill:
	mov	a, r4
	anl	a, #00000010b
	jnz	cmd_fill_ok
	ajmp	main			;don't allow if not in editing mode
cmd_fill_ok:
	acall	erase_commands
        mov     a, r4
        push    acc
	mov	dptr, #fill_prompt1
	acall	pcstr_hh
	lcall	ghex16
	jc	cmd_fill_abort
	jb	psw.5, cmd_fill_abort
	mov	r0, dpl
	mov	r1, dph
	mov	dptr, #fill_prompt2
	acall	pstr_hh
	lcall	ghex16
	jc	cmd_fill_abort
	jb	psw.5, cmd_fill_abort
	mov	r4, dpl
	mov	r5, dph
	mov	dptr, #fill_prompt3
	acall	pcstr_hh
	lcall	ghex
	jc	cmd_fill_abort
	jb	psw.5, cmd_fill_abort
	mov	r2, a
	mov	a, r4
	mov	r6, a
	mov	a, r5
	mov	r7, a
	pop	acc
	mov	r4, a
	mov	dpl, r0
	mov	dph, r1
	;now r4 is restored to its normal value, dptr holds the
	;first location to fill, and r6/r7 holds the last location to
	;fill, and r2 has the fill value.
cmd_fill_loop:
	mov	a, r2
	lcall	smart_wr
	mov	a, r6
	cjne	a, dpl, cmd_fill_next
	mov	a, r7
	cjne	a, dph, cmd_fill_next
	;when we get here, we're done!
	acall	erase_commands
	acall	print_commands
	acall	redraw_data
	ajmp	main
cmd_fill_next:
	inc	dptr
	sjmp	cmd_fill_loop

cmd_fill_abort:
        pop     acc
        mov     r4, a
        acall   erase_commands
        acall   print_commands
	acall	redraw_cursor
	ajmp	main

fill_prompt1:
        .db     "Fill",31,131,"; First: ",0
fill_prompt2:
        .db     "  Last: ",0
fill_prompt3:
        .db     " ",168,": ",0 

cmd_edit:
	acall	erase_commands
	mov	a, r4
	xrl	a, #00000010b
	mov	r4, a
        acall   print_commands
        acall   redraw_cursor
        ajmp    main

cmd_goto:
	acall	erase_commands
	mov	dptr, #goto_prompt
	acall	pcstr_hh
	mov	a, r4
	push	acc
	lcall	ghex16
	pop	acc
	mov	r4, a
	jc	cmdgt_abort
	jb	psw.5, cmdgt_abort
        mov     r6, dpl
        mov     r7, dph
        acall   cursor_home
        mov     a, #20
        acall   cursor_down
	acall	print_commands
	acall	redraw_data
	ajmp	main
cmdgt_abort:
        acall   cursor_home
        mov     a, #20
        acall   cursor_down
	acall	print_commands
	acall	redraw_cursor
	ajmp	main


goto_prompt:
	.db	31,131,31,129,": ",0

cmd_up:
	acall	blank_it
        mov     a, r6
        clr     c
        subb    a, #16
        mov     r6, a
        mov     a, r7
        subb    a, #0
        mov     r7, a
	mov	a, r6
	cpl	a
	anl	a, #11110000b
	jz	cmd_up_scroll
        mov     a, #1
        acall   cursor_up
        acall   invert_it
        ajmp    main
cmd_up_scroll:
	acall	redraw_data
	ajmp	main

cmd_pgup:
	dec	r7
	acall	redraw_data
	ajmp	main

cmd_pgdown:
	inc	r7
	acall	redraw_data
	ajmp	main

cmd_down:
	acall	blank_it
        mov     a, r6
        add     a, #16
        mov     r6, a
        mov     a, r7
        addc    a, #0
        mov     r7, a
	mov	a, r6
	anl	a, #11110000b
	jz	cmd_down_scroll
	mov	a, #1
	acall	cursor_down
	acall	invert_it
	ajmp	main
cmd_down_scroll:
	acall	redraw_data
	ajmp	main


cmd_left:
	acall	blank_it
	mov	a, #3
	acall	cursor_left
	mov	a, r6
	clr	c
	subb	a, #1
	mov	r6, a
	mov	a, r7
	subb	a, #0
	mov	r7, a
	mov	a, r6
	orl	a, #11110000b
	cpl	a
	jz	cmdlf2
	acall	invert_it
	ajmp	main
cmdlf2:
	mov	a, r6
	cpl	a
	anl	a, #11110000b
	jz	cmd_left_scroll
	mov	a, #48
	acall	cursor_right
	mov	a, #1
	acall	cursor_up
	acall	invert_it
	ajmp	main
cmd_left_scroll:
	acall	redraw_data
	ajmp	main


cmd_right:
	acall	blank_it
	mov	a, #3
	acall	cursor_right
	mov	dpl, r6
	mov	dph, r7
	inc	dptr
	mov	r6, dpl
	mov	r7, dph
	mov	a, r6
	anl	a, #00001111b
	jz	cmdrt2
	acall	invert_it
	ajmp	main

cmdrt2:
	mov	a, r6
	anl	a, #11110000b
	jz	cmd_right_scroll
	mov	a, #48
	acall	cursor_left
	mov	a, #1
	acall	cursor_down
	acall	invert_it
	ajmp	main
cmd_right_scroll:
	acall	redraw_data
	ajmp	main


space:	mov	a, #' '
	ajmp	cout_hh



;register usage:
; R4,    Flags:
;         bit0: 0=display CODE memory, 1=display DATA memory
;         bit1: 0=editing disabled, 1=editing enabled
;         bit2: 0=editing in hex, 1=editing in ascii
;	  bit3: 0=normal, 1=in middle of hex entry (value in r5)
; R6/R7, current memory location
;


redraw:
        mov     dptr, #str_cl		;clear screen
        acall   pstr_hh
	acall	print_title_line
	acall	newline_hh
	acall	print_addr_line
	acall	newline_hh
	acall	print_dash_line
        acall   newline_hh
	mov	a, #16
	acall	cursor_down
        acall   print_dash_line
        acall   newline_hh
        acall   print_commands
redraw_data:
	acall	cursor_home
	mov	a, #2
	acall	cursor_down
	;compute first byte address to display on the screen
	mov	dpl, #0
	mov	dph, r7
	;now display the data
	mov	r0, #16
rd2:	acall	newline_hh
	lcall	phex16
	mov	a, #':'
	acall	cout_hh
	mov	r2, dpl
	mov	r3, dph
rd3:	acall	space
	acall	read_dptr
	acall	phex_hh
	inc	dptr
	mov	a, dpl
	anl	a, #00001111b
	jnz	rd3
	mov	dpl, r2
	mov	dph, r3
	acall	space
	acall	space
	acall	space
rd4:	acall	read_dptr
	acall	ascii_only
	acall	cout_hh
	inc	dptr
	mov	a, dpl
	anl	a, #00001111b
	jnz	rd4
	djnz	r0, rd2

redraw_cursor:
	acall	cursor_home
	mov	a, r6
	swap	a
	anl	a, #00001111b
	add	a, #3
	acall	cursor_down
	;make the ascii character inverse
	mov	a, r6
	anl	a, #00001111b
	add	a, #56
	acall	cursor_right
	acall	inverse_on
	acall	read_r6r7
	acall	ascii_only
	acall	cout_hh
	acall	inverse_off

	;now make the hex value inverse
	mov	a, r6
	anl	a, #00001111b
	rl	a
	cpl	a
	add	a, #52
	acall	cursor_left
	acall	inverse_on
	acall	read_r6r7
	acall	phex_hh
	ajmp	inverse_off


blank_it:
	mov	a, r6
	anl	a, #00001111b
	rl	a
	cpl	a
	add	a, #49
	acall	cursor_right
	acall	read_r6r7
	acall	ascii_only
	acall	cout_hh
	mov	a, r6
	anl	a, #00001111b
	rl	a
	cpl	a
	add	a, #52
	acall	cursor_left
	acall	read_r6r7
	ajmp	phex_hh

invert_it:
        mov     a, r6
        anl     a, #00001111b
        rl      a
        cpl     a
        add     a, #49
        acall   cursor_right
        acall   read_r6r7
        acall   ascii_only
	acall	inverse_on
        acall   cout_hh
	acall	inverse_off
        mov     a, r6
        anl     a, #00001111b
        rl      a
        cpl     a
        add     a, #52
        acall   cursor_left
        acall   read_r6r7
	acall	inverse_on
        acall   phex_hh
	ajmp	inverse_off






quit:	mov	a, r6
	anl	a, #11110000b
	swap	a
	cpl	a
	add	a, #19
	acall	cursor_down
	ajmp	newline_hh


ascii_only:
        anl     a, #01111111b   ;avoid unprintable characters
	cjne	a, #127, aonly2
	mov	a, #' '
	ret
aonly2: clr     c
        subb    a, #32
        jnc     aonly3          ;avoid control characters
        mov     a, #(' ' - 32)
aonly3: add     a, #32
	ret





read_dptr:
	mov	a, r4
	jb	acc.0, rddptr2
	clr	a
	movc	a, @a+dptr
	ret
rddptr2:movx	a, @dptr
	ret

read_r6r7:
	push	dph
	push	dpl
	mov	dph, r7
	mov	dpl, r6
	mov	a, r4
	jb	acc.3, rdr6r7d
	jb	acc.0, rdr6r7b
	clr	a
	movc	a, @a+dptr
	sjmp	rdr6r7c
rdr6r7b:movx	a, @dptr
rdr6r7c:pop	dpl
	pop	dph
	ret
rdr6r7d:mov	a, r5
	sjmp	rdr6r7c


.equ	esc_char, 27

cursor_home:
	acall	term_esc
	mov	a, #'H'
	ajmp	cout_hh

cursor_down:	;acc is # of lines to move down
	acall	term_esc
	acall	pint_hh
	mov	a, #'B'
	ajmp	cout_hh

cursor_up:	;acc is # of lines to move up
	acall	term_esc
	acall	pint_hh
	mov	a, #'A'
	ajmp	cout_hh

cursor_left:	;acc is # of characters to move left
	acall	term_esc
	acall	pint_hh
	mov	a, #'D'
	ajmp	cout_hh

cursor_right:	;acc is # of characters to move right
	acall	term_esc
	acall	pint_hh
	mov	a, #'C'
	ajmp	cout_hh

inverse_on:
	mov	dptr, #str_so
	ajmp	pstr_hh

str_so:	.db	esc_char, "[0;7m", 0

inverse_off:
	mov	dptr, #str_se
	ajmp	pstr_hh

str_se:	.db	esc_char, "[0m", 0


term_esc:
	push	acc
	mov	a, #esc_char
	acall	cout_hh
	mov	a, #'['
	acall	cout_hh
	pop	acc
	ret
	
print_addr_line:
	mov	dptr, #str_addr
	acall	pstr_hh
	mov	r0, #0
paddrl: acall	space
	mov	a, #'+'
	acall	cout_hh
	mov	a, r0
	lcall	phex1
	inc	r0
	cjne	r0, #16, paddrl
	mov	dptr, #str_ascii_equiv
	ajmp	pstr_hh


print_dash_line:
	mov	r0, #72
pdashl: mov	a, #'-'
	acall	cout_hh
	djnz	r0, pdashl
	ret

print_title_line:
	mov	a, r4
	jb	acc.0, ptitle2
	mov	dptr, #str_code
	sjmp	ptitle3
ptitle2:mov	dptr, #str_data
ptitle3:acall	pstr_hh
	mov	r0, #8
ptitlel:acall	space
	djnz	r0, ptitlel
	mov	dptr, #str_title
	ajmp	pcstr_hh

erase_commands:
        acall   cursor_home
        mov     a, #20
        acall   cursor_down
        mov     r2, #72
ercmd2: acall   space
        djnz    r2, ercmd2
	mov	a, #72
	ajmp	cursor_left


; R4,    Flags:
;         bit0: 0=display CODE memory, 1=display DATA memory
;         bit1: 0=editing disabled, 1=editing enabled
;         bit2: 0=editing in hex, 1=editing in ascii
;         bit3: 0=normal, 1=in middle of hex entry (value in r5)

print_commands:
        mov     a, r4
        jnb     acc.1, pcmd_no_edit
	mov	dptr, #str_cmd3
	jb	acc.2, pcmd_ascii
	mov	dptr, #str_cmd4
pcmd_ascii:
	acall	pstr_hh
	mov	dptr, #str_cmd5
	acall	pstr_hh
	sjmp	pcmd_finish
pcmd_no_edit:
	mov     dptr, #str_cmd2
	acall   pstr_hh
pcmd_finish:
        mov     dptr, #str_cmd1
        ajmp    pstr_hh

str_cmd1: .db "  ^G=Goto  ^C=Code  ^D=Data  ^L=Redraw  ^Q=Quit", 0
str_cmd2: .db "^E-Edit",0
str_cmd3: .db "^A=", esc_char, "[0;7m", "ASCII", esc_char, "[0m", "  ^X=Hex", 0
str_cmd4: .db "^A=ASCII  ^X=", esc_char, "[0;7m", "Hex", esc_char, "[0m", 0
str_cmd5: .db "  ^F=Fill",0


str_cl:	.db	esc_char, "[H", esc_char, "[2J", 0

str_addr: .db "ADDR:",0
str_ascii_equiv: .db	"   ASCII EQUIVILANT",0
str_title: .db	"8051",31,154,31,131,31,216,"or,",31,248,31,254,", 1996",0
str_code: .db "CODE",0
str_data: .db "DATA",0


cout_hh:ljmp	cout
phex_hh:ljmp	phex
pstr_hh:ljmp	pstr
newline_hh:ljmp	newline
pcstr_hh:ljmp	pcstr
pint_hh:ljmp	pint



;---------------------------------------------------------;
;                                                         ;
;                    single step strings                  ;
;                                                         ;
;---------------------------------------------------------;


           
prompt4:.db     "), or <ESC> to exit: ",0 
prompt8:.db     13,31,136,128,131,129," (",0 
abort:  .db     " Command Aborted.",13,10,0


sserr1: .db     13,161,197," connect INT1 (pin 13) low"
        .db     128,186,207,204,13,0
sserr2:	.db	148,"2",179,199,174,129," 0013",13,0
sserr3:	.db	31,184,179,255,165," vector",174," ",0
ssmsg:  .db     13,"Now",134,"ning",166,207,204," mode:  "
        .db     "<RET>=",204,", ?= Help",13,13,0

sskip1: .db     "Skipping Instruction-> ",0
ssdmps1:.db     13,10,"Loc:  Int RAM Memory Contents",13,10,0
chaccs1:.db     "New Acc Value: ",0

help5txt:.db	13
        .db     31,207,31,204,31,158,":",13
        .db     "<RET> ",134,212,246,13
        .db     " <SP> ",134,212,246,13
        .db     " '?'  ",255,142,215,13
	.db	" '.'  ",255,196,253,"s",13
        .db     " 'R'  ",255," special function",196,"s",13
        .db     " 'H'  ",132,219,192,146,13
        .db     " 'S'  ",252,212,246,13
        .db     " 'A'  ",240,162," Acc value",13
	.db     " 'Q'  ",200,207,204,13,14

squit:	.db	"Quit",13,10,0

ssnames:.db	"  ACC B C DPTR  R0 R1 R2 R3 R4 R5 R6 R7  SP"
	.db	"   Addr  Instruction",13,10,0


;---------------------------------------------------------;
;                                                         ;
;                    disassembler data                    ;
;                                                         ;
;---------------------------------------------------------;



mnu_tbl:.db     "ACAL",'L'+128
        .db     0
        .db     "AD",'D'+128
        .db     0
        .db     "ADD",'C'+128
        .db     "AJM",'P'+128
        .db     "AN",'L'+128
        .db     "CJN",'E'+128
        .db     "CL",'R'+128
        .db     "CP",'L'+128
        .db     "D",'A'+128 
        .db     "DE",'C'+128
        .db     "DI",'V'+128
        .db     "DJN",'Z'+128
        .db     "IN",'C'+128
        .db     "J",'B'+128
        .db     "JB",'C'+128
        .db     "J",'C'+128
        .db     "JM",'P'+128
        .db     "JN",'B'+128
        .db     "JN",'C'+128
        .db     "JN",'Z'+128
        .db     "J",'Z'+128
        .db     "LCAL",'L'+128
        .db     "LJM",'P'+128
        .db     "MO",'V'+128
        .db     "MOV",'C'+128
        .db     "MOV",'X'+128
        .db     "MU",'L'+128
        .db     "NO",'P'+128
        .db     "OR",'L'+128
        .db     "PO",'P'+128
        .db     "PUS",'H'+128
        .db     "RE",'T'+128
        .db     "RET",'I'+128
        .db     "R",'L'+128
        .db     "RL",'C'+128
        .db     "R",'R'+128
        .db     "RR",'C'+128
        .db     "SET",'B'+128
        .db     "SJM",'P'+128
        .db     "SUB",'B'+128
        .db     "SWA",'P'+128
        .db     "XC",'H'+128
        .db     "XCH",'D'+128
        .db     "XR",'L'+128
        .db     "??",'?'+128



bitmnu: .db     'P','0'+128
        .db     "TCO",'N'+128
        .db     'P','1'+128
        .db     "SCO",'N'+128
        .db     'P','2'+128
        .db     'I','E'+128
        .db     'P','3'+128
        .db     'I','P'+128
        .db     'C','0'+128
        .db     "T2CO",'N'+128
        .db     "PS",'W'+128
        .db     'D','8'+128
        .db     "AC",'C'+128
        .db     'E'+'8'+128
        .db     'B'+128
        .db     'F'+'8'+128


