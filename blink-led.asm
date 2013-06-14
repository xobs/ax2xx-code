.equ    IEN0, 0xA8

.equ    SDICMD, 0x8D

.equ    SDODMABYTESL, 0x91
.equ    SDODMABYTESH, 0x92

.equ    SDOADDRL, 0x96
.equ    SDOADDRH, 0x97

.equ    SDOBYTESL, 0x93
.equ    SDOBYTESH, 0x94

.equ    SDOSTATE, 0x98

.equ    SDODMAADDRL, 0x99
.equ    SDODMAADDRH, 0x9A

.equ    FSR_80, 0x80
.equ    FSR_88, 0x88
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

; ---------------------------------------------------------------------------

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
.org 0x2932
                mov     IEN0, #0      ; Interrupt Enable Register 0
                mov     SP, #0x80     ; Stack Pointer
                acall   setup_outputs
                sjmp    blink_led

pause_a_while:
                mov     R2, #0
                mov     R3, #0
                mov     R4, #0x04
top_of_pause:
                djnz    R2, top_of_pause
                djnz    R3, top_of_pause
                djnz    R4, top_of_pause
                ret


blink_led:
                mov     0xf6, #0xff
                acall   pause_a_while
                mov     0xf6, #0
                acall   pause_a_while
                ajmp    blink_led
