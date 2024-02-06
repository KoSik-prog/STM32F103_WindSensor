/*
 * windSensor.h
 *
 *  Created on: Dec 30, 2023
 *      Author: Marcin Kosela (KoSik)
 *		e-mail: kosik84@gmail.com
 *		
 *	   version: 2.1
 */

#ifndef INC_WINDSENSOR_H_
#define INC_WINDSENSOR_H_

#include "hx711.h"

#define FLUCTUATION_NO_WIND 1200 //measurement fluctuation when no wind
#define NO_WIND_OK_MEASUREMENT 500 //max measurement without wind. Over that required calibration
#define ARRAY_SIZE 80 //array for measurements

struct WINDSENSOR {
	int32_t windX;
	int32_t windY;
	int32_t zeroX;
	int32_t zeroY;
	GPIO_TypeDef *xDtPort;
	uint16_t xDtPin;
	GPIO_TypeDef *xSckPort;
	uint16_t xSckPin;
	GPIO_TypeDef *yDtPort;
	uint16_t yDtPin;
	GPIO_TypeDef *ySckPort;
	uint16_t ySckPin;
	int32_t windXavg;
	int32_t windYavg;
	int32_t dataArrayX[ARRAY_SIZE];
	int32_t dataArrayY[ARRAY_SIZE];
};

HAL_StatusTypeDef wind_init(struct HX711 *sensorX, struct HX711 *sensorY);
void wind_collect(struct HX711 *sensorX, struct HX711 *sensorY, uint16_t offset, uint16_t count);
uint8_t wind_measure(struct HX711 *sensorX, struct HX711 *sensorY);

#endif /* INC_WINDSENSOR_H_ */
