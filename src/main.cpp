#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "config.h"
#include "LovyanGFX_Driver.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "display_manager.h"
#include "weather_manager.h"


unsigned long lastWeatherFetch = 0;
const unsigned long WEATHER_INTERVAL = 600000; // update weather data every 10 minutes (600,000 milliseconds)

const int BOOT_BUTTON_PIN = 0; // GPIO pin for the boot button
unsigned long buttonPressStartTime = 0;
bool buttonWasPressed = false;


void setup() {
    Serial.begin(115200);
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP); // configure boot button pin as input with pull-up resistor

    initializeDisplay();
    initializeWiFi();
    initializeWebServer();

    Serial.println("System ready.");
}

void loop() {
    server.handleClient(); // web server client requests
    handleWiFiLoop();      // Wi-Fi / NTP situation management
    updateDisplay();       // screen update based on Wi-Fi and sensor data

    bool currentButtonState = (digitalRead(BOOT_BUTTON_PIN) == LOW);

    if (currentButtonState && !buttonWasPressed) {
        // Butona yeni basıldı
        buttonPressStartTime = millis();
        buttonWasPressed = true;
    } 
    else if (currentButtonState && buttonWasPressed) {
        // Butona basılmaya devam ediliyor, süre kontrolü yapalım (Örn: 3 saniye)
        if (millis() - buttonPressStartTime > 3000) {
            Serial.println("\n[FABRİKA AYARLARI] Butona uzun basıldı! Veriler siliniyor...");
            
            // Wi-Fi ve kayıtlı ayarları temizle
            Preferences prefs;
            prefs.begin("wifi", false);
            prefs.clear();
            prefs.end();

            Serial.println("Veriler temizlendi. Cihaz yeniden başlatılıyor...");
            delay(1000);
            ESP.restart(); // Cihazı yeniden başlat
        }
    } 
    else if (!currentButtonState && buttonWasPressed) {
        // Buton bırakıldı
        buttonWasPressed = false;
    }

    // Hava durumu güncelleme döngüsü
    if (WiFi.status() == WL_CONNECTED) {
        if (millis() - lastWeatherFetch > WEATHER_INTERVAL || lastWeatherFetch == 0) {
            fetchWeatherData();
            lastWeatherFetch = millis();
        }
    }

    delay(10);
}