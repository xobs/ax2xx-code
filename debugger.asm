.equ    IEN, 0xA8
.equ    SDCMD, 0x8D

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

.equ    SDI1, 0x8C
.equ    SDI2, 0x8B
.equ    SDI3, 0x8A
.equ    SDI4, 0x89

.equ    RESET, 0
.equ    PORT1, 0xF6

.equ    LED_STATE, 0x40

.equ    SD_WAITING, 0x24

.org 0x2900

; This gets called from the ROM.
; ---------------------------------------------------------------------------
; This gets called from an interrupt context, from within ROM.  We want to
; manipulate the stack so that when we return from intterupt, code execution
; will continue at an address immediately following this code section.
reset_vector:
        anl     0x80, #0xFE
        orl     0x80, #2

        pop     PSW             ; Remove previous return addresses
        pop     ACC             ; from the stack

        mov     A, #0x13        ; Add the new return
        push    ACC             ; address to the stack, so that when
        mov     A, #0x29        ; we reti, we end up at our
        push    ACC             ; "start" address.

        reti                    ; Return from interrupt, ending up at 0x2913

; ---------------------------------------------------------------------------
.org 0x2913
start:
        mov     IEN, #0         ; Disable interrupts
        mov     SP, #0x80       ; Reset stack pointer
        mov     SDDIR, #0xff    ; Prevent the SD card from writing 0x00
        acall   setup
        ajmp    xmit_response   ; Send a "hello" packet

.org 0x2920
; ---------------------------------------------------------------------------
; SD interrupt handler.  Called from an interrupt context.
sdi_isr:
        push    ACC
        push    0xD2
        push    DPH
        push    DPL
        push    PSW
        mov     PSW, #8
        mov     0xd2, #0
        anl     0x8E, #0xFE
        clr     0x24.3

        lcall   wait_for_packet
        mov     0x31, SDCMD     ; Copy the incoming SD packet
        mov     0x20, SDI1      ; to an area of memory commonly used
        mov     0x21, SDI2      ; by the SD transmission engine.  Copy the
        mov     0x22, SDI3      ; incoming packet here as part of an echo-back
        mov     0x23, SDI4      ; program.

        orl     SDSM, #1        ; Kick the SD state machine (what does this do?)

exit_sdi_isr:
        pop     PSW
        pop     DPL
        pop     DPH
        pop     0xD2
        pop     ACC
        setb    SD_WAITING.1    ; Indicate we have a command waiting
        reti
; ---------------------------------------------------------------------------


; Point the ISR stored in DPTR to address 0x293f
; DPTR [in]: Address of the ISR to set
set_isr:
        mov     A, #0x02
        movx    @DPTR, A

        inc     DPTR
        mov     A, #0x29
        movx    @DPTR, A

        inc     DPTR
        mov     A, #0x20
        movx    @DPTR, A

        ret

; Wait for an SD packet to completely transfer
wait_for_packet:
        mov     A, SDSM
        anl     A, #0xC
        jnz     wait_for_packet
        ret


; ---------------------------------------------------------------------------


setup_sdport:
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

reset_sdport:
        clr     EA
        anl     0x8e, #0xfe
        anl     0x8e, #0xfd
        anl     0x8e, #0xfb
        anl     0x8e, #0xf7

        setb    0x98.3
        mov     IEN, #0x8f
        clr     0x98.3
        setb    EA
        ret

setup:
        ; Set up ISR to call function at 0x293f.  We're interested
        ; in both interrupt 0 and interrupt 1.
        ; These fire whenever we get an SD command.
        mov     DPTR, #0x0200
        lcall   set_isr
        mov     DPTR, #0x0203
        lcall   set_isr
        acall   setup_sdport

        ret

main_loop:
        mov     A, 0x31         ; Copy SD command register to accumulator
        cjne    A, #0x00, not_c00
        sjmp    cmd_null
not_c00:cjne    A, #0x01, not_c01
        sjmp    cmd_hello
not_c01:cjne    A, #0x02, not_c02
        sjmp    cmd_peek
not_c02:cjne    A, #0x03, not_c03
        sjmp    cmd_poke
not_c03:cjne    A, #0x04, not_c04
        sjmp    cmd_jump
not_c04:sjmp    cmd_error

; Send all zeroes
cmd_null:
        mov     0x20, #0
        mov     0x21, #0
        mov     0x22, #0
        mov     0x23, #0
        sjmp    xmit_response

; Send an echo back
cmd_hello:
        sjmp    xmit_response

; Peek at an area of memory
cmd_peek:
        mov     R0, 0x20    ; DPH start src
        mov     R1, 0x21    ; DPL start src
        mov     R2, 0x22    ; Number of bytes to copy
        mov     R4, 0x22    ; Number of bytes left
        mov     R5, #0x01   ; DPH start dest
        mov     R6, #0x00   ; DPL start dest

copy_one_byte:
        mov     DPH, R0
        mov     DPL, R1
        movx    A, @DPTR
        inc     DPTR
        mov     R0, DPH
        mov     R1, DPL

        mov     DPH, R5
        mov     DPL, R6
        movx    @DPTR, A
        inc     DPTR
        mov     R5, DPH
        mov     R6, DPL

        djnz    R4, copy_one_byte

        acall   reset_sdport

        dec     R2
        mov     SDDL, #0x20     ; SD Outgoing Address>>2
        mov     SDDH, #0x00     ; SD Outgoing Address

        mov     SDBL, R2        ; SD Outgoing bytes (low)
        mov     SDBH, #0        ; SD Outgoing bytes (high)

        mov     SDOS, #0x1      ; Kick off the transfer

        sjmp    wait_for_next_command

; Poke into an area of memory
cmd_poke:
        mov     DPH, 0x20
        mov     DPL, 0x21
        movx    A, @DPTR
        mov     B, A     ; Save the old value

        mov     A, 0x22     ; Copy the new value
        movx    @DPTR, A

        mov     0x22, B
        mov     0x23, #0
        sjmp    xmit_response

; Jump to an address
cmd_jump:
        mov     0x20, #1
        mov     0x21, #1
        mov     0x22, #1
        mov     0x23, #1
        sjmp    xmit_response

; Send an error packet back
cmd_error:
        mov     0x20, #0xa5
        mov     0x21, #0xa5
        mov     0x22, #0xa5
        mov     0x23, #0xa5
        sjmp    xmit_response

xmit_response:
        ; Set up the SD pins
        acall   reset_sdport

        mov     SDDL, #0x74     ; SD Outgoing Address>>2
        mov     SDDH, #0x05     ; SD Outgoing Address

        mov     SDBL, #0x03     ; SD Outgoing bytes (low)
        mov     SDBH, #0        ; SD Outgoing bytes (high)

        mov     SDOS, #0x1      ; Kick off the transfer

        sjmp    wait_for_next_command

wait_for_next_command:
        clr     SD_WAITING.1
wait_sd_cmd:
        jnb     SD_WAITING.1, wait_sd_cmd
        ajmp    main_loop


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
