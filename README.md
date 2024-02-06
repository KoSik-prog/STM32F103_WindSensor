<h1 align="center">WindSensor Library for STM32</h1>

- HAL library uses

<h2>Description:</h2>

    Library for handling a sensor constructed based on 2 load cells HX711. Sensors with a measurement range up to 1kg were used. The HX711 system operates at a frequency of 80Hz.

    In the "dataReader" directory, you can find Python scripts for visualizing measurements. The "visChart" program plots points on a Cartesian coordinate system (the red dot indicates the current calculated measurement). The "visWind" program draws a wind direction indicator and wind strength indicator.

    
<h2>How to use:</h2>

<h3>INIT:</h3>

    struct HX711 windSensX = {X_HX711_DT_GPIO_Port, X_HX711_DT_Pin, X_HX711_SCK_GPIO_Port, X_HX711_SCK_Pin, 0};
    struct HX711 windSensY = {Y_HX711_DT_GPIO_Port, Y_HX711_DT_Pin, Y_HX711_SCK_GPIO_Port, Y_HX711_SCK_Pin, 0};

    wind_init(&windSensX, &windSensY);

<h3>READ:</h3>

        wind_collect(&windSensX, &windSensY, 70, 10);
        needCalibFlag = wind_measure(&windSensX, &windSensY);

    #ifdef USB_ENABLED
        send_dataUSB();
    #endif

        if(needCalibFlag == 1){
            hx711_tara(&windSensX, 30);
            hx711_tara(&windSensY, 30);
        }

## :memo: License ##
This project is licensed under the MIT License. For more details, please refer to the [LICENSE](LICENSE.md) file.

<br/>
<p align="center">Made by <a href="https://github.com/kosik-prog/" target="_blank">KoSik</a><p/>
<br/>
<a href="#top">Back to top</a>