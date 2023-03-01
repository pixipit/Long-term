#include "fileSystemUtils.hpp"

/// @brief return file as string
/// @param path file path
/// @return return file as string if the file exist returns epmty string if the file does not exist
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
/// @brief adds data from "/savedWeatherData.json" to dataBuffer
/// @param dataBuffer buffer to write to
void getSavedWeatherData(StationData& dataBuffer){
  String s = getFileAsString(SAVED_WEATHER_DATA_PATH);
  DynamicJsonDocument doc = toJSON(s);
  jsonToStationData(doc, dataBuffer);
}
/// @brief adds data from "/savedWeatherData.json" to dataBuffer
/// if the file does not exist the dataBuffer is not changed
/// @param dataBuffer buffer to write to
void tryGetSavedWeatherData(StationData& dataBuffer){
    if(!SPIFFS.exists(SAVED_WEATHER_DATA_PATH)){
      Serial.println("Data not saved!");
      return;
    }
    getSavedWeatherData(dataBuffer);
}
/// @brief saves string to "/savedWeatherData.json"
/// @param data 
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