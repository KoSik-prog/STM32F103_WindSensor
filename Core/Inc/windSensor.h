/*
 * windSensor.h
 *
 *  Created on: Dec 30, 2023
 *      Author: Marcin Kosela (KoSik)
 *		e-mail: kosik84@gmail.com
 *		
 *	   version: 1.0
 */

#ifndef INC_WINDSENSOR_H_
#define INC_WINDSENSOR_H_

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
};

HAL_StatusTypeDef windSensor_init(GPIO_TypeDef *xdtPort, uint16_t xdtPin,
		GPIO_TypeDef *xsckPort, uint16_t xsckPin, GPIO_TypeDef *ydtPort,
		uint16_t ydtPin, GPIO_TypeDef *ysckPort, uint16_t ysckPin);
HAL_StatusTypeDef windSensor_tara(uint16_t zeroCount);
int32_t windSensor_read(void);
int32_t windSensor_getWeight(void);
void windSensor_turnOff(void);
void windSensor_turnOn(void);
HAL_StatusTypeDef windSensor_readyForRetrieval(void);
uint32_t getUs(void);
void delayUs(uint16_t micros);

#endif /* INC_WINDSENSOR_H_ */
