#include "display_manager.h"
#include "LovyanGFX_Driver.h"
#include "pins_config.h"
#include <WiFi.h>

LGFX gfx;

static String lastDisplayState = "";

void initializeDisplay() {
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);

    gfx.init();
    gfx.setRotation(1);

    gfx.fillScreen(TFT_BLACK);

    gfx.setTextColor(TFT_WHITE);
    gfx.setTextSize(2);
    gfx.setCursor(10, 20);

    gfx.println("ESP32-S3");
    gfx.println("Starting...");
}

void updateDisplay() {
    String currentState;

    if (WiFi.status() == WL_CONNECTED) {
        currentState = "CONNECTED:" + WiFi.localIP().toString();
    } else {
        currentState = "WAITING";
    }

    if (currentState == lastDisplayState) {
        return;
    }

    lastDisplayState = currentState;

    gfx.fillScreen(TFT_BLACK);
    gfx.setTextColor(TFT_WHITE);
    gfx.setTextSize(2);

    gfx.setCursor(10, 20);
    gfx.println("ESP32-S3");

    gfx.setCursor(10, 70);

    if (WiFi.status() == WL_CONNECTED) {
        gfx.println("WiFi Connected");

        gfx.setCursor(10, 110);
        gfx.setTextSize(1);
        gfx.println(WiFi.localIP().toString());
    } else {
        gfx.println("Setup Mode");

        gfx.setCursor(10, 110);
        gfx.setTextSize(1);
        gfx.println("Connect to ESP32-Setup");

        gfx.setCursor(10, 140);
        gfx.println("Open 192.168.4.1");
    }
}