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
HttpWeatherResponse getSavedWeatherData(){
  String s = getFileAsString(SAVED_WEATHER_DATA_PATH);
  DynamicJsonDocument doc = toJSON(s);
  return jsonToHttpWeatherResponse(doc);
}
HttpWeatherResponse tryGetSavedWeatherData(){
  if(!SPIFFS.exists(SAVED_WEATHER_DATA_PATH)){
      Serial.println("Data not saved!");
      return HttpWeatherResponse{};
    }
    return getSavedWeatherData();
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