MATH_8BIT = 0		; define MATH_8BIT

.text
; do 8 bit unsigned integer division
; value1 - r28 dividend
; value2 - r29 divider
; function return result in r24 and r25 registers
; r24 - integer result of division
; r25 - reminder of division 
div_uint_8bit:
	push r16
	push r17
	push r18
	push r19
	push r20
	
	; count leading zeroes in dividend
	mov r24, r28						; pass dividend as parameter
	rcall count_leading_zeroes_8bit
	mov r18, r24						; save result to r18 register
	; count zeroes in devider
	mov r24, r29
	rcall count_leading_zeroes_8bit
	mov r19, r24						; save result to r18 register
		
	clr r24								; r24 will be returned, so I should clear its value
	mov r25, r28						; set reminder to dividend
	
	; calculate number of leading zero bits in devider and subtract zero bits in dividend from zero bits in divider
	sub r19, r18
	brmi div_uint_8bit_exit				; if leading zeroes in dividend more that in divider I should exit with 0 in r24 and reminder in r25	
	
	mov r18, r19						; save zero counter subraction result to r18 register
	mov r17, r29						; move devider to r17 degister
	
shift_divider_loop:
	; shift divider left to number of bits in zero bit counter
	lsl r17								; logical shift devider left
	dec r19								; decrement zero counter, so it will show exact number of zeroes before first 1
	breq division_begining_label		; breq - Tests the Zero flag (Z) and branches relatively to PC if Z is set.
	rjmp shift_divider_loop
	
division_begining_label:
	mov r16, r28						; set intermediate result to dividend initial value
	mov r19, r18						; restore zero counter subtraction result
	inc r19								; increment zeroes counter by 1, because subtraction should be done (zero counter + 1) times
	clr r24								; clear return integer value

division_main_loop:	
	lsl r24								; shift bit that was set in previous instruction 1 position left (at the end of division bit should be in right position) (first time will shift all zeroes)
	mov r18, r16						; store intermediate division result in r18 register
	sub r18, r17						; subtract r17 from intermediate result (sub will change negative flag of SREG, so I should not do cpi...) (r17 is shifted divider)
	brmi division_main_loop_continue	; branch if result negative
	; if result greater that 0, set appropriate bit in r24 register		
	ori r24, 0x01
	; save intermediate rusult back to r16
	mov r16, r18
division_main_loop_continue:
	dec r19
	breq div_uint_8bit_exit
	lsr r17								; shit divider rigth
	rjmp division_main_loop
div_uint_8bit_exit:
	; move reminder to r25 register
	mov r25, r16

	pop r20
	pop r19
	pop r18
	pop r17
	pop r16
	ret
	
; this function is counting leading zeroes in 8bit number
; this fuction recieve 8bit number which leading zeroes should be counted in r24 register (to not override other register values)
; result is 8bit number, will be return in r24 register
count_leading_zeroes_8bit:
	push r17
	push r18
	push r19
	push r20
	
	mov r20, r24						; save initial passed value 						
	clr r24								; clear returning value
	mov r19, r20						; save temporary value of passed parameter to r19
	ldi r18, 0x80						; set 7th bit to 1, this valus will be used to mask value in r16 register and check for highest set bit in it
	ldi r17, 0x08						; set loop counter (count_leading_zeroes_loop loop should not take more than 8 interactions)
	
	; count leading zeroes
count_leading_zeroes_loop:
	and r19, r18						; put mask to passed parameter register
	brne exit_count_leading_zeroes_8bit	; Tests the Zero flag (Z) and branches relatively to PC if Z is cleared.
	inc r24								; increment zero bit counter
	lsr r18								; shift r19 left (change mast to set (1 will move to the next lowest bit))
	mov r19, r20						; set r16 to initial value
	dec r17
	brne count_leading_zeroes_loop		; Tests the Zero flag (Z) and branches relatively to PC if Z is cleared.
	
exit_count_leading_zeroes_8bit:
	pop r20
	pop r19
	pop r18
	pop r17
	ret
	