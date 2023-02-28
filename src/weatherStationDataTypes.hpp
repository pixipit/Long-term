#pragma once

#include <Arduino.h>

struct WeatherData
{
float temp, humidity;
String description;
WeatherData(float temp, String description, int humidity): description(description), temp(temp), humidity(humidity){}
};
struct DataTime
{
  String date;
  String time;
};

struct HttpWeatherResponse
{
  WeatherData weatherData;// = WeatherData(23.0, "", 68);
  DataTime timeData;// = DataTime{ "JAN 1", "12:00"};
  HttpWeatherResponse():weatherData(WeatherData(23.0, "", 68)), timeData(DataTime{ "JAN 1", "12:00"}){}
  HttpWeatherResponse(WeatherData weatherData, DataTime timeData):weatherData(weatherData), timeData(timeData){}
};