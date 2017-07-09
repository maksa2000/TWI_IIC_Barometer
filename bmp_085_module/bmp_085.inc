BMP_085_MODULE = 0

.ifndef ATMEGA328P_TWI
.include "atmega328p_core/twi.inc"
.endif

.ifndef WATCHDOG
.include "atmega328p_core/watchdog.inc"
.endif

;.text 0

; those functions expect that twi interrupt is enabled
.global bmp_085_get_temperature
.global bmp_085_read_calibrations
.global bmp_085_reset_actions_and_states

; initialize interfaces and internal variables for bmp085
; function does not take nor recieve parameters
bmp_085_init:
	push r24
	; set TWI control register to start mode
	call twi_init_twcr
	
	; set bit rate prescaler for atmega328p
	ldi r24, BMP085_BITRATE_PRESCALER
	call twi_set_twbr_atmega328p_prescaler
	
	call bmp_085_reset_actions_and_states
	
	pop r24
	ret

; reads calibration values from BMP085 sersor EEPROM
; function recieve through r24 and r25 registers what calibration it should read
; r24 - calibration LSB address
; r25 - calibration MSB address
; if some of calibration values equal 0x0000 or 0xFFFF, that I must reset arduino 
bmp_085_read_calibrations:
	; save current state of X register (will use X register as base pointer)
	push r26
	push r27
	; move current stack pointer position to Z register
	in r26, SPL
	in r27, SPH
	; save recieved values to stack
	push r24	;- LSB
	push r25	;- MSB
	
	call twi_send_start_condition
	
	call twi_get_status
	
	; if status not equal START or RSTART execute error_handler and exit
	cpi r24, TWI_START_CONDITION
	breq _bmp_085_read_calibrations_send_r_address
	
	cpi r24, TWI_RSTART_CONDITION
	breq _bmp_085_read_calibrations_send_r_address
	
	call bmp_085_error_handler
	rjmp _bmp_085_read_calibrations_exit
	
_bmp_085_read_calibrations_send_r_address:

	ldi r24, BMP085_MODULE_ADDR_W
	call twi_send_address
	
	;cpi r24, TWI_MR_SLA_R_ACK
	;breq _bmp_085_read_calibrations_send_reg_address
	
	;call bmp_085_error_handler
	;rjmp _bmp_085_read_calibrations_exit

_bmp_085_read_calibrations_send_reg_address:
	; load calibration LSB from stack
	;ld r24, X
	;call twi_send_address
	
	ldi r24, BMP085_AC1_MSB
	call twi_send_address
	
	call twi_send_stop_condition
	
;	ldi r24, 0x20
;dalay_waiting:
;	call delayFunc
;	dec r24
;	cpi r24, 0x00
;	brne dalay_waiting
	
	call twi_init_twcr
	call twi_send_start_condition
	
	ldi r24, BMP085_MODULE_ADDR_R
	call twi_send_address
	
	ldi r24, BMP085_AC1_MSB
	call twi_send_address
	
	call twi_read_data
	
	; must send acknowledgement before LSB read
	;call twi_read_data
	
	;call twi_get_status
	
	; debug -->
	push r16
	mov r16, r24
	call send_to_usart
	mov r24, r16
	pop r16
	; debug <--
	
	; load calibration MSB from stack
	;sbiw r26, 0x01		; move to the next address
	;ld r24, X
	;adiw r26, 0x01		; set base pointer back to initial value
	
	;call twi_send_data
	
	;call twi_get_status
	
;	cpi r24, TWI_MR_DATA_R_ACK
;	breq _bmp_085_read_calibrations_send_stop
;	
;	call bmp_085_error_handler
;	rjmp _bmp_085_read_calibrations_exit
;	
;_bmp_085_read_calibrations_send_stop:
;	call twi_send_stop_condition
;	
;	call twi_read_data
	
_bmp_085_read_calibrations_exit:
	pop r25
	pop r24
	pop r27
	pop r26
	ret
	
; resets actions and states variables values
bmp_085_reset_actions_and_states:
	push r26
	push r27
	push r28
	push r29
	
	; set current action
	ldi r26, 0x00
	sts twi_current_action, r26
	
	; set next action
	inc r26
	sts twi_next_action, r26
	
	; store pointer to next state to next state variable
	ldi r26, lo8(twi_next_state)
	ldi r27, hi8(twi_next_state)
	; load pointer to the next state from state queue to Y register
	ldi r28, lo8(twi_mt_states_queue)
	ldi r29, hi8(twi_mt_states_queue)
	call bmp_085_store_pointer_to_mem

	pop r29
	pop r28
	pop r27
	pop r26
	ret
	
; this function should be executed to process different twi statuses
bmp_085_process_twi_status:
	push r16
	push r17
	
	lds r16, twi_current_state
	lds r17, twi_current_action

	cpi r17, READ_CALIBRATION_ACTION
	breq _bmp_085_process_twi_status_read_data
	
	cpi r17, READ_DATA_ACTION
	breq _bmp_085_process_twi_status_read_data
	
	cpi r17, WRITE_DATA_ACTION
	breq _bmp_085_process_twi_status_write_data

_bmp_085_process_twi_status_read_data:
	rjmp _bmp_085_process_twi_status_exit
	
_bmp_085_process_twi_status_write_data:
	rjmp _bmp_085_process_twi_status_exit
	
_bmp_085_process_twi_status_exit:
	pop r17
	pop r16
	ret
	
; error handler function
; this function does not take nor recieve parameters
bmp_085_error_handler:
	push r16
	; if action equals read calibrations restart device
	lds r16, twi_next_action
	cpi r16, READ_CALIBRATION_ACTION
	brne _bmp_085_error_handler_other
	call bmp_085_device_reset
_bmp_085_error_handler_other:
	; if action not real calibrations, then reset watchdog and exit from function
	pop r16
	ret
	
; reset device
bmp_085_device_reset:
	call watchdog_init_reset_mode
	sleep
	ret	
	
; store memory pointer to memory location
; memory location (destination) must be set in X register
; pointer to store (source), must be set in Y register
; function does not return any value
bmp_085_store_pointer_to_mem:
	st X, r28
	adiw r26, 0x01		; move to high byte
	st X, r29
	ret

bmp_085_get_temperature:
	ret
	
; watchdog timeout interrupt
bmp_085_watchdog_timeout_iterrupt:
	push r24
	
	call watchdog_interrupt_disable
	
	; set indirect address for actions queue
	; clear X register HIGH and LOW byte
	clr r27	
	clr r26
	
	lds r24, twi_next_action			; get next action from variable
	sts twi_current_action, r24			; save next action to current action
	
_bmp_085_watchdog_timeout_iterrupt_exit:
	pop r24
	reti
	
.equ BMP085_MODULE_ADDR_W, 0xEE
.equ BMP085_MODULE_ADDR_R, 0xEF
.equ BMP085_TEMP_REG_ADDR, 0x2E
.equ BMP085_PRES_REG0_ADDR, 0x34
.equ BMP085_PRES_REG1_ADDR, 0x74
.equ BMP085_PRES_REG2_ADDR, 0xB4
.equ BMP085_PRES_REG3_ADDR, 0xF4
.equ BMP085_BITRATE_PRESCALER, 0x48

; Calibration coefficients registers
.equ BMP085_AC1_MSB, 0xAA
.equ BMP085_AC1_LSB, 0xAB
.equ BMP085_AC2_MSB, 0xAC
.equ BMP085_AC2_LSB, 0xAD
.equ BMP085_AC3_MSB, 0xAE
.equ BMP085_AC3_LSB, 0xAF
.equ BMP085_AC4_MSB, 0xB0
.equ BMP085_AC4_LSB, 0xB1
.equ BMP085_AC5_MSB, 0xB2
.equ BMP085_AC5_LSB, 0xB3
.equ BMP085_AC6_MSB, 0xB4
.equ BMP085_AC6_LSB, 0xB5
.equ BMP085_B1_MSB, 0xB6
.equ BMP085_B1_LSB, 0xB7
.equ BMP085_B2_MSB, 0xB8
.equ BMP085_B2_LSB, 0xB9
.equ BMP085_MB_MSB, 0xBA
.equ BMP085_MB_LSB, 0xBB
.equ BMP085_MC_MSB, 0xBC
.equ BMP085_MC_LSB, 0xBD
.equ BMP085_MD_MSB, 0xBE
.equ BMP085_MD_LSB, 0xBF

; actions
.equ READ_CALIBRATION_ACTION, 0x01
.equ READ_DATA_ACTION, 0x02
.equ WRITE_DATA_ACTION, 0x03
	
.data 0
twi_next_state:						; holds pointer to the next state in state queue
.word 0
twi_current_action:					; to void ponter movement complexity actions would be just byte and every next action is previous_action + 1		
.byte 0
twi_next_action:					; holds pointer to next action in actions queue
.byte 0
twi_current_state:
.byte 0

; NOTE! move those tables to Flash memory
twi_mt_states_queue:				; master transmis states
.byte TWI_START_CONDITION
.byte TWI_RSTART_CONDITION			
.byte TWI_MT_SLA_W_ACK				; master transmit slave address write acknowlengement recieved			
.byte TWI_MT_DATA_W_ACK				; master transmit slave address write not acknowlengement recieved
.byte TWI_NO_STATE_INFO
.byte TWI_BUS_ERR
.byte TWI_MT_SLA_W_NACK
.byte TWI_MT_DATA_W_NACK

twi_mr_states_queue:			; master recieve states
.byte TWI_START_CONDITION
.byte TWI_RSTART_CONDITION			
.byte TWI_MR_SLA_R_ACK			; master recieve slave address read acknowledgement recieved			
.byte TWI_MR_DATA_R_ACK			; master recieve slave address read not acknowledgement recieved
.byte TWI_NO_STATE_INFO
.byte TWI_BUS_ERR
.byte TWI_MR_SLA_R_NACK
.byte TWI_MR_DATA_R_NACK

; colibration data values
bmp085_ac1_val:
.byte 0
.byte 0
bmp085_ac2_val:
.byte 0
.byte 0
bmp085_ac3_val:
.byte 0
.byte 0
bmp085_ac4_val:
.byte 0
.byte 0
bmp085_ac5_val:
.byte 0
.byte 0
bmp085_ac6_val:
.byte 0
.byte 0
bmp085_b1_val:
.byte 0
.byte 0
bmp085_b2_val:
.byte 0
.byte 0
bmp085_mb_val:
.byte 0
.byte 0
bmp085_mc_val:
.byte 0
.byte 0
bmp085_md_val:
.byte 0
.byte 0
