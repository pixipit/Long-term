#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include<ctime>

#include "weatherStationDataTypes.hpp"
/// @brief converts string to DynamicJsonDocument
/// @param s 
/// @return 
DynamicJsonDocument toJSON(String s);
/// @brief adds data to dataBuffer from json doc
/// @param doc 
/// @param dataBuffer 
void jsonToStationData(const DynamicJsonDocument& doc, StationData& dataBuffer);