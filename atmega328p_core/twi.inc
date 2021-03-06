ATMEGA328P_TWI = 0

.ifndef ATMEGA328P_CONSTANTS
.include "atmega328p_core/const_codes.inc"
.endif

.text

.equ TWI_NO_STATE_INFO, 0xF8	; no relevant state information
.equ TWI_BUS_ERR, 0x00			; bus error
.equ TWI_START_CONDITION, 0x08
.equ TWI_RSTART_CONDITION, 0x10	; repeated start
.equ TWI_MT_SLA_W_ACK, 0x18		; slave address write acknowlegement recieved
.equ TWI_MT_SLA_W_NACK, 0x20	; not acknowledgment recieved
.equ TWI_MT_DATA_W_ACK, 0x28
.equ TWI_MT_DATA_W_NACK, 0x30
.equ TWI_M_ARB_LOST, 0x38		; master arbitration lost
.equ TWI_MR_SLA_R_ACK, 0x40		; master recieved slave address read acknowledgement
.equ TWI_MR_SLA_R_NACK, 0x48	; master recieved slave address read not acknowledgment recieved
.equ TWI_MR_DATA_R_ACK, 0x50	; master recieved data recieved acknowledgment
.equ TWI_MR_DATA_R_NACK, 0x58	; not acknowledgment
.equ TWINT_BIT, 0x07
.equ TWSTO_BIT, 0x04

	
; this function take 1 parameter from r24 register and initialize with it TWCR register
; r24 - initial value for TWCR register
; value from r24 register will be stored in twcr_init_value variable
twi_init_twcr:
	; When the TWEN bit in TWCR is set (one) to enable the 2-wire Serial Interface, pin PC5 is disconnected from the port and becomes the Serial Clock I/O pin for the 2-wire Serial Interface.
	; When the TWEN bit in TWCR is set (one) to enable the 2-wire Serial Interface, pin PC4 is disconnected from the port and becomes the Serial Data I/O pin for the 2-wire Serial Interface.
	push r16
	; clear TWDR register
	lds r16, TWDR
	cpi r16, 0xFF
	breq _twi_init_twcr_continue
	ldi r16, 0xFF
	sts TWDR, r16
	
_twi_init_twcr_continue:
	sts twi_twcr_init_value, r24	
	sts TWCR, r24
	
	rcall twi_clear_twsr_prescaler	
	rcall twi_set_pullup_registers

	pop r16
	ret
	
; send start condition to the device
; function does not take nor recieve parameters
twi_send_start_condition:
	push r16
	
	lds r16, twi_twcr_init_value
	; set START condition bit
	ori r16, 0xA4							; 1010 0100b	(to generate START and RSTART conditions all 3 bits must be writen simultaneously)
	sts TWCR, r16	
	
_twi_send_start_condition_wait:
	lds r16, TWCR
	sbrs r16, TWINT_BIT			; TWINT flag bit
	rjmp _twi_send_start_condition_wait	
	
	pop r16
	ret

; sends sstop condition to the device
; function does not take nor recieve parameters
; stop condition bit will be cleared automatically, when stop condition is send
twi_send_stop_condition:
	push r16
	
	lds r16, TWCR
	
	; set stop condition bit
	ori r16, 0x10					; 0001 0000b
	sts TWCR, r16
	
_twi_send_stop_condition_wait:
	lds r16, TWCR
	sbrc r16, TWSTO_BIT			; TWINT flag bit
	rjmp _twi_send_stop_condition_wait	
	pop r16
	ret
	
; clears start condition from TWCR register
; this function does not take or recieve any parameters
; NOTE: clearig START condition will clear high snipet (highest 4 bits), where is TWINT bit, so after that TWDR changing is not possible 
twi_clear_start_condition:
	push r16
	lds r16, TWCR
	andi r16, 0xDF		; 1101 1111b	
	sts TWCR, r16
	pop r16
	ret

; set prescaler to TWBR (bit rate register) for 16Mhz MCU
; get prescaler in r24 register
; function does not recieve any parameters
twi_set_twbr_atmega328p_prescaler:
	; set bit rate register value
	sts TWBR, r24
	ret
	
; set WTSR prescaler
; recieve prescaler in r24 register, use only 1:0 bits from r24 register to set TWSR prescaler bits
; function does not return any value
; This function is commented out to save RAM space
;twi_set_twsr_atmega328p_prescaler:
;	push r16
;	; set prescaler bit values in status register
;	mov r16, r24
;	ori r16, 0xFC		; 1111 1100b
;	sts TWSR, r16
;	pop r16
;	ret
	
; set slave devie address
; address should be passed in r24 register
; do not return any parameters
; only for slave devices
; This function is commented out to save RAM space
;twi_set_device_address:
;	sts TWAR, r24
;	ret
	
; set read/write mode to TWI address register
; should recieve mode in r24 register
; if r24 register is 0, 0 will be set in device mode bit, otherwise 1 will be set in device mode bit
; return changed value in r24 register
; This function is commented out to save RAM space
;twi_set_read_write_mode:
;	push r16
;	mov r16, r24
;	cbr r16, 1			; clear 0 bit in r16 register
;	cpi r24, 0x00
;	breq _exit_twi_set_save_device_mode
;	sbr r16, 1			; set first bit in r16
;_exit_twi_set_save_device_mode:
;	mov r24, r16
;	pop r16
;	ret
	
; this function send byte (address or data) to slave device through TWI bus
; function recieve address in r24 register
; function will not return any parameters
twi_send_byte:
	push r16
	
	sts TWDR, r24
	rcall twi_clear_start_condition	; clearing START condition will also clear TWINT bit
	
_twi_send_byte_wait:
	lds r16, TWCR
	sbrs r16, TWINT_BIT
	rjmp _twi_send_byte_wait
	
	pop r16
	ret
	
; reads data from TWI data register and return it through r24 register
twi_read_data:
	push r16
	rcall twi_twint_clear
	
_twi_read_data_wait:
	lds r16, TWCR
	sbrs r16, TWINT_BIT
	rjmp _twi_read_data_wait
	lds r24, TWDR
	
	pop r16
	ret
	
; return value from twi status register in r24 register
twi_get_status:
	lds r24, TWSR
	; mask 2:0bits of WTSR
	andi r24, 0xF8
	ret

; this function set TWINT bit of TWCR register to 0
; function does not recieve or return parameters	
twi_twint_clear:
	push r16
	lds r16, TWCR	; load TWCR register value to r16
	andi r16, 0x7F	; ands r16 value with 0111 1111b
	sts TWCR, r16
	pop r16
	ret

; set pullup resistors on PC4 and PC5
; assumes that DDRC pins 4 and 5 are zeroes	
; function does not recieve nor return parameters
twi_set_pullup_registers:
	push r16
	in r16, PINC
	ori r16, 0x30		;0011 0000b
	out PORTC, r16
	pop r16
	ret
	
; clears prescaler bits on twcr register
; function does not recieve nor return parameters
twi_clear_twsr_prescaler:
	push r16
	lds r16, TWSR
	andi r16, 0xFC		; 1111 1100b
	sts TWSR, r16
	pop r16
	ret
	
; enables twi interrupt
twi_interrupt_enable:
	push r16
	lds r16, TWCR
	ori r16, 0x01		; 0000 0001b
	sts TWCR, r16
	pop r16
	ret
	
; disables twi interrupt
twi_interrupt_disable:
	push r16
	lds r16, TWCR
	andi r16, 0xFE		; 1111 1110b
	sts TWCR, r16
	pop r16
	ret
	
; disable acknowledgement sending
; on start candition sending ack bit will be loaded from twi_twcr_init_value variable
twi_ack_diable:
	push r16
	lds r16, TWCR
	andi r16, 0xBF		; 1011 1111b
	sts TWCR, r16
	pop r16
	ret
	
	
.data 0
.org 0x00A0							; set correct address for data segment to 0x0100 (beginning of internal SRAM)	
twi_twcr_init_value:
.byte 0
	
	