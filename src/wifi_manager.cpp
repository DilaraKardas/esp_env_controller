#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>
#include <Preferences.h>

static Preferences preferences;

// Global paylaşılan değişkenler (static kaldırıldı)
String pendingSSID = "";
String pendingPassword = "";
bool connectionInProgress = false;
unsigned long connectionStartTime = 0;
static bool ntpConfigured = false;

void initializeWiFi() {
    WiFi.mode(WIFI_AP_STA);
    bool apStarted = WiFi.softAP(apSSID, apPassword);

    if (apStarted) {
        Serial.println("Access Point started.");
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("Access Point failed!");
    }

    preferences.begin("wifi", false);
    String savedSSID = preferences.getString("ssid", "");
    String savedPassword = preferences.getString("password", "");

    if (savedSSID.length() > 0) {
        Serial.println("Saved WiFi credentials found.");
        pendingSSID = savedSSID;
        pendingPassword = savedPassword;
        WiFi.begin(pendingSSID.c_str(), pendingPassword.c_str());
    }
}

void handleWiFiLoop() {
    // NTP Senkronizasyonu
    if (WiFi.status() == WL_CONNECTED && !ntpConfigured) {
        configTzTime("TRT-3", "pool.ntp.org", "time.nist.gov");
        ntpConfigured = true;
        Serial.println("NTP synchronization started.");
    }

    // Bağlantı başarılı olursa kaydet
    if (connectionInProgress && WiFi.status() == WL_CONNECTED) {
        connectionInProgress = false;
        Serial.println("\nWiFi connected successfully!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        
        preferences.putString("ssid", pendingSSID);
        preferences.putString("password", pendingPassword);
        Serial.println("Credentials saved!");
    }

    // Zaman aşımı kontrolü
    if (connectionInProgress && millis() - connectionStartTime > CONNECTION_TIMEOUT) {
        connectionInProgress = false;
        Serial.println("\nERROR: WiFi connection timeout!");
        Serial.print("WiFi status code: ");
        Serial.println(WiFi.status());
    }
}