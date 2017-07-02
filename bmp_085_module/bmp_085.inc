BMP_085_MODULE = 0

.ifndef ATMEGA328P_TWI
.include "atmega328p_core/twi.inc"
.endif

.equ BMP085_MODULE_ADDR_W, 0xEE
.equ BMP085_MODULE_ADDR_R, 0xEF
.equ BMP085_TEMP_REG_ADDR, 0x2E
.equ BMP085_PRES_REG0_ADDR, 0x34
.equ BMP085_PRES_REG1_ADDR, 0x74
.equ BMP085_PRES_REG2_ADDR, 0xB4
.equ BMP085_PRES_REG3_ADDR, 0xF4
.equ BMP085_BITRATE_PRESCALER, 0x48

.global bmp_085_get_temperature

bmp_085_get_temperature:
	ret
