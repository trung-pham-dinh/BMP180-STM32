/*
 * BMP180.h
 *
 *  Created on: Mar 6, 2022
 *      Author: fhdtr
 */

#ifndef INC_BMP180_H_
#define INC_BMP180_H_

#include "main.h"

#define BMP180_ADDR 119 // I2C address, you can run I2C_scanning() to find your address

#define BMP180_AC1_ADDR 0xAA // table 5
#define BMP180_AC2_ADDR 0xAC
#define BMP180_AC3_ADDR 0xAE
#define BMP180_AC4_ADDR 0xB0
#define BMP180_AC5_ADDR 0xB2
#define BMP180_AC6_ADDR 0xB4
#define BMP180_B1_ADDR  0xB6
#define BMP180_B2_ADDR  0xB8
#define BMP180_MB_ADDR  0xBA
#define BMP180_MC_ADDR  0xBC
#define BMP180_MD_ADDR  0xBE

#define BMP180_CTRL_REG  	0xF4 // table 8
#define BMP180_TEMP_CMD  	0x2E
#define BMP180_PRES0_CMD 	0x34
#define BMP180_PRES1_CMD 	0x74
#define BMP180_PRES2_CMD 	0xB4
#define BMP180_PRES3_CMD 	0xF4

#define BMP180_RESULT_REG  0xF6

uint8_t BMP180_init(I2C_HandleTypeDef* i2c);
char I2C_Scannning(); // scan for I2C address
uint8_t BMP180_readBytes(uint8_t* values, uint8_t size);
uint8_t BMP180_readUShort(uint8_t addr, uint16_t* value);
uint8_t BMP180_readShort(uint8_t addr, int16_t* value);
uint8_t BMP180_writeBytes(uint8_t *values, uint8_t length);
uint8_t BMP180_startTemp();
uint8_t BMP180_getTemp(int32_t* T);
void BMP180_timer_run();
void BMP180_delay(uint32_t ms);
uint8_t BMP180_startPressure(uint8_t oversampling);
uint8_t BMP180_getB5(int32_t* B5);
uint8_t BMP180_getPressure(int32_t* pressure, uint8_t oversampling);

#endif /* INC_BMP180_H_ */
