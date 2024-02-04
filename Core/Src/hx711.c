/*
 * sensor->c
 *
 *  Created on: Dec 30, 2023
 *      Author: Marcin Kosela (KoSik)
 *		e-mail: kosik84@gmail.com
 *		
 *	   version: 1.1
 */


#include <stdlib.h>
#include "stm32f1xx_hal.h"
#include "main.h"
#include "hx711.h"


HAL_StatusTypeDef hx711_init(struct HX711 *sensor){
    int32_t data;

    HAL_GPIO_WritePin(sensor->SckPort, sensor->SckPin, 0);
    hx711_readyForRetrieval(sensor);
    data = hx711_read(sensor);
    hx711_turnOff(sensor);

    if (data != 0) {
        return HAL_OK;
    } else {
        return HAL_ERROR;
    }
}

HAL_StatusTypeDef hx711_tara(struct HX711 *sensor, uint16_t zeroCycles){
	int32_t zeroWeight = 0;
	int32_t max = -5000000;
	int32_t min = 0xFFFFFF;

	hx711_readyForRetrieval(sensor);
	for(uint16_t i=0; i<zeroCycles; i++){
		zeroWeight = hx711_read(sensor);
		if(zeroWeight > max){
		  max = zeroWeight;
		}
		if(zeroWeight < min){
		  min = zeroWeight;
		}
	}

	sensor->zeroWeight = min + ((max - min)/2);
	return HAL_OK;
}

void hx711_setTara(struct HX711 *sensor, int32_t weight){
	sensor->zeroWeight = weight + sensor->zeroWeight;
}

int32_t hx711_read(struct HX711 *sensor){
	uint32_t data = 0;

	hx711_readyForRetrieval(sensor);
	for (uint8_t i = 0; i < 24; i++) {
		HAL_GPIO_WritePin(sensor->SckPort, sensor->SckPin, 1);
		delayUs(1);
		HAL_GPIO_WritePin(sensor->SckPort, sensor->SckPin, 0);
		if(HAL_GPIO_ReadPin(sensor->DtPort, sensor->DtPin) == 1){
			data |= (1 << (24-i));
		}
		delayUs(1);
	}
	HAL_GPIO_WritePin(sensor->SckPort, sensor->SckPin, 1);
	delayUs(1);
	HAL_GPIO_WritePin(sensor->SckPort, sensor->SckPin, 0);
	if (data & 0x800000) {
		data = data | 0xFF000000;
	}
	return data;
}

int32_t hx711_getWeight(struct HX711 *sensor){
	int32_t weight = hx711_read(sensor);
	return weight - sensor->zeroWeight;
}

void hx711_turnOff(struct HX711 *sensor){
	HAL_GPIO_WritePin(sensor->SckPort, sensor->SckPin, 1);
}

void hx711_turnOn(struct HX711 *sensor){
	int32_t data;

	hx711_readyForRetrieval(sensor);
	data = hx711_read(sensor);
	HAL_Delay(1);
}

HAL_StatusTypeDef hx711_readyForRetrieval(struct HX711 *sensor){
	uint16_t timeout = 0;

	HAL_GPIO_WritePin(sensor->SckPort, sensor->SckPin, 0);
	while(HAL_GPIO_ReadPin(sensor->DtPort, sensor->DtPin) == 1){ //check data readiness
		delayUs(1);
		timeout++;
		if(timeout > 60000){
			return HAL_ERROR;
		}
	}
	return HAL_OK;
}

uint32_t getUs(void){
	uint32_t usTicks = HAL_RCC_GetSysClockFreq() / 1000000;
	register uint32_t ms, cycle_cnt;
	do {
		ms = HAL_GetTick();
		cycle_cnt = SysTick->VAL;
	} while (ms != HAL_GetTick());
	return (ms * 1000) + (usTicks * 1000 - cycle_cnt) / usTicks;
}

void delayUs(uint16_t micros){
	uint32_t start = getUs();
	while (getUs() - start < (uint32_t) micros) {
		asm("nop");
	}
}
