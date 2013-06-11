.equ    IEN0, 0xA8
.equ    SDICMD, 0x8D

.equ    SDOSTATE, 0x98

.equ    SDOADDRL, 0x96
.equ    SDOADDRH, 0x97

.equ    SDOBYTESL, 0x93
.equ    SDOBYTESH, 0x94

.equ    SDODMAADDRL, 0x99
.equ    SDODMAADDRH, 0x9A

.equ    SDODMABYTESL, 0x91
.equ    SDODMABYTESH, 0x92

.equ    SDOS, 0x88
.equ    SDDIR, 0xEB
.equ    FSR_80, 0x80
.equ    FSR_8E, 0x8E
.equ    FSR_90, 0x90
.equ    FSR_D8, 0xD8
.equ    FSR_EB, 0xEB
.equ    FSR_FC, 0xFC

.equ    RESET, 0

.org 0x2900

; This gets called from an interrupt.
start:
        ljmp    reset_vector


setup_outputs:
        mov     FSR_D8, #0xE8 ; 'F'
        mov     FSR_EB, FSR_FC
        anl     FSR_8E, #0xFE
        anl     FSR_8E, #0xFD
        anl     FSR_8E, #0xFB
        anl     FSR_8E, #0xF7
        anl     FSR_80, #0x7F
        orl     FSR_80, #4
        orl     FSR_80, #8
        ret
; ---------------------------------------------------------------------------
reset_vector:                            ; CODE XREF: ROM:2900
        anl     FSR_80, #0xFE
        orl     FSR_80, #2
        pop     PSW             ; Program Status Word
        pop     ACC             ; Accumulator
        mov     A, #0x32 ; '2'
        push    ACC             ; Accumulator
        mov     A, #0x29 ; ')'
        push    ACC             ; Accumulator
        reti
; ---------------------------------------------------------------------------


        mov     IEN0, #0      ; Interrupt Enable Register 0
        mov     SP, #0x80     ; Stack Pointer
        acall   setup_outputs
        mov     SDDIR, #0xff
        sjmp    do_things


pause_a_while:
        mov     R2, #0x00
        mov     R3, #0
        mov     R4, #0x02
top_of_pause:
        djnz    R2, top_of_pause                      
        djnz    R3, top_of_pause
        djnz    R4, top_of_pause
        ret

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


;code_to_ext: ; code_to_ext(src[l], src[h], dst[l], dst[h], count)
;        inc     R4
;
;code_to_ext_loop:
;        mov     DPL, R0
;        mov     DPH, R1
;        clr     A
;        movc    A, @A+DPTR
;        mov     DPL, R2
;        mov     DPH, R3
;        movx    @DPTR, A
;
;        inc     R0
;        mov     A, R0
;        jnz     code_to_ext_skip_inc_r1
;        inc     R1
;code_to_ext_skip_inc_r1:
;
;        inc     R2
;        mov     A, R2
;        jnz     code_to_ext_skip_inc_r3
;        inc     R3
;code_to_ext_skip_inc_r3:
;
;        djnz    R4, code_to_ext_loop
;        ret


memcpy_to_zero: ; memcpy_to_zero(src[l], src[h], page, count)
        inc     R3
        mov     R7, #0

memcpy_to_zero_one_byte:
        mov     DPL, R0
        mov     DPH, R1
        movx    A, @DPTR

        mov     DPL, R7
        mov     DPH, R2
        movx    @DPTR, A

        inc     R7
        inc     R0
        mov     A, R0
        jnz     memcpy_to_zero_skip_inc_r1
        inc     R1
memcpy_to_zero_skip_inc_r1:
        djnz    R3, memcpy_to_zero_one_byte
        ret


do_things:
;        mov     R0, #0
;        mov     R1, #0
;        mov     R2, #0x77
;        mov     R3, #0xff
;        lcall   memsetx
;
;        mov     R0, #0x00
;        mov     R1, #1
;        mov     R2, #0x77
;        mov     R3, #0xff
;        lcall   memsetx
;
;        mov     R0, #0
;        mov     R1, #0
;        mov     R2, #0x00
;        mov     R3, #0
;        mov     R4, #0xff
;        lcall   code_to_ext
        mov     R0, #0x00
        mov     R1, #0xa5
        mov     R2, #0
        mov     R3, #0xff
        lcall   memcpy_to_zero

        mov     R0, #0x00
        mov     R1, #0xa6
        mov     R2, #1
        mov     R3, #0xff
        lcall   memcpy_to_zero

        lcall   pause_a_while
        mov     SDOADDRL, #0x00     ; SD Outgoing Address>>2
        mov     SDOADDRH, #0x01     ; SD Outgoing Address
        mov     SDOBYTESL, #0xff    ; SD Outgoing bytes (low)
        mov     SDOBYTESH, #1       ; SD Outgoing bytes (high)
        mov     SDOS, #0x71            ; Kick off the transfer

;        mov     SDODMAADDRL, #0    ; SD Outgoing DMA Address (low)
;        mov     SDODMAADDRH, #0    ; SD Outgoing DMA Address (high)
;        mov     SDODMABYTESL, #7   ; SD Outgoing DMA Bytes (low)
;        mov     SDODMABYTESH, #0   ; SD Outgoing DMA Bytes (high)
;        orl     SDOSTATE, #1       ; SD Outgoing State

inf_loop:
        sjmp    inf_loop
