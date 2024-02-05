/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hx711.h"
#include <string.h>
#include <stdio.h>

#ifdef USB_ENABLED
	#include "usbd_cdc_if.h"
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USB_ENABLED

#define FLUCTUATION_NO_WIND 1200 //measurement fluctuation when no wind
#define NO_WIND_OK_MEASUREMENT 500 //max measurement without wind. Over that required calibration
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t txBuffer[250];
uint8_t txtBuf[40];
int32_t windX, windY;
int32_t windXavg, windYavg;

uint16_t txLength = 0;
uint16_t messLength = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void swap(int32_t *a, int32_t *b) {
    int32_t temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int32_t arr[], int low, int high) {
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

int32_t calc_average(int32_t *dataArray, uint16_t arraySize){
	int32_t avgVal = dataArray[0];
	for(uint16_t i=1; i<arraySize; i++){
		if(dataArray[i] != 0){
			avgVal = (avgVal + dataArray[i]) / 2;
		}
	}
	return avgVal;
}

int32_t calc_wind(int32_t *dataArray, uint16_t arraySize){
	quickSort(dataArray, 0, arraySize-1);
	int32_t avrArray[arraySize/2];
	for(uint8_t i=0; i<(arraySize/2); i++){
		avrArray[i] = dataArray[i] + dataArray[(arraySize - 1)-i];
	}
	return calc_average(avrArray, arraySize/2);
}


void wind_measure(struct HX711 *sensorX, struct HX711 *sensorY){
	int32_t bufArray[80];
	int32_t dataArrayX[80];
	int32_t dataArrayY[80];
	uint16_t dataSize = sizeof(dataArrayX) / sizeof(dataArrayX[0]);
	uint8_t needTaraFlag = 0;
	for(uint16_t q=0; q<80; q++){
	  windX = hx711_getWeight(sensorX);
	  dataArrayX[q] = windX;
	  windY = hx711_getWeight(sensorY);
	  dataArrayY[q] = windY;
	}
	 while (1)
		{
		  for(uint16_t q=70; q<80; q++){
			  windX = hx711_getWeight(sensorX);
			  dataArrayX[q] = windX;
			  windY = hx711_getWeight(sensorY);
			  dataArrayY[q] = windY;
		  }
		  txLength = 0;
		  messLength = sprintf((char*)txtBuf, "{\"val\":[");

		  memcpy(txBuffer, txtBuf, messLength);
		  txLength += messLength;

		  for(uint8_t i=70; i<78; i++){
			  windX = dataArrayX[i];
			  windY = dataArrayY[i];
			  messLength = sprintf((char*)(txtBuf), "{\"X\":%i,\"Y\":%i},", (int)windX, (int)windY);
			  memcpy(txBuffer + txLength, txtBuf, messLength);
			  txLength += messLength;
		  }

		  memcpy(bufArray, dataArrayX, dataSize * sizeof(int32_t));
		  windXavg = calc_wind(bufArray, dataSize);
		  int32_t fluctuation = (bufArray[79] - bufArray[0]);
		  if(fluctuation < FLUCTUATION_NO_WIND && (windXavg > NO_WIND_OK_MEASUREMENT || windXavg < -NO_WIND_OK_MEASUREMENT)){
			  //need calibration
			  needTaraFlag = 1;
		  }

		  memcpy(bufArray, dataArrayY, dataSize * sizeof(int32_t));
		  windYavg = calc_wind(bufArray, dataSize);
		  fluctuation = (bufArray[79] - bufArray[0]);
		  if(fluctuation < FLUCTUATION_NO_WIND && (windYavg > NO_WIND_OK_MEASUREMENT || windYavg < -NO_WIND_OK_MEASUREMENT)){
			  //need calibration
			  needTaraFlag = 1;
		  }

		  messLength = sprintf((char*)txtBuf, "], \"avg\":{\"X\":%i,\"Y\":%i}}\n", (int)windXavg, (int)windYavg);
		  memcpy(txBuffer + txLength-1, txtBuf, messLength);
		  txLength += messLength-1;
		#ifdef USB_ENABLED
		  CDC_Transmit_FS(txBuffer, txLength);
		#endif

		  memmove(dataArrayX, dataArrayX+10, (size_t)(70 * sizeof(int32_t)));
		  memmove(dataArrayY, dataArrayY+10, (size_t)(70 * sizeof(int32_t)));

		  //sensor calibration
		  if(needTaraFlag == 1){
			  hx711_tara(sensorX, 30);
			  hx711_tara(sensorY, 30);
			  needTaraFlag = 0;
		  }

		  delayUs(1000);
		}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  struct HX711 windSensX = {X_HX711_DT_GPIO_Port, X_HX711_DT_Pin, X_HX711_SCK_GPIO_Port, X_HX711_SCK_Pin, 0};
  struct HX711 windSensY = {Y_HX711_DT_GPIO_Port, Y_HX711_DT_Pin, Y_HX711_SCK_GPIO_Port, Y_HX711_SCK_Pin, 0};
  hx711_init(&windSensX);
  hx711_init(&windSensY);
  HAL_Delay(100);
  hx711_turnOn(&windSensX);
  hx711_tara(&windSensX, 30);
  hx711_turnOn(&windSensY);
  hx711_tara(&windSensY, 30);

  wind_measure(&windSensX, &windSensY);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  delayUs(1000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(X_HX711_SCK_GPIO_Port, X_HX711_SCK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Y_HX711_SCK_GPIO_Port, Y_HX711_SCK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : X_HX711_SCK_Pin */
  GPIO_InitStruct.Pin = X_HX711_SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(X_HX711_SCK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Y_HX711_DT_Pin X_HX711_DT_Pin */
  GPIO_InitStruct.Pin = Y_HX711_DT_Pin|X_HX711_DT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Y_HX711_SCK_Pin */
  GPIO_InitStruct.Pin = Y_HX711_SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Y_HX711_SCK_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
