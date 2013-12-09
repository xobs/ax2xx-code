.equ	IEN, 0xA8

.equ	SFR_80, 0x80
.equ	SD_DATL, 0xD6
.equ	SD_DATH, 0xD7

.equ	SD_BYTES, 0xDE  ; SD byte count

.equ	SDDIR, 0xEB
.equ	SDSM, 0x90

.equ	SDCMD, 0xE6
.equ	SD_XMIT, 0xE7
.equ	SD_XMIT_STATE, 0xE8
.equ	SDI1, 0xE9
.equ	SDI2, 0xEA
.equ	SDI3, 0xEB
.equ	SDI4, 0xEC

.equ	RESET, 0
.equ	PORT1, 0xF6

.equ	NPS, 0x9e

.equ	LED_STATE, 0x40

.equ	NCMD, 0xa1
.equ	NSRCL, 0xa2
.equ	NSRCH, 0xa3

.equ	SD_WAITING, 0x24

.org 0x4700
; This gets called from the ROM.
; ---------------------------------------------------------------------------
; This gets called from an interrupt context, from within ROM.  We want to
; manipulate the stack so that when we return from interrupt, code execution
; will continue at an address immediately following this code section.
reset_vector:
	anl	0xDC, #0xFE
	pop	ACC
	pop	ACC
	mov	A, #0x10
	push	ACC
	mov	A, #0x47
	push	ACC
	reti			; Return from interrupt, ending up at 0x7b0c


;---------------------------------------------------------------------------
.org 0x4710
start:

	mov	IEN, #0		; Disable interrupts
	mov	SP, #0x80	; Reset stack pointer
	acall	setup_sd_rcv
;	acall	reset_isrs
	ljmp	emit_known_stuff
	acall	more_setup
	ljmp	main		; Enter the main() loop

; ---------------------------------------------------------------------------
; SD interrupt handler.  Called from an interrupt context.
;.org 0x7b80
sdi_isr:
	push	ACC
	push	0x80
	push	DPH
	push	DPL
	push	PSW
	mov	PSW, #8
	mov	SFR_80, #8
	clr	SD_XMIT_STATE.0
	clr	SD_XMIT_STATE.1
	orl	0xE3, #1
	anl	0xCE, #0xEC

	; Get ready to jump to the command, stored in SDCMD
	mov	A, SDCMD

	; Double the CMD value, because ajmp opcodes are two-bytes
	rl	A

	; Actually jump.  This jumps to 2*SDCMD
	mov	DPTR, #sdi_jumptable
	jmp	@A+DPTR

; ---------------------------------------------------------------------------

setup_sd_rcv:
	clr	0xA0.2
	anl	0xE8, #0xFE
	anl	0xE8, #0xFD
	anl	0xE8, #0xFB
	anl	0xE8, #0xF7
	orl	0xDC, #4
	orl	0xDC, #8
	ret

more_setup:
	lcall	pause_a_while
wait_for_packet_start:
	mov	R5, #0
	mov	A, 0xE8
	jnb	ACC.0, wait_for_packet_start
	anl	0xE8, #0xFE
wait_for_packet_end:
	jnb	0x90.4, wait_for_packet_end
	nop

	;;
	anl	0xe8, #0xFD
	mov	0x20, #2
	mov	0x21, #5
	mov	0x22, #32
	mov	0x23, #66
	mov	SD_DATL, #0xE8
	mov	SD_DATH, #0x1F
	mov	0xDE, #0x03
	mov	0xDF, #0
	mov	0xE7, #0x01

;	anl	0xE8, #0xFB
;	orl	0xE5, #0x40
;	mov	0xD4, #7
;	mov	0xD5, #3
;	mov	0xE1, #99
;	mov	0xE2, #101
;	orl	0xDC, #1
;
;transmitting:
;	mov	A, 0xE8
;	jnb	ACC.1, transmitting
;	anl	0xE8, #0xFD
;	anl	0xDC, #0xFE
;	anl	0xE5, #0xBF
;	orl	0xDC, #2

waiting:
	sjmp waiting

	ret

; Point the ISR stored in DPTR to address 0x7b30
; DPTR [in]: Address of the ISR to set
set_isr:
	mov	A, #0x02	; ljmp
	movx	@DPTR, A

	inc	DPTR
	mov	A, #0x7b
	movx	@DPTR, A

	inc	DPTR
	mov	A, #0x30
	movx	@DPTR, A

	ret

; Wait for an SD packet to completely transfer
;wait_for_packet:
;	mov	A, SDSM
;	anl	A, #0xC
;	jnz	wait_for_packet
;	ret


; ---------------------------------------------------------------------------


setup_sdport:
	mov	0xd1, #0xe8
	orl	0xe9, #1
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	orl	0xe9, #0x80
	ret

reset_sdport:
	clr	EA
	orl	0xd1, #1
	orl	0xb9, #2
wait_sd_ready:
	jnb	0xe8.1, wait_sd_ready
	clr	0xe8.1
	ret

reset_isrs:
	; Set up ISR to call function at 0x7b3f.  We're interested
	; in both interrupt 0 and interrupt 1.
	; These fire whenever we get an SD command.
	;mov	DPTR, #0x0003
	;lcall	set_isr
	;mov	DPTR, #0x0203
	;lcall	set_isr

	mov	DPTR, #0x0000	; IRQ0
	mov	A, #0x32	; reti instruction
	movx	@DPTR, A

	mov	DPTR, #0x0003	; IRQ1
	mov	A, #0x32	; reti instruction
	movx	@DPTR, A

	; Make IRQ2 and IRQ3 simply "reti"
	mov	DPTR, #0x000b	; IRQ2
	mov	A, #0x32	; reti instruction
	movx	@DPTR, A

	mov	DPTR, #0x0013	; IRQ3
	mov	A, #0x32	; reti instruction
	movx	@DPTR, A

	mov	DPTR, #0x001b	; IRQ4
	mov	A, #0x32	; reti instruction
	movx	@DPTR, A

	ret


sdi_jumptable:
	ajmp	cmd0
	ajmp	cmd1
	ajmp	cmd2
	ajmp	sd3_SendCSD


cmd0:
cmd1:
cmd2:
sd3_SendCSD:
	jnb	SDSM.4, sd3_SendCSD
	orl	0xCF, #0x10
	mov	SD_DATL, #0xE8
	mov	SD_DATH, #0x1F
	orl	0xCE, #1
	mov	SD_BYTES, #0xF
	mov	0xE7, #0x71
wait_for_csd_done:
	jnb	SD_XMIT_STATE.2, wait_for_csd_done
	clr	SD_XMIT_STATE.2
	ajmp	return_from_sdi_irq


return_from_sdi_irq:
	pop	PSW
	pop	DPL
	pop	DPH
	pop	0x80
	pop	ACC
	reti


;        lcall   wait_for_packet
;        mov     0x31, SDCMD     ; Copy the incoming SD packet
;        mov     0x20, SDI1      ; to an area of memory commonly used
;        mov     0x21, SDI2      ; by the SD transmission engine.  Copy the
;        mov     0x22, SDI3      ; incoming packet here as part of an echo-back
;        mov     0x23, SDI4      ; program.
;
;        orl     SDSM, #1        ; Kick the SD state machine (what does this do?)

exit_sdi_isr:
	pop	PSW
	pop	DPL
	pop	DPH
	pop	0x80
	pop	ACC
;	setb	SD_WAITING.1	; Indicate we have a command waiting
	reti
;
;main_loop:
;        mov     A, 0x31         ; Copy SD command register to accumulator
;        cjne    A, #0x00, not_c00
;        ajmp    cmd_null
;not_c00:cjne    A, #0x01, not_c01
;        ajmp    cmd_echo
;not_c01:cjne    A, #0x02, not_c02
;        ajmp    cmd_peek
;not_c02:cjne    A, #0x03, not_c03
;        ajmp    cmd_poke
;not_c03:cjne    A, #0x04, not_c04
;        ajmp    cmd_jump
;not_c04:cjne    A, #0x05, not_c05
;        ajmp    cmd_nand
;not_c05:cjne    A, #0x06, not_c06
;        ajmp    cmd_sfr_set
;not_c06:cjne    A, #0x07, not_c07
;        ajmp    cmd_sfr_get
;not_c07:cjne    A, #0x08, not_c08
;        ajmp    cmd_ext_op
;not_c08:ajmp    cmd_error
;
;; Send a 'hello' packet
;cmd_hello:
;        mov     0x20, #0x41
;        mov     0x21, #0x1f
;        mov     0x22, #0x0f
;        mov     0x23, #0x0f
;        ajmp    xmit_response
;
;; Send all zeroes
;cmd_null:
;        mov     0x20, #0
;        mov     0x21, #0
;        mov     0x22, #0
;        mov     0x23, #0
;        ajmp    xmit_response
;
;; Send an echo back
;cmd_echo:
;        ajmp    xmit_response
;
;; Peek at an area of memory
;cmd_peek:
;        mov     DPH, 0x20    ; DPH start src
;        mov     DPL, 0x21    ; DPL start src
;        movx    A, @DPTR
;        mov     0x20, A
;        inc     DPTR
;
;        movx    A, @DPTR
;        mov     0x21, A
;        inc     DPTR
;
;        movx    A, @DPTR
;        mov     0x22, A
;        inc     DPTR
;
;        movx    A, @DPTR
;        mov     0x23, A
;
;        sjmp    xmit_response
;
;; Poke into an area of memory
;cmd_poke:
;        mov     DPH, 0x20
;        mov     DPL, 0x21
;        movx    A, @DPTR
;        mov     B, A     ; Save the old value
;
;        mov     A, 0x22     ; Copy the new value
;        movx    @DPTR, A
;
;        mov     0x22, B
;        mov     0x23, #0
;        sjmp    xmit_response
;
;; Jump to an address
;cmd_jump:
;        mov     0x20, #1
;        mov     0x21, #1
;        mov     0x22, #1
;        mov     0x23, #1
;        sjmp    xmit_response
;
;cmd_nand:
;        mov     NSRCL, 0x21
;        mov     NSRCH, 0x22
;        mov     NCMD, 0x20
;        mov     0x20, #1
;        mov     0x21, #2
;        mov     0x22, #3
;        mov     0x23, #4
;        sjmp    xmit_response
;
;cmd_sfr_get:
;        ; This will get replaced by "mov    0x20, [SFR]" at runtime
;        .db     0xa5, 0x60, 0x61
;        mov     0x21, #0
;        mov     0x22, #0
;        mov     0x23, #0
;        sjmp    xmit_response
;
;cmd_sfr_set:
;        ; This will get replaced by "mov    [SFR], 0x20" at runtime
;        .db     0xa5, 0x62, 0x63
;        mov     0x21, #0
;        mov     0x22, #0
;        mov     0x23, #0
;        sjmp    xmit_response
;
;cmd_ext_op:
;        .db     0xa5, 0x64, 0x65
;        mov     0x20, #0
;        mov     0x21, #0
;        mov     0x22, #0
;        mov     0x23, #0
;        sjmp    xmit_response
;
;; Send an error packet back
;cmd_error:
;        mov     0x20, #0xa5
;        mov     0x21, #0xa5
;        mov     0x22, #0xa5
;        mov     0x23, #0xa5
;        sjmp    xmit_response
;
;xmit_response:
;        acall   reset_sdport    ; Set up the SD pins
;
;;        mov     SDDL, #0x74     ; Point the outgoing address at
;;        mov     SDDH, #0x05     ; the contents of RAM_0x20..RAM_0x24
;;
;;        mov     SD_BYTES, #0x03     ; Output four [sic] bytes
;;
;;        mov     SD_XMIT, #0x71      ; Kick off the transfer
;        mov     0xd4, #0x91
;        mov     0xd5, #0
;        mov     0xe1, #7
;        mov     0xe2, #0
;        orl     0xe5, #1
;        orl     0xb9, #8
;wait_xfer_done:
;        jnb     0xe8.3, wait_xfer_done
;        clr     0xe8.3
;        anl     0xd1, #0xfe
;        mov     0xcf, #4
;
;        sjmp    wait_for_next_command
;
;wait_for_next_command:
;        clr     SD_WAITING.1
;wait_sd_cmd:
;        jnb     SD_WAITING.1, wait_sd_cmd
;        ajmp    main_loop


; pause_a_while(while1, while2, while3)
; R5 [in]: "Small" amount to wait
; R6 [in]: "Medium" amount to wait
; R7 [in]: "Large" amount to wait
; Will count from 0 to R7R6R5.  Every time a smaller value underflows, its
; larger neighbor will decrement.  E.g. if R5 underflows from 0 to 255, R6
; will be decremented.  This will conitnue until all values are zero.
pause_a_while:
	mov	R5, #0x00
	mov	R6, #0x20
	mov	R7, #0x01
top_of_pause:
	djnz	R5, top_of_pause
	djnz	R6, top_of_pause
	;djnz	R7, top_of_pause
	ret

;====================== 
; END OF ISR STUFF
;====================== 

main:
	setb	0x22.0
	mov	A, 0x31
	jnz	do_something
	mov	A, 0xB5
	sjmp	main

do_something:
	clr	EA
	mov	DPTR, #main_jumptable

	; ljmp instructions are three bytes, so triple the cmd size
	mov	A, 0x31
	add	A, 0x31
	add	A, 0x31
	mov	0x31, #0
	setb	EA
	jmp	@A+DPTR

main_jumptable:
	ljmp	main_do_nothing


main_do_nothing:
	ljmp	main


; Debug routines
emit_known_stuff:
	mov	0xef, #0xff
	nop
	nop
	nop
	mov	R5, #0x37
	mov	R6, #0x16
loop1:
	djnz	R5, loop1
	djnz	R6, loop1
	mov	0xef, #0x00
	mov	R5, #0x37
	mov	R6, #0x16
loop2:
	djnz	R5, loop2
	djnz	R6, loop2
	sjmp	emit_known_stuff
