.equ    IEN, 0xA8

.equ    SDDL, 0xD6
.equ    SDDH, 0xD7

.equ    SDBC, 0xDE  ; SD byte count

.equ    SDOS, 0xE7
.equ    SDDIR, 0xEB
.equ    SDSM, 0x90

.equ    SDCMD, 0xE6
.equ    SDI1, 0xE9
.equ    SDI2, 0xEA
.equ    SDI3, 0xEB
.equ    SDI4, 0xEC

.equ    RESET, 0
.equ    PORT1, 0xF6

.equ    NPS, 0x9e

.equ    LED_STATE, 0x40

.equ    NCMD, 0xa1
.equ    NSRCL, 0xa2
.equ    NSRCH, 0xa3

.equ    SD_WAITING, 0x24

.org 0x7b00
; This gets called from the ROM.
; ---------------------------------------------------------------------------
; This gets called from an interrupt context, from within ROM.  We want to
; manipulate the stack so that when we return from intterupt, code execution
; will continue at an address immediately following this code section.
reset_vector:
;        anl     0x80, #0xFE
;        orl     0x80, #2
;
;        pop     PSW             ; Remove previous return addresses
;        pop     ACC             ; from the stack
;
;        mov     A, #0x13        ; Add the new return
;        push    ACC             ; address to the stack, so that when
;        mov     A, #0x7b        ; we reti, we end up at our
;        push    ACC             ; "start" address.

        anl     0xdc, #0xfe
        mov     A, #0x13
        push    ACC
        mov     A, #0x7b
        push    ACC
        reti                    ; Return from interrupt, ending up at 0x7b13

; ---------------------------------------------------------------------------
.org 0x7b13
start:
        mov     IEN, #0         ; Disable interrupts
        mov     SP, #0x80       ; Reset stack pointer
        acall   more_setup
        acall   setup
        ajmp    cmd_hello  ; Send a "hello" packet

.org 0x7b20
; ---------------------------------------------------------------------------
; SD interrupt handler.  Called from an interrupt context.
sdi_isr:
        push    ACC
        push    0x80
        push    DPH
        push    DPL
        push    PSW
        mov     PSW, #8
        mov     0x80, #8
        clr     0xE8.0
        clr     0xE8.1
        orl     0xE3, #1
        anl     0xCE, #0xEC

waiting:
        jnb     SDSM.4, waiting
        mov     0xD6, #0x8A
        mov     0xD7, #0
        mov     0xDE, #0xF
        mov     0xE7, #0x71
        ajmp    exit_sdi_isr

;        lcall   wait_for_packet
;        mov     0x31, SDCMD     ; Copy the incoming SD packet
;        mov     0x20, SDI1      ; to an area of memory commonly used
;        mov     0x21, SDI2      ; by the SD transmission engine.  Copy the
;        mov     0x22, SDI3      ; incoming packet here as part of an echo-back
;        mov     0x23, SDI4      ; program.
;
;        orl     SDSM, #1        ; Kick the SD state machine (what does this do?)

exit_sdi_isr:
        pop     PSW
        pop     DPL
        pop     DPH
        pop     0x80
        pop     ACC
;        setb    SD_WAITING.1    ; Indicate we have a command waiting
        reti
; ---------------------------------------------------------------------------

more_setup:
        clr     0xA0.2
        anl     0xE8, #0xf0
        orl     0xdc, #4
        orl     0xdc, #8
        ret

; Point the ISR stored in DPTR to address 0x7b20
; DPTR [in]: Address of the ISR to set
set_isr:
        mov     A, #0x02
        movx    @DPTR, A

        inc     DPTR
        mov     A, #0x7b
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
        orl     0xd1, #1
        orl     0xb9, #2
wait_sd_ready:
        jnb     0xe8.1, wait_sd_ready
        clr     0xe8.1
        ret

setup:
        ; Set up ISR to call function at 0x7b3f.  We're interested
        ; in both interrupt 0 and interrupt 1.
        ; These fire whenever we get an SD command.
        mov     DPTR, #0x0200
        lcall   set_isr
        mov     DPTR, #0x0203
        lcall   set_isr

        ; Make IRQ2 and IRQ3 simply "reti"
        mov     DPTR, #0x0206   ; IRQ2
        mov     A, #0x32        ; reti instruction
        movx    @DPTR, A
        mov     DPTR, #0x0209   ; IRQ3
        movx    @DPTR, A

        acall   setup_sdport

        ret

main_loop:
        mov     A, 0x31         ; Copy SD command register to accumulator
        cjne    A, #0x00, not_c00
        ajmp    cmd_null
not_c00:cjne    A, #0x01, not_c01
        ajmp    cmd_echo
not_c01:cjne    A, #0x02, not_c02
        ajmp    cmd_peek
not_c02:cjne    A, #0x03, not_c03
        ajmp    cmd_poke
not_c03:cjne    A, #0x04, not_c04
        ajmp    cmd_jump
not_c04:cjne    A, #0x05, not_c05
        ajmp    cmd_nand
not_c05:cjne    A, #0x06, not_c06
        ajmp    cmd_sfr_set
not_c06:cjne    A, #0x07, not_c07
        ajmp    cmd_sfr_get
not_c07:cjne    A, #0x08, not_c08
        ajmp    cmd_ext_op
not_c08:ajmp    cmd_error

; Send a 'hello' packet
cmd_hello:
        mov     0x20, #0x41
        mov     0x21, #0x1f
        mov     0x22, #0x0f
        mov     0x23, #0x0f
        ajmp    xmit_response

; Send all zeroes
cmd_null:
        mov     0x20, #0
        mov     0x21, #0
        mov     0x22, #0
        mov     0x23, #0
        ajmp    xmit_response

; Send an echo back
cmd_echo:
        ajmp    xmit_response

; Peek at an area of memory
cmd_peek:
        mov     DPH, 0x20    ; DPH start src
        mov     DPL, 0x21    ; DPL start src
        movx    A, @DPTR
        mov     0x20, A
        inc     DPTR

        movx    A, @DPTR
        mov     0x21, A
        inc     DPTR

        movx    A, @DPTR
        mov     0x22, A
        inc     DPTR

        movx    A, @DPTR
        mov     0x23, A

        sjmp    xmit_response

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

cmd_nand:
        mov     NSRCL, 0x21
        mov     NSRCH, 0x22
        mov     NCMD, 0x20
        mov     0x20, #1
        mov     0x21, #2
        mov     0x22, #3
        mov     0x23, #4
        sjmp    xmit_response

cmd_sfr_get:
        ; This will get replaced by "mov    0x20, [SFR]" at runtime
        .db     0xa5, 0x60, 0x61
        mov     0x21, #0
        mov     0x22, #0
        mov     0x23, #0
        sjmp    xmit_response

cmd_sfr_set:
        ; This will get replaced by "mov    [SFR], 0x20" at runtime
        .db     0xa5, 0x62, 0x63
        mov     0x21, #0
        mov     0x22, #0
        mov     0x23, #0
        sjmp    xmit_response

cmd_ext_op:
        .db     0xa5, 0x64, 0x65
        mov     0x20, #0
        mov     0x21, #0
        mov     0x22, #0
        mov     0x23, #0
        sjmp    xmit_response

; Send an error packet back
cmd_error:
        mov     0x20, #0xa5
        mov     0x21, #0xa5
        mov     0x22, #0xa5
        mov     0x23, #0xa5
        sjmp    xmit_response

xmit_response:
        acall   reset_sdport    ; Set up the SD pins

;        mov     SDDL, #0x74     ; Point the outgoing address at
;        mov     SDDH, #0x05     ; the contents of RAM_0x20..RAM_0x24
;
;        mov     SDBC, #0x03     ; Output four [sic] bytes
;
;        mov     SDOS, #0x71      ; Kick off the transfer
        mov     0xd4, #0x91
        mov     0xd5, #0
        mov     0xe1, #7
        mov     0xe2, #0
        orl     0xe5, #1
        orl     0xb9, #8
wait_xfer_done:
        jnb     0xe8.3, wait_xfer_done
        clr     0xe8.3
        anl     0xd1, #0xfe
        mov     0xcf, #4

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

