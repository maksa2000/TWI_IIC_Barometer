; this file contain any numeric converters functionality
; i.e. hex2str, dec2str, bin2str

NUMERIC_CONVERTERS = 0

.global hex2str
.global r16_value2str

; this function will convert hexadecimal byte to string
; byte value should be passed through r24 register
; string value will be returned in r24:r25 registers
hex2str:
	push r16
	push r17				; will use r17 as iteration counter
	
	ldi r17, 0x04
	mov r16, r24
	
	; get high half byte from r16 register
	swap r16				; swap high and low nibbles (half bytes) in r16
	andi r16, 0x0F
	
	; check what number do we have in upper half byte of passed parameter and put appropriate character to r25 register 
	rcall r16_value2str
	mov r25, r16			; move ascii character from r16 to r25
	
	mov r16, r24
	; clear high half byte of r16 value
	andi r16, 0x0F
	rcall r16_value2str
	mov r24, r16			; move ascii value from r16 to r24
	
	; switch r24 and r25 values
	push r24		; save value from r24 to the stack
	mov r24, r25
	pop r25			; restore value from r24 to r25
	
	pop r17
	pop r16
	ret;
	
; get value (hex, dec, oct, bin) from r16 register and replace it with appropriate ascii code
r16_value2str:
	push r17
	
	; next code comparing r16 with hex values and if values are equal move ascii code from r17 to r16 and exit
	ldi r17, 0x30					; move to r17 appropriate ascii code
	cpi r16, 0x00					; compare r16 with 0
	breq _r16_value2str_exit			; if equal, than move value from r17 to r16 and exit	
	
	ldi r17, 0x31
	cpi r16, 0x01
	breq _r16_value2str_exit
	
	ldi r17, 0x32
	cpi r16, 0x02
	breq _r16_value2str_exit
	
	ldi r17, 0x33
	cpi r16, 0x03
	breq _r16_value2str_exit
	
	ldi r17, 0x34
	cpi r16, 0x04
	breq _r16_value2str_exit
	
	ldi r17, 0x35
	cpi r16, 0x05
	breq _r16_value2str_exit
	
	ldi r17, 0x36
	cpi r16, 0x06
	breq _r16_value2str_exit
	
	ldi r17, 0x37
	cpi r16, 0x07
	breq _r16_value2str_exit
	
	ldi r17, 0x38
	cpi r16, 0x08
	breq _r16_value2str_exit
	
	ldi r17, 0x39
	cpi r16, 0x09
	breq _r16_value2str_exit
	
	ldi r17, 0x41
	cpi r16, 0x0A
	breq _r16_value2str_exit
	
	ldi r17, 0x42
	cpi r16, 0x0B
	breq _r16_value2str_exit
	
	ldi r17, 0x43
	cpi r16, 0x0C
	breq _r16_value2str_exit
	
	ldi r17, 0x44
	cpi r16, 0x0D
	breq _r16_value2str_exit
	
	ldi r17, 0x45
	cpi r16, 0x0E
	breq _r16_value2str_exit
	
	ldi r17, 0x46
	
_r16_value2str_exit:
	mov r16, r17		; move ascii code from r17 to r16
	pop r17
	ret
	