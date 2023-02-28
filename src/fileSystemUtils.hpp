#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#include "weatherStationDataTypes.hpp"
#include "jsonUtils.hpp"

#define SAVED_WEATHER_DATA_PATH "/savedWeatherData.json"

String getFileAsString(String path);
HttpWeatherResponse getSavedWeatherData();
HttpWeatherResponse tryGetSavedWeatherData();
void saveWeatherData(String data);