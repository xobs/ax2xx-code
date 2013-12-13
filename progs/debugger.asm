.equ	IEN, 0xA8

.equ	SD_DATL, 0xD6		; "Low" byte of SD transmission
.equ	SD_DATH, 0xD7		; "High" byte of SD transmission

.equ	SD_BYTESL, 0xDE		; SD transmission byte count
.equ	SD_BYTESH, 0xDF		; SD transmission byte count

.equ	SD_XMIT, 0xE7		; Write here to start an SD transmission
.equ	SD_XMIT_STATE, 0xE8	; State machine from active SD transmission


.equ	SDCMD, 0xE6		; A copy of the incoming SD command
.equ	SDI1, 0xE9		; A copy of the incoming SD argument 1
.equ	SDI2, 0xEA		; A copy of the incoming SD argument 2
.equ	SDI3, 0xEB		; A copy of the incoming SD argument 3
.equ	SDI4, 0xEC		; A copy of the incoming SD argument 4

; RAM map:
; 0x20 - 0x24: Outgoing SD transmission bytes
; 0x30 - 0x34: Incoming SD transmission bytes
; 0x35 - 0x39: ISR trigger count


.org 0x4700
; This gets called from the ROM.
; ---------------------------------------------------------------------------
; This gets called from an interrupt context, from within ROM.  We want to
; manipulate the stack so that when we return from interrupt, code execution
; will continue at an address immediately following this code section.
reset_vector:
	mov	A, #0x0C
	push	ACC
	mov	A, #0x47
	push	ACC
	reti			; Return from interrupt, ending up at 0x470c

;---------------------------------------------------------------------------
.org 0x470C
start:

	mov	IEN, #0		; Disable interrupts
	mov	SP, #0x80	; Reset stack pointer
	ljmp	main		; Enter the main() loop


main:
	lcall	receive_one_packet	; Grab the packet, leave it in
					; the SFRs

	mov	0x20, 0x30		; Put the incoming CMD in the outgoing
					; buffer, so it's part of the response.

        mov	0x21, #0		; Pre-clear these values, as they will
        mov	0x22, #0		; most commonly be zero in the response.
        mov	0x23, #0		; This saves many bytes later on.
        mov	0x24, #0

	mov	A, 0x30			; Grab the command that was received
	add	A, 0x30			; and multiply it by three, as ljmp
	add	A, 0x30			; opcodes are three bytes.

	mov	DPTR, #sdi_jumptable
	jmp	@A+DPTR
sdi_jumptable:
	ljmp	cmd0_null
	ljmp	cmd1_hello
	ljmp	cmd2_peek
	ljmp	cmd3_poke
	ljmp	cmd4_jump
	ljmp	cmd5_nand
	ljmp	cmd6_sfr_get
	ljmp	cmd7_sfr_set
	ljmp	cmd8_ext_op
	ljmp	cmd9_error
	ljmp	cmd10_irq

cmd0_null:
	ljmp	transmit_and_loop

cmd1_hello:
	mov	0x21, 0x31
	mov	0x22, 0x32
	mov	0x23, 0x33
	mov	0x24, 0x34
	ljmp	transmit_and_loop
	
cmd2_peek:
	; Take the first two received bytes, and use them as DPTR
	mov	DPH, 0x31	; DPH start src
	mov	DPL, 0x32	; DPL start src
	movx	A, @DPTR
	mov	0x21, A

	;inc	DPTR
	movx	A, @DPTR
	mov	0x22, A

	;inc	DPTR
	movx	A, @DPTR
	mov	0x23, A

	;inc	DPTR
	movx	A, @DPTR
	mov	0x24, A

	ljmp	transmit_and_loop

cmd3_poke:
	mov	DPH, 0x31
	mov	DPL, 0x32
	movx	A, @DPTR
	mov	0x21, A		; Save the old value

	mov	DPH, 0x31
	mov	DPL, 0x32
	mov	A, 0x33		; Copy the new value
	movx	@DPTR, A

	sjmp    transmit_and_loop

cmd4_jump:
	mov	DPH, 0x31
	mov	DPL, 0x32
	clr	A
	jmp	@A+DPTR
	sjmp    transmit_and_loop

cmd5_nand:
	mov	0x21, #1
	mov	0x22, #2
	mov	0x23, #3
	mov	0x24, #4
	sjmp	transmit_and_loop

cmd6_sfr_get:
        ; This will get replaced by "mov    0x21, [SFR]" at runtime
        .db	0xa5, 0x60, 0x61
        sjmp	transmit_and_loop

cmd7_sfr_set:
	; This will get replaced by "mov    [SFR], 0x31" at runtime
	.db	0xa5, 0x62, 0x63
	inc	0x31
	mov	0x21, 0x31
	sjmp	transmit_and_loop

cmd8_ext_op:
	.db	0xa5, 0x64, 0x65, 0x66, 0x67
	sjmp	transmit_and_loop

; Send an error packet back
cmd9_error:
	;mov	0x21, #0xa5
	sjmp	transmit_and_loop

cmd10_irq:
	mov	0x21, IE	; Save old interrupt enable state
	mov	A, 0x31
	jz	dont_update_irqs
	mov	IE, 0x32
dont_update_irqs:
	sjmp	transmit_and_loop


transmit_and_loop:
	lcall	transmit_from_ram
	ljmp	main



; Library routines
.org 0x4800


; pause_a_while(while1, while2, while3)
; R5 [in]: "Small" amount to wait
; R6 [in]: "Medium" amount to wait
; R7 [in]: "Large" amount to wait
; Will count from 0 to R7R6R5.  Every time a smaller value underflows, its
; larger neighbor will decrement.  E.g. if R5 underflows from 0 to 255, R6
; will be decremented.  This will conitnue until all values are zero.
pause_a_while:
	mov	R5, #0x00
	mov	R6, #0xf0
	mov	R7, #0x05
top_of_pause:
	djnz	R5, top_of_pause
	djnz	R6, top_of_pause
	;djnz	R7, top_of_pause
	ret


; ===================== SD FUNCTIONS ===================== 
; These functions allow you to transmit data over the SD bus, or
; receive data back.  All functions are blocking, and will only
; return when they are completely finished and the card is idle.

; void setup_sd_rcv(void)
; Prepares the SD registers to accept an SD packet.
; INPUT:	None
; OUTPUT:	None
setup_sd_rcv:
	clr	0xA0.2
	anl	SD_XMIT_STATE, #0xF0
	mov	0xE3, #0x0f		; Don't know what this does, but
					; having it here increases reliability.
	ret

; void receive_one_packet(void)
; Received a single four-byte-plus-command packet, and stores it in RAM,
; at offsets 0x30 - 0x34.
; RAM offset 0x30 contains the SD command, while offsets 0x31-0x34 contain
; the data bytes.
; Returns only once the bytes have been received.
;
; INPUT:	None
; OUTPUT:	Command byte stored at 0x30
;		Data bytes 1 - 4 stored at offsets 0x31 - 0x34
receive_one_packet:
	acall	setup_sd_rcv
wait_for_packet_start:
	mov	A, SD_XMIT_STATE
	jnb	ACC.0, wait_for_packet_start
	anl	SD_XMIT_STATE, #0xFE
	orl	0xE3, #1
wait_for_packet_end:
	jnb	0x90.4, wait_for_packet_end
	jb	0x90.0, wait_for_packet_start
	mov	0x30, SDCMD
	mov	0x31, SDI4
	mov	0x32, SDI3
	mov	0x33, SDI2
	mov	0x34, SDI1
	ret

; void transmit_from_ram(void)
; Transmits one four-byte-plus-command packet from RAM, located at
; offset 0x20.  Make sure the command packet at offset 0x20 has its
; start bit set (i.e. (byte & 0x7f)) so the receiving side knows when
; the stream starts.
; Returns once the packet has been completely transmitted.
;
; INPUT:	Command at offset 0x20, data at offsets 0x21-24
; OUTPUT:	None
transmit_from_ram:
	orl	0x20, #0x40		; Make sure the outgoing command packet
	anl	0x20, #0x7F		; has its sync and start bits set.
	anl	SD_XMIT_STATE, #0xFD
	mov	0x1f, #0x00
	mov	SD_DATL, #0xE8
	mov	SD_DATH, #0x12
	mov	SD_BYTESL, #0x04
	mov	SD_BYTESH, #0
	mov	SD_XMIT, #0xf1

wait_for_xmit_done:
	mov	A, SD_XMIT_STATE
	jnb	ACC.1, wait_for_xmit_done
	anl	SD_XMIT_STATE, #0xFD
	ret



; ===================== DEBUG FUNCTIONS ===================== 
; These functions can be useful for figuring out what the card
; is up to.  They are not used during normal operation.


; void noreturn emit_known_stuff(void)
; Emits a known pattern out the SD pins.  This can be useful for knowing
; where code execution has reached, when you don't know how to use the
; SPI bus.
;
; INPUT:	None
; OUTPUT:	None
emit_known_stuff:
	mov	0xef, #0xff
	nop
	nop
	nop
	mov	R5, #0xff
	mov	R6, #0x20
emit_known_loop1:
	djnz	R5, emit_known_loop1
	djnz	R6, emit_known_loop1
	mov	0xef, #0x00
	mov	R5, #0xff
	mov	R6, #0x20
emit_known_loop2:
	djnz	R5, emit_known_loop2
	djnz	R6, emit_known_loop2
	sjmp	emit_known_stuff

.org 0x4900
