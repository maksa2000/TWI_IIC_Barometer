; interrupt vector table
; NB! Better allways create whole vector table

;.text 0
.org 0x0000
jmp main									; reset vector
jmp return_from_interrupt					; 0x0002
jmp return_from_interrupt					; 0x0004
jmp return_from_interrupt					; 0x0006
jmp return_from_interrupt					; 0x0008
jmp return_from_interrupt					; 0x000A
jmp bmp_085_watchdog_timeout_iterrupt		; 0x000C		watchdog timeout interrupt
jmp return_from_interrupt					; 0x000E
jmp return_from_interrupt					; 0x0010
jmp return_from_interrupt					; 0x0012
jmp return_from_interrupt					; 0x0014
jmp return_from_interrupt					; 0x0016
jmp return_from_interrupt					; 0x0018
jmp return_from_interrupt					; 0x001A
jmp return_from_interrupt					; 0x001C
jmp return_from_interrupt					; 0x001E
jmp return_from_interrupt					; 0x0020		timer overflow interrupt
jmp return_from_interrupt					; 0x0022	
jmp return_from_interrupt					; 0x0024
jmp return_from_interrupt					; 0x0026	- this interrupt will work just in case :) when USART data register is empty and TWI interrupt will be lost, because when interrupt accurs global interrupt flag will be disabled until reti instruction execute
jmp return_from_interrupt					; 0x0028
jmp return_from_interrupt					; 0x002A
jmp return_from_interrupt					; 0x002C
jmp return_from_interrupt					; 0x002E
jmp return_from_interrupt					; 0x0030
jmp return_from_interrupt					; 0x0032

;.balign 2	; this will align main function to even byte (otherwise code is not executable)
main:
	; set debuging led for blinking
	call init_led
	
	; enable sleep mode (idle)
	ldi r24, 0x01				
	call set_sleep_mode
	
	; init usart
	; USART for some reason generates TWI interrupt, so I disable it
	call usart_init_rx_tx
	call usart_disable_interupts
	
	; initialize interfaces and internal variables for bmp085 
	call bmp_085_init
	
	;call watchdog_init_interrupt_mode							
	
	sei
	
	;call twi_send_start_condition
	
	ldi r24, BMP085_AC1_LSB
	ldi r25, BMP085_AC1_MSB
	call bmp_085_read_calibrations
	
_sleep_loop:	
	sleep
	;call bmp_085_reset_actions_and_states
	rjmp _sleep_loop
	ret
	
; send twi status to usrt
send_to_usart:
	push r24
	push r25
	call hex2str
	call usart_transmit_data
	mov r24, r25
	call usart_transmit_data
	pop r25
	pop r24
	ret
	
delayFunc:
    ldi  r18, 41
    ldi  r19, 150
    ldi  r20, 128
L1: dec  r20
    brne L1
    dec  r19
    brne L1
    dec  r18
    brne L1
	ret
	
return_from_interrupt:
	reti
	
; function includes should be allways after interrupt vector table and better after main function
.ifndef ATMEGA328P_CONSTANTS
.include "atmega328p_core/const_codes.inc"
.endif

.ifndef ATMEGA328P_TWI
.include "atmega328p_core/twi.inc"
.endif

.ifndef SLEEP_MODE
.include "atmega328p_core/set_sleep_mode.inc"
.endif

.ifndef ATMEGA328P_USART
.include "atmega328p_core/usart.inc"
.endif

.ifndef ARDUINO_LED
.include "arduino_core/led.inc"
.endif
	
.ifndef BMP_085_MODULE
.include "bmp_085_module/bmp_085.inc"
.endif

.ifndef WATCHDOG
.include "atmega328p_core/watchdog.inc"
.endif

.end
