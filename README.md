# ESP8266_MAX30102_SPO2_PULSE_METER

ESP8266 SPO2 and Pulse meter using MAX30102 sensor, tested with Wemos D1 Mini board.

This is a mashup of code from the following projects :

* MAX30102 sensor initialization and FIFO readout code from 
[Sparkfun](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library)

* SPO2 & pulse rate analysis code (algorithm by  Robert Fraczkiewicz) from 
[aromring](https://github.com/aromring/MAX30102_by_RF). 

I tweaked RF's implementation to use 50Hz sample rate. 

* ESP8266 AP & Webserver code from [Random Nerd tutorials](https://randomnerdtutorials.com/esp8266-nodemcu-access-point-ap-web-server/)
