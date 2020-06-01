# ESP8266_MAX30102_SPO2_PULSE_METER

ESP8266 SPO2 and Pulse meter using MAX30102 sensor

This is a mashup of sensor initialization and FIFO readout code from 
[Sparkfun](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library)
and spo2 & pulse rate analysis code (algorithm by  Robert Fraczkiewicz) from 
[aromring](https://github.com/aromring/MAX30102_by_RF). 

I tweaked the RF code to use 50Hz sample rate. Tested with Wemos D1 Mini board.
