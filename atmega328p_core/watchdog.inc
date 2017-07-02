WATCHDOG = 0

.ifndef ATMEGA328P_CONSTANTS
.include "atmega328p_core/const_codes.inc"
.endif

; initialize watchdog timer
; function does not take nor recieve parameters
watchdog_init:
	ret
