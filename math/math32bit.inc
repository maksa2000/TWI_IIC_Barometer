MATH_32BIT = 0

.ifndef ATMEGA328P_CONSTANTS
.include "atmega328p_core/const_codes.inc"
.endif

.text

; Invert sign of 32 bit value
; Value recieved in r27:r24 registers
; r27 - MSB
; r24 - LSB
; Value will be returned in r27:r24 registers
; r27 - MSB
; r24 - LSB
invert_sign_32bit:
	; save state of temporary register
	push r16
	; save sate of Y registers
	push r28
	push r29
	
	; set Y register to base pointer
	in r29, SPH
	in r28, SPL
	
	; save recieved value to the stack
	push r27			; [base] MSB
	push r26			; [base + 0x01]
	push r25			; [base + 0x02]
	push r24			; [base + 0x03]
	
	; check does recieved value positive
	sbrc r27, 0x07		; check does 7th bit is cleare, if so skips next instruction
	rjmp _invert_sign_32bit_set_positive
	
	; change sign to negative
	; step 1 change invert registers bits
	ldi r16, 0xFF
	eor r27, r16
	eor r26, r16
	eor r25, r16
	eor r24, r16
	
	; add to inverted registers 0x01
	ldi r16, 0x01
	
	clc			; clear carry flag
	
	add r24, r16
	ldi r16, 0x00
	adc r25, r16	; add carry bit if any
	adc r26, r16	; add carry bit if any
	adc r27, r16	; add carry bit if any
	
	rjmp _invert_sign_32bit_exit
	
_invert_sign_32bit_set_positive:
	; change sign to positive
	
	; subtract 0x01 from recieved value
	ldi r16, 0x01
	
	clc		; clear carry flag
	
	sub r24, r16
	ldi r16, 0x00
	sbc r25, r16		; subtract carry flag if any
	sbc r26, r16		; subtract carry flag if any
	sbc r27, r16		; subtract carry flag if any
	
	; invert registers bits
	ldi r16, 0xFF
	eor r27, r16
	eor r26, r16
	eor r25, r16
	eor r24, r16
	
_invert_sign_32bit_exit:
	; restore stack pointer value from base pointer
	out SPH, r29
	out SPL, r28
	
	; restore state of Y registers
	pop r29
	pop r28
	
	pop r16
	ret
