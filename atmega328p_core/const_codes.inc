ATMEGA328P_CONSTANTS = 0 							; define ATMEGA328P_CONSTANTS

.include "atmega328p_core/constants/ports.inc"		;Allow us to use names like "PORTB"
.include "atmega328p_core/constants/registers.inc"	;Allow us to use flag names like EIMSK
.include "atmega328p_core/constants/interrupts.inc"	;Allow us to use interrupt definitions like INT0

.text
.equ RAMEND, 0x08FF
