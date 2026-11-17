#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

// Diğer modüllerin (web_server vb.) erişebilmesi için extern tanımlar
extern String pendingSSID;
extern String pendingPassword;
extern bool connectionInProgress;
extern unsigned long connectionStartTime;

void initializeWiFi();
void handleWiFiLoop();

#endif