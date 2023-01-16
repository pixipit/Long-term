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

//https://api.openweathermap.org/data/2.5/weather?id=3067696&units=metric&appid=737ac864d66b83f1704ac5ae89476b25

#define SSD "NETWORKSSID"
#define PASSWORD "PASSWORD"
#define TIMEOUT_MS 20000
// constructor for AVR Arduino, copy from GxEPD_Example else

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

UIDocument* uiDoc;
UIRenderer renderer(&display);


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
    WeatherData data(0.0, "OFFLINE", 0);
    return data;
  }
}

bool connectToWifi(){
    Serial.println("Connecting to wifi");
    WiFi.mode(WIFI_STA);

    WiFi.begin(SSD, PASSWORD);

    unsigned long startAttempTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttempTime < TIMEOUT_MS)
    {
      Serial.print(".");
      delay(100);
    }
    Serial.println(".");

    if(WiFi.status() != WL_CONNECTED){
      Serial.println("Failed to connect!");
      return false;
    }else{
      Serial.print("Connected! ");
      Serial.println(WiFi.localIP());
      return true;
    }
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

void setUIDoc(){
  String jsonString = GetFileAsString("/menu.json");
  DynamicJsonDocument doc(2048);
  DeserializationError  error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
  JsonObject jObject = doc["visualElement"].as<JsonObject>();

  jsonString = GetFileAsString("/styleClasses.json");
  DynamicJsonDocument classes(512);
  error = deserializeJson(classes, jsonString);
  if (error) {
    Serial.print(F("(styleClass.json) deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
  JsonArray classArray = classes["classes"].as<JsonArray>();

  uiDoc = new UIDocument(jObject, display.width(), display.height(), &classArray);
}

void drawUIDoc(){
  renderer.render(uiDoc);
}

void AddWeatherDataToDoc(UIDocument* doc, String menuName, const WeatherData& data){
  Element* menu = doc->find(menuName);
  if(menu == nullptr){return;}
  menu->children[1].value = String(data.temp, 1) + "C";
  menu->children[2].value = String(data.humidity, 0) + "%";
}

void test(){
  setUIDoc();
  WeatherData data(23, "", 68);
  WeatherData onlineData(-10, "OFF", 0);
  AddWeatherDataToDoc(uiDoc, "InDoor", data);
  Serial.println("InDoor data added");
  display.init();
  display.eraseDisplay();
  
  if(connectToWifi())
  {
    onlineData =  getWeatherData();
  }else{
    //AP MODE
  }
  AddWeatherDataToDoc(uiDoc, "OutDoor", onlineData);
  uiDoc->find("DescriptionText")->value = onlineData.description;
  display.drawPaged(drawUIDoc); // version for AVR using paged drawing, works also on other processors
}

void setup()
{
  delay(3000);
  Serial.begin(115200);
  Serial.println("Hello World!");

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  test();
  Serial.println("Done");
}
