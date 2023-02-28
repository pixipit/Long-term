#include <Arduino.h>

#include <GxEPD.h>

// select the display class to use, only one, copy from GxEPD_Example
#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w 128x250, SSD1680, TTGO T5 V2.4.1, V2.3.1

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include<ctime>

#include<UIDocument.h>
#include<UIRenderer.h>

#include <FS.h>
#include <SPIFFS.h>

#include <Wire.h>
#include "SparkFunHTU21D.h"

#include "WiFiManager.h"


#define MINUTES_TO_uS_FACTOR 1000000 * 60  /* Conversion factor for micro seconds to minutes */
#define TIME_TO_SLEEP_MINUTES  15        /* Time ESP32 will go to sleep (in minutes) */

#define AP_TIMEOUT 180
#define AP_NAME "STATION"

const String PROTOKOL = "https://";
const String HOST_NAME = "api.openweathermap.org";
const String URL = "/data/2.5/weather?id=3067696&units=metric&appid=";
const String APPID = "37ac864d66b83f1704ac5ae89476b25";

//https://api.openweathermap.org/data/2.5/weather?id=3067696&units=metric&appid=737ac864d66b83f1704ac5ae89476b25

#define TIMEOUT_MS 50000
String SSID = "";
String PASSWORD = "";
// constructor for AVR Arduino, copy from GxEPD_Example else

#define SAVED_WEATHER_DATA_PATH "/savedWeatherData.json"

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

UIRenderer renderer(&display);
UIDocument* APuiDoc = nullptr;

HTU21D indoorSensor;

class WeatherData
{
public:
float temp, humidity;
String description;
WeatherData(float temp, String description, int humidity): description(description), temp(temp), humidity(humidity){}

};

DynamicJsonDocument toJSON(String s){
  DynamicJsonDocument doc(2048);
  DeserializationError  error = deserializeJson(doc, s);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
  return doc;
}

struct DataTime
{
  String date;
  String time;
};

struct HttpWeatherResponse
{
  WeatherData weatherData;// = WeatherData(23.0, "", 68);
  DataTime timeData;// = DataTime{ "JAN 1", "12:00"};
  HttpWeatherResponse():weatherData(WeatherData(23.0, "", 68)), timeData(DataTime{ "JAN 1", "12:00"}){}
  HttpWeatherResponse(WeatherData weatherData, DataTime timeData):weatherData(weatherData), timeData(timeData){}
};

HttpWeatherResponse processJson(const DynamicJsonDocument& doc){
  String description = doc["weather"][0]["main"];
  description.toUpperCase();
  float temp = doc["main"]["temp"];
  int humidity = doc["main"]["humidity"];
  int time = doc["dt"].as<int>();
  int timeZone = doc["timezone"];
  auto t = std::time_t(time + timeZone);
  String timeAsString = String(std::ctime(&t));
  Serial.println(timeAsString);
  DataTime d{timeAsString.substring(4, 10), timeAsString.substring(11, 16)};
  WeatherData data(temp, description, humidity);
  return HttpWeatherResponse{data, d};
}

void SaveWeatherData(String data){
  File f = SPIFFS.open(SAVED_WEATHER_DATA_PATH, "w");
  if (!f) {
    Serial.println("file open failed");
  }
  if (f.print(data)) {
    Serial.println("File was written");
  } else {
    Serial.println("File write failed");
  }
  f.close();
}
String GetFileAsString(String path);
HttpWeatherResponse getSavedWeatherData(){
  String s = GetFileAsString(SAVED_WEATHER_DATA_PATH);
  DynamicJsonDocument doc = toJSON(s);
  return processJson(doc);
}

HttpWeatherResponse tryGetSavedWeatherData(){
  if(!SPIFFS.exists(SAVED_WEATHER_DATA_PATH)){
      Serial.println("Data not saved!");
      return HttpWeatherResponse{};
    }
    return getSavedWeatherData();
}

HttpWeatherResponse getWeatherData(){
  HTTPClient client;
  
  client.begin("https://api.openweathermap.org/data/2.5/weather?id=3067696&units=metric&appid=737ac864d66b83f1704ac5ae89476b25");
  int http_code = client.GET();
  Serial.println("Status code: " + String(http_code));

  if(http_code == 200){
    String payload = client.getString();
    SaveWeatherData(payload);
    Serial.println("Saved data");
    DynamicJsonDocument doc = toJSON(payload);
    return processJson(doc);
  }else{
    return tryGetSavedWeatherData();
  }
}

UIDocument* getUIDoc(String path);

void configModeCallback (WiFiManager *myWiFiManager){
  Serial.println("Starting Portal CallBack");
  display.drawPaged([](const void* doc){renderer.render((const UIDocument*)doc);}, (const void*)APuiDoc); // not working
}

bool autoConnectToWifi(){
  WiFiManager wm;

  WiFi.mode(WIFI_STA);     

  //wm.resetSettings(); // for testing
  wm.setConfigPortalTimeout(AP_TIMEOUT);
  wm.setAPCallback(configModeCallback);

  return (bool)wm.autoConnect(AP_NAME);
}

String GetFileAsString(String path){
  if(!SPIFFS.exists(path)){
    Serial.println("file not found! path:" + path);
    return "";
  }
  
  File f = SPIFFS.open(path, "r");

  if (!f) {
    Serial.println("file open failed");
  }
  String s = f.readString();
  f.close();
  return s;
}

UIDocument* getUIDoc(String path){
  String jsonString = GetFileAsString(path);
  DynamicJsonDocument doc = toJSON(jsonString);
  JsonObject jObject = doc["visualElement"].as<JsonObject>();

  jsonString = GetFileAsString("/styleClasses.json");
  DynamicJsonDocument classes = toJSON(jsonString);

  JsonArray classArray = classes["classes"].as<JsonArray>();

  return new UIDocument(jObject, display.width(), display.height(), &classArray);
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
  UIDocument* uiDoc = getUIDoc("/menu.json");
  UIDocument* uiDocConnecting = getUIDoc("/ConnectMenu.json");
  APuiDoc = getUIDoc("/APMenu.json");
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
  UIDocument* uiDoc = getUIDoc("/menu.json");
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
