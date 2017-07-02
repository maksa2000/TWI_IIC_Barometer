ATMEGA_328P_TIMER = 0

.ifndef ATMEGA328P_CONSTANTS
.include "atmega328p_core/const_codes.inc"
.endif

; Initialize the timer0 
; function does not take nor return any parameters
timer0_init:
	push r16
	; set compare behaviour on compare match
	ldi r16, 0x00		; set zeroes
	out TCCR0A, r16
	; set clock selection for timer/counter
	ldi r16, 0x05		; set bits 2:0 to 101 (clk/1024 prescaling)
	out TCCR0B, r16
	pop r16
	ret
	
; depending of r24 register set of clear timer0 ov interrupt flag in TIMSK0 register
; recieve 1 parameter through r24 register
; if paremater 0 cleart interrupt flag, otherwise set flag
; does not return any value
timer0_set_ov_interrupt:
	push r16
	ldi r16, 0x00
	cpi r24, 0x00
	; if r24 equal 0x00, store 0x00 from r16 to TIMSK0 register
	breq timer0_set_ov_interrupt_exit
	; if not equal, set r16 to 0x01 and store this value to TIMSK0 register
	ldi r16, 0x01
timer0_set_ov_interrupt_exit:
	sts TIMSK0, r16
	pop r16
	ret
	
; set compare A value
; recieve value through r24 register
; does not return any value
timer0_set_compa_value:
	out OCR0A, r24
	ret
	
; reset timer0 compare A
; This function does not take nor recieve any parameters
timer0_reset:
	push r16
	ldi r16, 0x00
	out TCNT0, r16
	pop r16
	ret
	