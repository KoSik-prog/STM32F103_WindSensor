/*
 * windSensor.c
 *
 *  Created on: 20.01.2023
 *      Author: Marcin Kosela (KoSik)
 *		e-mail: kosik84@gmail.com
 *		
 *	   version: 1.0
 */


#include <stdlib.h>
#include "stm32f1xx_hal.h"
#include "main.h"
#include "windSensor.h"

struct WINDSENSOR windSensor;


HAL_StatusTypeDef windSensor_init(GPIO_TypeDef *xdtPort, uint16_t xdtPin,
		GPIO_TypeDef *xsckPort, uint16_t xsckPin, GPIO_TypeDef *ydtPort,
		uint16_t ydtPin, GPIO_TypeDef *ysckPort, uint16_t ysckPin){
	int32_t data;

	windSensor.xDtPort = xdtPort;
	windSensor.xDtPin = xdtPin;
	windSensor.xSckPort = xsckPort;
	windSensor.xSckPin = xsckPin;
	windSensor.yDtPort = ydtPort;
	windSensor.yDtPin = ydtPin;
	windSensor.ySckPort = ysckPort;
	windSensor.ySckPin = ysckPin;
	HAL_GPIO_WritePin(windSensor.xSckPort, windSensor.xSckPin, 0);
	windSensor_readyForRetrieval();
	data = windSensor_read();
	windSensor_turnOff();
	if(data != 0){
		return HAL_OK;
	} else {
		return HAL_ERROR;
	}
}

HAL_StatusTypeDef windSensor_tara(uint16_t zeroCycles){
	int32_t zeroWeight = 0;

	windSensor_readyForRetrieval();
	zeroWeight = windSensor_read();
	for(uint16_t i=0; i<zeroCycles; i++){
		zeroWeight += windSensor_read();
		zeroWeight = zeroWeight / 2;
	}

//	windSensor.zeroWeight = zeroWeight;
	return HAL_OK;
}

int32_t windSensor_read(void){
	uint32_t data = 0;

	windSensor_readyForRetrieval();
	for (uint8_t i = 0; i < 24; i++) {
		HAL_GPIO_WritePin(windSensor.xSckPort, windSensor.xSckPin, 1);
		delayUs(1);
		HAL_GPIO_WritePin(windSensor.xSckPort, windSensor.xSckPin, 0);
		if(HAL_GPIO_ReadPin(windSensor.xDtPort, windSensor.xDtPin) == 1){
			data |= (1 << (24-i));
		}
		delayUs(1);
	}
	HAL_GPIO_WritePin(windSensor.xSckPort, windSensor.xSckPin, 1);
	delayUs(1);
	HAL_GPIO_WritePin(windSensor.xSckPort, windSensor.xSckPin, 0);
	if (data & 0x800000) {
		data = data | 0xFF000000;
	}
	return data;
}

int32_t windSensor_getWeight(void){
	int32_t weight = windSensor_read();
//	return weight - windSensor.zeroWeight;
}

void windSensor_turnOff(void){
	HAL_GPIO_WritePin(windSensor.xSckPort, windSensor.xSckPin, 1);
}

void windSensor_turnOn(void){
	int32_t data;

	windSensor_readyForRetrieval();
	data = windSensor_read();
	HAL_Delay(1);
}

HAL_StatusTypeDef windSensor_readyForRetrieval(void){
	uint16_t timeout = 0;

	HAL_GPIO_WritePin(windSensor.xSckPort, windSensor.xSckPin, 0);
	while(HAL_GPIO_ReadPin(windSensor.xDtPort, windSensor.xDtPin) == 1){ //check data readiness
		delayUs(1);
		timeout++;
		if(timeout > 60000){
			return HAL_ERROR;
		}
	}
	return HAL_OK;
}

//uint32_t getUs(void){
//	uint32_t usTicks = HAL_RCC_GetSysClockFreq() / 1000000;
//	register uint32_t ms, cycle_cnt;
//	do {
//		ms = HAL_GetTick();
//		cycle_cnt = SysTick->VAL;
//	} while (ms != HAL_GetTick());
//	return (ms * 1000) + (usTicks * 1000 - cycle_cnt) / usTicks;
//}
//
//void delayUs(uint16_t micros){
//	uint32_t start = getUs();
//	while (getUs() - start < (uint32_t) micros) {
//		asm("nop");
//	}
//}
