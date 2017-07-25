; interrupt vector table
; NB! Better allways create whole vector table

.text
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

; function includes should be allways after interrupt vector table and better after main function
.ifndef ATMEGA328P_CONSTANTS
.include "atmega328p_core/const_codes.inc"
.endif

.ifndef WATCHDOG
.include "atmega328p_core/watchdog.inc"
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

.ifndef MATH_32BIT
.include "math/math32bit.inc"
.endif

.text
;.balign 2	; this will align main function to even byte (otherwise code is not executable)
main:
	; initialize stack pointer
	ldi r16, lo8(RAMEND)
	out SPL, r16
	ldi r16, hi8(RAMEND)
	out SPH, r16
	
	; set debuging led for blinking
	rcall init_led
	
	; enable sleep mode (idle)
	ldi r24, 0x01				
	rcall set_sleep_mode
	
	; init usart
	; USART for some reason generates TWI interrupt, so I disable it
	rcall usart_init_rx_tx
	rcall usart_disable_interupts	

	; initialize interfaces and internal variables for bmp085 
	rcall bmp_085_init	
	
	sei
	
	rcall read_bmp085_calibrations
	
_sleep_loop:	
	rcall watchdog_init_interrupt_mode
	sleep
	rcall read_bmp085_sensor_values
	rjmp _sleep_loop
	ret
	
; read values from BMP_085 sensor
read_bmp085_sensor_values:
	push r24
	push r25
	
	;ldi r24, BMP085_PRES0_CTRL_ADDR
	ldi r24, BMP085_TEMP_CTRL_ADDR
	rcall bmp_085_request_sensor_data
	
	rcall delayFunc
	
	ldi r23, BMP085_CONV_RESULT_MSB
	ldi r25, BMP085_CONV_RESULT_MSB
	ldi r24, BMP085_CONV_RESULT_LSB
	rcall bmp_085_read_data
	
	rcall bmp_085_calc_temperature
	
	; debug -->
	;push r24
	;mov r24, r25
	;rcall send_to_usart
	;pop r24
	;rcall send_to_usart
	; debug <--
	
	pop r25
	pop r24
	ret
	
; reads BMP085 calibrations
; function does not take nor recieve parameters
; r26 register will reffer to calibration address
; X register (r29:r28) will reffer to calibration values
read_bmp085_calibrations:
	push r16	; counter
	push r24
	push r25
	push r26
	push r28
	push r29
	
	; initializing calibration address
	ldi r26, BMP085_AC1_MSB
	
	; initial Y register
	ldi r28, lo8(bmp085_calibration_values)		; not sure what is difference, but seems that pm_lo8 is used in memory operations
	ldi r29, hi8(bmp085_calibration_values)
	
	; initial counter
	ldi r16, 0x0B				; 11 dec
	
_read_bmp085_calibrations_loop:		
	; load calibration MSB address to r23 register
	mov r23, r26
	; load calibration MSB address to r25 register
	mov r25, r26
	inc r26					; move next address
	mov r24, r26
	inc r26					; move next address
	
	rcall bmp_085_read_data
	
	; check that actual value was recieved (0x0000 or 0xFFFF will be recieved in case of error)
	cpi r24, 0xFF
	brne _read_bmp085_calibrations_zero_check
	
	cpi r25, 0xFF
	brne _read_bmp085_calibrations_zero_check
	
	rcall bmp_085_error_handler
	
_read_bmp085_calibrations_zero_check:
	; if recieved value not 0xFFFF, then check for 0x0000
	cpi r24, 0x00
	brne _read_bmp085_calibrations_save_callibration
	
	cpi r25, 0x00
	brne _read_bmp085_calibrations_save_callibration

	rcall bmp_085_error_handler
	
_read_bmp085_calibrations_save_callibration:

	; save readed values to variable
	st Y, r25
	adiw r28, 0x01		; move to next byte
	st Y, r24
	adiw r28, 0x01		; move to next byte
	
	; dbg
	;push r24
	;mov r24, r25
	;rcall send_to_usart
	;pop r24
	;rcall send_to_usart
	; dbg
	
	dec r16
	; continue while r16 != 0
	brne _read_bmp085_calibrations_loop
	
	pop r29
	pop r28
	pop r26
	pop r25
	pop r24
	pop r16
	ret
	
; send twi status to usrt
send_to_usart:
	push r24
	push r25
	rcall hex2str
	rcall usart_transmit_data
	mov r24, r25
	rcall usart_transmit_data
	pop r25
	pop r24
	ret
	
delayFunc:
	push r18
	push r19
	push r20
	
    ldi  r18, 254
    ldi  r19, 254
    ldi  r20, 254

L1:
	dec  r20
    brne L1
    dec  r19
    brne L1
    dec  r18
    brne L1
	
	pop r20
	pop r19
	pop r18
	ret
	
return_from_interrupt:
	reti

.end
