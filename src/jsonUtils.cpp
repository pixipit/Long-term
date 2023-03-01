#include "jsonUtils.hpp"

/// @brief converts string to DynamicJsonDocument
/// @param s 
/// @return 
DynamicJsonDocument toJSON(String s){
  DynamicJsonDocument doc(2048);
  DeserializationError  error = deserializeJson(doc, s);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
  return doc;
}
/// @brief adds data to dataBuffer from json doc
/// @param doc 
/// @param dataBuffer 
void jsonToStationData(const DynamicJsonDocument& doc, StationData& dataBuffer){
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
  WeatherData data(temp, humidity);
  dataBuffer.description = description;
  dataBuffer.timeData = d;
  dataBuffer.weatherDataOnline = data;
}
