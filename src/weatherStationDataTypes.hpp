#pragma once

#include <Arduino.h>

struct WeatherData
{
float temp, humidity;
WeatherData(float temp, int humidity): temp(temp), humidity(humidity){}
};
struct DataTime
{
  String date;
  String time;
};

struct StationData
{
  WeatherData weatherDataOnline;
  WeatherData weatherDataOffline;
  String description;
  DataTime timeData;
  StationData():weatherDataOnline(WeatherData(23.0, 68)),
   weatherDataOffline(WeatherData(23.0, 68)),
   description("OFF"),
   timeData(DataTime{ "JAN 1", "12:00"}){}
};