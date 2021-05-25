/*
  Copyright (C) 2021 Hossein Ghorbanfekr [hgh.comphys@gmail.com]

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  ----------------------------------------------------------------------
  
  Based on code and description presented in following references:
  https://RandomNerdTutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/
  http://acoptex.com/project/1515/basics-project-070p-esp32-development-board-with-18-spi-tft-lcd-128x160-module-at-acoptexcom/#sthash.C8gmE9Za.dpbs
  https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/

  Folowing parameters have to be set beforehand in the code:
  - SSID                  : WiFi network's name
  - PASSWORD              : WiFi password
  - APIKEY                : Open weather map api key (see the references)
  - CITY                  : e.g. Antwerp
  - COUNTRY               : e.g. BE
  - TIMEZONE              : Time zone respect to GMT e.g. 7200 for GMT+2h
*/

// WiFi and HTTPS connection to openweather
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// TFT display
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Date and Time
#include <NTPClient.h>
#include <WiFiUdp.h>

// **********************************************************************************

const char* ssid = "SSID";
const char* password = "PASSWORD";

// Your Domain name with URL path or IP address with path (see references)
String openWeatherMapApiKey = "APIKEY";

// Replace with your country code and city
String city = "CITY";                // e.g. Antwerp
String countryCode = "COUNTRY";      // e.g. BE
unsigned long timezone = "TIMEZONE"  // e.g. 7200 (GMT+2)

// **********************************************************************************

// LED indicators
#define LED_COLD        25    // pin
#define LED_NORM        26    // pin
#define LED_HOT         27    // pin
#define LED_WEATHER     32    // pin

// Set thresholds
#define TEMP_COLD       5.0   // Celsius
#define TEMP_HOT        25.0  // Celsius
#define WIND_SPEED_HIGH 30.0  // km/h 
#define VISIBILITY_LOW  2.0   // km
#define HUMIDITY_HIGH   90.0  // Percent

// Define settings for buzzer
#define BUZZER_PIN      15
#define BUZZER_FRQ      2000
#define BUZZER_CHANNEL  0

// Define pins of TFT screen
#define TFT_CS          12    // pin
#define TFT_RST         14    // pin 
#define TFT_DC          13    // pin
#define TFT_SCLK        22    // pin
#define TFT_MOSI        21    // pin    

// Define constants
#define K2C           273.15  // Kelvin to Celsius 
#define MS2KMH        3.6     // m/s to km/h
#define M2KM          0.001   // m to km

// For a final application, limit the API call per hour/minute to avoid getting blocked/banned
// Pulling time in Milliseconds
unsigned long timerDelay = 300000;  

// **********************************************************************************

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

bool entrance = true;
unsigned long lastTime = 0;
String jsonBuffer;
String formattedDate;
String dayStamp;
String timeStamp;

//**********************************************************************************

void setup()
{
  // Set serial baudrate 
  Serial.begin(115200);

  // Set pins mode
  pinMode(LED_COLD, OUTPUT);
  pinMode(LED_NORM, OUTPUT);
  pinMode(LED_HOT, OUTPUT);
  pinMode(LED_WEATHER, OUTPUT);

  // Initialize TFT display
  tft.initR(INITR_BLACKTAB);

  drawlogo();
  checkLEDs();

  // Connect to WiFi access point
  WiFi.begin(ssid, password);
  Serial.println("Connecting..."); 
  tft.println("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  tft.println("");
  tft.print("Connected to WiFi network with IP Address: ");
  tft.println(WiFi.localIP());
  
  // Start time client
  timeClient.begin();
  timeClient.setTimeOffset(timezone); 

  beep();
  delay(2000);
}

void loop()
{
  // Send an HTTP GET request
  if ( entrance || (millis() - lastTime) > timerDelay) {

    entrance = false;
    
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;

      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);

      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      Serial.print("JSON object = ");
      Serial.println(myObject);

      // update time and date
      while(!timeClient.update()) {
        timeClient.forceUpdate();
      }
      formattedDate = timeClient.getFormattedDate();

      // Draws
      drawlogo();
      drawHeadInfo(myObject);
      drawDetailInfo(myObject);
      drawDate(formattedDate);
      
      setTemperatureLED(double(myObject["main"]["temp"]) - K2C);
      setWeatherLED(myObject["weather"][0]["main"]);
    }
    else {
      Serial.println("WiFi Disconnected");
      tft.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverName)
{
  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void drawlogo()
{
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_CYAN);
  tft.setCursor(0, 0);
  tft.print("   Weather Monitor\n");
  tft.print("---------------------\n");
  tft.setTextColor(ST7735_WHITE);
}

void checkLEDs() 
{
  const int wait = 100;
  for (int i=0; i<3; i++) {
    digitalWrite(LED_HOT, HIGH);
    delay(wait);
    digitalWrite(LED_HOT, LOW);
    digitalWrite(LED_NORM, HIGH);
    delay(wait);
    digitalWrite(LED_NORM, LOW);
    digitalWrite(LED_COLD, HIGH);
    delay(wait);
    digitalWrite(LED_COLD, LOW);
    digitalWrite(LED_WEATHER, HIGH);
    delay(wait);
    digitalWrite(LED_WEATHER, LOW);
  }
}

void drawHeadInfo(JSONVar &myObject)
{  
  // Temperature
  tft.print("\n     ");
  tft.setTextSize(2);
  const double temperature = double(myObject["main"]["temp"]) - K2C;
  tft.print(temperature);
  tft.setTextSize(1);
  setTemperatureTextColor(temperature);
  tft.println("C");
  tft.setTextColor(ST7735_WHITE);
  
  // Weather description
  tft.print("\n     ");
  setWeatherTextColor(myObject["weather"][0]["main"]);
  const char* weather_dscrp = myObject["weather"][0]["description"];
  tft.print(weather_dscrp);
  tft.setTextColor(ST7735_WHITE);
}

void drawDetailInfo(JSONVar &myObject)
{
  tft.print("\n\n");
  //---
  //tft.print("Country   : ");
  //tft.println(myObject["sys"]["country"]);
  //---
  tft.print("City    : ");
  const char *city = myObject["name"];
  tft.println(city);
  //---
  tft.print("Weather : ");
  const char *weather = myObject["weather"][0]["main"];
  tft.println(weather);
  //---
  const double temp = double(myObject["main"]["temp"]) - K2C;
  setTextColorDefaultLow(temp, TEMP_HOT); 
  setTextColorDefaultHigh(temp, TEMP_COLD);
  tft.print("Temp.   : ");
  tft.print(temp);
  tft.println(" C");
  tft.setTextColor(ST7735_WHITE);
  //---
  const double fls_temp = double(myObject["main"]["feels_like"]) - K2C;
  setTextColorDefaultLow(fls_temp, TEMP_HOT); 
  setTextColorDefaultHigh(fls_temp, TEMP_COLD); 
  tft.print("Fls like: ");
  tft.print(fls_temp);
  tft.println(" C");
  tft.setTextColor(ST7735_WHITE);
  //---
  tft.print("Temp Min: ");
  tft.print(double(myObject["main"]["temp_min"]) - K2C);
  tft.println(" C");
  //---
  tft.print("Temp Max: ");
  tft.print(double(myObject["main"]["temp_max"]) - K2C);
  tft.println(" C");
  //---
  tft.print("Pressure: ");
  tft.print(myObject["main"]["pressure"]);
  tft.println(" mb");
  //---
  const double humidity = double(myObject["main"]["humidity"]);
  setTextColorDefaultLow(humidity, HUMIDITY_HIGH); 
  tft.print("Humidity: ");
  tft.print(humidity);
  tft.println("%");
  tft.setTextColor(ST7735_WHITE);
  //---
  //tft.print("Sunrise   : ");
  //tft.println(long(myObject["sys"]["sunrise"]));
  //---
  const double wind_speed = double(myObject["wind"]["speed"])*MS2KMH;  /* km/h */
  setTextColorDefaultLow(wind_speed, WIND_SPEED_HIGH); 
  tft.print("Wind Spd: ");
  tft.print(wind_speed);
  tft.println(" km/h");
  tft.setTextColor(ST7735_WHITE);
  //---
  const double visibility = double(myObject["visibility"])*M2KM;  /* km */
  setTextColorDefaultHigh(visibility, VISIBILITY_LOW); 
  tft.print("Vsibilty: ");
  tft.print(visibility);
  tft.println(" km");
  tft.setTextColor(ST7735_WHITE);
}

void drawDate(String formattedDate)
{
  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("Date: ");
  Serial.println(dayStamp);
  tft.print("Date    : ");
  tft.println(dayStamp);
  
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("Updated: ");
  Serial.println(timeStamp);
  tft.print("Updated : ");
  tft.println(timeStamp);
}

// Generate PWM signal for specific period of time (miliseconds)
void tone(unsigned long duration)  
{ 
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWriteTone(BUZZER_CHANNEL, BUZZER_FRQ);
  delay(duration);
  ledcDetachPin(BUZZER_PIN);
  ledcWriteTone(BUZZER_CHANNEL, 0);
}
void beep() { tone(100); }
void beepTwice() { beep(); delay(100); beep(); }

// // ------- Generic -------

int applyThreeFoldCondition(double param, double low, double high) 
{
  if ( param < low ) 
    return -1;
  else if ( param > high )
    return 1;
  else
    return 0;
}

int applyTwoFoldCondition(double param, double criterion)
{
  if ( param < criterion ) 
    return -1;
  else
    return 1;
}

void setTextColorDefaultHigh(double param, double criterion)
{
  switch( applyTwoFoldCondition(param, criterion) ) {
    case -1 : 
      tft.setTextColor(ST7735_YELLOW);
      break;
    case 1 :
      tft.setTextColor(ST7735_WHITE);
      break;  
  } 
}

void setTextColorDefaultLow(double param, double criterion)
{
  switch( applyTwoFoldCondition(param, criterion) ) {
    case -1 : 
      tft.setTextColor(ST7735_WHITE);
      break;
    case 1 :
      tft.setTextColor(ST7735_YELLOW);
      break;  
  } 
}

// ------- Temperature -------

void setTemperatureTextColor(double temperature)
{
  switch( applyThreeFoldCondition(temperature, TEMP_COLD, TEMP_HOT) ) {
    case -1 : 
      tft.setTextColor(ST7735_BLUE);
      break;
    case 1 :
      tft.setTextColor(ST7735_RED);
      break;
    case 0 :
      tft.setTextColor(ST7735_WHITE);  
      break;   
  } 
}

void setTemperatureLED(double temperature)
{
  // Reset all LEDs
  digitalWrite(LED_COLD, LOW);
  digitalWrite(LED_NORM, LOW);
  digitalWrite(LED_HOT, LOW);

  switch( applyThreeFoldCondition(temperature, TEMP_COLD, TEMP_HOT) ) {
    case -1 : 
      digitalWrite(LED_COLD, HIGH); 
      break;
    case 1 :
      digitalWrite(LED_HOT, HIGH);
      break;
    case 0 :
      digitalWrite(LED_NORM, HIGH);  
      break;
  }    
}

// ------- Weather -------

int applyWeatherCondition(String weather)
{
  if ( weather.equalsIgnoreCase("Rain") || weather.equalsIgnoreCase("Snow") )
    return 1;
  else
    return 0;
}

void setWeatherLED(const char* weather)
{
  switch( applyWeatherCondition(String(weather)) ) {
    case 1 :   
      if( digitalRead(LED_WEATHER) == LOW ) 
        beepTwice();
      digitalWrite(LED_WEATHER, HIGH);    
      break;
    case 0 :
      if( digitalRead(LED_WEATHER) == HIGH )
        beep();
      digitalWrite(LED_WEATHER, LOW);
      break; 
  }    
}

void setWeatherTextColor(const char* weather)
{
  switch( applyWeatherCondition(String(weather)) ) {
    case 1 : 
      tft.setTextColor(ST7735_YELLOW);
      break;
    case 0 :
      tft.setTextColor(ST7735_WHITE);
      break; 
  }       
}
