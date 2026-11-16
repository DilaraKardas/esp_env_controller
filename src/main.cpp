
#include <Arduino.h>
#include <WiFi.h>

#include <WebServer.h>
#include <Preferences.h>

#include "LovyanGFX_Driver.h"
#include "pins_config.h"

const char* ssid = "******";
const char* password = "******";

LGFX gfx;

void setup() {
    Serial.begin(115200);

    // Initialize display
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);

    gfx.init();
    gfx.setRotation(1);
    gfx.fillScreen(TFT_BLACK);

    gfx.setTextColor(TFT_WHITE);
    gfx.setTextSize(2);
    gfx.setCursor(10, 20);
    gfx.println("ESP32-S3");

    gfx.setCursor(10, 60);
    gfx.println("Connecting WiFi...");

    // Connect to WiFi
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");

    // Display connection information
    gfx.fillScreen(TFT_BLACK);
    gfx.setCursor(10, 20);
    gfx.println("WiFi Connected!");

    gfx.setCursor(10, 70);
    gfx.println(WiFi.localIP().toString());

    Serial.println(WiFi.localIP());
}

void loop() {
}
