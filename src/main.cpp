#include <Arduino.h>
#include "config.h"
#include "LovyanGFX_Driver.h"
#include <WiFi.h>

#include "wifi_manager.h"
#include "web_server.h"
#include "display_manager.h"
#include "weather_manager.h" 

unsigned long lastWeatherFetch = 0;
const unsigned long WEATHER_INTERVAL = 600000; // update weather data every 10 minutes (600,000 milliseconds)

void setup() {
    Serial.begin(115200);
    
    initializeDisplay();
    initializeWiFi();
    initializeWebServer();

    Serial.println("System ready.");
}

void loop() {
    server.handleClient(); // web server client requests
    handleWiFiLoop();      // Wi-Fi / NTP situation management
    updateDisplay();       // screen update based on Wi-Fi and sensor data

    
    if (WiFi.status() == WL_CONNECTED) {
        if (millis() - lastWeatherFetch > WEATHER_INTERVAL || lastWeatherFetch == 0) {
            fetchWeatherData();
            lastWeatherFetch = millis();
        }
    }

    delay(10);
}