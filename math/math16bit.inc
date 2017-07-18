MATH_16BIT = 0		; define MATH_16BIT

; do 16 bit unsigned integer division 
; value1 - r29:r28 dividend
; value2 - r31:r30 devider
; function return result in r25:r24 and r27:r28 registers
; r25:r24 - integer result of division
; r27:r26 - reminder of division
div_unit_16bit:
	push r16
	push r17
; TODO: create this functionality
	clr r17
div_unit_16bit_loop:
	

	pop r17
	pop r16
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
