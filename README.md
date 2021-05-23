# Weather Monitor Module

## What is it?
This repo shows my attempt to build a wireless _weather monitor module_ which connects to the internet via WiFi and pulling some useful weather status information every 5 minutes. 

<!-- ![Perspective view](docs/perspective.JPG) -->
<img src="docs/perspective.JPG" alt="drawing" width="500"/>

The brain of such a module is [ESP32][esp32ref] microcontroller, a low-powered with built-in WiFi and Bluetooth functionality, which submits an HTTP request to [OpenWeatherMap][openweatherref] as the weather data server. The server returns a response to the ESP32 which contains all the required information. Finally, it reads the data into JSON format, represents them in a TFT display, and reacting to them using different LED indicators or generating a gentle beep sound. 


## Features
- Wireless connection to WiFi
- Various weather info are available: temperature, feels-like temperature, min/max temperature, pressure, humidity, wind speed, visibility
- Temperature status LEDs: red (hot), green (normal), blue (cold)
- Weather temperature LED (yellow) and buzzer: in case of rain or snow
- Change texts color when a status is not normal (e.g. high wind speed, low visibility, rain) 
- report date and time of the latest update


## Prerequisites
- Components:
    - [EPS32][esp32ref] microcontroller
    - 1.8 SPI TFT 128x160 display 
    - Four LEDs (red, green, blue, and yellow)
    - A Piezo buzzer
    - A breadboard and some wires
- Other:
    - Arduino IDE with [ESP32 Add-on](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
    - Connection to a WiFi network 
    - [OpenWeatherMap][openweatherref] enabled API to request weather data (account creation is required, see [here][apikeyref])


## Wiring
The wiring is pretty straightforward as follows: 
- TFT display pins:
    - CS ➡ 12    
    - RST ➡ 14    
    - DC  ➡ 13 
    - SCLK ➡ 22   
    - MOSI ➡ 21 
- LEDs pin:
    - Blue  ➡ 25   
    - Green ➡ 26  
    - Red   ➡ 27  
    - Yellow ➡ 32
- Buzzer pin ➡ 15


## Code adjustment
A few parameters have to be known and set beforehand in the code.
- SSID                  : WiFi network's name
- PASSWORD              : WiFi password
- APIKEY                : Open weather map api key (see the references)
- CITY                  : e.g. Antwerp
- COUNTRY               : e.g. BE
- TIMEZONE              : Time zone respect to GMT e.g. 7200 for GMT+2h
```CPP
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Your Domain name with URL path or IP address with path (see references)
String openWeatherMapApiKey = "APIKEY";

// Replace with your country code and city
String city = "CITY";                // e.g. Antwerp
String countryCode = "COUNTRY";      // e.g. BE
unsigned long timezone = "TIMEZONE"  // e.g. 7200 (GMT+2)
```
_Note_: other parameters have default values but they can be adjusted as well.


## References
[esp32ref]: https://en.wikipedia.org/wiki/NodeMCU
[openweatherref]: https://openweathermap.org/
[apikeyref]: https://randomnerdtutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/
- [ESP32 HTTP GET with Arduino IDE (OpenWeatherMap.org and ThingSpeak)][apikeyref]
- [ESP32 development board with 1.8" SPI TFT LCD 128x160 module](http://acoptex.com/project/1515/basics-project-070p-esp32-development-board-with-18-spi-tft-lcd-128x160-module-at-acoptexcom/#sthash.C8gmE9Za.dpbs)
- [Getting Date and Time with ESP32 on Arduino IDE (NTP Client)](https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/)
