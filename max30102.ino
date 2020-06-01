/*****************************************************************
* Pulse rate and SPO2 meter using the MAX30102
* This is a mashup of 
* 1. sensor initialization and readout code from Sparkfun 
* https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library
*  
*  2. spo2 & pulse rate analysis from 
* https://github.com/aromring/MAX30102_by_RF  
* (algorithm by  Robert Fraczkiewicz)
* I tweaked this to use 50Hz sample rate
* 
* 3. ESP8266 AP & Webserver code from Random Nerd tutorials
* https://randomnerdtutorials.com/esp8266-nodemcu-access-point-ap-web-server/
******************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "algorithm_by_RF.h"
#include "MAX30105.h"

const char* ssidAP = "SP02-Pulse";
const char* passwordAP = "";

AsyncWebServer server(80);


//#define MODE_MEASURE_SAMPLE_RATE

MAX30105 sensor;


#ifdef MODE_MEASURE_SAMPLE_RATE
uint32_t startTime;
#endif

uint32_t  aun_ir_buffer[BUFFER_SIZE]; //infrared LED sensor data
uint32_t  aun_red_buffer[BUFFER_SIZE];  //red LED sensor data
int32_t   n_heart_rate; 
float     n_spo2;
int       numSamples;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     margin: 0px;
     text-align: left;
    }
    td {
      padding: 5px;
      }
    table {
      margin-left: auto;
      margin-right: auto;
      border: 1px solid black;
      border-collapse: collapse;
      }    
    body {
      margin:0;
      padding:0;
      font-size: 2.5rem;
      }
  </style>
</head>
<body>
<table border=1>
  <tr>
    <td>SPO2</td>
    <td id="spo2">%SPO2%</td>
    <td>&#37;</td>
  </tr>
  <tr>
    <td>Pulse</td>
    <td id="heartrate">%HEARTRATE%</td>
    <td>bpm</td>
  </tr>
</table>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("spo2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/spo2", true);
  xhttp.send();
}, 4000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("heartrate").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/heartrate", true);
  xhttp.send();
}, 4000 ) ;
</script>
</html>)rawliteral";


String processor(const String& var){
    if(var == "SPO2"){
      return n_spo2 > 0 ? String(n_spo2) : String("00.00");
      }
    else if(var == "HEARTRATE"){
      return n_heart_rate > 0 ? String(n_heart_rate) : String("00");
      }
    return String();
    }


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("SPO2/Pulse meter");

  WiFi.softAP(ssidAP, passwordAP); 
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
     
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
    });
  server.on("/spo2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", n_spo2 > 0 ? String(n_spo2).c_str() : "00.00");
    });
  server.on("/heartrate", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", n_heart_rate > 0 ? String(n_heart_rate).c_str() : "00");
  });

  server.begin();
  
  if (sensor.begin(Wire, I2C_SPEED_FAST) == false) {
    Serial.println("Error: MAX30102 not found");
    while (1);
    }
  byte ledBrightness = 0x80; //0 = Off,  255=50mA
  byte sampleAverage = 4; // 1, 2, 4, 8, 16, 32
  byte ledMode = 2; // 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green (MAX30105 only)
  int sampleRate = 200; // 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; // 69, 118, 215, 411
  int adcRange = 16384; // 2048, 4096, 8192, 16384
  
  sensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); 
  sensor.getINT1(); // clear the status registers by reading
  sensor.getINT2();
  numSamples = 0;

#ifdef MODE_MEASURE_SAMPLE_RATE
  startTime = millis();
#endif
  }


#ifdef MODE_MEASURE_SAMPLE_RATE
// measure the sample rate FS  (in Hz) to be used by the RF algorithm
void loop(){
  sensor.check(); 

  while (sensor.available())   {
    numSamples++;

    Serial.print("R[");
    Serial.print(sensor.getFIFORed());
    Serial.print("] IR[");
    Serial.print(sensor.getFIFOIR());
    Serial.print("] ");
    Serial.print((float)numSamples / ((millis() - startTime) / 1000.0), 2);
    Serial.println(" Hz");
    
    sensor.nextSample();
  }
}

#else // normal spo2 & heart-rate measure mode

void loop() {
  float ratio,correl; 
  int8_t  ch_spo2_valid;  
  int8_t  ch_hr_valid;  

  sensor.check();
  while (sensor.available())   {
      aun_red_buffer[numSamples] = sensor.getFIFORed(); 
      aun_ir_buffer[numSamples] = sensor.getFIFOIR();
      numSamples++;
      sensor.nextSample(); 
      if (numSamples == BUFFER_SIZE) {
        // calculate heart rate and SpO2 after BUFFER_SIZE samples (ST seconds of samples) using Robert's method
        rf_heart_rate_and_oxygen_saturation(aun_ir_buffer, BUFFER_SIZE, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid, &ratio, &correl);     
        Serial.printf("SP02 ");
        if (ch_spo2_valid) Serial.print(n_spo2); else Serial.print("x");
        Serial.print(", Pulse ");
        if (ch_hr_valid) Serial.print(n_heart_rate); else Serial.print("x");
        Serial.println();
        numSamples = 0;
        }
    }
  
  }
  
#endif
