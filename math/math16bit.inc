MATH_16BIT = 0		; define MATH_16BIT

.ifndef MATH_8BIT
.include "math/math8bit.inc"
.endif

.ifndef ATMEGA328P_CONSTANTS
.include "atmega328p_core/const_codes.inc"
.endif

.text
; do 16 bit unsigned integer division 
; value1 - r29:r28 dividend
; value2 - r31:r30 devider
; function return result in r25:r24 and r27:r28 registers
; r25:r24 - integer result of division
; r27:r26 - reminder of division
div_unit_16bit:		
	; Do not save state of X register, because it is used for returning values
	
	; set Y as base pointer register
	in r27, SPH
	in r26, SPL
	
	; save value1 to the stack
	push r29		; [base]
	push r28		; [base + 0x01]
	
	; save value2 to the stack
	push r31		; [base + 0x02]
	push r30		; [base + 0x03]
	
	; load MSB dividend from the stack
	ld r28, X
	
	; load MSB divider from the stack
	sbiw r26, 0x02
	ld r29, X
	adiw r26, 0x02
	
	rcall div_uint_8bit
	
	; respore stack pointer register
	out SPH, r27
	out SPL, r26
	
	ret

; subtract 2 16 bit unsigned integers (value1 - value2) and return 16 bit integer result
; value1 - 2 bits unsigned integer in registers r29:r28
; value2 - 2 bits unsigned integer in registers r31:r30
; result is 16 bit value returned through registers r24:r25
sub_uint_16bit:
	push r2
	clr r2
	
	; move value 1 to result registers
	mov r24, r28	; move low byte to result low byte
	mov r25, r29 	; move high byte to result high byte
	
	sub r24, r30	; subtract low byte of value2 from low byte of result
	sbc r25, r31 	; subtract with carry high byte of value2 from high byte of result
	sbc r24, r2		; subtract carry bit from low byte of result 
	
	pop r2
	ret

; add 2 16 bit integer values (both signed and unsigned)
; values are passed to function in registers:
; value1 - r29:r28
; value2 - r31:r30
; result is 32 bit value located in registers r24:r27 
add_uint_16bit:
	push r2
	clr r2			; clear r2 register
	
	; clear 2 highest bytes of the result
	clr r26
	clr r27
	
	;move first value to result registers
	mov r24, r28	; mov low byte
	mov r25, r29	; mov high byte
	
	add r24, r30	; add low of parameter 2 to low byte of result
	adc r25, r31	; add with carry high byte of parameter 2 to high byte of result
	adc r26, r2		; add carry bit
	
	pop r2
	ret

; multiply 2 16 bit unsigned integer values that are located in next registers (low byte:high byte):
; value1 - r29:r28 
; value2 - r31:r30
; result is 32 bit value that will be returned in registers r24:r27
mul_uint_16bit:	
	push r2
	
	; using return registers for calculation, so I will not save their state
	clr r2			; clear r2 register, will be used to in adc instruction in future, this register will contain 0, but I should add 0 to result to add the latest carry flag
	
	; result of mul operation will be on r0:r1 registers	
	mul r29, r31	; multiply high bytes of parameters
	movw r26, r0	; move 2 bytes starting from r0 register (r0:r1) to registers starting from r26 (r26:r27 - 2 high bytes of result)
	
	mul r28, r30	; multiply low bytes of parameters
	movw r24, r0	; move 2 bytes starting from r0 register (r0:r1) to registers starting from r24 (r24:r25 - 2 low bytes of result) 
	
	mul r29, r30	; multiply high byte of parameter 1 with low byte of parameter 2
	add r25, r0		; Add result to answer, NOTE adding to 2'nd register of result
	adc r26, r1		; NOTE add with carry to 3'rd register of result
	adc r27, r2		; add with carry bit (zero from r2 register)
	
	mul r28, r31	; multiply high byte of first parameter with low byte of second parameter
	add r25, r0		; Add result to answer, NOTE adding to 2'nd register of result
	adc r26, r1		; NOTE add with carry to 3'rd register of result
	adc r27, r2		; add with carry bit (zero from r2 register)
	
	pop r2
	ret
	
; perform logical shift left (lsl instruction) on 16 bit value
; 16 bit value recieved through r25:r24 registers 
; r25 - MSB
; r24 - LSB
; function will return logical shift result in r25:r24 registers
lsl_uint_16bit:
	lsl r25
	lsl r24
	
	brcc _lsl_uint_16bit_exit 			;  Branch if Carry Cleared
	
	; set first bit in r25 register
	ori r25, 0x01
	
_lsl_uint_16bit_exit:
	ret
	
; perform logical shift right (lsr instruction) on 16 bit value
; 16 bit value recieved through r25:r24 registers 
; r25 - MSB
; r24 - LSB
; function will return logical shift result in r25:r24 registers
lsr_uint_16bit:	
	lsr r24
	lsr r25
	
	brcc _lsr_uint_16bit_exit 			;  Branch if Carry Cleared
	
	; if Carry flag is 1, set 1 to 7th bit of r24 register
	ori r24, 0x80
	
_lsr_uint_16bit_exit:
	ret
