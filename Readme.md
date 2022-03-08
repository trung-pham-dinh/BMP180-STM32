## Steps to use library:
- Call BMP180_init(): to initialize I2C and read calibration coeff in EEPROM
- Call BMP180_startTemp(&hi2c1) + BMP180_getTemp(&temp): to read temperature (unit: 0.1°C)
- Call BMP180_startPressure(0) + BMP180_getPressure(&pressure, 0): to read pressure (unit: Pa)