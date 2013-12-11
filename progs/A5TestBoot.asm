.equ	RESET,	0
.equ	RAM_20, 0x20
.equ	RAM_21, 0x21
.equ	RAM_22, 0x22
.equ	RAM_23, 0x23
.equ    RAM_24, 0x24
.equ	SDSM,	0x90
.equ	ER20,	0xD8
.equ	ER21,	0xD9
.equ	ER22,	0xDA
.equ	ER23,	0xDB
.equ	ER30,	0xF8
.equ	ER31,	0xF9
.equ	ER32,	0xFA
.equ	ER33,	0xFB
.equ    SFR_80, 0x80
.equ    SFR_81, 0x81
.equ    SFR_82, 0x82
.equ    SFR_83, 0x83
.equ    SFR_84, 0x84
.equ    SFR_85, 0x85
.equ    SFR_86, 0x86
.equ    SFR_87, 0x87
.equ    SFR_88, 0x88
.equ    SFR_89, 0x89
.equ    SFR_8A, 0x8A
.equ    SFR_8B, 0x8B
.equ    SFR_8C, 0x8C
.equ    SFR_8D, 0x8D
.equ    SFR_8E, 0x8E
.equ    SFR_8F, 0x8F
.equ    SFR_90, 0x90
.equ    SFR_91, 0x91
.equ    SFR_92, 0x92
.equ    SFR_93, 0x93
.equ    SFR_94, 0x94
.equ    SFR_95, 0x95
.equ    SFR_96, 0x96
.equ    SFR_97, 0x97
.equ    SFR_98, 0x98
.equ    SFR_99, 0x99
.equ    SFR_9A, 0x9A
.equ    SFR_9B, 0x9B
.equ    SFR_9C, 0x9C
.equ    SFR_9D, 0x9D
.equ    SFR_9E, 0x9E
.equ    SFR_9F, 0x9F
.equ    SFR_A0, 0xA0
.equ    SFR_A1, 0xA1
.equ    SFR_A2, 0xA2
.equ    SFR_A3, 0xA3
.equ    SFR_A4, 0xA4
.equ    SFR_A5, 0xA5
.equ    SFR_A6, 0xA6
.equ    SFR_A7, 0xA7
.equ    SFR_A8, 0xA8
.equ    SFR_A9, 0xA9
.equ    SFR_AA, 0xAA
.equ    SFR_AB, 0xAB
.equ    SFR_AC, 0xAC
.equ    SFR_AD, 0xAD
.equ    SFR_AE, 0xAE
.equ    SFR_AF, 0xAF
.equ    SFR_B0, 0xB0
.equ    SFR_B1, 0xB1
.equ    SFR_B2, 0xB2
.equ    SFR_B3, 0xB3
.equ    SFR_B4, 0xB4
.equ    SFR_B5, 0xB5
.equ    SFR_B6, 0xB6
.equ    SFR_B7, 0xB7
.equ    SFR_B8, 0xB8
.equ    SFR_B9, 0xB9
.equ    SFR_BA, 0xBA
.equ    SFR_BB, 0xBB
.equ    SFR_BC, 0xBC
.equ    SFR_BD, 0xBD
.equ    SFR_BE, 0xBE
.equ    SFR_BF, 0xBF
.equ    SFR_C0, 0xC0
.equ    SFR_C1, 0xC1
.equ    SFR_C2, 0xC2
.equ    SFR_C3, 0xC3
.equ    SFR_C4, 0xC4
.equ    SFR_C5, 0xC5
.equ    SFR_C6, 0xC6
.equ    SFR_C7, 0xC7
.equ    SFR_C8, 0xC8
.equ    SFR_C9, 0xC9
.equ    SFR_CA, 0xCA
.equ    SFR_CB, 0xCB
.equ    SFR_CC, 0xCC
.equ    SFR_CD, 0xCD
.equ    SFR_CE, 0xCE
.equ    SFR_CF, 0xCF
.equ    SFR_D0, 0xD0
.equ    SFR_D1, 0xD1
.equ    SFR_D2, 0xD2
.equ    SFR_D3, 0xD3
.equ    SFR_D4, 0xD4
.equ    SFR_D5, 0xD5
.equ    SFR_D6, 0xD6
.equ    SFR_D7, 0xD7
.equ    SFR_D8, 0xD8
.equ    SFR_D9, 0xD9
.equ    SFR_DA, 0xDA
.equ    SFR_DB, 0xDB
.equ    SFR_DC, 0xDC
.equ    SFR_DD, 0xDD
.equ    SFR_DE, 0xDE
.equ    SFR_DF, 0xDF
.equ    SFR_E0, 0xE0
.equ    SFR_E1, 0xE1
.equ    SFR_E2, 0xE2
.equ    SFR_E3, 0xE3
.equ    SFR_E4, 0xE4
.equ    SFR_E5, 0xE5
.equ    SFR_E6, 0xE6
.equ    SFR_E7, 0xE7
.equ    SFR_E8, 0xE8
.equ    SDI1, 0xE9
.equ    SDI2, 0xEA
.equ    SDI3, 0xEB
.equ    SDI4, 0xEC
.equ    SFR_ED, 0xED
.equ    SFR_EE, 0xEE
.equ    SFR_EF, 0xEF
.equ    SFR_F0, 0xF0
.equ    SFR_F1, 0xF1
.equ    SFR_F2, 0xF2
.equ    SFR_F3, 0xF3
.equ    SFR_F4, 0xF4
.equ    SFR_F5, 0xF5
.equ    SFR_F6, 0xF6
.equ    SFR_F7, 0xF7
.equ    SFR_F8, 0xF8
.equ    SFR_F9, 0xF9
.equ    SFR_FA, 0xFA
.equ    SFR_FB, 0xFB
.equ    SFR_FC, 0xFC
.equ    SFR_FD, 0xFD
.equ    SFR_FE, 0xFE
.equ    SFR_FF, 0xFF

.org 0x4700
entry:
                ljmp    relocate_pc

; =============== S U B R O U T I N E =======================================


maybe_set_spi_pins:                     ; CODE XREF: ROM:7B23p
                clr     SFR_A0.2
                anl     SFR_E8, #0xF0
                orl     SFR_DC, #4
                orl     SFR_DC, #8
                ret
; End of function maybe_set_spi_pins

; ---------------------------------------------------------------------------

relocate_pc:                            ; CODE XREF: ROM:entryj
                anl     SFR_DC, #0xFE
                mov     A, #0x1B
                push    ACC             ; Accumulator
                mov     A, #0x7B ; '{'
                push    ACC             ; Accumulator
                reti
; ---------------------------------------------------------------------------

non_irq_entry:                          ; Disable interrupts
                mov     IE, #0
                mov     SP, #SFR_80     ; Reset stack pointer
                setb    RAM_24.0
                acall   maybe_set_spi_pins

wait_for_spi_ready:                     ; CODE XREF: ROM:7B29j
                                        ; ROM:7B5Aj ...
                mov     R5, #0
                mov     A, SFR_E8
                jnb     ACC.0, wait_for_spi_ready ; Accumulator
                anl     SFR_E8, #0xFE

ROM_472F:                               ; CODE XREF: ROM:ROM_472Fj
                jnb     SDSM.4, ROM_472F ; SD state machine
                mov     A, SFR_E6       ; SD Incoming Command
                jz      ROM_4756
                mov     A, SFR_E6       ; SD Incoming Command
                xrl     A, #0x14
                jz      ROM_47B2
                mov     A, SFR_E6       ; SD Incoming Command
                xrl     A, #0x32
                jz      ROM_475C
                mov     A, SFR_E6       ; SD Incoming Command
                xrl     A, #0x33
                jz      ROM_4760
                mov     A, SFR_E6       ; SD Incoming Command
                xrl     A, #9
                jz      ROM_4764
                mov     A, SFR_E6       ; SD Incoming Command
                xrl     A, #0x35
                jz      ROM_4768
                ljmp    ROM_4885
; ---------------------------------------------------------------------------

ROM_4756:                               ; CODE XREF: ROM:7B34j
                clr     RAM_24.0
                acall   xmit_something
                ajmp    wait_for_spi_ready
; ---------------------------------------------------------------------------

ROM_475C:                               ; CODE XREF: ROM:7B40j
                lcall   ROM_48A1
                ajmp    wait_for_spi_ready
; ---------------------------------------------------------------------------

ROM_4760:                               ; CODE XREF: ROM:7B46j
                lcall   ROM_48C6
                ajmp    wait_for_spi_ready
; ---------------------------------------------------------------------------

ROM_4764:                               ; CODE XREF: ROM:7B4Cj
                lcall   ROM_4832
                ajmp    wait_for_spi_ready
; ---------------------------------------------------------------------------

ROM_4768:                               ; CODE XREF: ROM:7B52j
                anl     SFR_E8, #0xFD
                mov     RAM_20, #0
                mov     SFR_D6, #0xE8 ; 'F' ; SD Outgoing Address / 4 (low)
                mov     SFR_D7, #0x1F   ; SD Outgoing Address / 4 (high)
                jb      RAM_24.0, ROM_4782
                mov     SFR_DE, #0
                mov     SFR_DF, #0
                mov     SFR_E7, #0xF1 ; 'Â±' ; SD output state
                sjmp    ROM_478E
; ---------------------------------------------------------------------------

ROM_4782:                               ; CODE XREF: ROM:7B74j
                mov     RAM_23, #0
                mov     SFR_DE, #3
                mov     SFR_DF, #0
                mov     SFR_E7, #1      ; SD output state

ROM_478E:                               ; CODE XREF: ROM:7B80j
                                        ; ROM:7B90j
                mov     A, SFR_E8
                jnb     ACC.1, ROM_478E  ; Accumulator
                anl     SFR_E8, #0xFD
                mov     SP, #SFR_80     ; Stack Pointer
                ljmp    RESET           ; Power-on program execution start

; =============== S U B R O U T I N E =======================================


xmit_something:                         ; CODE XREF: ROM:7B58p
                anl     SFR_E8, #0xFD
                mov     RAM_20, #0
                mov     SFR_D6, #0xE8 ; 'F' ; SD Outgoing Address / 4 (low)
                mov     SFR_D7, #0x1F   ; SD Outgoing Address / 4 (high)
                mov     SFR_DE, #0
                mov     SFR_DF, #0
                mov     SFR_E7, #0xF1 ; 'Â±' ; SD output state
                ret
; End of function xmit_something

; ---------------------------------------------------------------------------

ROM_47B2:                               ; CODE XREF: ROM:7B3Aj
                jnb     SDSM.0, ROM_47BA ; SD state machine
                orl     SFR_E3, #1
                ajmp    wait_for_spi_ready
; ---------------------------------------------------------------------------

ROM_47BA:                               ; CODE XREF: ROM:ROM_47B2j
                anl     SFR_E8, #0xFD
                mov     SFR_D6, #0xE8 ; 'F' ; SD Outgoing Address / 4 (low)
                mov     SFR_D7, #0x1F   ; SD Outgoing Address / 4 (high)
                mov     SFR_DE, #3
                mov     SFR_DF, #0
                mov     SFR_E7, #1      ; SD output state
                mov     R7, #0
                mov     R6, #0

ROM_47D0:                               ; CODE XREF: ROM:7C02j
                                        ; ROM:7C08j
                anl     SFR_E8, #0xFB
                mov     SFR_D4, R7
                mov     SFR_D5, R6
                clr     C
                mov     A, R7
                add     A, #0x80 ; 'Ã'
                mov     R7, A
                mov     A, R6
                addc    A, #0
                mov     R6, A
                mov     SFR_E1, #0xFF
                mov     SFR_E2, #1
                orl     SFR_DC, #1

ROM_47E9:                               ; CODE XREF: ROM:7BEBj
                mov     A, SFR_E8
                jnb     ACC.2, ROM_47E9  ; Accumulator
                mov     A, SFR_DC
                anl     A, #0x20
                jnz     ROM_480A
                mov     A, SFR_DC
                anl     A, #0x10
                jnz     ROM_480A
                mov     A, SFR_DC
                jb      ACC.6, ROM_4804  ; Accumulator
                orl     SFR_DC, #2
                ljmp    ROM_47D0
; ---------------------------------------------------------------------------

ROM_4804:                               ; CODE XREF: ROM:7BFCj
                inc     R5
                orl     SFR_DC, #2
                ljmp    ROM_47D0
; ---------------------------------------------------------------------------

ROM_480A:                               ; CODE XREF: ROM:7BF2j
                                        ; ROM:7BF8j
                anl     SFR_E8, #0xF8

ROM_480D:                               ; CODE XREF: ROM:7C11j
                mov     A, SFR_E3
                anl     A, #0xC
                jnz     ROM_480D
                mov     SFR_D6, #0xE8 ; 'F' ; SD Outgoing Address / 4 (low)
                mov     SFR_D7, #0x1F   ; SD Outgoing Address / 4 (high)
                mov     SFR_DE, #3
                mov     SFR_DF, #0
                mov     SFR_E7, #1      ; SD output state

ROM_4822:                               ; CODE XREF: ROM:7C24j
                mov     A, SFR_E8
                jnb     ACC.1, ROM_4822  ; Accumulator
                anl     SFR_E8, #0xFD
                anl     SFR_DC, #0xFE
                orl     SFR_DC, #2
                ljmp    wait_for_spi_ready

; =============== S U B R O U T I N E =======================================


ROM_4832:                               ; CODE XREF: ROM:ROM_4764p
                .db 0xa5,  0x1d
                mov     ER30, SDI1    ; SD Incoming Argument byte 0
                mov     ER31, SDI2    ; SD Incoming Argument byte 1
                mov     SFR_B7, #2
                .db 0xa5,  0xdc
                .db 0xa5,  0x19
                mov     ER20, SDI3    ; Extended (32-bit) register 2, byte 0
                mov     ER21, SDI4    ; Extended (32-bit) register 2, byte 1
                .db 0xa5,  0x1b
                anl     SFR_E8, #0xFD
                mov     RAM_20, #0
                mov     SFR_D6, #0xE8 ; 'F' ; SD Outgoing Address / 4 (low)
                mov     SFR_D7, #0x1F   ; SD Outgoing Address / 4 (high)
                mov     SFR_DE, #0
                mov     SFR_DF, #0
                mov     SFR_E7, #0xF1 ; 'Â±' ; SD output state
                anl     SFR_E8, #0xFB
                orl     SFR_E5, #0x40
                mov     SFR_D4, ER30    ; Extended (32-bit) register 3, byte 0
                mov     SFR_D5, ER31    ; Extended (32-bit) register 3, byte 1
                mov     SFR_E1, ER20    ; Extended (32-bit) register 2, byte 0
                mov     SFR_E2, ER21    ; Extended (32-bit) register 2, byte 1
                orl     SFR_DC, #1

ROM_4873:                               ; CODE XREF: ROM_4832+43j
                mov     A, SFR_E8
                jnb     ACC.2, ROM_4873  ; Accumulator
                anl     SFR_E8, #0xFB
                anl     SFR_DC, #0xFE
                anl     SFR_E5, #0xBF
                orl     SFR_DC, #2
                ret
; End of function ROM_4832

; ---------------------------------------------------------------------------

ROM_4885:                               ; CODE XREF: ROM:7B54j
                jb      SDSM.0, ROM_489C ; SD state machine
                anl     SFR_E8, #0xFD
                mov     SFR_D6, #0xE8 ; 'F' ; SD Outgoing Address / 4 (low)
                mov     SFR_D7, #0x1F   ; SD Outgoing Address / 4 (high)
                mov     SFR_DE, #3
                mov     SFR_DF, #0
                mov     SFR_E7, #1      ; SD output state
                ljmp    wait_for_spi_ready
; ---------------------------------------------------------------------------

ROM_489C:                               ; CODE XREF: ROM:ROM_4885j
                orl     SFR_E3, #1
                ljmp    wait_for_spi_ready

; =============== S U B R O U T I N E =======================================


ROM_48A1:                               ; CODE XREF: ROM:ROM_475Cp
                mov     A, SDI4       ; SD Incoming Argument byte 3
                anl     SFR_E8, #0xFD
                mov     SFR_D6, #0xE8 ; 'F' ; SD Outgoing Address / 4 (low)
                mov     SFR_D7, #0x1F   ; SD Outgoing Address / 4 (high)
                mov     SFR_DE, #3
                mov     SFR_DF, #0
                mov     SFR_E7, #1      ; SD output state
                acall   ROM_48F0

ROM_48B7:                               ; CODE XREF: ROM_48A1+18j
                mov     A, SFR_E8
                jnb     ACC.1, ROM_48B7  ; Accumulator
                anl     SFR_E8, #0xFD
                anl     SFR_E8, #0xFE
                orl     SFR_E3, #1
                ret
; End of function ROM_48A1


; =============== S U B R O U T I N E =======================================


ROM_48C6:                               ; CODE XREF: ROM:ROM_4760p
                mov     SFR_B4, SDI4  ; SD Incoming Argument byte 3
                mov     SFR_A1, SDI3  ; SD Incoming Argument byte 2
;                mov     SFR_B0, SDI2  ; SD Incoming Argument byte 1
;                mov     SFR_B2, SDI1  ; SD Incoming Argument byte 0
                mov     SFR_B1, #0
                anl     SFR_E8, #0xFD
                mov     SFR_D6, #0xE8 ; 'F' ; SD Outgoing Address / 4 (low)
                mov     SFR_D7, #0x1F   ; SD Outgoing Address / 4 (high)
                mov     SFR_DE, #3
                mov     SFR_DF, #0
                mov     SFR_E7, #1      ; SD output state

ROM_48E7:                               ; CODE XREF: ROM_48C6+23j
                mov     A, SFR_E8
                jnb     ACC.1, ROM_48E7  ; Accumulator
                anl     SFR_E8, #0xFD
                ret
; End of function ROM_48C6


; =============== S U B R O U T I N E =======================================


ROM_48F0:                               ; CODE XREF: ROM_48A1+14p
                mov     SFR_B3, A
                setb    SFR_B0.3

ROM_48F4:                               ; CODE XREF: ROM_48F0:ROM_48F4j
                jnb     SFR_B0.3, ROM_48F4
                clr     SFR_B0.3
                ret
; End of function ROM_48F0

; ---------------------------------------------------------------------------
.org 0x4900
