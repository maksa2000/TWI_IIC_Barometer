; this function set atmega328p int0 interrupt
; it is not reieve either return any parameters
; interrupt will occure every int0 statuc change
; NB! do not forget to enable interrupts globalywith sei instruction

.ifndef ATMEGA328P_CONSTANTS
.include "atmega328p_core/const_codes.inc"
.endif

; .global is needed to set functions visible outside the module
.global init_int0

init_int0:
	push r16		; store value of r16 register, this register will be using in function
	; set PORTD pin2 as input
	in r16, DDRD	; get current DDRD value
	andi r16, 0xFB	; 11111011b
	out DDRD, r16
	; enable INT0 interrupt
	in r16, EIMSK
	ori r16, 0x01
	out EIMSK, r16
	; set external interrupt control register A bits (ISC01 and ISC00)
	lds r16, EICRA
	ori r16, 0x01			; Any logical pin change on INT0 generates an interrupt request. (for more information see Atmea328P datasheet)
	sts EICRA, r16
	pop r16			; restore value of r16 register
	ret
	