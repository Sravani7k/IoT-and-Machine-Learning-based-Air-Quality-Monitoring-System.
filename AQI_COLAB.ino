#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>  // For HTTPS connections
#include "DFRobot_AirQualitySensor.h"
#include <Wire.h>



#define I2C_ADDRESS 0x19
DFRobot_AirQualitySensor particle(&Wire, I2C_ADDRESS);

// WiFi credentials
const char* ssid = "GPONWIFI_4400";
const char* password = "0000004406";

// Google Apps Script URL (replace with your own)
const char* googleSheetsUrl = "https://script.google.com/macros/s/AKfycbwTYHAcwssIC0cUAXUU4sf6InfdMTG7KWb2dsdY0ALEFrURwCmKLsfGrC_1tawOuY_x/exec";  // Use your full HTTPS URL here

// ThingsBoard server and token
const char* TOKEN = "85h65ZpWkZJga3pkrIr3";  // Replace with your actual ThingsBoard token
const char* serverPath = "http://thingsboard.cloud/api/v1/"; // ThingsBoard server URL

WiFiClientSecure wifiClient;  // Secure client for HTTPS
WiFiClient wifiClientThingsBoard;  // Client for ThingsBoard




void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Initialize PM sensor
    while (!particle.begin()) {
        Serial.println("NO Devices!");
        delay(1000);
    }
    Serial.println("PM sensor begin success!");

    // Initialize WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");

    // Trust all certificates (insecure, but sometimes necessary for ESP8266)
    wifiClient.setInsecure();
}




// Function to send data to Google Sheets
void sendToGoogleSheets(uint16_t pm1_0, uint16_t pm2_5, uint16_t pm10) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String payload = "{\"PM1_0\":" + String(pm1_0) + ",\"PM2_5\":" + String(pm2_5) + ",\"PM10\":" + String(pm10) + "}";

        Serial.println("Sending payload to Google Sheets: " + payload);
        http.begin(wifiClient, googleSheetsUrl);  // Initialize with HTTPS client
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Google Sheets Response: " + response);
        } else {
            Serial.print("Error on sending POST to Google Sheets: ");
            Serial.println(httpResponseCode);  // Should print the actual error code
        }
        http.end();
    } else {
        Serial.println("WiFi not connected for Google Sheets!");
    }
}

// Function to send data to ThingsBoard
void sendToThingsBoard(uint16_t pm1_0, uint16_t pm2_5, uint16_t pm10) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String fullServerPath = String(serverPath) + TOKEN + "/telemetry";

        // Creating the JSON payload
        String payload = "{\"PM1_0\":" + String(pm1_0) + 
                         ",\"PM2_5\":" + String(pm2_5) +
                         ",\"PM10\":" + String(pm10) + "}";

        Serial.println("Sending payload to ThingsBoard: " + payload);
        http.begin(wifiClientThingsBoard, fullServerPath);  // Initialize with ThingsBoard client
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("ThingsBoard Response: " + response);
        } else {
            Serial.print("Error on sending POST to ThingsBoard: ");
            Serial.println(httpResponseCode);  // Should print the actual error code
        }
        http.end();
    } else {
        Serial.println("WiFi not connected for ThingsBoard!");
    }
}





void loop() {
    // Read PM sensor data
    uint16_t pm1_0 = particle.gainParticleConcentration_ugm3(PARTICLE_PM1_0_STANDARD);
    uint16_t pm2_5 = particle.gainParticleConcentration_ugm3(PARTICLE_PM2_5_STANDARD);
    uint16_t pm10 = particle.gainParticleConcentration_ugm3(PARTICLE_PM10_STANDARD);

    // Send data to Google Sheets
    sendToGoogleSheets(pm1_0, pm2_5, pm10);

    // Send data to ThingsBoard
    sendToThingsBoard(pm1_0, pm2_5, pm10);

    delay(1000); // Adjust delay as needed
}
