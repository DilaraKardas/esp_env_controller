#include "web_server.h"
#include "config.h"
#include "wifi_manager.h"
#include <WebServer.h>
#include <WiFi.h>

WebServer server(80);

// Dışarıdan (wifi_manager veya main'den) erişilebilmesi ya da tetiklenebilmesi gereken değişkenler için extern tanımlar
extern String pendingSSID;
extern String pendingPassword;
extern bool connectionInProgress;
extern unsigned long connectionStartTime;
extern const unsigned long CONNECTION_TIMEOUT;

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

            <span id="device-date">
                Waiting...
            </span>

            <span id="device-time">
                --:--:--
            </span>

            <span id="ntp-status">
                Waiting for NTP synchronization
            </span>

        </span>

        <span id="lastUpdate">
            Last update: --
        </span>

    </div>


    <div class="cards">

        <div class="card">

            <div class="card-title">
                Temperature
            </div>

            <div class="value">
                <span id="temperature">--</span>
                <span class="unit">°C</span>
            </div>

        </div>

        <div class="card">

            <div class="card-title">
                Humidity
            </div>

            <div class="value">
                <span id="humidity">--</span>
                <span class="unit">%</span>
            </div>

        </div>

        <div class="card">

            <div class="card-title">
                Pressure
            </div>

            <div class="value">
                <span id="pressure">--</span>
                <span class="unit">hPa</span>
            </div>

        </div>

        <div class="card" style="border-top-color: #16a34a;">
            <div class="card-title">Outdoor Temp (Bursa)</div>
            <div class="value" style="color: #16a34a;">
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
        document.getElementById("pressure").textContent    = Number(data.pressure).toFixed(1);

        // Open-Meteo dış hava verileri
        document.getElementById("weatherTemp").textContent = Number(data.weatherTemp).toFixed(1);
        document.getElementById("weatherHum").textContent  = Number(data.weatherHum).toFixed(1);

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

                y: height - 20 -
                    ((value - min) / (max - min)) * (height - 40)
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

        // Grafik çizgisi altında hafif ana renk dolgusu
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

// Web page handler
void handleRoot() {
    server.send_P(200, "text/html", PAGE);
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

// Simulated Sensor Data API
// weather_manager.h içindeki değişkenleri buraya tanıtıyoruz
extern float currentWeatherTemperature;
extern float currentWeatherHumidity;

void handleSensorData() {
    // Sensör/Sistem simülasyon değerleri
    float elapsedTime = millis() / 1000.0f;
    float temperature = 25.4f + 1.2f * sinf(elapsedTime * 0.35f);
    float humidity = 48.2f + 3.0f * sinf(elapsedTime * 0.22f + 1.0f);
    float pressure = 1012.5f + 1.8f * sinf(elapsedTime * 0.12f + 2.0f);

    struct tm timeInfo;
    char dateBuffer[11] = "--.--.----";
    char timeBuffer[9] = "--:--:--";

    bool timeSynced = getLocalTime(&timeInfo, 50);

    if (timeSynced) {
        strftime(dateBuffer, sizeof(dateBuffer), "%d.%m.%Y", &timeInfo);
        strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeInfo);
    }

    // JSON response içine dış hava durumu verilerini de ekliyoruz
    String json = "{";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"pressure\":" + String(pressure, 1) + ",";
    json += "\"weatherTemp\":" + String(currentWeatherTemperature, 1) + ",";
    json += "\"weatherHum\":" + String(currentWeatherHumidity, 1) + ",";
    json += "\"date\":\"" + String(dateBuffer) + "\",";
    json += "\"time\":\"" + String(timeBuffer) + "\",";
    json += "\"timeSynced\":";
    json += timeSynced ? "true" : "false";
    json += "}";

    server.send(200, "application/json", json);
}

// Start web server and register routes
void initializeWebServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/connect", HTTP_POST, handleConnect);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/api/sensors", HTTP_GET, handleSensorData);

    server.begin();
    Serial.println("Web server started.");
}