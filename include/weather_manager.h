#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>

// Open-Meteo'dan hava durumu verisini çeken fonksiyon
void fetchWeatherData();

// Çekilen son sıcaklık bilgisini diğer modüllerle paylaşmak için extern tanım
extern float currentWeatherTemperature;
extern float currentWeatherHumidity;

#endif