SLEEP_MODE = 0

.ifndef ATMEGA328P_REGISTER_CODES
.include "atmega328p_core/constants/registers.inc"
.endif

; .global is needed to set functions visible outside the module
.global set_sleep_mode

; this function is set atmega328p sleep mode
; it recieve argument to set in r24 register
set_sleep_mode:
	out SMCR, r24
	ret
	