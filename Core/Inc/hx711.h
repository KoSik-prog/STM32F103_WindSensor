/*
 * hx711.h
 *
 *  Created on: Dec 30, 2023
 *      Author: Marcin Kosela (KoSik)
 *		e-mail: kosik84@gmail.com
 *		
 *	   version: 1.1
 */

#ifndef INC_HX711_H_
#define INC_HX711_H_

struct HX711 {
	GPIO_TypeDef *DtPort;
	uint16_t DtPin;
	GPIO_TypeDef *SckPort;
	uint16_t SckPin;
	int32_t zeroWeight;
};

HAL_StatusTypeDef hx711_init(struct HX711 *sensor);
HAL_StatusTypeDef hx711_tara(struct HX711 *sensor, uint16_t zeroCount);
void hx711_setTara(struct HX711 *sensor, int32_t weight);
int32_t hx711_read(struct HX711 *sensor);
int32_t hx711_getWeight(struct HX711 *sensor);
void hx711_turnOff(struct HX711 *sensor);
void hx711_turnOn(struct HX711 *sensor);
HAL_StatusTypeDef hx711_readyForRetrieval(struct HX711 *sensor);
uint32_t getUs(void);
void delayUs(uint16_t micros);

#endif /* INC_HX711_H_ */
