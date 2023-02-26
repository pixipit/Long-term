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


#define MINUTES_TO_uS_FACTOR 1000000 * 60  /* Conversion factor for micro seconds to minutes */
#define TIME_TO_SLEEP_MINUTES  15        /* Time ESP32 will go to sleep (in minutes) */

#define AP_TIMEOUT 30
#define AP_NAME "STATION"

//https://api.openweathermap.org/data/2.5/weather?id=3067696&units=metric&appid=737ac864d66b83f1704ac5ae89476b25

#define TIMEOUT_MS 50000
String SSID = "";
String PASSWORD = "";
// constructor for AVR Arduino, copy from GxEPD_Example else

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

UIRenderer renderer(&display);
UIDocument* APuiDoc = nullptr;

HTU21D myHumidity;

class WeatherData
{
public:
float temp, humidity;
String description;
WeatherData(float temp, String description, int humidity): description(description), temp(temp), humidity(humidity){}

};

DynamicJsonDocument toJSON(String s){
  s.replace(" ", "");
  s.replace("\n", "");

  s.trim();
  s.remove(0,1);

  s = "{" + s + "}";


  char jsonArray [s.length() + 1];
  s.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[s.length() + 1] = '\0';

  DynamicJsonDocument doc(1024);
  DeserializationError  error = deserializeJson(doc, jsonArray);


  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
 
  return doc;
}

WeatherData getWeatherData(){
  HTTPClient client;

  client.begin("https://api.openweathermap.org/data/2.5/weather?id=3067696&units=metric&appid=737ac864d66b83f1704ac5ae89476b25");
  int http_code = client.GET();

  if(http_code > 0){
    String payload = client.getString();
    Serial.println("Status code: " + String(http_code));
    //Serial.println(payload);
    DynamicJsonDocument doc = toJSON(payload);
    String description = doc["weather"][0]["main"];
    description.toUpperCase();
    float temp = doc["main"]["temp"];
    int humidity = doc["main"]["humidity"];
    WeatherData data(temp, description, humidity);
    return data;

  }else{
    Serial.println("Code:" + String(http_code));
    WeatherData data(0.0, "OFF", 0);
    return data;
  }
}

void drawAPMode();
UIDocument* getUIDoc(String path);

void configModeCallback (WiFiManager *myWiFiManager){
  Serial.println("Starting Portal CallBack");
  //UIDocument* uiDoc = getUIDoc("/APMenu.json");
  display.drawPaged([](const void* doc){renderer.render((const UIDocument*)doc);}, (const void*)APuiDoc); // not working
  //display.drawPaged(drawAPMode);
}

bool autoConnectToWifi(){
  WiFiManager wm;

  WiFi.mode(WIFI_STA);     

  wm.resetSettings(); // for testing
  wm.setConfigPortalTimeout(AP_TIMEOUT);
  wm.setAPCallback(configModeCallback);

  return (bool)wm.autoConnect(AP_NAME);
}

String GetFileAsString(String path){
  if(!SPIFFS.exists(path)){
    Serial.println("file not found! path:" + path);
  }
  
  File f = SPIFFS.open(path, "r");

  if (!f) {
    Serial.println("file open failed");
  }
  String jsonMenu = f.readString();
  f.close();
  return jsonMenu;
}

UIDocument* getUIDoc(String path){
  String jsonString = GetFileAsString(path);
  DynamicJsonDocument doc(2048);
  DeserializationError  error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
  JsonObject jObject = doc["visualElement"].as<JsonObject>();

  jsonString = GetFileAsString("/styleClasses.json");
  DynamicJsonDocument classes(1024);
  error = deserializeJson(classes, jsonString);
  if (error) {
    Serial.print(F("(styleClass.json) deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
  JsonArray classArray = classes["classes"].as<JsonArray>();

  return new UIDocument(jObject, display.width(), display.height(), &classArray);
}

void AddWeatherDataToDoc(UIDocument* doc, String menuName, const WeatherData& data){
  Element* menu = doc->find(menuName);
  if(menu == nullptr){return;}
  menu->children[1].value = String(data.temp, 1) + "C";
  menu->children[2].value = String(data.humidity, 0) + "%";
}

void drawConnecting(){
  display.setTextSize(2);
  display.setTextColor(GxEPD_BLACK);

  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds("CONNECTING", 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setCursor(x, y);
  display.print("CONNECTING");
}

void drawAPMode(){
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds("AP MODE", 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setCursor(x, y);
  display.print("AP MODE");
}

WeatherData getIndoorData(){
  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();
  return WeatherData(temp, "", humd);
}

void test(){
  UIDocument* uiDoc = getUIDoc("/menu.json");
  UIDocument* uiDocConnecting = getUIDoc("/ConnectMenu.json");
  APuiDoc = getUIDoc("/APMenu.json");
  WeatherData data(10, "0", 0); //getIndoorData();
  WeatherData onlineData(-10, "OFF", 0);
  AddWeatherDataToDoc(uiDoc, "InDoor", data);
  Serial.println("InDoor data added");
  display.init();
  display.eraseDisplay();
  display.setRotation(1);
  display.drawPaged([](const void* doc){renderer.render((const UIDocument*)doc);}, (const void*)uiDocConnecting);


  bool connected = autoConnectToWifi();
  
  if(connected)
  {
    onlineData =  getWeatherData();
  }

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

  //myHumidity.begin();

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
