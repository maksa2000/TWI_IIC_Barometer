ATMEGA328P_USART = 0

.ifndef ATMEGA328P_CONSTANTS
.include "atmega328p_core/const_codes.inc"
.endif

.ifndef NUMERIC_CONVERTERS
.include "converters/numeric_convetrers.inc"
.endif

.text
; init usart communication
; this function does not recieve any arguments and does not return any value
; Global Interrupt Flag should be cleared (and interrupts globally disabled) when doing the initialization
usart_init_rx_tx:
	push r16
	; set pins states
	rcall usart_set_pins_state
	
	; setting boud rate to 9600
	rcall usart_set_baud_rate_9600
	
	; enable transmit, recieve or data register empty inerrupts
	;0xF8				; set bit 7, 6, 5, 4, 3 to 1 (transmit, recieve and data register empty interrupt)
	ldi r16, 0xF8	
	sts UCSR0B, r16	
	
	; Set frame format: 8data, 2stop bit
	; 0x0E - 8data, 2stop bit
	; 0x0C - 7bit data and 2stop bits
	; 0x06 - 8bit and 1 stop bit (used by Arduino IDE)
	ldi r16, 0x06
	sts UCSR0C, r16
	
	pop r16
	ret
	
; disable usart interrupts
; function does not recieve nor return parameters
usart_disable_interupts:
	push r16
	lds r16, UCSR0B
	andi r16, 0x1F			; 0001 1111b
	sts UCSR0B, r16
	pop r16
	ret
	
	
; recieve data to transmitt through r24 register
; do not return any value
usart_transmit_data:
	push r16
_usart_transmit_data_loop:
	; check if UCSR0A usart data register empty bit is set, if not go to the beginning of function
	lds r16, UCSR0A
	sbrs r16, UCSR0A_UDRE			; skip next instruction if UCSR0A_UDRE bit in UCSR0A register is set
	rjmp _usart_transmit_data_loop	; using relative jump because it takes lass memory

	; store data to USART data register (transmit data through USART)
	sts UDR0, r24

	pop r16
	ret
	
; sets pins state for USART
; do not recieve and return parameters
; this function should be rcalling only from init_usart_rx_tx, so I will not save r16 register to stack 
usart_set_pins_state:
	in r16, DDRD
	ori r16, 0x02		; set DDRD pin1 as output (transmit pin)
	andi r16, 0xFE		; set DDRD pon0 as input (recieve pin)
	out DDRD, r16
	ret
	
; setting boud rate to 9600 (working for 16Mhz clock)
; UBRR register not hold baud rate, it holds baud rate prescaler 
; in async normal mode UBRR = (System Oscillator clock frequency / 16 * baud) - 1 // baud = baud rate for example 9600
; examples can be found at table 20-4 Examples of UBRRn Settings for Commonly Used Oscillator Frequencies (in atmega328p datasheet)
; this function should be rcalling only from init_usart_rx_tx, so I will not save r16 register to stack 
usart_set_baud_rate_9600:
	push r17
	
	ldi r16, 0x00			
	ldi r17, 0x67
	
	sts UBRR0H, r16		
	sts UBRR0L, r17
	
	pop r17
	ret
	
; set recieve complete bit
; function does not recieve or return parameters
; function was commented out to save RAM space
;usart_set_receive_complete_bit:
;	push r16
;	lds r16, UCSR0A
;	ori r16, 0x80
;	sts UCSR0A, r16
;	pop r16
;	ret
	
; return recieved data through r24 register
; function was commented out to save RAM space
;usart_recieve_data:
;	; read data from usart data register
;	lds r24, UDR0
;	rcall usart_set_udre_bit
;	ret
	
; set usart data register empty bit
; do not recieve and not return any parameters
usart_set_udre_bit:
	push r16
	lds r16, UCSR0A
	ori r16, 0x20
	sts UCSR0A, r16
	pop r16
	ret
	
; this function recieve boolean parameter through r24 register
; if r24 not zero, than transmitter should be enabled, otherwise disabled
usart_enable_transmitter:
	push r16
	lds r16, UCSR0B
	cpi r24, 0x00
	breq _usart_disable_transmitter_branch
	ori r16, 0x04
	rjmp _usart_enable_transmitter_exit
_usart_disable_transmitter_branch:
	andi r16, 0xF7
_usart_enable_transmitter_exit:
	sts UCSR0B, r16
	pop r16
	ret
	
; this function recieve boolean parameter through r24 register
; if r24 not zero, than reciever should be enabled, otherwise disabled
usart_enable_reciever:
	push r16
	lds r16, UCSR0B
	cpi r24, 0x00
	breq _usart_disable_reciever_branch
	ori r16, 0x05
	rjmp _usart_enable_reciever_exit
_usart_disable_reciever_branch:
	andi r16, 0xEF
_usart_enable_reciever_exit:
	sts UCSR0B, r16
	pop r16
	ret

; transmit 32bit data from registers r24:r27
usart_transmit_32bit_data:
	push r16
	push r17
	; save passed parameters to stack for restoring in the end of function
	push r24
	push r25
	push r26
	push r27
	; store passed values to sack for next usage in this function
	push r24
	push r25
	push r26
	push r27
	
	ldi r17, 0x04				; load loop counter to register
_usart_transmit_32bit_data_load:
	pop r24						; pop r27 register value from stack to r24 register
	rcall hex2str				; convert hex value to string (if value was 0x6D, it should return 6D string in r24:r25 registers)
	mov r16, r24				
	mov r24, r25				; should transmit high half byte value first
	rcall usart_transmit_data
	mov r24, r16
	rcall usart_transmit_data	; transmit next value from hex2str function
	dec r17						; decrement loop counter
	cpi r17, 0x00
	brne _usart_transmit_32bit_data_load
	; insert line break
	ldi r24, 0x0A
	rcall usart_transmit_data
	; restore passed values from stack
	pop r27
	pop r26
	pop r25
	pop r24
	pop r17
	pop r16
	ret	
	
; transmit 16bit data from registers r24:r25
usart_transmit_16bit_data:
	push r16
	push r17
	; save passed parameters to stack for restoring in the end of function
	push r24
	push r25
	; store passed values to sack for next usage in this function
	push r24
	push r25
	
	ldi r17, 0x02				; load loop counter to register
_usart_transmit_16bit_data_load:
	pop r24						; pop r27 register value from stack to r24 register
	rcall hex2str				; convert hex value to string (if value was 0x6D, it should return 6D string in r24:r25 registers)
	mov r16, r24				
	mov r24, r25				; should transmit high half byte value first
	rcall usart_transmit_data
	mov r24, r16
	rcall usart_transmit_data	; transmit next value from hex2str function
	dec r17						; decrement loop counter
	cpi r17, 0x00
	brne _usart_transmit_16bit_data_load
	; insert line break
	ldi r24, 0x0A
	rcall usart_transmit_data
	; restore passed values from stack
	pop r25
	pop r24
	pop r17
	pop r16
	ret	

; not recieve nor return any value
; blink if UCSR0B UDRIE0 bit is set
debug_check_ucsr0b_udrie0_bit:
	push r16
	lds r16, UCSR0B
	sbrc r16, 6				; skip next instruction if bit in register is cleared
	rcall flash_led
	pop r16
	ret
	
debug_check_sreg_interrupt_flag:
	push r16
	in r16, SREG
	;lds r16, SREG_MEM
	sbrc r16, 7			; global interrupt flag
	rcall flash_led
	pop r16
	ret
	