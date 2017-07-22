BMP_085_MODULE = 0

.ifndef ATMEGA328P_TWI
.include "atmega328p_core/twi.inc"
.endif

.ifndef WATCHDOG
.include "atmega328p_core/watchdog.inc"
.endif

.ifndef MATH_16BIT
.include "math/math16bit.inc"
.endif

.ifndef MATH_32BIT
.include "math/math32bit.inc"
.endif

.text
.equ BMP085_MODULE_ADDR_W, 0xEE
.equ BMP085_MODULE_ADDR_R, 0xEF
.equ BMP085_TEMP_CTRL_ADDR, 0x2E
.equ BMP085_PRES0_CTRL_ADDR, 0x34
.equ BMP085_PRES1_CTRL_ADDR, 0x74
.equ BMP085_PRES2_CTRL_ADDR, 0xB4
.equ BMP085_PRES3_CTRL_ADDR, 0xF4
.equ BMP085_MEASURE_REG_ADDR, 0xF4
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
	ldi r24, BMP085_MEASURE_REG_ADDR
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MT_DATA_W_ACK
	breq _bmp_085_request_sensor_data_send_control_value_w
	rcall bmp_085_error_handler
	rjmp _bmp_085_request_sensor_data_exit

_bmp_085_request_sensor_data_send_control_value_w:
	; load control address from the stack
	ld r24, X
	
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
	


; reads data from BMP085 EEPROM or sensors
; function recieve 3 parameters through r23, r25:r24 registers
; r23 - register address to read
; r25 - MSB address to read
; r24 - LSB address to read
; function returns 16 bit value in r25:r24 registers
; r24 - recieved LSB value
; r25 - recieved MSB value
bmp_085_read_data:
	; save current state of X register (will use X register as base pointer)
	push r26
	push r27
	; move current stack pointer position to Z register
	in r26, SPL	
	in r27, SPH
	; save recieved value to stack
	push r23	; register address [base]
	push r25	; MSB address [base + 0x01]
	push r24 	; LSB address [base + 0x02]
	; reserve 16bit for return data
	; [base + 0x03] - return MSB
	; [base + 0x04] - return LSB
	
	sbiw r26, 0x05		; because since saving stack pointer value stack pointer was growing with 3 bytes, current r26 should be decreased for 5 bytes
	out SPL, r26
	out SPH, r27
	adiw r26, 0x05		; set base pointer back
	
	rcall twi_send_start_condition
	
	rcall twi_get_status
	
	; if status not equal START or RSTART execute error_handler and exit
	cpi r24, TWI_START_CONDITION
	breq _bmp_085_read_data_send_address_w
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_data_exit
	
_bmp_085_read_data_send_address_w:

	ldi r24, BMP085_MODULE_ADDR_W
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MT_SLA_W_ACK
	breq _bmp_085_read_data_send_reg_address_w
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_data_exit

_bmp_085_read_data_send_reg_address_w:	
	; load register address from the stack
	ld r24, X
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MT_DATA_W_ACK
	breq _bmp_085_read_data_send_stop_w
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_data_exit
	
_bmp_085_read_data_send_stop_w:	
	
	rcall twi_send_start_condition
	
	rcall twi_get_status
	
; if status not equal START or RSTART execute error_handler and exit
	
	cpi r24, TWI_RSTART_CONDITION
	breq _bmp_085_read_data_send_address_r
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_data_exit
	
_bmp_085_read_data_send_address_r:
	ldi r24, BMP085_MODULE_ADDR_R
	rcall twi_send_byte
	
	rcall twi_get_status
	
	cpi r24, TWI_MR_SLA_R_ACK
	breq _bmp_085_read_data_send_reg_address_r
	
	rcall bmp_085_error_handler
	rjmp _bmp_085_read_data_exit

_bmp_085_read_data_send_reg_address_r:	; read MSB byte
	; load MSB address from stack
	sbiw r26, 0x01
	ld r24, X
	adiw r26, 0x01
	rcall twi_send_byte
	
	; read data
	rcall twi_read_data
	
	; save data to MSB return value
	sbiw r26, 0x03
	st X, r24
	adiw r26, 0x03
	
	rcall twi_get_status
	
	cpi r24, TWI_MR_DATA_R_ACK
	breq _bmp_085_read_data_send_reg_address2_r
	rcall bmp_085_error_handler
	
_bmp_085_read_data_send_reg_address2_r:	; read LSB byte
	rcall twi_ack_diable
	
	; load LSB address from stack
	sbiw r26, 0x02
	ld r24, X
	adiw r26, 0x02
	rcall twi_send_byte
	
	; read data
	rcall twi_read_data
	
	; save data to LSB return value
	sbiw r26, 0x04
	st X, r24
	adiw r26, 0x04
	
	rcall twi_get_status
	
	cpi r24, TWI_MR_DATA_R_NACK
	breq _bmp_085_read_data_exit
	rcall bmp_085_error_handler
	
_bmp_085_read_data_exit:	
	
	rcall twi_send_stop_condition
	
	; restore r23 from the stack
	ld r23, X

	; resore return value from stack
	sbiw r26, 0x03 		; move to correct stack address
	ld r25, X			; load MSB data from stack	
	sbiw r26, 0x01
	ld r24, X			; load LSB data from stack
	adiw r26, 0x04		; set base pointer back to initial value

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
	
; error handler function
; this function does not take nor recieve parameters
bmp_085_error_handler:
	push r16
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

; This function recieve UT value through r25:r24 registers
; r25 - temperature MSB 
; r24 - temperature LSB
; this function return actual temperature in r25:r24 registers
; r25 - temperature MSB
; r24 - temperature LSB
bmp_085_calc_temperature:
	; save values of temporary registers
	push r16
	push r17
	push r18
	push r19
	
	; save state of X register
	push r26
	push r27
	
	; save state of Y register
	push r28
	push r29
	
	; save base pointer value
	in r26, SPL	
	in r27, SPH
	
	; save UT temperature value to the stack
	push r25		; MSB
	push r24		; LSB
	
	; reserve space in the stack for 3 32bit variables and 1 16bit variable
	sbiw r26, 0x10	; 2 bites was already added to stack so I should increase X register by 16 bytes (14 + 2)
	out SPL, r26
	out SPH, r27
	adiw r26, 0x10
	
	; X1 - 32bit value
	; X1 byte 4 - [base + 0x02]
	; X1 byte 3 - [base + 0x03]
	; X1 byte 2 - [base + 0x04]
	; X1 byte 1 - [base + 0x05]
	; X2 - 32 bit value
	; X2 byte 4 - [base + 0x06]
	; X2 byte 3 - [base + 0x07]
	; X2 byte 2 - [base + 0x08]
	; X2 byte 1 - [base + 0x09]
	; B5 - 32 bit value
	; B5 byte 4 - [base + 0x0A]
	; B5 byte 4 - [base + 0x0B]
	; B5 byte 4 - [base + 0x0C]
	; B5 byte 4 - [base + 0x0D]
	; T MSB - [base + 0x0E]
	; T LSB - [base + 0x0F]
	
	; initialize Y register with beginning of bmp085_calibration_values array
	ldi r29, hi8(bmp085_calibration_values)
	ldi r28, lo8(bmp085_calibration_values)
	
	; begining of X1 calculation
	
	push r28
	push r29
	push r30
	push r31
	
	; calculate UT - AC6
	; load value of AC6 from memory to first argument of sub_uint_16bit function
	adiw r28, 0x0A
	ld r31, Y			; MSB
	adiw r28, 0x01			
	ld r30, Y			; LSB
	sbiw r28, 0x0B
	
	; load UT value to r29:r28 registers
	ld r29, X
	sbiw r26, 0x01
	ld r28, X
	adiw r26, 0x01
	
	rcall sub_uint_16bit
	
	; move result to temporary registers
	mov r17, r25
	mov r16, r24
	
	pop r31
	pop r30
	pop r29
	pop r28
	
	; calculare TEMP value * AC5
	push r28
	push r29
	push r30
	push r31
	
	; load value2 with temporary value
	mov r31, r17
	mov r30, r16
	
	; load temporary value with AC5 value
	adiw r28, 0x08
	ld r17, Y
	adiw r28, 0x01
	ld r16, Y
	sbiw r28, 0x09
	
	; move AC5 from temporary registers to value1 registers
	mov r29, r17
	mov r28, r16
	
	push r26
	push r27
	
	rcall mul_uint_16bit
	
	; move result to temporary registers
	mov r19, r27
	mov r18, r26
	mov r17, r25
	mov r16, r24
	
	pop r27
	pop r26
	
	pop r31
	pop r30
	pop r29
	pop r28
	
	; shift right temporary value for 15 positions
	mov r16, r17
	; reset all bits in r16 except bit 0
	andi r16, 0x01
	
	; move 7 lowest bits from r18 to r16
	mov r17, r18
	lsl r17
	or r16, r17
	
	; set 7th bit of r18 to 0th bit of r17
	mov r17, r18
	rol r17					; move 7th bit of r17 to Carry flag
	rol r17					; move bit from Carry flag to zero bit
	andi r17, 0x01			; clear all bits except bit 0
	
	; move 7 lowest bits from r19 to r17
	mov r18, r19
	lsl r18
	or r17, r18
	
	; set 7th bit of r19 to 0th bit of r18
	mov r18, r19
	rol r18					; move 7th bit of r18 to Carry flag
	rol r18					; move bit from Carry flag to zero bit
	andi r18, 0x01			; clear all bits except bit 0
	
	clr r19
	
	; save result to appropriate position in the stack
	sbiw r26, 0x02
	st X, r19
	sbiw r26, 0x01
	st X, r18
	sbiw r26, 0x01
	st X, r17
	sbiw r26, 0x01
	st X, r16
	adiw r26, 0x05
	
	; end of X1 calculation
	
	; X2 calculation
	
	
	; dbg
	push r24
	mov r24, r19
	rcall send_to_usart
	mov r24, r18
	rcall send_to_usart
	mov r24, r17
	rcall send_to_usart
	mov r24, r16
	rcall send_to_usart
	pop r24
	; dbg
	
	; set stack pointer to base pointer
	out SPL, r26	
	out SPH, r27

	pop r29
	pop r28
	
	pop r27
	pop r26
	
	pop r19
	pop r18
	pop r17
	pop r16
	
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
;bmp085_ac1_val:
.byte 0
.byte 0
;bmp085_ac2_val:
.byte 0
.byte 0
;bmp085_ac3_val:
.byte 0
.byte 0
;bmp085_ac4_val:
.byte 0
.byte 0
;bmp085_ac5_val:
.byte 0
.byte 0
;bmp085_ac6_val:
.byte 0
.byte 0
;bmp085_b1_val:
.byte 0
.byte 0
;bmp085_b2_val:
.byte 0
.byte 0
;bmp085_mb_val:
.byte 0
.byte 0
;bmp085_mc_val:
.byte 0
.byte 0
;bmp085_md_val:
.byte 0
.byte 0
