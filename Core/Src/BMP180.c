/*
 * BMP180.c
 *
 *  Created on: Mar 6, 2022
 *      Author: fhdtr
 */

#include "BMP180.h"
#include "math.h"

static int16_t AC1, AC2, AC3, B1, B2, MB, MC, MD; // calibration coefficient
static uint16_t AC4, AC5, AC6;// see type at page 15


static I2C_HandleTypeDef* hi2c;

static uint32_t delay;

void BMP180_timer_run() { // call in timer interrupt (use this function in case your library does not have delay function)
	if(delay) {
		delay--;
	}
}
void BMP180_delay(uint32_t ms) {
	delay = ms;
	while(delay);
}

uint8_t BMP180_init(I2C_HandleTypeDef* i2c) {
	hi2c = i2c;
	if(	BMP180_readShort(BMP180_AC1_ADDR, &AC1) &&
		BMP180_readShort(BMP180_AC2_ADDR, &AC2) &&
		BMP180_readShort(BMP180_AC3_ADDR, &AC3) &&
		BMP180_readUShort(BMP180_AC4_ADDR, &AC4) &&
		BMP180_readUShort(BMP180_AC5_ADDR, &AC5) &&
		BMP180_readUShort(BMP180_AC6_ADDR, &AC6) &&
		BMP180_readShort(BMP180_B1_ADDR, &B1) &&
		BMP180_readShort(BMP180_B2_ADDR, &B2) &&
		BMP180_readShort(BMP180_MB_ADDR, &MB) &&
		BMP180_readShort(BMP180_MC_ADDR, &MC) &&
		BMP180_readShort(BMP180_MD_ADDR, &MD)) {
		return 1;
	}
	return 0;
}

char I2C_Scannning() {
	int count = 1000000;
	for(int addr = 1; addr < 128; addr++) {
		if(HAL_I2C_IsDeviceReady(hi2c, (addr<<1), 3, 5) == HAL_OK) {
			return addr;
		}
		while(count--);
		count=1000000;
	}
	return 128;
}

uint8_t BMP180_readShort(uint8_t addr, int16_t* value) {
	uint8_t bytes[2] = {addr, 0};
	if(BMP180_readBytes(bytes, 2)) {
		*value = (int16_t)(bytes[0] << 8) | (bytes[1]);
		return 1;
	}
	return 0;
}
uint8_t BMP180_readUShort(uint8_t addr, uint16_t* value) {
	uint8_t bytes[2] = {addr, 0};
	if(BMP180_readBytes(bytes, 2)) {
		*value = ((uint16_t)bytes[0] << 8) | ((uint16_t)bytes[1]);
		return 1;
	}
	return 0;
}

uint8_t BMP180_readBytes(uint8_t* values, uint8_t size) { // values[0] = address
	if(HAL_I2C_Master_Transmit(hi2c, BMP180_ADDR<<1, &values[0], 1, HAL_MAX_DELAY) == HAL_OK) {
		HAL_I2C_Master_Receive(hi2c, BMP180_ADDR<<1, values, size, HAL_MAX_DELAY);
		return 1;
	}
	return 0;
}

uint8_t BMP180_writeBytes(uint8_t *values, uint8_t length) {
	if(HAL_I2C_Master_Transmit(hi2c, BMP180_ADDR<<1, values, length, HAL_MAX_DELAY) == HAL_OK) {
		return(1);
	}
	return(0);
}

uint8_t BMP180_startTemp() {
	uint8_t data[2];
	data[0] = BMP180_CTRL_REG;
	data[1] = BMP180_TEMP_CMD;
	if(BMP180_writeBytes(data, 2)) {
		HAL_Delay(5);
//		BMP180_delay(5);
		return 1;
	}
	return 0;
}

uint8_t BMP180_startPressure(uint8_t oversampling) {
	uint8_t data[2],delay;
	data[0] = BMP180_CTRL_REG;
	switch(oversampling) {
		case 0:
			data[1] = BMP180_PRES0_CMD;
			delay = 5;
			break;
		case 1:
			data[1] = BMP180_PRES1_CMD;
			delay = 8;
			break;
		case 2:
			data[1] = BMP180_PRES2_CMD;
			delay = 14;
			break;
		case 3:
			data[1] = BMP180_PRES3_CMD;
			delay = 26;
			break;
		default:
			data[1] = BMP180_PRES0_CMD;
			delay = 5;
			break;
	}
	if(BMP180_writeBytes(data, 2)) {
		HAL_Delay(delay);
//		BMP180_delay(delay);
		return 1;
	}
	return 0;
}

uint8_t BMP180_getTemp(int32_t* T) {
	int32_t X1,X2;
	uint16_t UT;

	if(!BMP180_readUShort(BMP180_RESULT_REG, &UT)) {
		return 0;
	}

	X1 = ((int32_t)UT - (int32_t)AC6) * ((int32_t)AC5) >> 15;
	X2 = (MC << 11) / (X1 + MD);

	*T = (X1+X2+8) >> 4;
	return 1;

}

uint8_t BMP180_getB5(int32_t* B5) {
	int32_t X1,X2;
	uint16_t UT;

	if(!BMP180_startTemp()) return 0;
	if(!BMP180_readUShort(BMP180_RESULT_REG, &UT)) return 0;

	X1 = ((int32_t)UT - (int32_t)AC6) * ((int32_t)AC5) >> 15;
	X2 = (MC << 11) / (X1 + MD);
	*B5 = X1+X2;
	return 1;
}



uint8_t BMP180_getPressure(int32_t* pressure, uint8_t oversampling) {
	uint8_t data[3];

	int32_t UP, B5, B6, X1, X2, X3, B3, p;
	uint32_t B4, B7;

	// get raw pressure
	data[0] = BMP180_RESULT_REG;
	if(!BMP180_readBytes(data, 3)) {
		return 0;
	}
	UP = ((data[0]<<16) + (data[1]<<8) + data[2])>>(8-oversampling);

	// get B5
	if(!BMP180_getB5(&B5)) return 0;

	B6 = B5 - 4000;
	X1 = ((int32_t)B2 * ((B6 * B6) >> 12)) >> 11;
	X2 = ((int32_t)AC2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = ((((int32_t)AC1 * 4 + X3) << oversampling) + 2) / 4;

	X1 = ((int32_t)AC3 * B6) >> 13;
	X2 = ((int32_t)B1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	B4 = ((uint32_t)AC4 * (uint32_t)(X3 + 32768)) >> 15;
	B7 = ((uint32_t)UP - B3) * (uint32_t)(50000UL >> oversampling);

	if(B7 < 0x80000000) {
		p = (B7<<1)/B4;
	}
	else {
		p = (B7/B4)<<1;
	}
	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * p) >> 16;

	*pressure =  p + ((X1 + X2 + (int32_t)3791) >> 4);
	return 1;
}


