#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#include "weatherStationDataTypes.hpp"
#include "jsonUtils.hpp"

#define SAVED_WEATHER_DATA_PATH "/savedWeatherData.json"

/// @brief return file as string
/// @param path file path
/// @return return file as string if the file exist returns epmty string if the file does not exist
String getFileAsString(String path);
/// @brief adds data from "/savedWeatherData.json" to dataBuffer
/// @param dataBuffer buffer to write to
void getSavedWeatherData(StationData& dataBuffer);
/// @brief adds data from "/savedWeatherData.json" to dataBuffer
/// if the file does not exist the dataBuffer is not changed
/// @param dataBuffer buffer to write to
void tryGetSavedWeatherData(StationData& dataBuffer);
/// @brief saves string to "/savedWeatherData.json"
/// @param data 
void saveWeatherData(String data);