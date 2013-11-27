.equ    IEN0, 0xA8

.equ    RESET, 0

.equ    reg1, 0xef
.equ    reg2, 0x99
.equ    reg3, 0xb4
.equ    reg4, 0x84

.equ    sleep1, 0
.equ    sleep2, 128
.equ    sleep3, 1

.org 0x7b00

; This gets called from an interrupt.
;start:
;        anl     0x80, #0xFE
;        orl     0x80, #2
;        pop     PSW             ; Program Status Word
;        pop     ACC             ; Accumulator
;        mov     A, #0x20 ; '2'
;        push    ACC             ; Accumulator
;        mov     A, #0x7b ; ')'
;        push    ACC             ; Accumulator
;        reti
;; ---------------------------------------------------------------------------
;.org 0x7b20
;        mov     IEN0, #0      ; Interrupt Enable Register 0
;        mov     SP, #0x80     ; Stack Pointer


start:
        mov     reg1, #0x00
;        mov     reg2, #0x6f
;        mov     reg3, #0xe5
;        mov     reg4, #0x7d

pause_a_while1:
        mov     R2, #sleep1
        mov     R3, #sleep2
        mov     R4, #sleep3
top_of_pause1:
        djnz    R2, top_of_pause1
        djnz    R3, top_of_pause1
        djnz    R4, top_of_pause1


        mov     reg1, #0xff
;        mov     reg2, #0xc7
;        mov     reg3, #0x7f
;        mov     reg4, #0x3d

pause_a_while2:
        mov     R2, #sleep1
        mov     R3, #sleep2
        mov     R4, #sleep3
top_of_pause2:
        djnz    R2, top_of_pause2
        djnz    R3, top_of_pause2
        djnz    R4, top_of_pause2


        sjmp    start
