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

.org 0x7b00

; This gets called from an interrupt.
start:
        .db     0xa5, 0x00, 0x00
        .db     0xa5, 0x01, 0x01
        .db     0xa5, 0x02, 0x02
        .db     0xa5, 0x03, 0x03
        .db     0xa5, 0x04, 0x04
        .db     0xa5, 0x05, 0x05
        .db     0xa5, 0x06, 0x06
        .db     0xa5, 0x07, 0x07
        mov     R2, #0
        mov     R3, #3
        mov     R4, #0
top_of_pause1:
        djnz    R2, top_of_pause1
        djnz    R3, top_of_pause1
        djnz    R4, top_of_pause1
        .db     0xa5, 0x80, 0x80
        .db     0xa5, 0x81, 0x81
        .db     0xa5, 0x82, 0x82
        .db     0xa5, 0x83, 0x83
        .db     0xa5, 0x84, 0x84
        .db     0xa5, 0x85, 0x85
        .db     0xa5, 0x86, 0x86
        .db     0xa5, 0x87, 0x87
        mov     R2, #0
        mov     R3, #3
        mov     R4, #0
top_of_pause2:
        djnz    R2, top_of_pause2
        djnz    R3, top_of_pause2
        djnz    R4, top_of_pause2
        sjmp    start
