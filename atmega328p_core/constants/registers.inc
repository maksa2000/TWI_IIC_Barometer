; this file contains definition of flag registers

ATMEGA328P_REGISTER_CODES = 0 	; define ATMEGA328P_REGISTER_CODES

.equ EIMSK, 0x1D			; EIMSK lies in the address range that is supported by OUT. if you refer it via data store/load instructions you have to add some constant (0x20) to its address
.equ EICRA, 0x69			; external interupt control register A
.equ MCUCR, 0x35			; MCU Control register
.equ MCUCR_PUD, 0x04		; Pull-up disable bit
.equ IVSEL, 0x01
.equ IVSE, 0x00
.equ SMCR, 0x33				; Sleep-mode control register
.equ PRR, 0x64				; Power Reduction Register	
.equ PRR_PRUSART0, 0x01		; Writing a logic one to this bit shuts down the USART by stopping the clock to the module.
.equ UBRR0L, 0xC4			; USART boud rate register low byte
.equ UBRR0H, 0xC5			; USART boud rate register high byte
.equ UCSR0A, 0xC0			; USART Control and Status Register n A
.equ UCSR0B, 0xC1			; USART Control and Status Register n B
.equ UCSR0C, 0xC2			; USART Control and Status Register n C
.equ UDR0, 0xC6				; USART I/O Data Register
.equ UCSR0A_UDRE, 0x05		; USART Data Register Empty bit
.equ SREG, 0x3F				; port I/O address
.equ SREG_MEM, 0x5F			; memory address of SREG register
.equ SPCR, 0x2C				; port I/O address
.equ SPCR_MEM, 0x4C			; memory address of SPI control register
.equ TWCR, 0xBC				; TWI control register
.equ TWDR, 0xBB				; TWI data register
.equ TWAR, 0xBA				; TWI address register
.equ TWSR, 0xB9				; TWI status register
.equ TWAMR, 0xBD			; TWI address mask register
.equ TWBR, 0xB8				; TWI bit rate register
.equ GTCCR, 0x23			; General Timer/Counter Control Register (I/O port address)
.equ TCCR0A, 0x24			; Timer/Counter Control Register A (I/O port address)
.equ TCCR0B, 0x25			; Timer/Counter Control Register B (I/O port address)
.equ TCNT0, 0x26			; Timer/Counter Register (I/O port address)
.equ OCR0A, 0x27			; Output Compare Register A (I/O port address)
.equ OCR0B, 0x28			; Output Compare Register B (I/O port address)
.equ TIMSK0, 0x6E			; Timer/Counter Interrupt Mask Register
.equ TIFR0, 0x15			; Timer/Counter Interrupt Flag Register (I/O port address)
; watchdog registers
.equ MCUSR, 0x34			; MCU status register I/O port address
.equ WDTCSR, 0x60			; Watchdog Timer Control Register
; stack pointer registers
.equ SPH, 0x3E				; Stack Pointer High byte (I/O port address)
.equ SPL, 0x3D				; Stack Pointer Low byte (I/O post address)
