#include "web_server.h"
#include "config.h"
#include "wifi_manager.h"
#include <WebServer.h>
#include <WiFi.h>

WebServer server(80);

extern String pendingSSID;
extern String pendingPassword;
extern bool connectionInProgress;
extern unsigned long connectionStartTime;
extern const unsigned long CONNECTION_TIMEOUT;

// Simulated Sensor Data API
extern float currentWeatherTemperature;
extern float currentWeatherHumidity;
extern String currentCity;
extern float currentLatitude;
extern float currentLongitude;

// HTML page stored in ESP32 flash memory
const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Sensor Dashboard</title>
    <style>
        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            padding: 24px;
            background: #fdf8f9;
            color: #2a161b;
            font-family: Arial, sans-serif;
        }

        .container {
            max-width: 1100px;
            margin: auto;
        }

        header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 12px;
            margin-bottom: 30px;
            padding-bottom: 18px;
            border-bottom: 2px solid #f3e2e6;
        }

        h1 {
            margin: 0;
            font-size: 25px;
            color: #ab2041;
        }

        .subtitle {
            color: #7a5c63;
            margin-top: 8px;
        }

        .status {
            padding: 9px 16px;
            border-radius: 20px;
            background: #ab2041;
            color: #ffffff;
            font-size: 13px;
            font-weight: bold;
            box-shadow: 0 2px 8px rgba(171, 32, 65, 0.25);
        }

        .status.disconnected {
            background: #f3e2e6;
            color: #821731;
            box-shadow: none;
        }

        .info {
            display: flex;
            justify-content: space-between;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 22px;
            color: #7a5c63;
            font-size: 14px;
        }
        
        .device-datetime {
            display: flex;
            align-items: center;
            flex-wrap: wrap;
            gap: 8px;
        }

        .datetime-label {
            font-weight: bold;
            color: #2a161b;
        }

        #device-date {
            color: #7a5c63;
        }

        #device-time {
            color: #ab2041;
            font-weight: bold;
            font-variant-numeric: tabular-nums;
        }

        #ntp-status {
            font-size: 12px;
            color: #9e7b83;
        }

        .cards {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 18px;
        }

        .card {
            background: #ffffff;
            padding: 22px;
            border-radius: 14px;
            border: 1px solid #f3e2e6;
            border-top: 4px solid #ab2041;
            box-shadow: 0 4px 15px rgba(171, 32, 65, 0.05);
        }

        .card-title {
            color: #7a5c63;
            font-size: 14px;
            font-weight: bold;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        .value {
            margin-top: 16px;
            font-size: 32px;
            font-weight: bold;
            color: #ab2041;
        }

        .unit {
            color: #9e7b83;
            font-size: 15px;
            font-weight: normal;
        }

        .chart-card {
            margin-top: 22px;
            background: #ffffff;
            padding: 22px;
            border-radius: 14px;
            border: 1px solid #f3e2e6;
            box-shadow: 0 4px 15px rgba(171, 32, 65, 0.05);
        }

        .chart-card h3 {
            margin-top: 0;
            color: #2a161b;
            font-size: 17px;
        }

        canvas {
            width: 100%;
            height: 220px;
        }

        footer {
            margin-top: 25px;
            color: #9e7b83;
            font-size: 12px;
            text-align: center;
        }

        @media (max-width: 650px) {
            body {
                padding: 15px;
            }

            .cards {
                grid-template-columns: 1fr;
            }

            .value {
                font-size: 28px;
            }
        }
    </style>
</head>

<body>
<div class="container">
    <header>
        <div>
            <h1>Sensor Dashboard</h1>
            <div class="subtitle">ESP32-S3 Monitoring System</div>
        </div>
        <div class="status" id="deviceStatus">
            ● Connected
        </div>
    </header>
    
    <div class="info">
        <span class="device-datetime">
            <span class="datetime-label">Date & Time:</span>
            <span id="device-date">Waiting...</span>
            <span id="device-time">--:--:--</span>
            <span id="ntp-status">Waiting for NTP synchronization</span>
        </span>
        <span id="lastUpdate">
            Last update: --
        </span>
    </div>

    <div class="cards">
        <div class="card">
            <div class="card-title">Temperature</div>
            <div class="value">
                <span id="temperature">--</span>
                <span class="unit">°C</span>
            </div>
        </div>

        <div class="card">
            <div class="card-title">Humidity</div>
            <div class="value">
                <span id="humidity">--</span>
                <span class="unit">%</span>
            </div>
        </div>

        <div class="card" style="border-top-color: #3b82f6;">
            <div class="card-title">Smart Lighting</div>
            <div class="value" style="color: #3b82f6; font-size: 20px; margin-top: 12px;">
                Status: <span id="lightStatusText" style="font-weight: bold;">OFF</span>
            </div>
            <div style="display: flex; gap: 10px; margin-top: 15px;">
                <button onclick="controlLight('on')" style="flex: 1; padding: 10px; background: #16a34a; color: white; border: none; border-radius: 8px; cursor: pointer; font-weight: bold;">Turn ON</button>
                <button onclick="controlLight('off')" style="flex: 1; padding: 10px; background: #dc2626; color: white; border: none; border-radius: 8px; cursor: pointer; font-weight: bold;">Turn OFF</button>
            </div>
        </div>

        <div class="card" style="border-top-color: #16a34a;">
            <div class="card-title" id="weatherCardTitle">Outdoor Temp</div>
            <div class="card-subtitle" id="weatherCoords" style="font-size: 11px; color: #9e7b83; margin-top: 2px;">--</div>
            <div class="value" style="color: #16a34a; margin-top: 10px;">
                <span id="weatherTemp">--</span>
                <span class="unit">°C</span>
            </div>
        </div>

        <div class="card" style="border-top-color: #16a34a;">
            <div class="card-title">Outdoor Humidity</div>
            <div class="value" style="color: #16a34a;">
                <span id="weatherHum">--</span>
                <span class="unit">%</span>
            </div>
        </div>
    </div>

    <div class="chart-card">
        <h3>Temperature History</h3>
        <canvas id="temperatureChart"></canvas>
    </div>

    <footer>
        ESP32-S3 Sensor Monitoring System
    </footer>
</div>

<script>
    const history = [];

    async function updateDashboard() {
        try {
            const response = await fetch("/api/sensors");
            if (!response.ok) throw new Error("API request failed");
            const data = await response.json();

            document.getElementById("device-date").textContent = data.date;
            document.getElementById("device-time").textContent = data.time;

            const ntpStatus = document.getElementById("ntp-status");
            if (data.timeSynced) {
                ntpStatus.textContent = "NTP synchronized";
                ntpStatus.style.color = "#16a34a";
            } else {
                ntpStatus.textContent = "Waiting for NTP synchronization...";
                ntpStatus.style.color = "#d97706";
            }

            document.getElementById("temperature").textContent = Number(data.temperature).toFixed(1);
            document.getElementById("humidity").textContent    = Number(data.humidity).toFixed(1);

            document.getElementById("weatherTemp").textContent = Number(data.weatherTemp).toFixed(1);
            document.getElementById("weatherHum").textContent  = Number(data.weatherHum).toFixed(1);

            document.getElementById("weatherCardTitle").textContent = "Outdoor Temp (" + data.cityName + ")";
            document.getElementById("weatherCoords").textContent = data.latitude + ", " + data.longitude;

            const statusEl = document.getElementById("deviceStatus");
            statusEl.textContent = "● Connected";
            statusEl.classList.remove("disconnected");

            document.getElementById("lastUpdate").textContent =
                "Last update: " + new Date().toLocaleTimeString();

            history.push(data.temperature);
            if (history.length > 30) history.shift();
            drawChart();

        } catch (error) {
            console.error(error);
            const statusEl = document.getElementById("deviceStatus");
            statusEl.textContent = "● Disconnected";
            statusEl.classList.add("disconnected");
        }
    }

    // Lamba kontrol fonksiyonu
    async function controlLight(action) {
        try {
            const response = await fetch("/api/light/" + action, { method: "POST" });
            const result = await response.json();
            if (result.success) {
                updateLightUI(action === "on");
            }
        } catch (error) {
            console.error("Lamba komutu gönderilemedi:", error);
        }
    }

    // Arayüzdeki lamba durumunu güncelleyen yardımcı fonksiyon
    function updateLightUI(isOn) {
        const statusText = document.getElementById("lightStatusText");
        statusText.textContent = isOn ? "ON" : "OFF";
        statusText.style.color = isOn ? "#16a34a" : "#dc2626";
    }

    // updateDashboard fonksiyonunun içindeki try-catch bloğuna şu satırı ekle:
    // updateUI(data.lightState); (veya doğrudan statusText güncellenebilir)

    function drawChart() {
        const canvas = document.getElementById("temperatureChart");
        const ctx = canvas.getContext("2d");
        const width = canvas.clientWidth;
        const height = 220;
        const ratio = window.devicePixelRatio || 1;

        canvas.width = width * ratio;
        canvas.height = height * ratio;
        ctx.scale(ratio, ratio);
        ctx.clearRect(0, 0, width, height);

        if (history.length < 2) return;

        const min = Math.min(...history) - 1;
        const max = Math.max(...history) + 1;

        const points = history.map((value, index) => {
            return {
                x: index * width / (history.length - 1),
                y: height - 20 - ((value - min) / (max - min)) * (height - 40)
            };
        });

        ctx.beginPath();
        points.forEach((point, index) => {
            if (index === 0) {
                ctx.moveTo(point.x, point.y);
            } else {
                ctx.lineTo(point.x, point.y);
            }
        });

        ctx.strokeStyle = "#ab2041";
        ctx.lineWidth = 3;
        ctx.stroke();

        ctx.lineTo(points[points.length - 1].x, height);
        ctx.lineTo(points[0].x, height);
        ctx.closePath();
        ctx.fillStyle = "rgba(171, 32, 65, 0.08)";
        ctx.fill();
    }

    updateDashboard();
    setInterval(updateDashboard, 2000);
    window.addEventListener("resize", drawChart);
</script>
</body>
</html>
)rawliteral";

const char SETUP_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 WiFi Setup</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #fdf8f9;
            color: #2a161b;
            padding: 24px;
            margin: 0;
        }
        .container {
            max-width: 420px;
            margin: 50px auto;
            background: white;
            padding: 25px;
            border-radius: 14px;
            border: 1px solid #f3e2e6;
            box-shadow: 0 4px 15px rgba(171, 32, 65, 0.05);
        }
        h2 { 
            color: #ab2041; 
            margin-top: 0;
        }
        p {
            color: #7a5c63;
            font-size: 14px;
        }
        input, button {
            width: 100%;
            padding: 12px;
            margin-top: 12px;
            box-sizing: border-box;
            border-radius: 8px;
            font-size: 14px;
        }
        input { 
            border: 1px solid #f3e2e6; 
            outline: none;
        }
        input:focus {
            border-color: #ab2041;
        }
        button {
            border: none;
            background: #ab2041;
            color: white;
            cursor: pointer;
            font-weight: bold;
            box-shadow: 0 2px 8px rgba(171, 32, 65, 0.25);
            transition: background 0.2s;
        }
        button:hover {
            background: #8e1a35;
        }
        #message {
            margin-top: 18px;
            font-size: 14px;
            color: #7a5c63;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>WiFi Setup</h2>
        <p>Enter your WiFi credentials to connect the device.</p>

        <form id="wifiForm">
            <input
                type="text"
                id="ssid"
                placeholder="WiFi Name (SSID)"
                required
            >

            <input
                type="password"
                id="password"
                placeholder="WiFi Password"
            >

            <button type="submit">Connect</button>
        </form>

        <div id="message"></div>
    </div>

    <script>
        const form = document.getElementById("wifiForm");
        const message = document.getElementById("message");

        form.addEventListener("submit", async function(event) {
            event.preventDefault();

            const ssid = document.getElementById("ssid").value;
            const password = document.getElementById("password").value;

            message.textContent = "Connecting to WiFi...";
            message.style.color = "#d97706";

            try {
                const response = await fetch("/connect", {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/x-www-form-urlencoded"
                    },
                    body: "ssid=" + encodeURIComponent(ssid) +
                          "&password=" + encodeURIComponent(password)
                });

                const result = await response.json();
                message.textContent = result.message;

                if (!response.ok) {
                    message.style.color = "#ab2041";
                    return;
                }

                const interval = setInterval(async () => {
                    try {
                        const statusResponse = await fetch("/status");
                        const status = await statusResponse.json();

                        if (status.state === "connected") {
                            clearInterval(interval);
                            message.style.color = "#16a34a";
                            message.textContent = "Connected! IP: " + status.ip;
                        } else if (status.state === "failed") {
                            clearInterval(interval);
                            message.style.color = "#ab2041";
                            message.textContent = status.message;
                        }
                    } catch (error) {
                        message.style.color = "#d97706";
                        message.textContent = "Checking connection...";
                    }
                }, 1000);

            } catch (error) {
                message.style.color = "#ab2041";
                message.textContent = "Request failed.";
            }
        });
    </script>
</body>
</html>
)rawliteral";

// Web page handler (Wi-Fi durumuna göre dinamik yönlendirme)
void handleRoot() {
    if (WiFi.status() == WL_CONNECTED) {
        server.send_P(200, "text/html", PAGE);        // Wi-Fi bağlıysa Dashboard göster
    } else {
        server.send_P(200, "text/html", SETUP_PAGE);  // Bağlı değilse/Setup modundaysa Kurulum Sayfası göster
    }
}

void handleSetup() {
    server.send_P(200, "text/html", SETUP_PAGE);
}

// Receive WiFi credentials
void handleConnect() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.length() == 0) {
        server.send(400, "application/json", "{\"ok\":false,\"message\":\"WiFi name is required.\"}");
        return;
    }

    pendingSSID = ssid;
    pendingPassword = password;

    connectionInProgress = true;
    connectionStartTime = millis();

    Serial.println();
    Serial.println("New WiFi credentials received.");
    Serial.print("Target SSID: ");
    Serial.println(pendingSSID);
    Serial.println("Starting WiFi connection...");

    WiFi.disconnect(false, false);
    WiFi.begin(pendingSSID.c_str(), pendingPassword.c_str());

    server.send(200, "application/json", "{\"ok\":true,\"message\":\"Connecting... Please wait.\"}");
}

// Return WiFi connection status
void handleStatus() {
    bool connected = WiFi.status() == WL_CONNECTED;

    String state = "disconnected";
    String message = "Not connected";
    
    if (connected) {
        state = "connected";
        message = "WiFi connected successfully!";
    }
    else if (connectionInProgress) {
        if (millis() - connectionStartTime > CONNECTION_TIMEOUT) {
            connectionInProgress = false;
            state = "failed";
            message = "Connection timeout. Check WiFi credentials.";
            Serial.println("WiFi connection timeout!");
        }
        else {
            state = "connecting";
            message = "Connecting to WiFi...";
        }
    }

    String ip = connected ? WiFi.localIP().toString() : "";

    String json = "{\"state\":\"" + state +
                  "\",\"connected\":" + String(connected ? "true" : "false") +
                  ",\"message\":\"" + message +
                  "\",\"ip\":\"" + ip + "\"}";

    server.send(200, "application/json", json);
}

bool lightState = false;

void handleLightOn(){
    lightState = true;
    Serial.println("[CHECK] lights turned on.");
    server.send(200, "application/json", "{\"success\":true,\"state\":\"ON\"}");
}

void handleLightOff(){
    lightState = false;
    Serial.println("[CHECK] lights turned off");
    server.send(200, "application/json", "{\"success\":true,\"state\":\"OFF\"}");
}

void handleSensorData() {
    // Sensor/system simulation data
    float elapsedTime = millis() / 1000.0f;
    float temperature = 25.4f + 1.2f * sinf(elapsedTime * 0.35f);
    float humidity = 48.2f + 3.0f * sinf(elapsedTime * 0.22f + 1.0f);

    struct tm timeInfo;
    char dateBuffer[11] = "--.--.----";
    char timeBuffer[9] = "--:--:--";

    bool timeSynced = getLocalTime(&timeInfo, 50);

    if (timeSynced) {
        strftime(dateBuffer, sizeof(dateBuffer), "%d.%m.%Y", &timeInfo);
        strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeInfo);
    }

    String json = "{";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"weatherTemp\":" + String(currentWeatherTemperature, 1) + ",";
    json += "\"weatherHum\":" + String(currentWeatherHumidity, 1) + ",";
    json += "\"cityName\":\"" + currentCity + "\",";
    json += "\"latitude\":" + String(currentLatitude, 4) + ",";
    json += "\"longitude\":" + String(currentLongitude, 4) + ",";
    json += "\"date\":\"" + String(dateBuffer) + "\",";
    json += "\"time\":\"" + String(timeBuffer) + "\",";
    json += "\"lightState\":" + String(lightState ? "true" : "false") + ",";
    json += "\"timeSynced\":";
    json += timeSynced ? "true" : "false";
    json += "}";

    server.send(200, "application/json", json);
}

// Start web server and register routes
void initializeWebServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/setup", HTTP_GET, handleSetup);
    server.on("/connect", HTTP_POST, handleConnect);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/api/sensors", HTTP_GET, handleSensorData);

    server.on("/api/light/on", HTTP_POST, handleLightOn);
    server.on("/api/light/off", HTTP_POST, handleLightOff);

    server.begin();
    Serial.println("Web server started.");
}