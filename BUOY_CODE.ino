#include <WiFiS3.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

// WiFi settings
const char* ssid = "rbruce85@uw.edu";
const char* password = "MyGrace1999!";
WiFiServer server(80);

// Create software serial object for Bluetooth
SoftwareSerial BTSerial(2, 3); // RX, TX 

// Battery monitoring pins and constants
const int batteryPin = A0;
const int solarChargingPin = 7;
const int faradayChargingPin = 8;
const float referenceVoltage = 3.3;  // R4 WiFi uses 3.3V reference
const float maxBatteryVoltage = 4.2;
const float minBatteryVoltage = 3.0;
const float threshold = 20.0;

 // for Faraday minimum voltage
const float faradayMinVoltage = 2.0; 

// Timing parameters
const unsigned long CHECK_INTERVAL = 60000;  // 1 minute
unsigned long lastCheckTime = 0;
const unsigned long BT_UPDATE_INTERVAL = 1000; // 1 second for BT updates
unsigned long lastBTUpdateTime = 0;

// JSON document for data transmission
StaticJsonDocument<200> jsonDoc;
// Function to read Faraday voltage
float readFaradayVoltage() {
    const int numReadings = 5;
    long sum = 0;
    
    // Assuming you have a voltage divider connected to another analog pin
    // You'll need to add the appropriate pin definition
    const int faradayVoltagePin = A1;  // Change this to your actual pin
    
    for(int i = 0; i < numReadings; i++) {
        sum += analogRead(faradayVoltagePin);
        delay(10);
    }
    int averageReading = sum / numReadings;
    return (averageReading / 4095.0) * referenceVoltage;
}

void setupWiFi() {
    // Attempt to connect to WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();
}

float readBatteryVoltage() {
    const int numReadings = 5;
    long sum = 0;
    
    for(int i = 0; i < numReadings; i++) {
        sum += analogRead(batteryPin);
        delay(10);
    }
    
    int averageReading = sum / numReadings;
    // R4 WiFi uses 12-bit ADC (0-4095)
    return (averageReading / 4095.0) * referenceVoltage;
}

void setup() {
    pinMode(solarChargingPin, OUTPUT);
    pinMode(faradayChargingPin, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
     // Initialize Bluetooth Serial
    BTSerial.begin(9600); // HC-05 default baud rate

    setupWiFi();
    Serial.println("System initialized with WiFi capabilities & Bluetooth");
}

void loop() {
    // WiFi check remains the same
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost! Reconnecting...");
        setupWiFi();
    }

    unsigned long currentTime = millis();
    
    if (currentTime - lastCheckTime >= CHECK_INTERVAL) {
        lastCheckTime = currentTime;
        
        float batteryVoltage = readBatteryVoltage();
        float faradayVoltage = readFaradayVoltage();
        float batteryPercentage = map(batteryVoltage * 1000, 
                                    minBatteryVoltage * 1000, 
                                    maxBatteryVoltage * 1000, 
                                    0, 100);
        batteryPercentage = constrain(batteryPercentage, 0, 100);

        bool solarActive = false;
        bool faradayActive = false;

        // Overcharging Protection
        if (batteryVoltage > maxBatteryVoltage) {
            digitalWrite(solarChargingPin, LOW);
            digitalWrite(faradayChargingPin, LOW);
        } 
        else {
            // Modified charging logic - Faraday first, Solar as backup
            if (batteryPercentage < threshold) {
                // Check if Faraday system can provide charge
                if (faradayVoltage >= faradayMinVoltage) {
                    digitalWrite(faradayChargingPin, HIGH);
                    digitalWrite(solarChargingPin, LOW);
                    faradayActive = true;
                    solarActive = false;
                }
                // If Faraday voltage is too low, try solar as backup
                else {
                    digitalWrite(faradayChargingPin, LOW);
                    digitalWrite(solarChargingPin, HIGH);
                    faradayActive = false;
                    solarActive = true;
                }
            } else {
                // If battery above threshold, turn off both charging systems
                digitalWrite(faradayChargingPin, LOW);
                digitalWrite(solarChargingPin, LOW);
                faradayActive = false;
                solarActive = false;
            }
        }

        // Enhanced status printing
        Serial.print("Battery Voltage: ");
        Serial.print(batteryVoltage);
        Serial.print("V, Percentage: ");
        Serial.print(batteryPercentage);
        Serial.print("%, Faraday Voltage: ");
        Serial.print(faradayVoltage);
        Serial.print("V, Faraday: ");
        Serial.print(faradayActive ? "ON" : "OFF");
        Serial.print(", Solar: ");
        Serial.println(solarActive ? "ON" : "OFF");

        // Handle client connections
        WiFiClient client = server.available();
        if (client) {
            Serial.println("New client connected");
            
            String request = client.readStringUntil('\r');
            Serial.println(request);
            client.flush();

            // Enhanced JSON response
            jsonDoc.clear();
            jsonDoc["battery_voltage"] = batteryVoltage;
            jsonDoc["faraday_voltage"] = faradayVoltage;
            jsonDoc["percentage"] = batteryPercentage;
            jsonDoc["faraday"] = faradayActive;
            jsonDoc["solar"] = solarActive;
            jsonDoc["primary_source"] = faradayActive ? "FARADAY" : 
                                      solarActive ? "SOLAR" : "NONE";
            
            if (batteryPercentage <= 10) {
                jsonDoc["status"] = "CRITICAL";
            } else if (batteryPercentage <= 30) {
                jsonDoc["status"] = "LOW";
            } else {
                jsonDoc["status"] = "NORMAL";
            }

            // HTTP response remains the same
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connection: close");
            client.println();
            
            serializeJson(jsonDoc, client);
            
            delay(10);
            client.stop();
            Serial.println("Client disconnected");
        }
    }
}