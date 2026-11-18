#include "weather_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

float currentWeatherTemperature = 0.0f;
float currentWeatherHumidity = 0.0f;

String currentCity = "Bursa";
float currentLatitude = 40.1828f;
float currentLongitude = 29.0665f;

bool fetchLocationFromIP(){
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi is not connected. Cannot fetch location data.");
        return false;
    }
    HTTPClient http;
    http.begin("http://ip-api.com/json/");
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error && doc["status"] == "success") {
            currentLatitude = doc["lat"];
            currentLongitude = doc["lon"];
            
            Serial.print("Dynamic Position taken -> Latitude: ");
            Serial.print(currentLatitude);
            Serial.print(", Longitude: ");
            Serial.println(currentLongitude);
            http.end();
            return true;
        } else {
            Serial.println("Location data JSON parse error!!");
        }
    } else {
        Serial.print("Location HTTP error code: ");
        Serial.println(httpResponseCode);
    }

    http.end();
    return false;
}

void fetchWeatherData() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi is not connected. Cannot fetch weather data.");
        return;
    }

    if (!fetchLocationFromIP()) {
        Serial.println("location cannot fetch, the last known coordinates will be used");
    }

    HTTPClient http;
    String url = "http://api.open-meteo.com/v1/forecast?latitude=" + String(currentLatitude, 4) + 
                 "&longitude=" + String(currentLongitude, 4) + 
                 "&current=temperature_2m,relative_humidity_2m";

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("Dynamic weather data has been successfully retrieved");

        JsonDocument doc; 
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            currentWeatherTemperature = doc["current"]["temperature_2m"];
            currentWeatherHumidity = doc["current"]["relative_humidity_2m"];
            
            Serial.print("Current outdoor temperature: ");
            Serial.print(currentWeatherTemperature);
            Serial.println(" °C");
        } else {
            Serial.println("Weather data JSON parse error!");
        }
    } else {
        Serial.print("Weather HTTP error code: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}