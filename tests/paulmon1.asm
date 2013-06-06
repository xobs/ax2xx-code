;  PAULMON 8051 Debugger by Paul Stoffregen
;   Please distribute freely -- may not be sold, period.

;        .command +h58   ;set page height to 58 in listing file...

        .equ    start,0000h    ;address for start of EPROM (0000h)
        .equ    program,2000h  ;address for program loading location

        .ORG    start        
rst:    lJMP    poweron

        .org    start+3       ;ext int #0
        LJMP    program+3
        .org    start+11      ;timer #0
        LJMP    program+11
        .org    start+13h     ;external interrupt routine #1
        jnb     tcon.2,intr0
        ljmp    program+13h     ;don't do ssrun if edge trigger'd
intr0:  ajmp    step            ;but do ssrun if level trigger'd
        .org    start+1bh     ;timer #1
        ljmp    program+1bh   
        .org    start+23h     ;serial port
        ljmp    program+23h   
        .org    start+2bh     ;timer #2 (8052 only)
        ljmp    program+2bh


        .org    start+30h     ;the jump table
        ajmp    cout
        ajmp    cin
        ajmp    phex
        ajmp    phex16
        ajmp    pstr
        ajmp    ghex
        ajmp    ghex16
        ajmp    esc
        ajmp    upper
        ljmp    init

step:    ;this is the single step interrupt processor code...
        push    psw     ;better save these while we still can
        push    acc
        clr     psw.3   ;gotta be set to bank zero...
        clr     psw.4
step1:  acall   cin
        acall   upper
step2:  cjne    a,#13,step7
        ajmp    status
step7:  cjne    a,#32,step8    ;check space
        ajmp    done
step8:  cjne    a,#'?',step10  ;check '?'
        acall   sshelp
        ajmp    step1
step10: cjne    a,#'Q',step11  ;check 'Q'=quit and run normal
        push    dpl
        push    dph
        mov     dptr,#squit
        acall   pstr
        pop     dph
        pop     dpl
        clr     ie.2
        ajmp    done
step11: cjne    a,#'H',step12  ;check 'H'=hex dump internal ram
        acall   ssdmp
        ajmp    step1
step12: cjne    a,#'R',step13  ;check 'R'=print out registers
        ajmp    ssreg
step13: cjne    a,#'S',step14  ;check 'S'=skip this inst
        ajmp    skip0
step14: cjne    a,#'A',step20  ;check 'A'=change acc value
        ajmp    chacc
        
step20: ajmp    step1
   
pequal:        ; prints '='
        mov     a,#'='
        acall   cout
        ret

status:         ;prints two-line status during single step run
        mov     a,r0
        push    acc
        acall   space
        mov     a,#'S'
        acall   cout
        mov     a,#'P'
        acall   cout
        acall   pequal
        mov     r0,sp
        push    b
        acall   phex
        dec     r0
        acall   space
        lcall   pa
        acall   pequal
        mov     a,@r0
        acall   phex
        dec     r0
        acall   space
        lcall   prc
        acall   pequal
        mov     a,@r0
        mov     c,acc.7
        clr     a
        rlc     a
        acall   phex1
        acall   space
        lcall   pdptr
        acall   pequal
        mov     a,dph
        acall   phex
        mov     a,dpl
        acall   phex
        clr     a
        acall   pregsn
        mov     r0,sp
        dec     r0
        mov     a,@r0
        acall   phex
        mov     a,#1
        acall   pregsn
        mov     a,r1
        acall   phex
        mov     a,#2
        acall   pregsn
        mov     a,r2
        acall   phex
        mov     a,#3
        acall   pregsn
        mov     a,r3
        acall   phex
        mov     a,#4
        acall   pregsn
        mov     a,r4
        acall   phex
        mov     a,#5
        acall   pregsn
        mov     a,r5
        acall   phex
        mov     a,#6
        acall   pregsn
        mov     a,r6
        acall   phex
        mov     a,#7
        acall   pregsn
        mov     a,r7
        acall   phex
        acall   newline
        acall   space           ;now begin printing the 2nd line
        mov     a,#'P'
        acall   cout
        lcall   prc
        acall   pequal
        clr     c              
        mov     a,sp
        subb    a,#4
        mov     r0,a
        push    dpl
        push    dph
        lcall   inst
        pop     dph
        pop     dpl
        pop     b
        pop     acc
        mov     r0,a
done:   pop     acc
        pop     psw
        reti


sshelp: push    dpl
        push    dph
        acall   newline
        mov     dptr,#help5txt
        acall   pstr
        pop     dph
        pop     dpl
        ret

pregsn: push    acc
        acall   space
        mov     a,#'R'
        acall   cout
        pop     acc
        acall   phex1
        acall   pequal
        ret

ssdmp:                    ;.
        push    0
        push    1
        push    b
        push    dpl
        push    dph
        mov     dptr,#ssdmps1
        acall   pstr
        pop     dph
        pop     dpl
        clr     a
        acall   phex
        mov     a,#':'
        acall   cout
        acall   space
        mov     a,r0
        acall   phex
        acall   space
        mov     a,r1
        acall   phex
        mov     r0,#2
        mov     r1,#14
        ajmp    ssdmp2
ssdmp1: mov     a,r0
        acall   phex
        mov     a,#':'
        acall   cout
        mov     r1,#16
ssdmp2: acall   space
        mov     a,@r0
        acall   phex
        inc     r0
        djnz    r1,ssdmp2
        acall   newline
        cjne    r0,#80h,ssdmp1
        acall   newline
        pop     b
        pop     1
        pop     0
        ret

ssreg:           
        push    b             ;.
        acall   space
        mov     a,#'B'
        acall   cout
        acall   pequal
        mov     a,b
        acall   phex
        acall   space
        mov     a,#'P'
        acall   cout
        mov     a,#'S'
        acall   cout
        mov     a,#'W'
        acall   cout
        acall   pequal
        mov     a,r0
        push    acc
        mov     r0,sp
        dec     r0
        dec     r0
        dec     r0
        mov     a,@r0
        acall   phex
        acall   space
        push    dpl
        push    dph
        mov     dptr,#sfr3+1
        mov     r0,0xA8
        acall   psfr
        mov     dptr,#sfr4+1
        mov     r0,0xB8
        acall   psfr
        mov     dptr,#sfr5+1
        mov     r0,0x89
        acall   psfr
        mov     dptr,#sfr6+1
        mov     r0,0x88
        acall   psfr
        mov     dptr,#sfr7+1
        mov     r0,0x98
        acall   psfr
        mov     dptr,#sfr8+1
        mov     r0,0x87
        acall   psfr
        mov     a,#'T'
        acall   cout
        mov     a,#'0'
        acall   cout
        acall   pequal
        mov     a,8Ch
        acall   phex
        mov     a,8Ah
        acall   phex
        acall   space
        mov     a,#'T'
        acall   cout
        mov     a,#'1'
        acall   cout
        acall   pequal
        mov     a,8Dh
        acall   phex
        mov     a,8Bh
        acall   phex
        acall   newline
        pop     dph
        pop     dpl
        pop     acc
        mov     r0,a
        pop     b
        ajmp    step1

psfr:   acall   pstr
        acall   pequal
        mov     a,r0
        acall   phex
        acall   space
        ret

skip0:                       ;.
        push    b
        mov     a,r0
        push    acc
        mov     a,sp
        clr     c
        subb    a,#4
        mov     r0,a
        push    dpl
        push    dph
        mov     dptr,#sskip1
        acall   pstr
        lcall   inst          ;print skipped instruction r0 points to pc
        mov     a,sp
        clr     c
        subb    a,#6
        mov     r0,a
        mov     @r0,dph         ;actually change the pc!
        dec     r0
        mov     @r0,dpl
        mov     dptr,#sskip2
        acall   pstr
        inc     r0
        lcall   inst            ;print new instruction
        pop     dph
        pop     dpl
        pop     acc
        mov     r0,a
        pop     b
        ajmp    step1

chacc:
        mov     a,r0
        push    acc
        push    b
        mov     r0,sp
        dec     r0
        dec     r0
        push    dpl
        push    dph
        mov     dptr,#chaccs1
        acall   pstr
        acall   ghex
        jc      chacc2
        mov     @r0,a
        acall   newline
        pop     dph
        pop     dpl
        pop     b
        pop     acc
        mov     r0,a
        ajmp    step1
chacc2: mov     dptr,#abort
        acall   pstr
        pop     dph
        pop     dpl
        pop     b
        pop     acc
        mov     r0,a
        ajmp    step1

DownLoad:       ;Note, this is a modified version of the
                ;auto baud rate detection routine from
                ;MDP/51.  Thank You, Kei-Yong Khoo (3-31-87)
        push    dpl
        push    dph
        mov     dptr,#dwlds1            
        acall   pstr            ;"begin sending file <ESC> to abort"
dwld0a: aCALL   cin 
        CJNE    A, #27, DWLD0   ; Test for escape
dwldesc:mov     dptr,#dwlds2    
        acall   pstr            ;"download aborted."
        pop     dph
        pop     dpl
        ret
DWLD0:                         
        CJNE    a, #0x3A, DWLD0a     ; wait for ':'
        ACALL   ghex
        jc      dwldesc
        MOV     R0, A             ; R0 = # of data bytes
        ACALL   ghex
        jc      dwldesc
        MOV     DPH, A            ; High byte of load address
        ACALL   ghex
        jc      dwldesc
        MOV     DPL, A            ; Low byte of load address
        ACALL   ghex              ; Record type
        jc      dwldesc
        CJNE    A, #1, DWLD1      ; End record?
        mov     dptr,#dwlds3
        acall   pstr              ;"download went ok..."
        pop     dph
        pop     dpl
        ret
DWLD1:  INC     R0                ; adjust for repeat loop
        AJMP    DWLD3
DWLD2:  ACALL   ghex              ; Get data byte
        jc      dwldesc
        MOVX    @DPTR, A
        INC     DPTR
DWLD3:  DJNZ    R0, DWLD2
        ACALL   ghex              ; Discard checksum
        jc      dwldesc
        aJMP    DWLD0a

INIT:           ;Note, this is a modified version of the
                ;auto baud rate detection routine from
                ;MDP/51.  Thank You, Kei-Yong Khoo (3-31-87)
        orl     PCON,#10000000b   ; set double baud rate
        MOV     TMOD,#00010001b
        MOV     SCON,#01010000b  ; Set Serial for mode 1 &
                                 ; Enable reception
        ORL     TCON,#01010010b  ; Start timer 1 both timer
        mov     a,7Bh
        mov     r1,7Ah
        mov     r2,79h
        mov     r3,78h
        xrl     1,#01010101b
        xrl     2,#11001100b
        xrl     3,#00011101b
        cjne    a,1,auto
        cjne    a,2,auto
        cjne    a,3,auto
        sjmp    autoend          ;baud rate is known from last time...
AUTO:   CLR     TR1              ; Stop timer 1
        MOV     TH1, #0          ; Clear timer 1
        MOV     TL1, #0
        JB      RXD, *           ; Wait for start bit
        JB      RXD,AUTO         ; make sure it's not just noise
        JB      RXD,AUTO
        JB      RXD,AUTO
        JB      RXD,AUTO
        JNB     RXD, *           ; skip start bit
        SETB    TR1
        JB      RXD, * 
        JNB     RXD, *
        JB      RXD, *
        JNB     RXD, *         ; Count 3 more bits
        CLR     TR1
         ;  Compute baud rate
        MOV     A, TL1           ; divide TH1-TL1 by 128
        RLC     A                
        MOV     A, TH1
        RLC     A
        CPL     A
        INC     A                ; 2's complement
        mov     b,a              ; store the reload value four times
        mov     7Bh,a            ;so that it might be there later
        xrl     a,#01010101b     ;we'll store the reload value
        mov     7Ah,a            ;four times, just to be safe
        mov     a,b
        xrl     a,#11001100b
        mov     79h,a
        mov     a,b
        xrl     a,#00011101b
        mov     78h,a
        mov     a,b
autoend:MOV     TH1,A
        mov     tmod,#00100001b  ;now it's 8 bit auto-reload
        SETB    TR1
        RET

HELP:
        push    dpl
        push    dph
        MOV     DPTR,#HELP1txt
        ACALL   PSTR
        mov     dptr,#cmd_tbl
        clr     a
help0:  movc    a,@a+dptr
        jz      help_2
        inc     dptr
        acall   space
        acall   cout
        mov     r0,#4
help1:  acall   space
        djnz    r0,help1
        clr     a
        movc    a,@a+dptr   ;(high)
        mov     b,a
        inc     dptr
        clr     a
        movc    a,@a+dptr   ;(low)
        inc     dptr
        inc     dptr
        inc     dptr
        push    dpl
        push    dph
        mov     dpl,a
        mov     dph,b
        clr     a
        acall   pstr
        acall   newline
        pop     dph
        pop     dpl
        sjmp    help0
help_2: mov     dptr,#help2txt
        acall   pstr
        pop     dph
        pop     dpl
        RET

run:
        push    dpl
        push    dph
        mov     dptr,#prompt6
        acall   pstr
        acall   cin
        acall   upper
        cjne    a,#27,run1
        mov     dptr,#abort             ;if they press <ESC>
        acall   pstr
        pop     dph
        pop     dpl
        ret
run1:   cjne    a,#'?',run3
        mov     dptr,#help3txt          ;if they pressed ?
        acall   pstr
        mov     dptr,#prompt7
        acall   pstr
        acall   cin
        cjne    a,#27,run2
        pop     dph
        pop     dpl
        ret
run2:   mov     dptr,#help4txt
        acall   pstr
        mov     dptr,#help5txt
        acall   pstr
        pop     dph
        pop     dpl
        ret
run3:   cjne    a,#'S',run4
        mov     dptr,#runss
        acall   pstr
        ajmp    ssrun
run4:   mov     dptr,#runstd
        acall   pstr
        mov     dptr,#prompt8              ;run the user's program
        acall   pstr
        pop     dph
        pop     dpl
        acall   phex16
        push    dpl
        push    dph
        mov     dptr,#prompt4
        acall   pstr
        pop     dph
        pop     dpl
        acall   ghex16
        jnc     run5
        mov     dptr,#abort
        acall   pstr
        ret
run5:   mov     a,#'\r'
        acall   cout
        push    dpl
        push    dph
        mov     dptr,#runs1
        acall   pstr
        pop     dph
        pop     dpl
        mov     a,#rst & 0xFF           
        push    acc
        mov     a,#rst
        push    acc
        push    dpl
        push    dph
        ret                     ;<-- actually jumps to user's program

        
ssrun:  mov     dptr,#prompt8              ;run single step
        acall   pstr
        pop     dph
        pop     dpl
        push    dpl
        push    dph
        acall   phex16
        mov     dptr,#prompt4
        acall   pstr
        pop     dph
        pop     dpl
        acall   ghex16
        push    dpl
        push    dph
        jnc     ssrun1
        mov     dptr,#abort
        acall   pstr
        acall   newline
        pop     dph
        pop     dpl
        ret
ssrun1: clr     tcon.2
        jnb     p3.3,ssrun2
        mov     dptr,#sserr1      ;give error msg if int1 not grounded
        acall   pstr
        pop     dph
        pop     dpl
        ret
ssrun2: mov     dptr,#prompt9           ;ask for priority
        acall   pstr
        acall   cin
        acall   upper
        cjne    a,#27,ssrun3
        mov     dptr,#abort
        acall   pstr
        pop     dph
        pop     dpl
        ret
ssrun3: cjne    a,#'L',ssrun4
        mov     ip,#00000000b
        sjmp    ssrun5
ssrun4: mov     ip,#00000100b
ssrun5: mov     dptr,#ssmsg     ;tell 'em it now time, remind to
        acall   pstr            ;to hit <RET> for next inst
        pop     dph
        pop     dpl
        mov     sp,#38h
        mov     a,#rst & 0xFF           
        push    acc
        mov     a,#0
        push    acc
        clr     a
        clr     c
        mov     r0,#0
        mov     r1,#1
        mov     r2,#0
        mov     r3,#0
        mov     r4,#0
        mov     r5,#0
        mov     r6,#0
        mov     r7,#0
        clr     tcon.2
        setb    p3.3
        mov     ie,#10000100b
        jmp     @a+dptr




dump:   
        mov     r2,#16          ;number of lines to print
        acall   newline
dump1:  mov     r0,#20h         ;pointer to memory to store bytes
        acall   phex16
        mov     a,#':'
        acall   cout
        mov     a,#' '
        acall   cout
dump2:  clr     A                  ;<--acquire 16 bytes
        movc    a,@a+dptr          ;and store from 20 to 2F
        mov     @r0,a
        acall   phex
        mov     a,#' '
        acall   cout
        inc     dptr
        inc     r0
        cjne    r0,#30h,dump2
        acall   cout
        mov     r0,#20h           ;now we'll print it in ascii
dump3:  mov     a,@r0
        anl     a,#01111111b      ;avoid unprintable characters
        mov     b,a
        clr     c
        subb    a,#20h
        jnc     dump4
        mov     b,#' '
dump4:  mov     a,b
        acall   cout
        inc     r0
        cjne    r0,#30h,dump3
        acall   newline
        acall   esc
        jc      dump5
        djnz    r2,dump1        ;loop back up to print next line
dump5:  acall   newline
        ret


new_loc:push    dph
        push    dpl
        acall   newline
        mov     dptr,#prompt5
        acall   pstr
        pop     dpl
        pop     dph
        push    dph
        push    dpl
        acall   ghex16
        acall   newline
        jc      newloc1
        acall   newline
        pop     acc
        pop     acc
        ret
newloc1:mov     dptr,#abort
        acall   pstr
        pop     dpl
        pop     dph
        acall   newline
        ret

edit:      ;edit external ram...
        push    dpl
        push    dph
        mov     dptr,#edits1
        acall   pstr
        pop     dph
        pop     dpl
edit1:  acall   phex16
        mov     a,#':'
        acall   cout
        acall   space
        mov     a,#'('
        acall   cout
        movx    a,@dptr
        acall   phex
        push    dpl
        push    dph
        mov     dptr,#prompt10
        acall   pstr
        acall   ghex
        jb      psw.5,edit2
        jc      edit2
        pop     dph
        pop     dpl
        movx    @dptr,a
        acall   newline
        inc     dptr
        ajmp    edit1
edit2:  mov     dptr,#edits2
        acall   pstr
        pop     dph
        pop     dpl
        ret

list:       ;prints out dis-assembly list of memory
        mov     r2,#18  ;# of lines to list
        acall   newline
list1:  mov     a,r2
        push    acc
        acall   disasm          ;this obviously does all the work
        pop     acc
        mov     r2,a
        acall   esc
        jc      list2
        djnz    r2,list1
list2:  acall   newline
        ret



CIN:
        JNB     RI,cin  ; wait for character
        CLR     RI
        MOV     A,SBUF
        RET

COUT:    ;note, improved... much faster transmission
        jnb     ti,*       ;wait if a character is still sending
        MOV     SBUF,A
        CLR     TI         ;note: hardware will set ti when the
        RET                ;      character finishes sending...
 
esc:  ;checks to see if <ESC> is waiting on serial port.
      ;C=clear if no <ESC>, C=set if <ESC> pressed, buffer flushed
        push    acc
        clr     c
        jnb     ri,esc1
        mov     a,sbuf
        cjne    a,#27,esc1
        setb    c
        clr     ri
esc1:   pop     acc
        ret

NEWLINE:
        PUSH    ACC
        MOV     A,#'\r'
        ACALL   COUT
        POP     ACC
        RET

g1hex:     ;gets one character.  Converts to hex and places in Acc
           ;C=1 if <ESC> pressed, 0 otherwise
           ;PSW.5=1 if <RET> pressed, 0 otherwise
           ;PSW.2=1 if backspace or delete, 0 otherwise  (not yet)
        acall   cin
        acall   upper
        clr     c
        clr     psw.5
        clr     psw.2
        cjne    a,#13,g1hex2
        setb    psw.5
        ret
g1hex2: cjne    a,#27,g1hex2a
        cpl     c
        ret
g1hex2a:cjne    a,#8,g1hex2c
g1hex2b:setb    psw.2
        ret
g1hex2c:cjne    a,#127,g1hex3
        sjmp    g1hex2b
g1hex3: push    b
        mov     b,a
        acall   asc2hex        
        jc      g1hex4           ;they typed a bad char, so go back
        xch     a,b
        acall   cout
        xch     a,b
        pop     b
        ret
g1hex4: pop     b
        sjmp    g1hex

ghex:      ; gets an 8-bit hex value from keyboard, returned in A.
           ; C=1 if <ESC> pressed, 0 otherwise
           ; PSW.5 if <RET> pressed w/ no input, 0 otherwise
        acall   g1hex
        jnb     psw.5,ghex1
        ret
ghex1:  jnc     ghex2
        ret
ghex2:  swap    a
        mov     b,a
ghex3:  acall   g1hex
        jb      psw.2,ghex6
        jnb     psw.5,ghex4
        clr     psw.5
        mov     a,b
        swap    a
        ret
ghex4:  jnc     ghex5
        ret
ghex5:  orl     a,b
        ret
ghex6:  mov     a,#8
        acall   cout
        sjmp    ghex



        
ghex16:    ; gets a 16-bit hex value from keyboard, returned in DPTR.
           ; C=0 if normal <RET>, C=1 if <ESC> pressed

        push    acc
        mov     a,r0
        push    acc
        mov     a,r1
        push    acc
        mov     a,r2
        push    acc
        mov     a,r3
        push    acc
ghex16a:acall   g1hex           ;get first character
        mov     r0,a
        jc      ghex16z
        jb      psw.5,ghex16z
        jb      psw.2,ghex16a
ghex16e:acall   g1hex           ;get second character
        mov     r1,a
        jc      ghex16z
        jb      psw.5,ghex16s
        jnb     psw.2,ghex16i
        mov     a,#8
        acall   cout
        sjmp    ghex16a
ghex16i:acall   g1hex           ;get third character
        mov     r2,a
        jc      ghex16z
        jb      psw.5,ghex16t
        jnb     psw.2,ghex16m
        mov     a,#8
        acall   cout
        sjmp    ghex16e
ghex16m:acall   g1hex           ;get fourth character
        mov     r3,a
        jc      ghex16z
        jb      psw.5,ghex16u
        jnb     psw.2,ghex16v
        mov     a,#8
        acall   cout
        sjmp    ghex16i        
ghex16s:mov     dph,#0          ;one digit entered
        mov     dpl,r0
        sjmp    ghex16z
ghex16t:mov     dph,#0          ;two digit entered
        mov     a,r0
        swap    a
        orl     a,r1
        mov     dpl,a
        sjmp    ghex16z
ghex16u:mov     dph,r0          ;three digit entered
        mov     a,r1
        swap    a
        orl     a,r2
        mov     dpl,a
        sjmp    ghex16z
ghex16v:mov     a,r0            ;all four digit entered
        swap    a
        orl     a,r1
        mov     dph,a
        mov     a,r2
        swap    a
        orl     a,r3
        mov     dpl,a 
ghex16z:pop     acc             ;exit, at last...
        mov     r3,a
        pop     acc
        mov     r2,a
        pop     acc
        mov     r1,a
        pop     acc
        mov     r0,a
        pop     acc
        ret
        


PHEX:
        PUSH    ACC
        SWAP    A
        ACALL   PHEX1
        POP     ACC
        ACALL   PHEX1
        RET
PHEX1:  ANL     A,#0FH
        ADD     A,#3
        MOVC    A,@A+PC
        ACALL   COUT
        RET
        .DB     "0123456789ABCDEF"

asc2hex:             ;carry set if invalid input
        clr     c
        push    b
        subb    a,#'0'
        mov     b,a
        subb    a,#10
        jc      a2h1
        mov     a,b
        subb    a,#7
        mov     b,a
a2h1:   mov     a,b
        clr     c
        anl     a,#11110000b     ;just in case...
        jz      a2h2
        setb    c
a2h2:   mov     a,b
        pop     b
        ret

upper:  ;converts the ascii code in Acc to uppercase, if it is lowercase
        push    b
        mov     b,a
        clr     c
        subb    a,#61h
        jc      upper1;   not a lowercase character
        subb    a,#26
        jnc     upper1
        clr     c
        mov     a,b
        subb    a,#20h
        mov     b,a
upper1: mov     a,b
        pop     b
        ret

space:  push    acc
        mov     a,#' '
        acall   cout
        pop     acc
        ret


PHEX16:
        PUSH    ACC
        MOV     A,DPH
        ACALL   PHEX
        MOV     A,DPL
        ACALL   PHEX
        POP     ACC
        RET

PSTR:                  ;print string
        PUSH    ACC
PSTR1:  CLR     A
        MOVC    A,@A+DPTR
        jb      ri,pstr3
        JZ      PSTR2
        mov     c,acc.7
        anl     a,#01111111b
        jnb     ti,*            ;wait for last character to finish sending
        mov     sbuf,a
        clr     ti
        Jc      pstr2
        inc     dptr
        SJMP    PSTR1                                          
PSTR2:  POP     ACC
        RET    
pstr3:  clr     ri
        mov     a,sbuf
        cjne    a,#27,pstr1
        mov     a,#13
        acall   cout
        sjmp    pstr2

disasm:    ;prints out instruction @dptr, and advances dptr
        acall   phex16        ;they'd like to know the address...
        clr     a
        movc    a,@a+dptr
        inc     dptr
        mov     r3,a
        push    dph
        push    dpl
        mov     dptr,#mnot    ;mnot=mnunonic offset table
        movc    a,@a+dptr
        mov     r2,a
        mov     a,r3
        mov     dptr,#opot    ;opot=operand offset table
        movc    a,@a+dptr
        mov     r6,a              ;these 2 tables are a bitch to enter!!!
        anl     a,#11111100b
        rr      a
        rr      a
        mov     r7,a
        mov     a,r6
        anl     a,#00000011b
        pop     dpl
        pop     dph
        mov     r6,a
        dec     a
        jz      disasm1
        push    acc
        clr     a
        movc    a,@a+dptr       ;fetch 2nd byte (if nec)
        inc     dptr
        mov     r4,a
        pop     acc
        dec     a
        jz      disasm1
        clr     a
        movc    a,@a+dptr       ;fetch 3rd byte (if nec)
        inc     dptr  
        mov     r5,a
disasm1:acall   disasm4         ;prints the bytes+spaces
        push    dpl
        push    dph
        mov     dptr,#mnu_tbl   ;mnu_tlb: table with actual text...
        acall   disasm5         ;prints the mnenonic+spaces
        mov     a,r7
        mov     r0,a
        pop     acc
        pop     b
        push    b
        push    acc
        mov     r6,a      ;(high) in case of ajmp or acall or rel
        mov     a,b
        mov     r7,a      ;(low)
        mov     a,r0
        dec     a
        clr     c
        rlc     a
        mov     b,#dasm2 & 0xff  ;(low)
        push    b
        mov     b,#dasm2 / 256    ;(high)
        push    b
        mov     dptr,#oprt      ;oprt=operand routine table
        jmp     @a+dptr
dasm2:  pop     dph
        pop     dpl
        acall   newline
        ret

disasm4:mov     a,#':'       ;print bytes and some punctuation
        acall   cout
        acall   space
        mov     a,r3
        acall   phex
        acall   space
        cjne    r6,#1,dis4a
        mov     r1,#11
        sjmp    dis4c
dis4a:  mov     a,r4
        acall   phex
        acall   space
        cjne    r6,#2,dis4b
        mov     r1,#8
        sjmp    dis4c
dis4b:  mov     a,r5
        acall   phex
        mov     r1,#6
dis4c:  acall   space
        djnz    r1,dis4c
        ret

disasm5:mov     r0,#7+1         ;prints the mnunonic+spaces
dis5a:  mov     a,r2
        MOVC    A,@A+DPTR
        INC     r2
        dec     r0
        mov     c,acc.7
        anl     a,#01111111b
        acall   cout
        Jc      dis5b
        SJMP    dis5a
dis5b:  acall   space
        djnz    r0,dis5b
        RET


;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-;
;       2k page boundry must exist between these dividers             ;
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-;

           
prompt1:.db     ">Loc=",0
prompt2:.db     ": (Version 1.0) Command>",0
prompt3:.db     "Location (",0
prompt4:.db     "=Default): ",0
prompt5:.db     "New memory pointer location: ",0
prompt6:.db     "\rS=Single Step, N=Normal (default), ?=Help >",0
prompt7:.db     "Press any key: ",0
prompt8:.db     "\r\rRun from memory loaction (",0
prompt9:.db     "\r\rInterrupt priority> "
        .db     "L=Low, H=High (default): ",0 
prompt10:.db    ")  New Value: ",0
abort:  .db     "  Command Aborted!\r",0


;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-;
;       2k page boundry must exist between these dividers             ;
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-;


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
        ajmp   opcd14          ;A
        ajmp   opcd15          ;C
        ajmp   opcd16          ;bit
        ajmp   opcd17          ;direct
        ajmp   opcd18          ;@Ri
        ajmp   opcd19          ;AB
        ajmp   opcd20          ;Rn,rel
        ajmp   opcd21          ;direct,rel
        ajmp   opcd22          ;Rn
        ajmp   opcd23          ;DPTR
        ajmp   opcd24          ;bit,rel
        ajmp   opcd25          ;rel
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
        ajmp   opcd43          ; <nothing>

p_reg_n:mov     a,#'R'
        lcall   cout
        mov     a,r3
        anl     a,#00000111b
        lcall   phex1
        ret

p_reg_i:mov     a,#'@'
        lcall   cout
        mov     a,#'R'
        lcall   cout
        mov     a,r3
        anl     a,#00000001b
        lcall   phex1
        ret

pdirect:
        mov     a,r4
        jb      acc.7,pdir1
pdir0:  mov     a,r4
        lcall   phex
        ret
pdir1:  mov     dptr,#sfrmnu
pdir2:  clr     a
        movc    a,@a+dptr
        inc     dptr
        jz      pdir0
        mov     r0,a
        clr     c
        subb    a,r4
        jnz     pdir3
        lcall   pstr
        ret
pdir3:  clr     a
        movc    a,@a+dptr
        inc     dptr
        jnb     acc.7,pdir3
        sjmp    pdir2
                
pbit: 
        mov     a,r4
        anl     a,#01111000b
        rl      a
        swap    a
        mov     r0,a
        mov     a,r4
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
pbit0:  lcall   pstr
        sjmp    pbit2
pbit1:  mov     a,r0            ;it's between 20h and 2Fh
        add     a,#20h
        lcall   phex
pbit2:  mov     a,#'.'
        lcall   cout
        mov     a,r4
        anl     a,#00000111b
        lcall   phex1
        ret
prel:
        mov     a,r4
        jb      acc.7,prel4
        clr     c
        addc    a,r7
        mov     r7,a
        jnc     prel8
        inc     r6
        sjmp    prel8
prel4:  cpl     a
        inc     a
        mov     r4,a
        mov     a,r7
        clr     c
        subb    a,r4
        mov     r7,a
        jnc     prel8
        dec     r6
prel8:  mov     a,r6
        lcall   phex
        mov     a,r7
        lcall   phex    
        ret


opcd1:  mov     a,r6        ;addr11             done
        anl     a,#11111000b
        mov     r0,a
        mov     a,r3
        swap    a
        rr      a
        anl     a,#00000111b
        orl     a,r0
        lcall   phex
        mov     a,r4
        lcall   phex
        ret       
opcd2:                      ;A,Rn               done
        acall   pac
        acall   p_reg_n
        ret
opcd3:                      ;A,direct           done
        acall   pac
        acall   pdirect
        ret
opcd4:                      ;A,@Ri              done
        acall   pac
        acall   p_reg_i
        ret
opcd5:                      ;A,#data            done
        acall   pa
pdata:  acall   pcomma
        acall   plb
        mov     a,r4
        lcall   phex
        ret
opcd6:                      ;direct,A           done
        acall   pdirect
        acall   pcomma
        acall   pa
        ret
opcd7:                      ;direct,#data       done
        acall   pdirect
        mov     a,r5
        mov     r4,a
        ajmp    pdata
opcd8:                      ;C,bit              done
        acall   prc
        acall   pcomma
        acall   pbit
        ret
opcd9:                      ;C,/bit             done
        acall   prc
        acall   pcomma
        mov     a,#'/'
        lcall   cout
        acall   pbit
        ret
opcd10:                     ;A,direct,rel       done
        acall   pac
        acall   pdirect
opcd10a:acall   pcomma
        mov     a,r5
        mov     r4,a
        acall   prel
        ret
opcd11:                     ;A,#data,rel        done
        acall   pa
opcd11a:acall   pcomma
        acall   plb
        mov     a,r4
        lcall   phex
        ajmp    opcd10a
opcd12:                     ;Rn,#data,rel       done
        acall   p_reg_n
        ajmp    opcd11a
opcd13:                     ;@Ri,#data,rel      done
        acall   p_reg_i
        ajmp    opcd11a
opcd14:                     ;A                  done
        acall   pa
        ret
opcd15:                     ;C                  done
        acall   prc
        ret
opcd16:                     ;bit                done
        acall   pbit
        ret
opcd17:                     ;direct             done
        acall   pdirect
        ret
opcd18:                     ;@Ri                done
        acall   p_reg_i
        ret
opcd19:                     ;AB                 done
        acall    pa
        mov     a,#'B'
        lcall   cout
        ret
opcd20:                     ;Rn,rel             done
        acall   p_reg_n
        acall   pcomma
        acall   prel
        ret
opcd21:                     ;direct,rel         done
        acall   pdirect
        ajmp    opcd10a
opcd22:                     ;Rn                 done
        acall   p_reg_n
        ret
opcd23:                     ;DPTR               done
        acall   pdptr
        ret
opcd24:                     ;bit,rel            done
        acall   pbit
        ajmp    opcd10a
opcd25:                     ;rel                done
        acall   prel
        ret
opcd26:                     ;@A+DPTR            done
        acall   pat
        acall   pa
        mov     a,#'+'
        lcall   cout
        acall   pdptr
        ret
opcd28:                     ;Rn,A               done
        acall   p_reg_n
        acall   pcomma
        acall   pa
        ret    
opcd29:                     ;Rn,direct          done
        acall   p_reg_n
        acall   pcomma          
        acall   pdirect
        ret
opcd30:                     ;Rn,#data           done
        acall   p_reg_n
        ajmp    pdata
opcd31:                     ;direct,Rn          done
        acall   pdirect
        acall   pcomma
        acall   p_reg_n
        ret
opcd32:                     ;direct,direct      done
        mov     a,r4
        push    acc
        mov     a,r5
        mov     r4,a
        acall   pdirect
        acall   pcomma
        pop     acc
        mov     r4,a
        acall   pdirect
        ret
opcd33:                     ;direct,@Ri         done
        acall   pdirect
        acall   pcomma
        acall   p_reg_i
        ret
opcd34:                     ;@Ri,A              done
        acall   p_reg_i
        acall   pcomma
        acall   pa
        ret
opcd35:                     ;@Ri,direct         done
        acall   p_reg_i
        acall   pcomma
        acall   pdirect
        ret
opcd36:                     ;@Ri,#data          done
        acall   p_reg_i
        ajmp    pdata
opcd37:                     ;bit,C              done
        acall   pbit
        acall   pcomma
        acall   prc
        ret
opcd38:                     ;DPTR,#data16       done
        acall   pdptr
        acall   pcomma
        acall   plb
opcd27: mov     a,r4           ;addr16          done
        lcall   phex
        mov     a,r5
        lcall   phex
        ret
opcd39:                     ;A,@A+DPTR          done
        acall   pac
        acall   pat
        acall   pa
        mov     a,#'+'
        lcall   cout
        acall   pdptr
        ret
opcd40:                     ;A,@A+PC            done
        acall   pac
        acall   pat
        acall   pa
        mov     a,#'+'
        lcall   cout
        mov     a,#'P'
        lcall   cout
        acall   prc
        ret
opcd41:                     ;A,@DPTR            done
        acall   pac
        acall   pat
        acall   pdptr
        ret
opcd42:                     ;@DPTR,A            done
        acall   pat
        acall   pdptr
        acall   pcomma
        acall   pa
opcd43: ret                 ;<nothing>          done

pat:            ;prints the '@' symbol
        mov     a,#'@'
        lcall   cout
        ret
pcomma:         ;prints a comma
        mov     a,#','
        lcall   cout
        lcall   space
        ret
plb:            ;prints the '#' symbol
        mov     a,#'#'
        lcall   cout
        ret
pa:             ;prints 'A'
        mov     a,#'A'
        lcall   cout
        ret
prc:             ;prints 'C'
        mov     a,#'C'
        lcall   cout
        ret
pac:            ;print "A,"
        acall   pa
        acall   pcomma
        ret
pdptr:          ;prints DPTR
        push    dph
        push    dpl
        mov     dptr,#sdptr
        lcall   pstr
        pop     dpl
        pop     dph
        ret

poweron:
        MOV     SP,#30H
        clr     psw.3           ;set for register bank 0 (init needs it)
        clr     psw.4
        LCALL   INIT
        setb    ti              ;ti is normally set in this program
        clr     ri              ;ri is normallt cleared
        mov     r0,#8
        lcall   newline
        djnz    r0,*
        MOV     DPTR,#logon
        lCALL   PSTR
        mov     dptr,#program
        
MENU:
        PUSH    DPL
        PUSH    DPH
        MOV     DPTR,#PROMPT1
        lCALL   PSTR
        POP     DPH
        POP     DPL
        lCALL   PHEX16
        PUSH    DPL
        PUSH    DPH
        MOV     DPTR,#PROMPT2
        lCALL   PSTR
        lCALL   CIN             ;GET THE INPUT CHARACTER
        lcall   upper
	.equ	char, 0x20
        MOV     CHAR,A
        MOV     DPTR,#CMD_TBL   ;BEGIN SEARCH THRU THE TABLE
MENU1:  CLR     A 
        MOVC    A,@A+DPTR
        JZ      MENU3           ;JUMP IF END OF TABLE REACHED
        CJNE    A,CHAR,MENU2    ;JUMP IF THIS IS NOT THE COMMAND TYPED
        INC     DPTR            ;OK, SO THIS IS THE RIGHT ONE...
        CLR     A
        MOVC    A,@A+DPTR
        PUSH    Acc
        INC     DPTR
        CLR     A
        MOVC    A,@A+DPTR
        pop     b
        push    dpl
        push    dph
        MOV     DPL,A
        mov     DPH,b
        lCALL   PSTR            ;PRINT THE COMMAND NAME
        lCALL   NEWLINE
        pop     dph
        pop     dpl
        INC     DPTR
        CLR     A
        MOVC    A,@A+DPTR
        MOV     22h,A
        CLR     A
        INC     DPTR
        MOVC    A,@A+DPTR
        mov     21h,a
        POP     23h
        POP     24h
        mov     dptr,#menu
        push    dpl
        push    dph
        mov     dpl,24h
        mov     dph,23h
        PUSH    21h
        PUSH    22h
        RET                     ;SIMULATED CALL TO THE ROUTINE
        lJMP    MENU
MENU2:  INC     DPTR            ;SKIP THIS TABLE ENTRY  
        INC     DPTR
        INC     DPTR
        INC     DPTR
        INC     DPTR
        lJMP    MENU1
MENU3:  POP     DPH             ;NOW WE TELL 'EM THEY TYPED 
        POP     DPL             ;AN ILLEGAL CHARACTER
        lCALL   NEWLINE
        lJMP    MENU


  ;this prints the instructions for status and skip in single-step
inst:   mov     a,r1            ;r0 must point to pc
        push    acc
        mov     a,r2
        push    acc
        mov     a,r3
        push    acc
        mov     a,r4
        push    acc
        mov     a,r5
        push    acc
        mov     a,r6
        push    acc
        mov     a,r7
        push    acc
        mov     dph,@r0         ;put pc into dptr for disasm
        dec     r0
        mov     dpl,@r0
        lcall   disasm
        pop     acc
        mov     r7,a
        pop     acc
        mov     r6,a
        pop     acc
        mov     r5,a
        pop     acc
        mov     r4,a
        pop     acc
        mov     r3,a
        pop     acc
        mov     r2,a
        pop     acc
        mov     r1,a
        ret
        



;---------------------------------------------------------;
;                                                         ;
;      Here begins the data tables and strings:           ;
;                                                         ;
;---------------------------------------------------------;
                                                         
logon:  .db     "\r   \r        "
        .db  12,"Welcome to the new and possibly even "
        .db     "improved 8031 monitor/debugger\r"
        .db     "by Paul Stoffregen on 14-OCT-91 "
        .db     "for no good reason whatsoever...\r\r"
    ;columbs     1234567890123456789012345678901234567890
        .db     "These are some of the features offered "
        .db     "by this particular debugger:\r\r"
        .db     "     Download programs from PC          "
        .db     "User-Friendliness!!!!!\r"
        .db     "     Run Program                        "
        .db     "Uses no external RAM\r"
        .db     "       (Normal, or single-step)         "
        .db     "<ESC> key is supported\r"
        .db     "     List assemble code from memory     "
        .db     "automatic baud rate detection\r"
        .db     "     Hex Dump and Edit external RAM     "
        .db     "On-line help (type '?')\r"
        .db     "\rHowever, you don't get somethin' for "
        .db     "nothin'...  The code for this debugger\r"
        .db     "requires quite a bit more room than is "
        .db     "typical for a debugger, but it will all\r"
        .db     "fit into a 2764 (8K x 8) EPROM.\r"
        .db     "\rNO Copyright!!  Please distribute freely.  "
        .db     "Make as many copies as you want.\r\r\r",0
                
mnu_tbl:.db     "ACAL",'L'+128         ;comma is ok
        .db     "AD",'D'+128,' '
        .db     "ADD",'C'+128
        .db     "AJM",'P'+128
        .db     "AN",'L'+128
        .DB     "CJN",'E'+128
        .DB     "CL",'R'+128
        .DB     "CP",'L'+128
        .DB     "D",'A'+128 
        .DB     "DE",'C'+128
        .DB     "DI",'V'+128
        .DB     "DJN",'Z'+128
        .DB     "IN",'C'+128
        .DB     "J",'B'+128
        .DB     "JB",'C'+128
        .DB     "J",'C'+128
        .DB     "JM",'P'+128
        .DB     "JN",'B'+128
        .DB     "JN",'C'+128
        .DB     "JN",'Z'+128
        .DB     "J",'Z'+128
        .DB     "LCAL",'L'+128
        .DB     "LJM",'P'+128
        .DB     "MO",'V'+128
        .DB     "MOV",'C'+128
        .DB     "MOV",'X'+128
        .DB     "MU",'L'+128
        .DB     "NO",'P'+128
        .DB     "OR",'L'+128
        .DB     "PO",'P'+128
        .DB     "PUS",'H'+128
        .DB     "RE",'T'+128
        .DB     "RET",'I'+128
        .DB     "R",'L'+128
        .DB     "RL",'C'+128
        .DB     "R",'R'+128
        .DB     "RR",'C'+128
        .DB     "SET",'B'+128
        .DB     "SJM",'P'+128
        .DB     "SUB",'B'+128
        .DB     "SWA",'P'+128
        .DB     "XC",'H'+128
        .DB     "XCH",'D'+128
        .DB     "XR",'L'+128
        .DB     "??",'?'+128,0

bitmnu: .db     'P','0'+128
        .db     "TCO",'N'+128
        .db     'P','1'+128
        .db     "SCO",'N'+128
        .DB     'P','2'+128
        .DB     'I','E'+128
        .DB     'P','3'+128
        .DB     'I','P'+128
        .DB     'C','0'+128
        .DB     "T2CO",'N'+128
        .DB     "PS",'W'+128
        .DB     'D','8'+128
        .DB     "AC",'C'+128
        .DB     'E'+'8'+128
        .DB     'B'+128
        .DB     'F'+'8'+128
        .DB     0

sfrmnu: .db     0xE0,"AC",'C'+128
        .db     0x81,'S','P'+128
        .DB     0x82,"DP",'L'+128
        .DB     0x83,"DP",'H'+128
        .DB     0x80,'P','0'+128
        .DB     0x90,'P','1'+128
        .DB     0xA0,'P','2'+128
        .DB     0xB0,'P','3'+128
        .DB     0x99,"SBU",'F'+128
        .DB     0xCD,"TH",'2'+128
        .DB     0xC8,"T2CO",'N'+128
        .DB     0xCC,"TL",'2'+128
        .DB     0xCB,"RCAP2",'H'+128
        .DB     0xCA,"RCAP2",'L'+128
        .DB     0x8C,"TH",'0'+128
        .DB     0x8A,"TL",'0'+128
        .DB     0x8D,"TH",'1'+128
        .DB     0x8B,"TL",'1'+128
sfr1:   .db     0xF0,'B'+128               ;5
sfr2:   .db     0xD0,"PS",'W'+128          ;7
sfr3:   .DB     0xA8,'I','E'+128
sfr4:   .DB     0xB8,'I','P'+128
sfr5:   .DB     0x89,"TMO",'D'+128         ;8
sfr6:   .DB     0x88,"TCO",'N'+128         ;8
sfr7:   .DB     0x98,"SCO",'N'+128         ;8
sfr8:   .DB     0x87,"PCO",'N'+128         ;8
        .DB     0


mnot:        ;mnunonic offset table (gives offset into above table)

        .db     5Ah,0Eh,48h,73h,2Bh,2Bh,2Bh,2Bh
        .DB     2Bh,2Bh,2Bh,2Bh,2Bh,2Bh,2Bh,2Bh ;INC
        .DB     30h,00h,43h,75h,21h,21h,21h,21h
        .DB     21h,21h,21h,21h,21h,21h,21h,21h ;DEC
        .DB     2Eh,0Eh,67h,6Eh,06h,06h,06h,06h
        .DB     06h,06h,06h,06h,06h,06h,06h,06h ;ADD
        .DB     38h,00h,6Ah,70h,0Ah,0Ah,0Ah,0Ah
        .DB     0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah ;ADDC
        .DB     33h,0Eh,5Dh,5Dh,5Dh,5Dh,5Dh,5Dh
        .DB     5Dh,5Dh,5Dh,5Dh,5Dh,5Dh,5Dh,5Dh ;ORL
        .DB     3Bh,00h,12h,12h,12h,12h,12h,12h
        .DB     12h,12h,12h,12h,12h,12h,12h,12h ;ANL
        .DB     41h,0Eh,8Fh,8Fh,8Fh,8Fh,8Fh,8Fh
        .DB     8Fh,8Fh,8Fh,8Fh,8Fh,8Fh,8Fh,8Fh ;XLR
        .DB     3Eh,00h,5Dh,35h,4Ch,4Ch,4Ch,4Ch
        .DB     4Ch,4Ch,4Ch,4Ch,4Ch,4Ch,4Ch,4Ch ;MOV
        .DB     7Ch,0Eh,12h,4Fh,24h,4Ch,4Ch,4Ch
        .DB     4Ch,4Ch,4Ch,4Ch,4Ch,4Ch,4Ch,4Ch ;MOV
        .DB     4Ch,00h,4Ch,4Fh,80h,80h,80h,80h
        .DB     80h,80h,80h,80h,80h,80h,80h,80h ;SUBB
        .DB     5Dh,0Eh,4Ch,2Bh,57h,92h,4Ch,4Ch
        .DB     4Ch,4Ch,4Ch,4Ch,4Ch,4Ch,4Ch,4Ch ;MOV
        .DB     12h,00h,1Ch,1Ch,15h,15h,15h,15h
        .DB     15h,15h,15h,15h,15h,15h,15h,15h ;CJNE
        .DB     63h,0Eh,19h,19h,84h,88h,88h,88h
        .DB     88h,88h,88h,88h,88h,88h,88h,88h ;XCH
        .DB     60h,00h,78h,78h,1Fh,27h,8Bh,8Bh
        .DB     27h,27h,27h,27h,27h,27h,27h,27h ;DJNZ
        .DB     53h,0Eh,53h,53h,19h,4Ch,4Ch,4Ch
        .DB     4Ch,4Ch,4Ch,4Ch,4Ch,4Ch,4Ch,4Ch ;MOV
        .DB     53h,00h,53h,53h,1Ch,4Ch,4Ch,4Ch
        .DB     4Ch,4Ch,4Ch,4Ch,4Ch,4Ch,4Ch,4Ch ;MOV

bitptr: .db     00h,02h,06h,08h,0Ch,0Eh,10h,12h
        .db     14h,16h,1Bh,1Eh,20h,23h,24h,25h
                                              
opot:        ;opcode offset table (gives #bytes for the instruction
             ;and the number of the routine to print the operands)

        .db     43*4+1,1*4+2,27*4+3,14*4+1        ;00
        .db     14*4+1,17*4+2,18*4+1,18*4+1
        .db     89,89,89,89,89,89,89,89         ;inc
        .db     24*4+3,1*4+2,27*4+3,14*4+1        ;10
        .db     14*4+1,17*4+2,18*4+1,18*4+1
        .db     89,89,89,89,89,89,89,89         ;dec
        .db     24*4+3,1*4+2,43*4+1,14*4+1        ;20
        .db     5*4+2,3*4+2,4*4+1,4*4+1
        .db     9,9,9,9,9,9,9,9                 ;add
        .db     24*4+3,1*4+2,43*4+1,14*4+1        ;30
        .db     5*4+2,3*4+2,4*4+1,4*4+1
        .db     9,9,9,9,9,9,9,9                 ;addc
        .db     25*4+2,1*4+2,6*4+2,7*4+3        ;40
        .db     5*4+2,3*4+2,4*4+1,4*4+1
        .db     9,9,9,9,9,9,9,9                 ;orl
        .db     25*4+2,1*4+2,6*4+2,7*4+3        ;50
        .db     5*4+2,3*4+2,4*4+1,4*4+1
        .db     9,9,9,9,9,9,9,9                 ;anl
        .db     25*4+2,1*4+2,6*4+2,7*4+3        ;60
        .db     5*4+2,3*4+2,4*4+1,4*4+1
        .db     9,9,9,9,9,9,9,9                 ;xrl
        .db     25*4+2,1*4+2,8*4+2,26*4+1        ;70
        .db     5*4+2,7*4+3,36*4+2,33*4+2
        .db     122,122,122,122,122,122,122,122 ;mov
        .db     25*4+2,1*4+2,34,40*4+1        ;80
        .db     19*4+1,32*4+3,33*4+2,33*4+2
        .db     126,126,126,126,126,126,126,126 ;mov
        .db     38*4+3,1*4+2,37*4+2,39*4+1        ;90
        .db     5*4+2,3*4+2,4*4+1,4*4+1
        .db     9,9,9,9,9,9,9,9                 ;subb
        .db     9*4+2,1*4+2,8*4+2,23*4+1        ;A0
        .db     19*4+1,43*4+1,35*4+2,35*4+2
        .db     118,118,118,118,118,118,118,118 ;mov
        .db     9*4+2,1*4+2,16*4+2,15*4+1        ;B0
        .db     11*4+3,10*4+3,13*4+3,13*4+3
        .db     51,51,51,51,51,51,51,51      ;cjne
        .db     17*4+2,1*4+2,16*4+2,15*4+1        ;C0
        .db     14*4+1,3*4+2,4*4+1,4*4+1
        .db     9,9,9,9,9,9,9,9                 ;xch
        .db     17*4+2,1*4+2,16*4+2,15*4+1        ;D0
        .db     14*4+1,21*4+3,4*4+1,4*4+1
        .db     82,82,82,82,82,82,82,82         ;djnz
        .db     41*4+1,1*4+2,4*4+1,4*4+1        ;E0
        .db     14*4+1,3*4+2,4*4+1,4*4+1
        .db     9,9,9,9,9,9,9,9                 ;mov
        .db     42*4+1,1*4+2,34*4+1,34*4+1        ;F0
        .db     14*4+1,6*4+2,34*4+1,34*4+1
        .db     113,113,113,113,113,113,113,113 ;mov

edits1: .db     "\rEditing External RAM...<ESC> to quit\r",0
edits2: .db     "  Editing finished, this location unchanged\r\r",0
dwlds1: .db     "\r\rBegin ascii transmission of "
        .db     "Intel HEX format file, "
        .db     "or <ESC> to abort\r\r",0
dwlds2: .db     "Download aborted by user\r\r",0
dwlds3: .db     "\r\r\r\rDownload completed\r\r",0
runstd: .db     "Run normally",0
runs1:  .db     "\rNow running the program...\r\r",0
runss:  .db     "Run in single step mode",0
sserr1: .db     "\r\rThe single step run feature will not function"
        .db     " unless INT1 (pin #13) is\r"
        .db     "connected to ground or otherwise held low.\r\r",0
ssmsg:  .db     "\rNow running in single step mode:  "
        .db     "<RET>=default, ?=Help\r\r",0
sskip1: .db     "Skipping ------>",0
sskip2: .db     "Next will be -->",0
ssdmps1:.db     "\rLoc:  Internal Ram Memory Contents\r",0
chaccs1:.db     "New Acc Value: ",0
squit:  .db     "\rQuit single step mode, now running normally.\r\r",0
sdptr:  .db     "DPTR",0
CMD_TBL:.DB     '?'
        .DW     CMD_hlp
        .DW     HELP
        .db     'R'
        .dw     CMD_run
        .dw     run
        .db     'D'                                      
        .dw     CMD_dwl
        .dw     download
        .db     'N'
        .dw     CMD_new
        .dw     new_loc
        .db     'H'
        .dw     CMD_dmp
        .dw     dump
        .db     'L'
        .dw     CMD_lst
        .dw     list
        .db     'E'
        .dw     CMD_edt
        .dw     edit
        .DB     00h
CMD_run:.db     " Run program",0
CMD_new:.db     " New memory location",0
CMD_dmp:.db     " HEX Dump Memory to the Screen",0
CMD_lst:.db     " List assembly code",0
CMD_hlp:.DB     " Help???",0
CMD_dwl:.Db     " Download program from PC",0
CMD_edt:.db     " Edit external ram",0
Help1txt:
        .db     12,"These commands are currently "
        .db     "supported:\r\r",0
help2txt:
        ;        1234567890123456789012345678901234567890
        .db     "\rAll numerical values are shown in hex.  "
           
        .db     "Pressing <ESC> key will exit the\r"
        .db     "current command, even while listing/dumping.  "
        .db     "Most prompts require a\r"
        .db     "single character, typically the first letter "
        .db     "of the desired option.\r\rFor information "
        .db     "regarding the single-step run feature,\r"
        .db     "type '?' when asked 'Single-step/Normal' "
        .db     "before running the program.\r\r",0
Help3txt:
        ;        1234567890123456789012345678901234567890
        .db  12,"The single step run feature allows you "
        .db     "to execute your program from memory\r"          ;1
        .db     "one instruction at a time, while monitoring "
        .db     "the registers and instructions.\r"              ;2
        .db     "It it NOT a simulation, the program is "
        .db     "executed by the 8031/51 processor.\r"           ;3
        .db     "External Interrupt #1 must be held low "
        .db     "to make the single step function.\r"            ;4
        .db     "\r"                                             ;5
        .db     "Despite attempts to make the single "
        .db     "step run compatible with all programs,\r"       ;6
        .db     "there will always be some basic limitations "
        .db     "due to its nature:\r\r"                         ;7 8
        .db     " 1- External Interrupt #1 must not be"
        .db     " disabled, e.g. MOV IE,#81h\r"                  ;9
        .db     " 2- Timer #1 must be correctly generating "
        .db     " the baud rate for the serial port\r"           ;10
        .db     " 3- TI and RI will not work normally, "
        .db     "e.g. 2003: JNB RI,2003, use skip...\r"          ;11
        .db     " 4- Interrupts will not get service " 
        .db     "or will interrupt the single step\r"            ;12
        .db     " 5- About 30 bytes of space must be "
        .db     "left available on the stack!\r"                 ;13
        .db     " 6- ???  Other problems may also "
        .db     "exist, (this program is FREE, you know)\r"      ;14
        ;        1234567890123456789012345678901234567890
        .db     "\r"                                             ;15
        .db     "Perhaps the worst limitation of the "
        .db     "single step run is that it takes a\r"           ;16
        .db     "very long time to execute even short "
        .db     "pieces of code.  It is recommended that\r"      ;17
        .db     "a normal run be attempted first to "
        .db     "estimate where the program goes astray,\r"      ;18
        .db     "the attempt a single step at the beginning "
        .db     "of the questionable code, with a\r"             ;19
        .db     "hardcopy of the assembly listing file "
        .db     "on-hand for memory location reference.\r\r",0   ;20 21


        ;        1234567890123456789012345678901234567890 
Help4txt:
        .db  12,"Between steps, the monitor uses absolutely "
        .db     "NO internal or external memory.\r"
        .db     "However, about 30 bytes of stack space "
        .db     "must be left available...\r\r"
        .db     "During a single step run, pressing RETURN "
        .db     "repeatedly will be the usual course\r"
        .db     "of action.  However, other options are "
        .db     "available.\r\r"
        .db     "            "
        .db     "Typing '?' will display this help screen\r\r",0

Help5txt:
        .db     "Single Step Commands:\r\r"
        .db     " <RET>    Print Status and execute "
        .db                   "the next instruction\r"
        .db     " <SPACE>  Execute next instruction w/out status lines\r"
        .db     "  '?'     Display this on-line help\r"
        .db     "  'R'     Print out Special Function Registers\r"
        .db     "  'H'     Hex dump internal ram\r"
        .db     "  'S'     Skip this instruction\r"
        .db     "  'A'     Change the Accumulator's value\r"
        .db     "  'Q'     Quit Single Step, continue executing normally\r"
        .db     "\r",0

