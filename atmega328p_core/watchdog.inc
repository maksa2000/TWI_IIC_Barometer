WATCHDOG = 0

.ifndef ATMEGA328P_CONSTANTS
.include "atmega328p_core/const_codes.inc"
.endif

; initialize watchdog timer
; function does not take nor recieve parameters
watchdog_init_interrupt_mode:
	push r16
	; set WDCE (watchdog change enambe bit)
	; this bit should be set when I want to change watchdog prescaler
	; hardware will clear this bit after 4 clock cicles	
	; WDCE bit can be set only while WDE bit is set
	ldi r16, 0x58			; 0111 0000b
	sts WDTCSR, r16
	ldi r16, 0x47			; 0100 0111b
	sts WDTCSR, r16
	pop r16
	ret
	
watchdog_interrupt_disable:
	push r16
	lds r16, WDTCSR
	andi r16, 0xBF		; 1011 1111b
	sts WDTCSR, r16
	pop r16	
	ret
