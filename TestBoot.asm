.equ    IEN0, 0xA8
.equ    SDICMD, 0x8D

.equ    SDOADDRL, 0x96
.equ    SDOADDRH, 0x97

.equ    SDOBYTESL, 0x93
.equ    SDOBYTESH, 0x94

.equ    SDODMAADDRL, 0x99
.equ    SDODMAADDRH, 0x9A

.equ    SDODMABYTESL, 0x91
.equ    SDODMABYTESH, 0x92

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

ROM_2903:                               ; CODE XREF: ROM:2938
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
                acall   ROM_2903

ROM_293A:                               ; CODE XREF: ROM:293E
                                        ; ROM:2953 ...
                clr     0x24.0
                mov     A, FSR_8E
                jnb     ACC.0, ROM_293A  ; Accumulator
                anl     FSR_8E, #0xFE

ROM_2944:                               ; CODE XREF: ROM:2948
                mov     A, FSR_90
                anl     A, #0xC
                jnz     ROM_2944
                mov     A, SDICMD       ; SD Incoming Command
                xrl     A, #0xB
                jz      ROM_2956
                orl     FSR_90, #1
                ljmp    ROM_293A
; ---------------------------------------------------------------------------

ROM_2956:                               ; CODE XREF: ROM:294E
                anl     FSR_8E, #0xFD
                mov     SDOADDRL, #0x74 ; 't' ; SD Outgoing Address / 4 (low)
                mov     SDOADDRH, #5    ; SD Outgoing Address / 4 (high)
                mov     SDOBYTESL, #3   ; SD Outgoing bytes (low)
                mov     SDOBYTESH, #0   ; SD Outgoing bytes (high)
                mov     0x88, #1
                mov     R7, #0
                mov     R6, #0

ROM_296C:                               ; CODE XREF: ROM:29A0
                                        ; ROM:29A8
                anl     FSR_8E, #0xFB
                mov     SDODMAADDRL, R7 ; SD Outgoing DMA Address (low)
                mov     SDODMAADDRH, R6 ; SD Outgoing DMA Address (high)
                clr     C
                mov     A, R7
                add     A, #0x40 ; '@'
                mov     R7, A
                mov     A, R6
                addc    A, #0
                mov     R6, A
                mov     SDODMABYTESL, #0xFF ; SD Outgoing DMA Bytes (low)
                mov     SDODMABYTESH, #1 ; SD Outgoing DMA Bytes (high)
                orl     FSR_80, #1

ROM_2985:                               ; CODE XREF: ROM:2987
                mov     A, FSR_8E
                jnb     ACC.2, ROM_2985  ; Accumulator
                mov     A, FSR_80
                anl     A, #0x20
                jnz     ROM_29AB
                mov     A, FSR_80
                anl     A, #0x10
                jnz     ROM_29AB
                mov     A, FSR_80
                jb      ACC.6, ROM_29A3  ; Accumulator
                clr     0x24.0
                orl     FSR_80, #2
                ljmp    ROM_296C
; ---------------------------------------------------------------------------

ROM_29A3:                               ; CODE XREF: ROM:2998
                setb    0x24.0
                orl     FSR_80, #2
                ljmp    ROM_296C
; ---------------------------------------------------------------------------

ROM_29AB:                               ; CODE XREF: ROM:298E
                                        ; ROM:2994
                anl     FSR_8E, #0xFE
                anl     FSR_8E, #0xFD
                anl     FSR_8E, #0xFB

ROM_29B4:                               ; CODE XREF: ROM:29B8
                mov     A, FSR_90
                anl     A, #0xC
                jnz     ROM_29B4
                mov     SDOADDRL, #0x74 ; 't' ; SD Outgoing Address / 4 (low)
                mov     SDOADDRH, #5    ; SD Outgoing Address / 4 (high)
                mov     SDOBYTESL, #3   ; SD Outgoing bytes (low)
                mov     SDOBYTESH, #0   ; SD Outgoing bytes (high)
                mov     FSR_88, #1

ROM_29C9:                               ; CODE XREF: ROM:29CB
                mov     A, FSR_8E
                jnb     ACC.1, ROM_29C9  ; Accumulator
                anl     FSR_8E, #0xFD
                anl     FSR_80, #0xFE
                orl     FSR_80, #2
                jnb     0x24.0, ROM_29DD
                ljmp    ROM_293A
; ---------------------------------------------------------------------------

ROM_29DD:                               ; CODE XREF: ROM:29D7
                mov     SP, #0x80       ; Stack Pointer
                ljmp    RESET           ; Power-on program execution start
