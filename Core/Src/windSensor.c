/*
 * windSensor.c
 *
 *  Created on: 20.01.2023
 *      Author: Marcin Kosela (KoSik)
 *		e-mail: kosik84@gmail.com
 *		
 *	   version: 2.1
 */

#include <stdlib.h>
#include "stm32f1xx_hal.h"
#include "main.h"
#include "windSensor.h"
#include "hx711.h"
#include <string.h>

struct WINDSENSOR windSensor;

static int32_t calc_average(int32_t *dataArray, uint16_t arraySize);
void quickSort(int32_t arr[], int low, int high);
static int32_t calc_wind(int32_t *dataArray, uint16_t arraySize);


/*
 * Wind sensor init:
 * @param *sensorX - pointer to first HX711 sensor
 * @param *sensorY - pointer to second HX711 sensor
 * @retval status
 * */
HAL_StatusTypeDef wind_init(struct HX711 *sensorX, struct HX711 *sensorY){
	HAL_StatusTypeDef ret;
	ret = hx711_init(sensorX);
	if(ret != HAL_OK){
		return ret;
	}
	ret = hx711_init(sensorY);
	if(ret != HAL_OK){
		return ret;
	}
	HAL_Delay(100);
	hx711_turnOn(sensorX);
	hx711_tara(sensorX, 30);
	hx711_turnOn(sensorY);
	hx711_tara(sensorY, 30);
	wind_collect(sensorX, sensorY, 0, ARRAY_SIZE);
	return HAL_OK;
}

/*
 * Collect single measurements:
 * @param *sensorX - pointer to first HX711 sensor
 * @param *sensorY - pointer to second HX711 sensor
 * @param offset - start address in dataArray
 * @param count
 * */
void wind_collect(struct HX711 *sensorX, struct HX711 *sensorY, uint16_t offset, uint16_t count) {
	for (uint16_t q = offset; q < (offset + count); q++) {
		windSensor.dataArrayX[q] = hx711_getWeight(sensorX);
		windSensor.dataArrayY[q] = hx711_getWeight(sensorY);
	}
}

/*
 * Function calculate collected wind measurements:
 * @param *sensorX - pointer to first HX711 sensor
 * @param *sensorY - pointer to second HX711 sensor
 * @retval flag -> need calibration
 * */
uint8_t wind_measure(struct HX711 *sensorX, struct HX711 *sensorY) {
	int32_t bufArray[80];
	uint16_t dataSize = sizeof(windSensor.dataArrayX) / sizeof(windSensor.dataArrayX[0]);
	uint8_t needCalibFlag = 0;

	memcpy(bufArray, windSensor.dataArrayX, dataSize * sizeof(int32_t));
	windSensor.windXavg = calc_wind(bufArray, dataSize);
	int32_t fluctuation = (bufArray[79] - bufArray[0]);
	if(fluctuation < FLUCTUATION_NO_WIND && (windSensor.windYavg > NO_WIND_OK_MEASUREMENT || windSensor.windXavg < -NO_WIND_OK_MEASUREMENT)){
	  //need calibration
		needCalibFlag = 1;
	}

	memcpy(bufArray, windSensor.dataArrayY, dataSize * sizeof(int32_t));
	windSensor.windYavg = calc_wind(bufArray, dataSize);
	fluctuation = (bufArray[79] - bufArray[0]);
	if(fluctuation < FLUCTUATION_NO_WIND && (windSensor.windYavg > NO_WIND_OK_MEASUREMENT || windSensor.windYavg < -NO_WIND_OK_MEASUREMENT)){
	  //need calibration
	  needCalibFlag = 1;
	}
	memmove(windSensor.dataArrayX, windSensor.dataArrayX + 10, (size_t) (70 * sizeof(int32_t)));
	memmove(windSensor.dataArrayY, windSensor.dataArrayY + 10, (size_t) (70 * sizeof(int32_t)));

	return needCalibFlag;
}

static int32_t calc_wind(int32_t *dataArray, uint16_t arraySize) {
	quickSort(dataArray, 0, arraySize - 1);
	int32_t avrArray[arraySize / 2];
	for (uint8_t i = 0; i < (arraySize / 2); i++) {
		avrArray[i] = dataArray[i] + dataArray[(arraySize - 1) - i];
	}
	return calc_average(avrArray, arraySize / 2);
}

static void swap(int32_t *a, int32_t *b) {
	int32_t temp = *a;
	*a = *b;
	*b = temp;
}

static int partition(int32_t arr[], int low, int high) {
	int32_t pivot = arr[high];
	int i = (low - 1);

	for (int j = low; j <= high - 1; j++) {
		if (arr[j] < pivot) {
			i++;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}

void quickSort(int32_t arr[], int low, int high) {
	if (low < high) {
		int pi = partition(arr, low, high);
		quickSort(arr, low, pi - 1);
		quickSort(arr, pi + 1, high);
	}
}

static int32_t calc_average(int32_t *dataArray, uint16_t arraySize) {
	int32_t avgVal = dataArray[0];
	for (uint16_t i = 1; i < arraySize; i++) {
		if (dataArray[i] != 0) {
			avgVal = (avgVal + dataArray[i]) / 2;
		}
	}
	return avgVal;
}
