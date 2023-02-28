#include <Arduino.h>

#include <GxEPD.h>

// select the display class to use, only one, copy from GxEPD_Example
#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w 128x250, SSD1680, TTGO T5 V2.4.1, V2.3.1

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include<UIDocument.h>
#include<UIRenderer.h>

#include <FS.h>
#include <SPIFFS.h>

#include <Wire.h>
#include "SparkFunHTU21D.h"

#include "WiFiManager.h"

#include "weatherStationDataTypes.hpp"
#include "fileSystemUtils.hpp"
#include "jsonUtils.hpp"

#define MINUTES_TO_uS_FACTOR 1000000 * 60  /* Conversion factor for micro seconds to minutes */
#define TIME_TO_SLEEP_MINUTES  15        /* Time ESP32 will go to sleep (in minutes) */

#define AP_TIMEOUT 180
#define AP_NAME "STATION"

const String PROTOKOL = "https://";
const String HOST_NAME = "api.openweathermap.org";
const String URL = "/data/2.5/weather?id=3067696&units=metric&appid=";
const String APPID = "37ac864d66b83f1704ac5ae89476b25";

//https://api.openweathermap.org/data/2.5/weather?id=3067696&units=metric&appid=737ac864d66b83f1704ac5ae89476b25

// constructor for AVR Arduino, copy from GxEPD_Example else
GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

UIRenderer renderer(&display);
UIDocument* APuiDoc = nullptr;

HTU21D indoorSensor;

HttpWeatherResponse getWeatherData(){
  HTTPClient client;
  
  client.begin("https://api.openweathermap.org/data/2.5/weather?id=3067696&units=metric&appid=737ac864d66b83f1704ac5ae89476b25");
  int http_code = client.GET();
  Serial.println("Status code: " + String(http_code));

  if(http_code == 200){
    String payload = client.getString();
    saveWeatherData(payload);
    Serial.println("Saved data");
    DynamicJsonDocument doc = toJSON(payload);
    return jsonToHttpWeatherResponse(doc);
  }else{
    return tryGetSavedWeatherData();
  }
}

void configModeCallback (WiFiManager *myWiFiManager){
  Serial.println("Starting Portal CallBack");
  display.drawPaged([](const void* doc){renderer.render((const UIDocument*)doc);}, (const void*)APuiDoc); // not working
}

bool autoConnectToWifi(){
  WiFiManager wm;

  WiFi.mode(WIFI_STA);     

  wm.resetSettings(); // for testing
  wm.setConfigPortalTimeout(AP_TIMEOUT);
  wm.setAPCallback(configModeCallback);

  return (bool)wm.autoConnect(AP_NAME);
}

void AddWeatherDataToDoc(UIDocument* doc, String menuName, const WeatherData& data){
  Element* menu = doc->find(menuName);
  if(menu == nullptr){return;}
  menu->children[1].value = String(data.temp, 1) + "C";
  menu->children[2].value = String(data.humidity, 0) + "%";
}

WeatherData getIndoorData(){
  //reading data from sensor
  float humd = indoorSensor.readHumidity();
  float temp = indoorSensor.readTemperature();
  return WeatherData(temp, "", humd);
}

void test(){
  UIDocument* uiDoc = new UIDocument("/menu.json", display.width(), display.height());
  UIDocument* uiDocConnecting = new UIDocument("/ConnectMenu.json", display.width(), display.height());
  APuiDoc = new UIDocument("/APMenu.json", display.width(), display.height());
  WeatherData sensorData(10, "0", 0); //getIndoorData();
  WeatherData onlineData(-10, "OFF", 0);
  AddWeatherDataToDoc(uiDoc, "InDoor", sensorData);
  Serial.println("InDoor data added");
  display.init();
  display.eraseDisplay();
  display.setRotation(1);
  display.drawPaged([](const void* doc){renderer.render((const UIDocument*)doc);}, (const void*)uiDocConnecting);


  bool connected = autoConnectToWifi();

  HttpWeatherResponse data;

  if(connected)
  {
    data = getWeatherData();
  }else{
    Serial.println("loading data from FLASH");
    data = tryGetSavedWeatherData();
  }
  onlineData =  data.weatherData;
  uiDoc->find("DateText")->value = data.timeData.date;
  uiDoc->find("TimeText")->value = data.timeData.time;

  AddWeatherDataToDoc(uiDoc, "OutDoor", onlineData);
  uiDoc->find("DescriptionText")->value = onlineData.description;
  display.drawPaged([](const void* doc){renderer.render((const UIDocument*)doc);}, (const void*)uiDoc);
  delete uiDoc;
}

void offlineTest(){
  UIDocument* uiDoc = new UIDocument("/menu.json", display.width(), display.height());
  WeatherData data(23, "", 68);
  WeatherData onlineData(-10, "CLEAR", 0);
  AddWeatherDataToDoc(uiDoc, "InDoor", data);
  AddWeatherDataToDoc(uiDoc, "OutDoor", onlineData);

  Serial.println("InDoor data added");
  display.init();
  display.eraseDisplay();
  
  uiDoc->find("DescriptionText")->value = onlineData.description;
  display.drawPaged([](const void* doc){renderer.render((const UIDocument*)doc);}, (const void*)uiDoc);
  delete uiDoc;
}

void testWifi(){
  WiFi.mode(WIFI_STA);
  WiFiManager wm;    
     
  bool res;
  wm.resetSettings();
  res = wm.autoConnect("AutoConnectAP");
  if(!res) {
      Serial.println("Failed to connect");
  } 
  else {
        //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }
}

void testOnDemandAP(){
  WiFiManager wm;
 
  //reset settings - for testing
  //wm.resetSettings();
  
  // set configportal timeout
  wm.setConfigPortalTimeout(120);
 
  if (!wm.startConfigPortal("OnDemandAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }
 
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
}

void setup()
{
  delay(3000);
  Serial.begin(115200);
  Serial.println("Hello World!");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_MINUTES * MINUTES_TO_uS_FACTOR);
  //esp_sleep_enable_timer_wakeup(0.1 * MINUTES_TO_uS_FACTOR); // for testing deep sleep

  //indoorSensor.begin();

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  test();
  //offlineTest();
  Serial.println("Done going to sleep");
  esp_deep_sleep_start();
}

void loop(){
    //This is not going to be called  
}
