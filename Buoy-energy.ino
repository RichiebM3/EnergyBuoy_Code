/*
 * Triple-Source Power Buoy Monitoring System
 * Arduino R4 WiFi
 * 
 * Power Sources:
 * 1. Dual Solar Power (DFRobot SP110 x2)
 * 2. Faraday Wave Energy Generator
 * 
 * Features:
 * - Real-time monitoring via Serial, WiFi, and Bluetooth
 * - Intelligent power source switching
 * - Battery health monitoring
 * - JSON API for remote monitoring
 */

#include <Wire.h>
#include <WiFiS3.h>
#include <ArduinoJson.h>
#include "DFRobot_SP110.h"

// WiFi settings
const char* ssid = "rbruce85@uw.edu";
const char* password = "MyGrace1999!";
WiFiServer server(80);

// Solar Power Manager objects
DFRobot_SP110_I2C solarManager1(&Wire, 0x10);
DFRobot_SP110_I2C solarManager2(&Wire, 0x11);

// Pin definitions
const int batteryPin = A0;
const int faradayVoltagePin = A1;
const int solarChargingPin = 7;
const int faradayChargingPin = 8;

// Voltage and battery constants
const float referenceVoltage = 3.3;
const float maxBatteryVoltage = 4.2;
const float minBatteryVoltage = 3.0;
const float faradayMinVoltage = 2.0;
const float batteryThreshold = 20.0;

// Timing parameters
const unsigned long CHECK_INTERVAL = 5000;  // 5 seconds for demo, increase for production
const unsigned long WIFI_UPDATE_INTERVAL = 60000;  // 1 minute
unsigned long lastCheckTime = 0;
unsigned long lastWiFiUpdateTime = 0;

// Solar system data structures
struct SolarSystem {
    float solarVoltage;
    float solarCurrent;
    float batteryVoltage;
    float batteryCurrent;
    float batteryLevel;
    float powerOutput;
    float totalEnergy;
    unsigned long lastMeasurement;
    bool isCharging;
};

SolarSystem system1 = {0};
SolarSystem system2 = {0};

// Faraday system data
struct FaradaySystem {
    float voltage;
    float current;
    float powerOutput;
    float totalEnergy;
    bool isActive;
    unsigned long lastMeasurement;
};

FaradaySystem faradaySystem = {0};

// System status
struct SystemStatus {
    float mainBatteryVoltage;
    float mainBatteryPercentage;
    bool solarActive;
    bool faradayActive;
    String primarySource;
    String batteryStatus;
};

SystemStatus systemStatus = {0};

// JSON document
StaticJsonDocument<512> jsonDoc;

void setup() {
    // Pin setup
    pinMode(solarChargingPin, OUTPUT);
    pinMode(faradayChargingPin, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Serial setup
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    Serial.println(F("\n========================================"));
    Serial.println(F("Triple-Source Power Buoy System"));
    Serial.println(F("========================================"));
    
    // Initialize I2C
    Wire.begin();
    
    // Initialize Solar Managers
    Serial.println(F("\nInitializing Solar Power Managers..."));
    while (!solarManager1.begin()) {
        Serial.println(F("Solar Manager 1 failed. Retrying..."));
        delay(1000);
    }
    Serial.println(F("✓ Solar Manager 1 initialized"));
    
    while (!solarManager2.begin()) {
        Serial.println(F("Solar Manager 2 failed. Retrying..."));
        delay(1000);
    }
    Serial.println(F("✓ Solar Manager 2 initialized"));
    
    // Configure solar managers
    configureSolarManager(solarManager1);
    configureSolarManager(solarManager2);
    
    // Initialize WiFi
    setupWiFi();
    
    // Initialize timing
    system1.lastMeasurement = millis();
    system2.lastMeasurement = millis();
    faradaySystem.lastMeasurement = millis();
    
    Serial.println(F("\n✓ System Ready!"));
    Serial.println(F("========================================\n"));
}

void configureSolarManager(DFRobot_SP110_I2C &manager) {
    manager.setBatteryChargeVoltage(4200);
    manager.setConstantChargeCurrent(1000);
    manager.setConstantVoltage(4200);
    manager.enableCharge();
}

void setupWiFi() {
    Serial.print(F("Connecting to WiFi: "));
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("\n✓ WiFi connected"));
        Serial.print(F("IP address: "));
        Serial.println(WiFi.localIP());
        server.begin();
    } else {
        Serial.println(F("\n✗ WiFi connection failed - continuing without WiFi"));
    }
}

float readBatteryVoltage() {
    const int numReadings = 5;
    long sum = 0;
    
    for(int i = 0; i < numReadings; i++) {
        sum += analogRead(batteryPin);
        delay(10);
    }
    
    int averageReading = sum / numReadings;
    return (averageReading / 4095.0) * referenceVoltage;
}

float readFaradayVoltage() {
    const int numReadings = 5;
    long sum = 0;
    
    for(int i = 0; i < numReadings; i++) {
        sum += analogRead(faradayVoltagePin);
        delay(10);
    }
    
    int averageReading = sum / numReadings;
    return (averageReading / 4095.0) * referenceVoltage;
}

void readSolarSystem(DFRobot_SP110_I2C &manager, SolarSystem &system) {
    unsigned long currentTime = millis();
    float timeElapsed = (currentTime - system.lastMeasurement) / 3600000.0;
    
    system.solarVoltage = manager.getSolarVoltage() / 1000.0;
    system.solarCurrent = manager.getSolarCurrent() / 1000.0;
    system.batteryVoltage = manager.getBatteryVoltage() / 1000.0;
    system.batteryCurrent = manager.getBatteryCurrent() / 1000.0;
    system.batteryLevel = calculateBatteryLevel(system.batteryVoltage);
    system.powerOutput = system.solarVoltage * system.solarCurrent;
    system.isCharging = (system.batteryCurrent > 0);
    
    system.totalEnergy += system.powerOutput * timeElapsed;
    system.lastMeasurement = currentTime;
}

void readFaradaySystem() {
    unsigned long currentTime = millis();
    float timeElapsed = (currentTime - faradaySystem.lastMeasurement) / 3600000.0;
    
    faradaySystem.voltage = readFaradayVoltage();
    // Estimate current based on voltage (adjust based on your system)
    faradaySystem.current = (faradaySystem.voltage > faradayMinVoltage) ? 0.1 : 0.0;
    faradaySystem.powerOutput = faradaySystem.voltage * faradaySystem.current;
    faradaySystem.isActive = (faradaySystem.voltage >= faradayMinVoltage);
    
    faradaySystem.totalEnergy += faradaySystem.powerOutput * timeElapsed;
    faradaySystem.lastMeasurement = currentTime;
}

float calculateBatteryLevel(float voltage) {
    float percentage = (voltage - minBatteryVoltage) / (maxBatteryVoltage - minBatteryVoltage) * 100;
    return constrain(percentage, 0, 100);
}

void managePowerSources() {
    float batteryVoltage = readBatteryVoltage();
    float batteryPercentage = calculateBatteryLevel(batteryVoltage);
    
    systemStatus.mainBatteryVoltage = batteryVoltage;
    systemStatus.mainBatteryPercentage = batteryPercentage;
    
    // Overcharging protection
    if (batteryVoltage > maxBatteryVoltage) {
        digitalWrite(solarChargingPin, LOW);
        digitalWrite(faradayChargingPin, LOW);
        systemStatus.solarActive = false;
        systemStatus.faradayActive = false;
        systemStatus.primarySource = "NONE - FULL";
        systemStatus.batteryStatus = "FULL";
        return;
    }
    
    // Charging logic when battery below threshold
    if (batteryPercentage < batteryThreshold) {
        // Priority 1: Faraday (wave energy)
        if (faradaySystem.voltage >= faradayMinVoltage) {
            digitalWrite(faradayChargingPin, HIGH);
            digitalWrite(solarChargingPin, LOW);
            systemStatus.faradayActive = true;
            systemStatus.solarActive = false;
            systemStatus.primarySource = "FARADAY";
        }
        // Priority 2: Solar (backup)
        else if (system1.solarVoltage > 4.0 || system2.solarVoltage > 4.0) {
            digitalWrite(faradayChargingPin, LOW);
            digitalWrite(solarChargingPin, HIGH);
            systemStatus.faradayActive = false;
            systemStatus.solarActive = true;
            systemStatus.primarySource = "SOLAR";
        }
        // No charging available
        else {
            digitalWrite(faradayChargingPin, LOW);
            digitalWrite(solarChargingPin, LOW);
            systemStatus.faradayActive = false;
            systemStatus.solarActive = false;
            systemStatus.primarySource = "NONE";
        }
    } else {
        // Battery above threshold - no charging needed
        digitalWrite(faradayChargingPin, LOW);
        digitalWrite(solarChargingPin, LOW);
        systemStatus.faradayActive = false;
        systemStatus.solarActive = false;
        systemStatus.primarySource = "NONE - SUFFICIENT";
    }
    
    // Determine battery status
    if (batteryPercentage <= 10) {
        systemStatus.batteryStatus = "CRITICAL";
    } else if (batteryPercentage <= 30) {
        systemStatus.batteryStatus = "LOW";
    } else if (batteryPercentage >= 90) {
        systemStatus.batteryStatus = "FULL";
    } else {
        systemStatus.batteryStatus = "NORMAL";
    }
}

void printSystemStatus() {
    Serial.println(F("\n╔════════════════════════════════════════════════════════╗"));
    Serial.println(F("║          TRIPLE-SOURCE POWER BUOY STATUS              ║"));
    Serial.println(F("╚════════════════════════════════════════════════════════╝"));
    
    // Main Battery Status
    Serial.println(F("\n┌─── MAIN BATTERY ───────────────────────────────────────┐"));
    Serial.print(F("│ Voltage: ")); Serial.print(systemStatus.mainBatteryVoltage, 2); Serial.println(F(" V"));
    Serial.print(F("│ Level: ")); Serial.print(systemStatus.mainBatteryPercentage, 1); Serial.println(F(" %"));
    Serial.print(F("│ Status: ")); Serial.println(systemStatus.batteryStatus);
    Serial.print(F("│ Primary Source: ")); Serial.println(systemStatus.primarySource);
    Serial.println(F("└────────────────────────────────────────────────────────┘"));
    
    // Solar System 1
    Serial.println(F("\n┌─── SOLAR SYSTEM 1 ─────────────────────────────────────┐"));
    Serial.print(F("│ Panel: ")); Serial.print(system1.solarVoltage, 2); Serial.print(F(" V @ "));
    Serial.print(system1.solarCurrent * 1000, 1); Serial.println(F(" mA"));
    Serial.print(F("│ Power: ")); Serial.print(system1.powerOutput, 2); Serial.println(F(" W"));
    Serial.print(F("│ Battery: ")); Serial.print(system1.batteryVoltage, 2); Serial.print(F(" V ("));
    Serial.print(system1.batteryLevel, 1); Serial.println(F("%)"));
    Serial.print(F("│ Energy: ")); Serial.print(system1.totalEnergy, 3); Serial.println(F(" Wh"));
    Serial.print(F("│ Status: ")); Serial.println(system1.isCharging ? "CHARGING" : "IDLE");
    Serial.println(F("└────────────────────────────────────────────────────────┘"));
    
    // Solar System 2
    Serial.println(F("\n┌─── SOLAR SYSTEM 2 ─────────────────────────────────────┐"));
    Serial.print(F("│ Panel: ")); Serial.print(system2.solarVoltage, 2); Serial.print(F(" V @ "));
    Serial.print(system2.solarCurrent * 1000, 1); Serial.println(F(" mA"));
    Serial.print(F("│ Power: ")); Serial.print(system2.powerOutput, 2); Serial.println(F(" W"));
    Serial.print(F("│ Battery: ")); Serial.print(system2.batteryVoltage, 2); Serial.print(F(" V ("));
    Serial.print(system2.batteryLevel, 1); Serial.println(F("%)"));
    Serial.print(F("│ Energy: ")); Serial.print(system2.totalEnergy, 3); Serial.println(F(" Wh"));
    Serial.print(F("│ Status: ")); Serial.println(system2.isCharging ? "CHARGING" : "IDLE");
    Serial.println(F("└────────────────────────────────────────────────────────┘"));
    
    // Faraday System
    Serial.println(F("\n┌─── FARADAY WAVE GENERATOR ─────────────────────────────┐"));
    Serial.print(F("│ Voltage: ")); Serial.print(faradaySystem.voltage, 2); Serial.println(F(" V"));
    Serial.print(F("│ Power: ")); Serial.print(faradaySystem.powerOutput, 3); Serial.println(F(" W"));
    Serial.print(F("│ Energy: ")); Serial.print(faradaySystem.totalEnergy, 3); Serial.println(F(" Wh"));
    Serial.print(F("│ Status: ")); Serial.println(faradaySystem.isActive ? "ACTIVE" : "INACTIVE");
    Serial.print(F("│ Charging: ")); Serial.println(systemStatus.faradayActive ? "ON" : "OFF");
    Serial.println(F("└────────────────────────────────────────────────────────┘"));
    
    // Combined Statistics
    float totalPower = system1.powerOutput + system2.powerOutput + faradaySystem.powerOutput;
    float totalEnergy = system1.totalEnergy + system2.totalEnergy + faradaySystem.totalEnergy;
    
    Serial.println(F("\n┌─── COMBINED SYSTEM ────────────────────────────────────┐"));
    Serial.print(F("│ Total Power: ")); Serial.print(totalPower, 2); Serial.println(F(" W"));
    Serial.print(F("│ Total Energy: ")); Serial.print(totalEnergy, 3); Serial.println(F(" Wh"));
    Serial.print(F("│ Uptime: ")); Serial.print(millis() / 1000); Serial.println(F(" seconds"));
    Serial.println(F("└────────────────────────────────────────────────────────┘\n"));
}

void handleWiFiClient() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println(F("New WiFi client connected"));
        
        String request = client.readStringUntil('\r');
        client.flush();
        
        // Build JSON response
        jsonDoc.clear();
        
        // Main battery
        jsonDoc["main_battery"]["voltage"] = systemStatus.mainBatteryVoltage;
        jsonDoc["main_battery"]["percentage"] = systemStatus.mainBatteryPercentage;
        jsonDoc["main_battery"]["status"] = systemStatus.batteryStatus;
        
        // Solar systems
        jsonDoc["solar1"]["voltage"] = system1.solarVoltage;
        jsonDoc["solar1"]["current"] = system1.solarCurrent;
        jsonDoc["solar1"]["power"] = system1.powerOutput;
        jsonDoc["solar1"]["energy"] = system1.totalEnergy;
        jsonDoc["solar1"]["charging"] = system1.isCharging;
        
        jsonDoc["solar2"]["voltage"] = system2.solarVoltage;
        jsonDoc["solar2"]["current"] = system2.solarCurrent;
        jsonDoc["solar2"]["power"] = system2.powerOutput;
        jsonDoc["solar2"]["energy"] = system2.totalEnergy;
        jsonDoc["solar2"]["charging"] = system2.isCharging;
        
        // Faraday system
        jsonDoc["faraday"]["voltage"] = faradaySystem.voltage;
        jsonDoc["faraday"]["power"] = faradaySystem.powerOutput;
        jsonDoc["faraday"]["energy"] = faradaySystem.totalEnergy;
        jsonDoc["faraday"]["active"] = faradaySystem.isActive;
        
        // System status
        jsonDoc["system"]["primary_source"] = systemStatus.primarySource;
        jsonDoc["system"]["solar_active"] = systemStatus.solarActive;
        jsonDoc["system"]["faraday_active"] = systemStatus.faradayActive;
        jsonDoc["system"]["uptime"] = millis() / 1000;
        
        // Send HTTP response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Access-Control-Allow-Origin: *");
        client.println("Connection: close");
        client.println();
        
        serializeJson(jsonDoc, client);
        
        delay(10);
        client.stop();
        Serial.println(F("Client disconnected"));
    }
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        static unsigned long lastReconnect = 0;
        if (currentTime - lastReconnect > 30000) {
            Serial.println(F("WiFi disconnected - attempting reconnect..."));
            setupWiFi();
            lastReconnect = currentTime;
        }
    }
    
    // Regular system monitoring
    if (currentTime - lastCheckTime >= CHECK_INTERVAL) {
        lastCheckTime = currentTime;
        
        // Read all power systems
        readSolarSystem(solarManager1, system1);
        readSolarSystem(solarManager2, system2);
        readFaradaySystem();
        
        // Manage power source switching
        managePowerSources();
        
        // Print status to Serial
        printSystemStatus();
    }
    
    // Handle WiFi clients
    handleWiFiClient();
}
