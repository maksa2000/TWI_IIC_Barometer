; This file contains definition of Atmel 328p ports

ATMEGA328P_PORT_CODES = 0 			; define ATMEGA328P_PORT_CODES

.equ	DDRD, 0x0a
.equ	DDRC, 0x07
.equ	DDRB, 0x04
.equ	PORTB, 0x05
.equ	PORTC, 0x08
.equ	PORTD, 0x0b

; port registers
.equ 	PINB, 0x03
.equ	PINB0, 0
.equ 	PINB1, 1
.equ	PINB2, 2
.equ 	PINB3, 3
.equ 	PINB4, 4
.equ 	PINB5, 5
.equ 	PINB6, 6
.equ 	PINB7, 7

.equ 	PINC, 0x06	; I/O address

.equ 	PIND, 0x09
