#include "weather_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// Dışarıdan erişilebilecek global değişkenler
float currentWeatherTemperature = 0.0f;
float currentWeatherHumidity = 0.0f;

void fetchWeatherData() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi bağlı değil, hava durumu alınamadı.");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();   // sertifika doğrulamasını atlar

    HTTPClient http;
    http.begin(client, "https://api.open-meteo.com/v1/forecast?latitude=40.1828&longitude=29.0665&current=temperature_2m,relative_humidity_2m");

    int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Hava durumu verisi alındı.");

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            currentWeatherTemperature = doc["current"]["temperature_2m"];
            currentWeatherHumidity = doc["current"]["relative_humidity_2m"];

            Serial.print("Dış Hava Sıcaklığı: ");
            Serial.print(currentWeatherTemperature);
            Serial.println(" °C");
        } else {
            Serial.print("JSON parse hatası: ");
            Serial.println(error.c_str());
        }
    } else {
        Serial.printf("HTTP Hata kodu: %d\n", httpResponseCode);
    }

    http.end();
}