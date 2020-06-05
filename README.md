# ESP8266_MAX30102_SPO2_PULSE_METER

ESP8266 SPO2 and Pulse meter using MAX30102 sensor. The ESP8266 acts as a WiFi Access Point and web page server. 
SPO2 and heart rate measurements are displayed on a browser and updated every 4 seconds.

## Development Environment

* ESP8266 Wemos D1 Mini board
* home-brew MAX30102 breakout board (MAX30102 modules are available on AliExpress)
* Arduino 1.8.12 on Ubuntu 20.04 amdx64

## Prototype

<img src="sensor_in_glove.jpg">

<img src="glove_wemos.jpg">


## Credits

This is a mashup of code from the following projects :

* MAX30102 sensor initialization and FIFO readout code from 
[Sparkfun](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library)

* SPO2 & pulse rate measurement code by Robert Fraczkiewicz from 
[aromring's repository](https://github.com/aromring/MAX30102_by_RF). I tweaked RF's implementation to use 50Hz sample rate. 

* ESP8266 WiFi Access Point & Webpage server code from [Random Nerd tutorials](https://randomnerdtutorials.com/esp8266-nodemcu-access-point-ap-web-server/)

## Usage

* Connect to the WiFi Access Point "SPO2-Pulse". There is no password, but you can define one if you like.
* Open the url 192.168.4.1 in your web browser
* A new measurement is displayed every 4 seconds

<img src="tablet_display.jpg">

* If the measurement fails (finger not present, disturbed while reading), the value will display as 0
* If unable to detect the MAX30102 sensor on power-up, the board built-in LED will blink rapidly. Try cycling power to the board.
* In normal operation, the board built-in LED will toggle every 4-second measurement cycle
* You can compile the code in debug mode to check the actual sample rate (required for the SPO2/Pulse algorithm). Or you
can display real-time red and ir sensor data on the Arduino Serial Plotter window, to determine an optimal sensor led current setting.

<img src="waveform.png"/>


