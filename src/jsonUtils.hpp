#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include<ctime>

#include "weatherStationDataTypes.hpp"

DynamicJsonDocument toJSON(String s);
HttpWeatherResponse jsonToHttpWeatherResponse(const DynamicJsonDocument& doc);