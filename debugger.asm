.equ    IEN, 0xA8
.equ    SDICMD, 0x8D

.equ    SDDL, 0x96
.equ    SDDH, 0x97

.equ    SDBL, 0x93
.equ    SDBH, 0x94

.equ    SDODMAADDRL, 0x99
.equ    SDODMAADDRH, 0x9A

.equ    SDODMABYTESL, 0x91
.equ    SDODMABYTESH, 0x92

.equ    SDOS, 0x88
.equ    SDDIR, 0xEB
.equ    SDSM, 0x90

.equ    RESET, 0
.equ    PORT1, 0xF6

.equ    SD_WAITING, 0x24

.org 0x2900

; This gets called from an interrupt.
start:
        ljmp    reset_vector


setup_outputs:
        mov     0xd8, #0xE8 ; 'F'
        mov     0xEB, 0xFC

        anl     0x8E, #0xFE ; Acknowledge
        anl     0x8E, #0xFD ; all
        anl     0x8E, #0xFB ; possible
        anl     0x8E, #0xF7 ; interrupts
        anl     0x80, #0x7F
        orl     0x80, #4
        orl     0x80, #8
        ret

; ---------------------------------------------------------------------------
; This gets called from an interrupt context.  Manipulate the stack so that
; when we return from intterupt, code execution will continue at an address
; immediately following (i.e. 0x2932).
reset_vector:
        anl     0x80, #0xFE
        orl     0x80, #2

        pop     PSW             ; Program Status Word
        pop     ACC             ; Accumulator

        mov     A, #0x32
        push    ACC             ; Accumulator
        mov     A, #0x29
        push    ACC             ; Accumulator

        reti
; ---------------------------------------------------------------------------

.org 0x2932
        mov     IEN, #0      ; Interrupt Enable Register 0
        mov     SP, #0x80     ; Stack Pointer
;        acall   setup_outputs
        mov     SDDIR, #0xff
        sjmp    do_things

.org 0x293f
; ---------------------------------------------------------------------------
; SD interrupt handler.  Called from an interrupt context.
        push    ACC
        push    0xD2
        push    DPH
        push    DPL
        push    PSW
        mov     PSW, #8
        mov     0xd2, #0
        anl     0x8E, #0xFE
        clr     0x24.3
        mov     0x30, 0x8D      ;SD incoming command
        mov     A, 0x30
        subb    A, #0x38

        lcall   wait_for_packet

        orl     SDSM, #1
        ljmp    exit_sdi_isr
; ---------------------------------------------------------------------------
    


memsetx: ; memsetx(addr[l], addr[h], val, count)
        mov     DPL, R0
        mov     DPH, R1
        mov     A, R2
        inc     R3      ; Get djnz to work

memsetx_loop:
        movx    @DPTR, A
        inc     DPTR
        djnz    R3, memsetx_loop
        ret



do_things:
        mov     0x40, #0xff

        ; Set up ISR to call function at 0x293f
        mov     DPTR, #0x0200
        lcall   set_isr
        mov     DPTR, #0x0203
        lcall   set_isr
;        mov     DPTR, #0x0206
;        lcall   set_isr
;        mov     DPTR, #0x0209
;        lcall   set_isr


        clr     EA
        acall   setup1
        acall   setup2
        acall   setup3
        setb    EA

        mov     DPTR, #0x0100
        mov     A, #0x40
        mov     R0, #0x10
        acall   set_ram_values

        mov     B, #0x48
main_loop:
;        inc     B
;        mov     R0, #0
;        mov     R1, #0
;        mov     R2, B
;        mov     R3, #0xff
;        lcall   memsetx

;        inc     B
;        mov     R0, #0
;        mov     R1, #1
;        mov     R2, B
;        mov     R3, #0xff
;        lcall   memsetx

        lcall   pause_a_while

        mov     DPTR, #0x0100
        mov     R0, #0x10
        lcall   inc_ram_values

        mov     DPTR, #0x0100
        mov     A, 0x8d
        movx    @DPTR, A

        inc     DPTR
        mov     A, 0x8b
        movx    @DPTR, A

        inc     DPTR
        mov     A, 0x8a
        movx    @DPTR, A

        inc     DPTR
        mov     A, 0x89
        movx    @DPTR, A

        xrl     0x40, #0xff
        mov     PORT1, 0x40    ; Turn LEDs off

        mov     SDDL, #0x20     ; SD Outgoing Address>>2
        mov     SDDH, #0x00     ; SD Outgoing Address

        mov     SDBL, #0x05     ; SD Outgoing bytes (low)
        mov     SDBH, #0        ; SD Outgoing bytes (high)

        mov     SDOS, #0x71      ; Kick off the transfer


        clr     SD_WAITING.1
wait_sd_cmd:
        jnb     SD_WAITING.1, wait_sd_cmd
        sjmp    main_loop


; pause_a_while(while1, while2, while3)
; R5 [in]: "Small" amount to wait
; R6 [in]: "Medium" amount to wait
; R7 [in]: "Large" amount to wait
; Will count from 0 to R7R6R5.  Every time a smaller value underflows, its
; larger neighbor will decrement.  E.g. if R5 underflows from 0 to 255, R6
; will be decremented.  This will conitnue until all values are zero.
pause_a_while:
        mov     R5, #0x00
        mov     R6, #0x80
        mov     R7, #0x01
top_of_pause:
        djnz    R5, top_of_pause                      
        djnz    R6, top_of_pause
        djnz    R7, top_of_pause
        ret


setup1:
        mov     0xd1, #0xe8
        orl     0xe9, #1
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        orl     0xe9, #0x80
        ret

setup2:
        orl     0x9b, #0x8
        lcall   their_pause_a_while
        anl     0x9b, #0xf7
        anl     0x8e, #0xfe
        anl     0x8e, #0xfd
        anl     0x8e, #0xfb
        anl     0x8e, #0xf7

        orl     0x80, #0x04
        orl     0x80, #0x08
        anl     0x80, #0x7f

        setb    0x98.3
        mov     IEN, #0x8f
        clr     0x98.3
        ret

setup3:
        acall   setup31
        acall   setup32
        acall   setup33
        ret

setup31:
        mov     0xA0, #0
        setb    0xA0.5
        setb    0xA0.4
        orl     0xF3, #0x10
        setb    0xA0.2
        clr     0xA0.3
        mov     0x9E, #1
        mov     0x9F, #0x28
        mov     0xA4, #0xff
        ret

setup32:
        mov     0xA1, #1
setup32_loop:
        jnb     0xA0.7, setup32_loop
        clr     0xA0.7
        ret

setup33:
        ret

their_pause_a_while:
        mov     R7, #1
their_pause_a_while_1:
        mov     R6, #0x14
their_pause_a_while_2:
        mov     R5, #0x14
their_pause_a_while_3:
        djnz    R5, their_pause_a_while_3
        djnz    R6, their_pause_a_while_2
        djnz    R7, their_pause_a_while_1
        ret


set_isr:
        mov     A, #0x02
        movx    @DPTR, A

        inc     DPTR
        mov     A, #0x29
        movx    @DPTR, A

        inc     DPTR
        mov     A, #0x3f
        movx    @DPTR, A

        ret


wait_for_packet:
        mov     A, SDSM
        anl     A, #0xC
        jnz     wait_for_packet
        ret

exit_sdi_isr:
        pop     PSW
        pop     DPL
        pop     DPH
        pop     0xD2
        pop     ACC
        xrl     0x40, #0xff
        mov     PORT1, 0x40
        setb    SD_WAITING.1
        reti

set_ram_values:
        movx    @DPTR, A
        inc     A
        inc     DPTR
        djnz    R0, set_ram_values
        ret

inc_ram_values:
        movx    A, @DPTR
        inc     A
        anl     A, #0x7f
        jnz     dont_add_twenty
        add     A, #0x20
dont_add_twenty:
        movx    @DPTR, A
        inc     DPTR
        djnz    R0, inc_ram_values
        ret
