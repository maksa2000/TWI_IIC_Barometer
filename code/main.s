; interrupt vector table
; NB! Better allways create whole vector table
.org 0x0000
jmp main									; reset vector
jmp return_from_interrupt					; 0x0002
jmp return_from_interrupt					; 0x0004
jmp return_from_interrupt					; 0x0006
jmp return_from_interrupt					; 0x0008
jmp return_from_interrupt					; 0x000A
jmp watchdog_timeout_iterrupt				; 0x000C		watchdog timeout interrupt
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
jmp twi_interrupt							; 0x0030
jmp return_from_interrupt					; 0x0032

; function includes should be allways after interrupt vector table
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
	
	call watchdog_init_interrupt_mode
	
	sei							; enable global interupts and reset TWCR register
	
begin_transmission:
	
;	ldi r24, BMP085_MODULE_ADDR_W				; pass BMP085 module address as parameter
;	call twi_send_address
	
	; Check value of TWI Status Register. Mask prescaler bits. If status different from MT_SLA_ACK go to ERROR
;	call twi_get_status
;	cpi r24, 0x48
;	brne continue
;	call twi_send_stop_condition
;	rjmp begin_transmission
;continue:
	; send device address
	;ldi r24, BMP085_TEMP_REG_ADDR
	;call twi_send_address
	; send control register
	;ldi r24, BMP085_TEMP_REG_ADDR
	;call twi_send_address
	
	;call twi_send_stop_condition
	
	;call twi_get_status
	;call send_to_usart
	
sleep_loop:	
	lds r16, SREG
	sbrs r16, 0x07		; skip next instruction if global interupt enable bit is set
	sei
	sleep
	rjmp sleep_loop
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
	
return_from_interrupt:
	reti
	
twi_interrupt:
	push r16
	
	; load to r16 value from address in X register
	ld r16, X
	adiw r26, 0x01		; increment X register by 1
	; check TWI status
	call twi_get_status
	cp r24, r16
	
	;brne sleep_loop
	
	;call send_to_usart
exit_twi_interrupt:
	pop r16
	reti
	
; watchdog timeout interrupt
watchdog_timeout_iterrupt:
	push r24
	
	;call flash_led
	call watchdog_interrupt_disable
	
	; set indirect address to begining of twi_mt_states
	; clear X register HIGH and LOW byte
	clr r27	
	clr r26
	; load twi_mt_states address to X register (address is 16 bit value)
	ldi r26, lo8(twi_mt_states)
	ldi r27, hi8(twi_mt_states)
	
	; set TWI control register to start mode
	call twi_init_twcr
	
	; set bit rate prescaler for atmega328p
	ldi r24, BMP085_BITRATE_PRESCALER
	call twi_set_twbr_atmega328p_prescaler
	
	call twi_send_start_condition
	
	pop r24
	reti
	
.section data
timer0_overflow_interrupt_cnt:
.byte 0
twi_data_value:
.byte 0
twi_current_state:
.byte 0
twi_next_state:
.byte TWI_START_CONDITION

twi_mt_states:					; master transmis states
twi_mt_start:
.byte TWI_START_CONDITION
twi_mt_rstart:
.byte TWI_RSTART_CONDITION
twi_mt_mt_sla_w_ack:			; master transmit slave address write acknowlengement recieved
.byte TWI_MT_SLA_W_ACK
twi_mt_sla_data_w_ack:				; master transmit slave address write not acknowlengement recieved
.byte TWI_MT_DATA_W_ACK
twi_no_mt_state_info:
.byte TWI_NO_STATE_INFO
twi_bus_mt_err:
.byte TWI_BUS_ERR
twi_mt_sla_w_nack:
.byte TWI_MT_SLA_W_NACK
twi_mt_data_w_nack:
.byte TWI_MT_DATA_W_NACK

twi_mr_states:					; master recieve states
twi_mr_start:
.byte TWI_START_CONDITION
twi_mr_rstart:
.byte TWI_RSTART_CONDITION
twi_mr_mr_sla_r_ack:			; master recieve slave address read acknowledgement recieved
.byte TWI_MR_SLA_R_ACK
twi_mr_data_r_ack:				; master recieve slave address read not acknowledgement recieved
.byte TWI_MR_DATA_R_ACK
twi_no_mr_state_info:
.byte TWI_NO_STATE_INFO
twi_bus_mr_err:
.byte TWI_BUS_ERR
twi_mr_sla_r_nack:
.byte TWI_MR_SLA_R_NACK
twi_mr_data_r_nack:
.byte TWI_MR_DATA_R_NACK

.end
