#include "fileSystemUtils.hpp"

String getFileAsString(String path){
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
void getSavedWeatherData(StationData& dataBuffer){
  String s = getFileAsString(SAVED_WEATHER_DATA_PATH);
  DynamicJsonDocument doc = toJSON(s);
  jsonToStationData(doc, dataBuffer);
}
void tryGetSavedWeatherData(StationData& dataBuffer){
    if(!SPIFFS.exists(SAVED_WEATHER_DATA_PATH)){
      Serial.println("Data not saved!");
      return;
    }
    getSavedWeatherData(dataBuffer);
}
void saveWeatherData(String data){
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