#include "jsonUtils.hpp"

DynamicJsonDocument toJSON(String s){
  DynamicJsonDocument doc(2048);
  DeserializationError  error = deserializeJson(doc, s);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
  return doc;
}
HttpWeatherResponse jsonToHttpWeatherResponse(const DynamicJsonDocument& doc){
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
