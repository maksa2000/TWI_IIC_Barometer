BMP_085_MODULE = 0

.ifndef ATMEGA328P_TWI
.include "atmega328p_core/twi.inc"
.endif

.ifndef WATCHDOG
.include "atmega328p_core/watchdog.inc"
.endif

.text
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

.equ BMP085_CONV_RESULT_MSB, 0xF6
.equ BMP085_CONV_RESULT_LSB, 0xF7

; actions
.equ READ_CALIBRATION_ACTION, 0x01
.equ READ_DATA_ACTION, 0x02
.equ WRITE_DATA_ACTION, 0x03

; initialize interfaces and internal variables for bmp085
; function does not take nor recieve parameters
bmp_085_init:
	push r24
	
	; set TWI control register to start mode
	ldi r24, 0x44					; pass initial TWCR register value to the function (0100 0100b)
	rcall twi_init_twcr
	
	; set bit rate prescaler for atmega328p
	ldi r24, BMP085_BITRATE_PRESCALER
	rcall twi_set_twbr_atmega328p_prescaler
	
	rcall bmp_085_reset_actions_and_states
	
	pop r24
	ret
	
; reads sensor values from BMP085
; function recieve through r24 sensor address
; r24 - sensor address
; function does not return values
bmp_085_request_sensor_data:
	; save current state of X register (will use X register as base pointer)
	push r26
	push r27
	
	; move current stack pointer position to X register
	in r26, SPL	
	in r27, SPH
	
	push r24	;- sensor address
	
	rcall twi_send_start_condition
	
	rcall twi_get_status
	
	; if status not equal START or RSTART execute error_handler and exit
	cpi r24, TWI_START_CONDITION
	breq _bmp_085_request_sensor_data_send_address_w
	
	cpi r24, TWI_RSTART_CONDITION
	breq _bmp_085_request_sensor_data_send_address_w
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_request_sensor_data_exit
	
_bmp_085_request_sensor_data_send_address_w:

	ldi r24, BMP085_MODULE_ADDR_W
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MT_SLA_W_ACK
	breq _bmp_085_request_sensor_data_send_reg_address_w
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_request_sensor_data_exit
	
_bmp_085_request_sensor_data_send_reg_address_w:
	; load calibration address from the stack
	ldi r24, 0xF4
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MT_DATA_W_ACK
	breq _bmp_085_request_sensor_data_send_control_value_w
	rcall bmp_085_error_handler
	rjmp _bmp_085_request_sensor_data_exit

_bmp_085_request_sensor_data_send_control_value_w:
	; load control address from the stack
	sbiw r26, 0x01		; set X register to location of control address position
	ld r24, X
	adiw r26, 0x01		; set X register back to base pointer 
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MT_DATA_W_ACK
	breq _bmp_085_request_sensor_data_exit
	rcall bmp_085_error_handler
	rjmp _bmp_085_request_sensor_data_exit
	
_bmp_085_request_sensor_data_exit:
	rcall twi_send_stop_condition
	
	; set stack pointer to base pointer
	out SPL, r26	
	out SPH, r27
	
	pop r27
	pop r26
	
	ret
	
	
	
; reads sensor values from BMP085
; function recieve through r24 sensor address
; r24 - sensor address
; function returns 16 bit calibration value
; r24 - data LSB
; r25 - data MSB
bmp_085_read_sensor_data:
	; save current state of X register (will use X register as base pointer)
	push r26
	push r27
	; move current stack pointer position to X register
	in r26, SPL	
	in r27, SPH
	; save recieved values to stack
	push r24	;- sensor address
	; reserve 16 bit in the stack for saving return value
	sbiw r26, 0x02
	out SPL, r26
	out SPH, r27
	adiw r26, 0x02		; set base pointer back
	
	rcall twi_send_start_condition
	
	rcall twi_get_status
	
; if status not equal START or RSTART execute error_handler and exit
	cpi r24, TWI_START_CONDITION
	breq _bmp_085_read_sensor_data_send_address_w
	
	cpi r24, TWI_RSTART_CONDITION
	breq _bmp_085_read_sensor_data_send_address_w
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_sensor_data_exit

_bmp_085_read_sensor_data_send_address_w:

	ldi r24, BMP085_MODULE_ADDR_W
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MT_SLA_W_ACK
	breq _bmp_085_read_sensor_data_send_restart_condition
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_sensor_data_exit
	
_bmp_085_read_sensor_data_send_restart_condition:

	;rcall twi_send_stop_condition	

	rcall twi_send_start_condition
	
	rcall twi_get_status
	
; if status not equal START or RSTART execute error_handler and exit
	cpi r24, TWI_START_CONDITION
	breq _bmp_085_read_sensor_data_send_address_r
	
	cpi r24, TWI_RSTART_CONDITION
	breq _bmp_085_read_sensor_data_send_address_r
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_sensor_data_exit
	
_bmp_085_read_sensor_data_send_address_r:
	ldi r24, BMP085_MODULE_ADDR_R
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MR_SLA_R_ACK
	breq _bmp_085_read_sensor_data_send_reg_address_r
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_sensor_data_exit

_bmp_085_read_sensor_data_send_reg_address_r:
	; send sensor address
	ldi r24, BMP085_CONV_RESULT_MSB		; read data from MSB register	
	rcall twi_send_byte
	
	rcall twi_get_status
	
	; read data MSB
	rcall twi_read_data
	
	; save MSB to reserved stack position
	sbiw r26, 0x02 		; move to correct stack address
	st X, r24			; save MSB to stack
	adiw r26, 0x02		; set base pointer back to initial value
	
	rcall twi_get_status
	
	cpi r24, TWI_MR_DATA_R_ACK
	breq _bmp_085_read_sensor_data_send_reg_address2_r
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_sensor_data_exit
	
_bmp_085_read_sensor_data_send_reg_address2_r:
	; send sensor address
	ldi r24, BMP085_CONV_RESULT_LSB			; read data from LSB register
	rcall twi_send_byte
	
	; read data LSB
	rcall twi_read_data
	
	; save LSB to reserved stack position
	sbiw r26, 0x01 		; move to correct stack address
	st X, r24			; save LSB to stack
	adiw r26, 0x01		; set base pointer back to initial value
	
	rcall twi_get_status
	
	cpi r24, TWI_MR_DATA_R_ACK
	breq _bmp_085_read_sensor_data_exit
	
	rcall bmp_085_error_handler	
	
_bmp_085_read_sensor_data_exit:	

	rcall twi_send_stop_condition	
	
	; resore return value from stack
	sbiw r26, 0x01 		; move to correct stack address
	ld r24, X			; save LSB from stack
	adiw r26, 0x01		; set base pointer back to initial value
	sbiw r26, 0x02 		; move to correct stack address
	ld r25, X			; save MSB from stack
	adiw r26, 0x02		; set base pointer back to initial value

	; set stack pointer to base pointer
	out SPL, r26	
	out SPH, r27
	pop r27
	pop r26
	ret

; reads calibration values from BMP085 sersor EEPROM
; function recieve through r24 register what calibration it should read
; r24 - calibration address
; function returns 16 bit calibration value in r25:r24 registers
; r24 - LSB calibration value
; r25 - MSB calibration value
bmp_085_read_calibration:
	; save current state of X register (will use X register as base pointer)
	push r26
	push r27
	; move current stack pointer position to Z register
	in r26, SPL	
	in r27, SPH
	; save recieved value to stack
	push r24	;- callibration address
	; reserve 16bit for return data
	; [base + 0x01] - MSB
	; [base + 0x02] - LSB
	
	sbiw r26, 0x03		; because since saving stack pointer value stack pointer was growing with 3 bytes
	out SPL, r26
	out SPH, r27
	adiw r26, 0x03		; set base pointer back
	
	rcall twi_send_start_condition
	
	rcall twi_get_status
	
	; if status not equal START or RSTART execute error_handler and exit
	cpi r24, TWI_START_CONDITION
	breq _bmp_085_read_calibration_send_address_w
	
	cpi r24, TWI_RSTART_CONDITION
	breq _bmp_085_read_calibration_send_address_w
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_calibration_exit
	
_bmp_085_read_calibration_send_address_w:

	ldi r24, BMP085_MODULE_ADDR_W
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MT_SLA_W_ACK
	breq _bmp_085_read_calibration_send_reg_address_w
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_calibration_exit

_bmp_085_read_calibration_send_reg_address_w:	
	; load calibration address from the stack
	ld r24, X
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MT_DATA_W_ACK
	breq _bmp_085_read_calibration_send_stop_w
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_calibration_exit
	
_bmp_085_read_calibration_send_stop_w:	
	;rcall twi_send_stop_condition
	
	rcall twi_send_start_condition
	
	rcall twi_get_status
	
; if status not equal START or RSTART execute error_handler and exit
	cpi r24, TWI_START_CONDITION
	breq _bmp_085_read_calibration_send_address_r
	
	cpi r24, TWI_RSTART_CONDITION
	breq _bmp_085_read_calibration_send_address_r
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_calibration_exit
	
_bmp_085_read_calibration_send_address_r:
	ldi r24, BMP085_MODULE_ADDR_R
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MR_SLA_R_ACK
	breq _bmp_085_read_calibration_send_reg_address_r
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_calibration_exit

_bmp_085_read_calibration_send_reg_address_r:	; read MSB byte
	; load calibration address from stack
	ld r24, X
	rcall twi_send_byte
	
	; read calibration data
	rcall twi_read_data
	
	; save data to MSB return value
	sbiw r26, 0x01
	st X, r24
	adiw r26, 0x01
	
	rcall twi_get_status
	
	cpi r24, TWI_MR_DATA_R_ACK
	breq _bmp_085_read_calibration_send_reg_address2_r
	rcall bmp_085_error_handler
	
_bmp_085_read_calibration_send_reg_address2_r:	; read LSB byte
	rcall twi_ack_diable
	
	; load calibration address from stack
	ld r24, X
	rcall twi_send_byte
	
	; read calibration data
	rcall twi_read_data
	
	; save data to LSB return value
	sbiw r26, 0x02
	st X, r24
	adiw r26, 0x02
	
	rcall twi_get_status
	
	cpi r24, TWI_MR_DATA_R_NACK
	breq _bmp_085_read_calibration_exit
	rcall bmp_085_error_handler
	
_bmp_085_read_calibration_exit:	
	
	rcall twi_send_stop_condition

	; resore return value from stack
	sbiw r26, 0x01 		; move to correct stack address
	ld r25, X			; load MSB data from stack	
	sbiw r26, 0x01
	ld r24, X			; load LSB data from stack
	adiw r26, 0x02		; set base pointer back to initial value

	; set stack pointer to base pointer
	out SPL, r26	
	out SPH, r27
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
	ldi r26, READ_CALIBRATION_ACTION
	sts twi_current_action, r26
	
	; set next action
	inc r26
	sts twi_next_action, r26

	pop r29
	pop r28
	pop r27
	pop r26
	ret
	
; this function should be executed to process different twi statuses
bmp_085_process_twi_status:
	push r16
	push r17
	
	;lds r16, twi_current_state
	;lds r17, twi_current_action

	;cpi r17, READ_CALIBRATION_ACTION
	;breq _bmp_085_process_twi_status_read_data
	
	;cpi r17, READ_DATA_ACTION
	;breq _bmp_085_process_twi_status_read_data
	
	;cpi r17, WRITE_DATA_ACTION
	;breq _bmp_085_process_twi_status_write_data

;_bmp_085_process_twi_status_read_data:
;	rjmp _bmp_085_process_twi_status_exit
	
;_bmp_085_process_twi_status_write_data:
;	rjmp _bmp_085_process_twi_status_exit
	
;_bmp_085_process_twi_status_exit:
	pop r17
	pop r16
	ret
	
; error handler function
; this function does not take nor recieve parameters
bmp_085_error_handler:
	push r16
	;rcall flash_led
	; if action equals read calibrations restart device
	lds r16, twi_current_action
	cpi r16, READ_CALIBRATION_ACTION
	brne _bmp_085_error_handler_other
	rcall bmp_085_device_reset
_bmp_085_error_handler_other:
	; if action not read calibrations, then reset watchdog and exit from function
	pop r16
	ret
	
; reset device
bmp_085_device_reset:
	rcall watchdog_init_reset_mode
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
	
	rcall watchdog_interrupt_disable
	
	; set indirect address for actions queue
	; clear X register HIGH and LOW byte
	clr r27	
	clr r26
	
	;lds r24, twi_next_action			; get next action from variable
	;sts twi_current_action, r24			; save next action to current action
	
_bmp_085_watchdog_timeout_iterrupt_exit:
	pop r24
	reti
	
.data 0
;.org 0x00A2							; set correct address for data segment to 0x0100 (beginning of internal SRAM)		
.balign 2								; this will align variables to even byte (otherwise code is not executable)
twi_current_action:					; current action on twi bus
.byte 0		
twi_next_action:					; next action on twi bus
.byte 0			

; NOTE! move those tables to Flash memory
twi_mt_states_queue:				; master transmis states
.byte TWI_START_CONDITION				; define byte constant in program memory
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
bmp085_calibration_values:
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
