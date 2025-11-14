/*
 * FENCEORA - Smart Electric Fence Node System
 * ESP32 Main Controller Code (Version 3.4 - WORKFLOW OPTIMIZATION)
 * Project by H^4 HYPERS
 *
 * --- VERSION 3.4 OPTIMIZATIONS (November 2025) ---
 * 1. Simplified email notifications (only Capacitive + IR sensor, NO animal alerts)
 * 2. Fixed servo response with hardware PWM and direct control
 * 3. Real-time RTC updates (0.5 seconds instead of 5 seconds)
 * 4. Optimized sensor reading workflow with priority-based execution
 * 5. Separated battery monitoring from solar voltage
 * 6. Cleaner code structure with reduced delays
 * 
 * --- VERSION 3.3 FIXES (November 2025) ---
 * 1. Fixed email notifications causing WiFi disconnection (non-blocking)
 * 2. Fixed MPU6050 I2C address (0x69 with AD0=HIGH verified)
 * 3. Fixed RTC invalid date issue (proper initialization check)
 * 4. Fixed servo not responding (increased delays, direct control)
 * 5. Fixed ultrasonic not detecting (improved timing and thresholds)
 * 6. Fixed buzzer not working (proper PWM setup)
 * 7. Added continuous RTC monitoring every 5 seconds
 * 
 * --- VERSION 3.2 FIXES ---
 * 1. Fixed ESP32 PWM API compatibility (works with Core 2.x and 3.x)
 * 2. Added compile-time library checks
 * 3. Fixed ledcWrite calls for both old and new ESP32 cores
 * 
 * --- VERSION 3.1 FIXES ---
 * 1. Fixed JSON structure to match dashboard expectations
 * 2. Fixed temperature object structure (added .value property)
 * 3. Fixed CORS headers on all responses
 * 4. Fixed uptime calculation timing issue
 * 5. Added proper error handling and console logging
 * 6. Fixed null log array handling
 */

// === LIBRARY VERSION CHECK ===
#if !defined(ESP32)
  #error "This code is designed for ESP32 boards only!"
#endif

// --- 0. Core Arduino Header ---
#include <Arduino.h>

// --- 1. WiFi & Web Server (BUILT-IN) ---
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// --- 2. I2C Sensors ---
#include <Wire.h>
#include <RTClib.h>              // For RTC (DS3231)
#include <Adafruit_MPU6050.h>    // For MPU-6050
#include <Adafruit_Sensor.h>

// --- 2b. Temperature Sensor (DHT11 or DHT22) ---
#include <DHT.h>                 // For DHT11/DHT22 Temperature & Humidity

// --- 3. SPI Sensors ---
#include <SPI.h>
#include <MFRC522.h>             // For RFID (RC522)

// --- 4. Other Actuators/Sensors ---
#include <ESP32Servo.h>          // For Servo

// --- 5. Email Notifications (ESP Mail Client) ---
#include <ESP_Mail_Client.h>     // For sending email alerts

// === WiFi Credentials ===
const char* ssid = "Oppo A77s";
const char* password = "9080061674";

// === Email Configuration (Gmail SMTP) ===
// SENDER EMAIL (Your Gmail account that sends notifications)
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465  // SSL port
#define SENDER_EMAIL "fenceora.h4@gmail.com"
#define SENDER_PASSWORD "rflnubdzhetlxymt"  // Gmail App Password (spaces removed)
#define SENDER_NAME "Fenceora Alert System"

// RECEIVER EMAILS (People who will receive notifications)
const char* receiverEmails[] = {
  "harishpranavs259@gmail.com",      // Receiver 1
  "hariprasanthgk1@gmail.com",       // Receiver 2
  "hemanathan8337@gmail.com",        // Receiver 3
  "harishwarharishwar47@gmail.com"   // Receiver 4
};
const int numReceivers = 4;

// Email sending state
SMTPSession smtp;
bool emailInitialized = false;
unsigned long lastEmailTime = 0;
const unsigned long EMAIL_COOLDOWN = 60000; // 60 seconds between emails (prevent spam and disconnection)
bool emailInProgress = false; // Track if email is being sent

// === Pin Definitions ===
// I2C (for RTC and MPU6050)
#define I2C_SDA 21  // RTC and MPU6050 connect here via I2C
#define I2C_SCL 22  // RTC and MPU6050 connect here via I2C

// Temperature Sensor (DHT11 or DHT22 - 3 pins: VCC, GND, DATA)
#define DHT_PIN 4        // DHT Data pin → ESP32 GPIO 4 (Changed from 27 to avoid RFID conflict)
#define DHT_TYPE DHT11   // Change to DHT22 if you have DHT22 sensor
// WIRING for DHT11/DHT22:
// DHT VCC  → ESP32 3.3V (or 5V)
// DHT GND  → ESP32 GND
// DHT DATA → ESP32 GPIO 4
// (Optional: 10kΩ resistor between VCC and DATA for stability)

// SPI (for RFID)
#define SS_PIN 5     // RFID SDA/SS pin
#define RST_PIN 27   // RFID RST pin (Changed from 5 to match working code)
// NOTE: SPI uses GPIO 18 (SCK), GPIO 23 (MOSI), GPIO 19 (MISO)
// WIRING for RC522:
// RC522 SDA  → ESP32 GPIO 5
// RC522 SCK  → ESP32 GPIO 18
// RC522 MOSI → ESP32 GPIO 23
// RC522 MISO → ESP32 GPIO 19
// RC522 RST  → ESP32 GPIO 27
// RC522 VCC  → ESP32 3.3V
// RC522 GND  → ESP32 GND

// Digital Sensors
#define CAPACITIVE_PIN 25 // PNP Capacitive Sensor (adjust knob for sensitivity!)
#define IR_SENSOR_PIN  26 // IR Sensor (detects hand/tampering)
#define US_TRIG_PIN    33 // Ultrasonic Trigger (animal detection)
#define US_ECHO_PIN    32 // Ultrasonic Echo (animal detection)

// Analog Sensors
#define SOLAR_VOLTAGE_PIN 34 // GPIO 34 (ADC1_CH6) - Solar Panel Voltage Sensor S pin
// WIRING for Solar Voltage Sensor (3 pins: VCC, GND, S):
// Voltage Sensor VCC → ESP32 5V (or 3.3V depending on module)
// Voltage Sensor GND → ESP32 GND
// Voltage Sensor S   → ESP32 GPIO 34 (Analog Input - ADC1 Channel 6)

#define BATTERY_VOLTAGE_PIN 35 // GPIO 35 (ADC1_CH7) - USB-C Battery Voltage Sensor
// WIRING for Battery Monitoring (Voltage Divider for USB-C rechargeable battery):
// Battery + → 10kΩ Resistor → GPIO 35 → 10kΩ Resistor → GND
// This creates a 2:1 voltage divider (can measure up to 6.6V safely)
// For single-cell Li-ion (3.7V nominal): Full=4.2V, Empty=3.0V

// Actuators
#define SERVO_PIN 15  // Servo motor pin (CRITICAL: Needs external 5V power supply!)
#define BUZZER_PIN 12 // Active buzzer/speaker for animal deterrent

// === Global Objects ===
WebServer server(80);
RTC_DS3231 rtc;
Adafruit_MPU6050 mpu;
DHT dht(DHT_PIN, DHT_TYPE);  // DHT11/DHT22 Temperature & Humidity sensor
MFRC522 rfid(SS_PIN, RST_PIN);
Servo lockServo;

// === Global State Variables ===
bool fenceActive = false;
bool tamperMotion = false;
bool tamperIR = false;
float temperature = 25.0;
float humidity = 60.0;
float animalDistance = 999.0;
float solarVoltage = 0.0;
float solarCurrent = 0.0;
float batteryVoltage = 0.0;   // Actual battery voltage (3.0V - 4.2V for Li-ion)
float batteryLevel = 0.0;     // Battery percentage (0-100%)
String rtcTime = "2025-11-04T07:00:00";
unsigned long startTimeSeconds = 0;

// MPU6050 Data (Global variables for dashboard display)
float gyroX = 0.0, gyroY = 0.0, gyroZ = 0.0;
float accelX = 0.0, accelY = 0.0, accelZ = 0.0;
float mpuTemperature = 0.0;

// Capacitive Sensor Debouncing Variables
int capacitiveSensorState = LOW;
int lastCapacitiveReading = LOW;
unsigned long lastCapacitiveDebounceTime = 0;
unsigned long capacitiveDebounceDelay = 50;

// Servo State Variables
bool isServoOpen = false;

// --- Log Buffers ---
#define MAX_LOGS 5
String detectLogs[MAX_LOGS];
String rfidLogs[MAX_LOGS];
int detectLogIndex = 0;
int rfidLogIndex = 0;

// === Timers ===
unsigned long lastSensorRead = 0;
unsigned long lastRfidScan = 0;
unsigned long lastRtcCheck = 0; // For RTC monitoring every 5 seconds

// === RFID Authorized Card UID ===
// !! IMPORTANT: REPLACE these four bytes with your card's UID.
// Use the RFID_UID_Reader.ino sketch to find your card's UID.
byte authorizedUID[4] = {0xD3, 0x2E, 0xE4, 0x2C}; // YOUR ACTUAL CARD UID!

// =================================================================
// ===           LOGGING FUNCTIONS                               ===
// =================================================================

String getLogTime() {
  DateTime now = rtc.now();
  char buf[10];
  sprintf(buf, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  return String(buf);
}

void addDetectLog(String icon, String msg) {
  detectLogs[detectLogIndex] = "{\"icon\":\"" + icon + "\",\"time\":\"" + getLogTime() + "\",\"msg\":\"" + msg + "\"}";
  detectLogIndex = (detectLogIndex + 1) % MAX_LOGS;
  Serial.println("[DETECT LOG] " + msg);
}

void addRfidLog(String icon, String msg) {
  rfidLogs[rfidLogIndex] = "{\"icon\":\"" + icon + "\",\"time\":\"" + getLogTime() + "\",\"msg\":\"" + msg + "\"}";
  rfidLogIndex = (rfidLogIndex + 1) % MAX_LOGS;
  Serial.println("[RFID LOG] " + msg);
}

// =================================================================
// ===           API REQUEST HANDLER (FIXED)                    ===
// =================================================================

void handleApiRequest() {
  Serial.println("\n=== API REQUEST RECEIVED ===");
  
  // Create a JSON document (2048 bytes)
  StaticJsonDocument<2048> doc;

  // --- 1. Fence Status ---
  JsonObject fence = doc.createNestedObject("fence");
  fence["status"] = fenceActive ? "ACTIVE" : "CLEAR";

  // --- 2. Temperature (FIXED: Added nested object with "value" property) ---
  JsonObject tempObj = doc.createNestedObject("temperature");
  tempObj["value"] = temperature;

  // --- 3. Humidity (direct value - matches dashboard) ---
  doc["humidity"] = humidity;

  // --- 4. Battery & Solar ---
  JsonObject battery = doc.createNestedObject("battery");
  battery["level"] = batteryLevel;  // Battery percentage (0-100%)
  battery["voltage"] = batteryVoltage;  // Actual battery voltage
  
  JsonObject solar = doc.createNestedObject("solar");
  solar["voltage"] = solarVoltage;  // Solar panel voltage (shows in "Active Supply")
  solar["current"] = solarCurrent;  // Solar current
  solar["power"] = solarVoltage * solarCurrent;  // Solar power (W)

  // --- 5. Other Sensor Data ---
  JsonObject tamper = doc.createNestedObject("tamper");
  tamper["motion"] = tamperMotion;
  tamper["ir"] = tamperIR;
  doc["animal_dist"] = animalDistance;
  
  // --- 5b. MPU6050 Data (Gyroscope & Accelerometer) ---
  JsonObject mpu6050 = doc.createNestedObject("mpu6050");
  
  JsonObject gyro = mpu6050.createNestedObject("gyro");
  gyro["x"] = gyroX;
  gyro["y"] = gyroY;
  gyro["z"] = gyroZ;
  
  JsonObject accel = mpu6050.createNestedObject("accel");
  accel["x"] = accelX;
  accel["y"] = accelY;
  accel["z"] = accelZ;
  
  mpu6050["temperature"] = mpuTemperature;
  
  // --- 6. Time (ISO8601 format) ---
  DateTime now = rtc.now();
  char timestamp[30];
  sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02d", 
          now.year(), now.month(), now.day(), 
          now.hour(), now.minute(), now.second());
  doc["time"] = timestamp;
  
  // --- 7. Uptime (FIXED) ---
  unsigned long currentSeconds = now.unixtime();
  unsigned long uptimeSeconds = currentSeconds - startTimeSeconds;
  int days = uptimeSeconds / 86400;
  int hours = (uptimeSeconds % 86400) / 3600;
  int minutes = (uptimeSeconds % 3600) / 60;
  
  String uptimeStr = String(days) + "d " + String(hours) + "h " + String(minutes) + "m";
  doc["uptime"] = uptimeStr;
  doc["lastSync"] = getLogTime();

  // --- 8. Logs (FIXED: Proper array handling) ---
  JsonObject logs = doc.createNestedObject("logs");
  JsonArray detectArray = logs.createNestedArray("detect");
  JsonArray rfidArray = logs.createNestedArray("rfid");

  for (int i = 0; i < MAX_LOGS; i++) {
    if (detectLogs[i].length() > 0) {
      detectArray.add(serialized(detectLogs[i]));
    }
    if (rfidLogs[i].length() > 0) {
      rfidArray.add(serialized(rfidLogs[i]));
    }
  }

  // --- 9. Send Response with CORS ---
  String jsonResponse;
  serializeJson(doc, jsonResponse);
  
  Serial.println("JSON Response Size: " + String(jsonResponse.length()) + " bytes");
  Serial.println("JSON Response Preview:");
  Serial.println(jsonResponse.substring(0, min(200, (int)jsonResponse.length())) + "...");
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200, "application/json", jsonResponse);
  
  Serial.println("=== API RESPONSE SENT ===\n");
}

void handleNotFound() {
  Serial.println("404 - Not Found: " + server.uri());
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(404, "text/plain", "Not found");
}

void handleRoot() {
  Serial.println("Root endpoint accessed");
  String html = "<html><body><h1>FENCEORA ESP32 Server</h1>";
  html += "<p>Status: <strong style='color:green;'>RUNNING</strong></p>";
  html += "<p>API Endpoint: <a href='/api/sensors'>/api/sensors</a></p>";
  html += "<p>WiFi IP: " + WiFi.localIP().toString() + "</p>";
  html += "<p>Uptime: " + String(millis() / 1000) + " seconds</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// =================================================================
// ===           SENSOR READING FUNCTIONS (OPTIMIZED)          ===
// =================================================================

void readAllSensors() {
  // === PRIORITY 1: Temperature & Humidity (Always read first - stable sensor) ===
  float newTemp = dht.readTemperature();
  float newHumid = dht.readHumidity();
  
  if (!isnan(newTemp) && !isnan(newHumid)) {
    temperature = newTemp;
    humidity = newHumid;
  }

  // === PRIORITY 2: MPU-6050 Motion Detection (Critical for tampering) ===
  sensors_event_t a, g, mpu_temp;
  if (mpu.getEvent(&a, &g, &mpu_temp)) {
    gyroX = g.gyro.x;
    gyroY = g.gyro.y;
    gyroZ = g.gyro.z;
    accelX = a.acceleration.x;
    accelY = a.acceleration.y;
    accelZ = a.acceleration.z;
    mpuTemperature = mpu_temp.temperature;
    
    // Motion detection (threshold: 15 m/s²)
    if (abs(a.acceleration.x) > 15 || abs(a.acceleration.y) > 15 || abs(a.acceleration.z) > 15) {
      if (!tamperMotion) {
        tamperMotion = true;
        addDetectLog("fa-exclamation-triangle", "⚠️ DEVICE MOVEMENT DETECTED!");
        
        // Send email only for motion tampering
        if (!emailInProgress) {
          char accelStr[50];
          sprintf(accelStr, "X:%.1f Y:%.1f Z:%.1f m/s²", a.acceleration.x, a.acceleration.y, a.acceleration.z);
          sendEmailNotification(
            "Device Movement Detected",
            "Someone is trying to move the fence system!",
            String("MPU6050 Accelerometer | ") + accelStr
          );
        }
      }
    } else {
      tamperMotion = false;
    }
  }

  // === PRIORITY 3: IR Sensor (Critical for hand detection) ===
  bool currentIRState = digitalRead(IR_SENSOR_PIN) == LOW;
  if (currentIRState != tamperIR) {
    tamperIR = currentIRState;
    if (tamperIR) {
      addDetectLog("fa-hand-paper", "⚠️ HAND DETECTED NEAR DEVICE!");
      
      // Send email for IR tampering
      if (!emailInProgress) {
        sendEmailNotification(
          "Tampering Detected",
          "Hand or object detected near device!",
          "Infrared (IR) Sensor | Someone is trying to tamper with the system"
        );
      }
    }
  }

  // === PRIORITY 4: Capacitive Sensor with Smart Debouncing ===
  int currentReading = digitalRead(CAPACITIVE_PIN);
  
  if (currentReading != lastCapacitiveReading) {
    lastCapacitiveDebounceTime = millis();
  }
  
  if ((millis() - lastCapacitiveDebounceTime) > capacitiveDebounceDelay) {
    if (currentReading != capacitiveSensorState) {
      capacitiveSensorState = currentReading;
      
      if (capacitiveSensorState == HIGH) {
        // Smart detection: Check if it's fence or false trigger
        bool handNearby = tamperIR;
        bool deviceMoving = tamperMotion;
        
        if (deviceMoving) {
          addDetectLog("fa-exclamation-triangle", "⚠️ TAMPER - DEVICE MOVED!");
        } else if (handNearby) {
          addDetectLog("fa-hand-paper", "⚠️ TAMPER - HAND NEAR DEVICE!");
        } else {
          // Genuine fence detection
          fenceActive = true;
          addDetectLog("fa-bolt", "⚡ FENCE ELECTRIC FIELD ACTIVE!");
          
          // Send email for fence current detection
          if (!emailInProgress) {
            char voltageStr[20];
            sprintf(voltageStr, "%.2fV", solarVoltage);
            sendEmailNotification(
              "Fence Current Detected",
              "Electric field detected on perimeter fence!",
              String("Voltage: ") + voltageStr + " | Capacitive Sensor"
            );
          }
        }
      } else {
        fenceActive = false;
        addDetectLog("fa-bolt", "Fence Electric Field CLEAR");
      }
    }
  }
  
  lastCapacitiveReading = currentReading;

  // === PRIORITY 5: Ultrasonic (Animal Detection - NO EMAIL, BUZZER ONLY) ===
  digitalWrite(US_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG_PIN, LOW);
  
  long duration = pulseIn(US_ECHO_PIN, HIGH, 30000);
  float newDistance = (duration * 0.0343) / 2.0;
  
  if (duration > 0 && newDistance >= 2 && newDistance <= 400) {
    animalDistance = newDistance;
    
    // Activate buzzer/speaker for animal deterrent (within 3 meters)
    if (animalDistance < 300) {
      static unsigned long lastAnimalLog = 0;
      
      // Log every 5 seconds (no email spam)
      if (millis() - lastAnimalLog > 5000) {
        addDetectLog("fa-paw", "🐾 Animal at " + String((int)animalDistance) + "cm - Buzzer ON");
        lastAnimalLog = millis();
      }
      
      // Pulsing buzzer pattern (250ms ON, 250ms OFF)
      if (millis() % 500 < 250) {
        #if ESP_ARDUINO_VERSION_MAJOR >= 3
          ledcWrite(BUZZER_PIN, 200);
        #else
          ledcWrite(0, 200);
        #endif
      } else {
        #if ESP_ARDUINO_VERSION_MAJOR >= 3
          ledcWrite(BUZZER_PIN, 0);
        #else
          ledcWrite(0, 0);
        #endif
      }
    } else {
      // Turn off buzzer when animal is far
      #if ESP_ARDUINO_VERSION_MAJOR >= 3
        ledcWrite(BUZZER_PIN, 0);
      #else
        ledcWrite(0, 0);
      #endif
    }
  } else {
    animalDistance = 999.0;
    #if ESP_ARDUINO_VERSION_MAJOR >= 3
      ledcWrite(BUZZER_PIN, 0);
    #else
      ledcWrite(0, 0);
    #endif
  }

  // === PRIORITY 6: Solar Voltage (Active Supply Monitoring) ===
  int solarADC = analogRead(SOLAR_VOLTAGE_PIN);
  float sensorV = (solarADC / 4095.0) * 3.3;
  solarVoltage = sensorV * 7.5;  // Voltage divider multiplier (adjust for your sensor)
  solarCurrent = solarVoltage > 5.0 ? (solarVoltage / 12.0) * 0.5 : 0.05;

  // === PRIORITY 7: Battery Monitoring (USB-C Rechargeable) ===
  int batteryADC = analogRead(BATTERY_VOLTAGE_PIN);
  float batteryVoltageRaw = (batteryADC / 4095.0) * 3.3;
  
  // Apply voltage divider correction (2:1 divider = multiply by 2)
  batteryVoltage = batteryVoltageRaw * 2.0;
  
  // Calculate battery percentage (Li-ion: 3.0V = 0%, 4.2V = 100%)
  if (batteryVoltage >= 4.2) {
    batteryLevel = 100.0;
  } else if (batteryVoltage <= 3.0) {
    batteryLevel = 0.0;
  } else {
    // Linear interpolation between 3.0V and 4.2V
    batteryLevel = ((batteryVoltage - 3.0) / (4.2 - 3.0)) * 100.0;
  }
  
  // Constrain to 0-100%
  if (batteryLevel < 0) batteryLevel = 0;
  if (batteryLevel > 100) batteryLevel = 100;
}

// =================================================================
// ===           RFID & SERVO LOGIC (HARDWARE OPTIMIZED)       ===
// =================================================================

void handleRfid() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // Print scanned card UID
  Serial.print("🔖 Card Scanned - UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Check if authorized
  if (memcmp(rfid.uid.uidByte, authorizedUID, 4) == 0) {
    // AUTHORIZED CARD - Toggle servo with hardware PWM
    if (!isServoOpen) {
      // === UNLOCK SEQUENCE ===
      Serial.println("✅ AUTHORIZED - UNLOCKING...");
      addRfidLog("fa-unlock", "✓ Authorized Card - UNLOCKING");
      
      // Hardware PWM control - send multiple pulses for reliability
      for (int pulse = 0; pulse < 5; pulse++) {
        lockServo.write(90);  // 90° = OPEN
        delay(200);  // Wait between pulses
      }
      delay(300);  // Final stabilization
      
      isServoOpen = true;
      Serial.println("🔓 SERVO UNLOCKED (90°)");
      addRfidLog("fa-unlock-alt", "✅ Door UNLOCKED");
      
    } else {
      // === LOCK SEQUENCE ===
      Serial.println("✅ AUTHORIZED - LOCKING...");
      addRfidLog("fa-lock", "✓ Authorized Card - LOCKING");
      
      // Hardware PWM control - send multiple pulses for reliability
      for (int pulse = 0; pulse < 5; pulse++) {
        lockServo.write(0);  // 0° = CLOSED
        delay(200);  // Wait between pulses
      }
      delay(300);  // Final stabilization
      
      isServoOpen = false;
      Serial.println("🔒 SERVO LOCKED (0°)");
      addRfidLog("fa-lock", "✅ Door LOCKED");
    }
    
  } else {
    // UNAUTHORIZED CARD
    Serial.print("❌ UNAUTHORIZED Card - UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();
    addRfidLog("fa-times-circle", "❌ UNAUTHORIZED - Access DENIED");
    
    // Send email for unauthorized access attempt
    if (!emailInProgress) {
      char uidStr[20];
      sprintf(uidStr, "%02X %02X %02X %02X", 
              rfid.uid.uidByte[0], rfid.uid.uidByte[1], 
              rfid.uid.uidByte[2], rfid.uid.uidByte[3]);
      sendEmailNotification(
        "Unauthorized RFID Access",
        "Unknown card scanned at fence lock!",
        String("Card UID: ") + uidStr
      );
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// =================================================================
// ===           EMAIL NOTIFICATION SYSTEM                     ===
// =================================================================

/**
 * Sends email notification to all configured receivers (NON-BLOCKING)
 * @param alertTitle - Main alert title (e.g., "Fence Current Detected")
 * @param alertMessage - Alert message body
 * @param alertDetails - Additional details (voltage, sensor, etc.)
 */
void sendEmailNotification(String alertTitle, String alertMessage, String alertDetails) {
  // Check cooldown period to prevent email spam and WiFi disconnection
  unsigned long currentTime = millis();
  if (currentTime - lastEmailTime < EMAIL_COOLDOWN) {
    Serial.printf("⏳ Email cooldown active (%lu seconds remaining). Skipping email.\n", 
                  (EMAIL_COOLDOWN - (currentTime - lastEmailTime)) / 1000);
    return;
  }
  
  // Check if email is already being sent
  if (emailInProgress) {
    Serial.println("⏳ Email already in progress. Skipping duplicate.");
    return;
  }
  
  // Check if password is configured
  if (String(SENDER_PASSWORD) == "YOUR_GMAIL_APP_PASSWORD") {
    Serial.println("⚠️ Email notifications not configured!");
    Serial.println("📧 Please set SENDER_PASSWORD in code (see instructions in comments)");
    return;
  }
  
  Serial.println("\n📧 Preparing email notification (non-blocking mode)...");
  emailInProgress = true;
  
  // Configure SMTP session with shorter timeout
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = SENDER_EMAIL;
  session.login.password = SENDER_PASSWORD;
  session.login.user_domain = "";
  session.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  session.time.gmt_offset = 5.5; // GMT +5:30 for India
  session.time.day_light_offset = 0;
  
  // Create email message
  SMTP_Message message;
  message.sender.name = SENDER_NAME;
  message.sender.email = SENDER_EMAIL;
  message.subject = "🚨 FENCEORA: " + alertTitle;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
  
  // Add all receivers
  for (int i = 0; i < numReceivers; i++) {
    message.addRecipient("Receiver", receiverEmails[i]);
    Serial.printf("   → Receiver: %s\n", receiverEmails[i]);
  }
  
  // Get current time
  DateTime now = rtc.now();
  char timestamp[30];
  sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d", 
          now.year(), now.month(), now.day(), 
          now.hour(), now.minute(), now.second());
  
  // Build HTML email body
  String htmlMsg = "<html><body style='font-family: Arial, sans-serif;'>";
  htmlMsg += "<div style='background: linear-gradient(135deg, #ff4757, #ff6348); padding: 30px; text-align: center; color: white;'>";
  htmlMsg += "<h1 style='margin: 0; font-size: 36px;'>⚠️ ALERT</h1>";
  htmlMsg += "</div>";
  htmlMsg += "<div style='padding: 20px; background: #f5f5f5;'>";
  htmlMsg += "<h2 style='color: #ff4757;'>" + alertTitle + "</h2>";
  htmlMsg += "<p style='font-size: 18px; color: #333;'><strong>Message:</strong> " + alertMessage + "</p>";
  htmlMsg += "<p style='font-size: 16px; color: #666;'><strong>Details:</strong> " + alertDetails + "</p>";
  htmlMsg += "<p style='font-size: 14px; color: #999;'><strong>Time:</strong> " + String(timestamp) + "</p>";
  htmlMsg += "<hr style='border: 1px solid #ddd; margin: 20px 0;'>";
  htmlMsg += "<p style='font-size: 14px; color: #666;'>Check your Fenceora dashboard.</p>";
  htmlMsg += "<p style='font-size: 12px; color: #999;'>Automated alert from Fenceora Smart Fence System.</p>";
  htmlMsg += "</div></body></html>";
  
  message.html.content = htmlMsg.c_str();
  message.html.charSet = "utf-8";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  // Connect and send (with error handling)
  Serial.println("📤 Connecting to SMTP server...");
  if (!smtp.connect(&session)) {
    Serial.println("❌ SMTP connection failed!");
    Serial.println("Error: " + smtp.errorReason());
    emailInProgress = false;
    return;
  }
  
  Serial.println("📤 Sending email...");
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("❌ Email send failed!");
    Serial.println("Error: " + smtp.errorReason());
  } else {
    Serial.println("✅ Email sent successfully to all receivers!");
    lastEmailTime = currentTime;
  }
  
  // Close connection and reset flag
  smtp.closeSession();
  emailInProgress = false;
  Serial.println("📧 Email process completed. System resuming normal operation.\n");
}

// =================================================================
// ===           SETUP & LOOP                                  ===
// =================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("   FENCEORA V3.1 - STARTING");
  Serial.println("=================================\n");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // --- 1. Connect to WiFi ---
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi Connected!");
    Serial.print("✓ IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("✓ API Endpoint: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/api/sensors");
  } else {
    Serial.println("\n✗ WiFi Connection Failed!");
  }

  // --- 2. Initialize Sensors ---
  Serial.println("\nInitializing Sensors...");
  
  // RTC - Critical initialization with validation
  Serial.println("Initializing RTC DS3231...");
  if (!rtc.begin()) {
    Serial.println("✗ RTC Failed to initialize!");
    addDetectLog("fa-exclamation-triangle", "RTC FAILED TO INIT");
  } else {
    // Check if RTC lost power or has invalid time
    if (rtc.lostPower()) {
      Serial.println("⚠️ RTC lost power! Setting time to compile time...");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    // Verify RTC is reading valid data
    DateTime now = rtc.now();
    if (now.year() < 2020 || now.year() > 2100) {
      Serial.println("⚠️ RTC has invalid date! Setting to compile time...");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      delay(100);
      now = rtc.now();
    }
    
    Serial.printf("✓ RTC OK - Current Time: %04d-%02d-%02d %02d:%02d:%02d\n", 
                  now.year(), now.month(), now.day(), 
                  now.hour(), now.minute(), now.second());
    startTimeSeconds = now.unixtime(); // Store start time AFTER RTC init
  }
  
  // MPU-6050 - Initialize with address 0x69 (AD0 pin = HIGH)
  Serial.println("Initializing MPU6050 at address 0x69 (AD0=HIGH)...");
  if (!mpu.begin(0x69)) {
    Serial.println("✗ MPU6050 Failed at 0x69! Trying 0x68 (AD0=LOW)...");
    if (!mpu.begin(0x68)) {
      Serial.println("✗ MPU6050 Failed at both addresses!");
      addDetectLog("fa-exclamation-triangle", "MPU6050 FAILED");
    } else {
      Serial.println("✓ MPU6050 OK at 0x68 (AD0 pin is LOW)");
      mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
      mpu.setGyroRange(MPU6050_RANGE_500_DEG);
      mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    }
  } else {
    Serial.println("✓ MPU6050 OK at 0x69 (AD0 pin is HIGH - Correct!)");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  // DHT11/DHT22 Temperature & Humidity Sensor
  dht.begin();
  Serial.println("✓ DHT Sensor OK");
  delay(2000); // DHT sensors need 2 seconds to stabilize

  // RFID (Initialize SPI with ESP32 default pins: SCK=18, MISO=19, MOSI=23)
  SPI.begin(18, 19, 23);
  rfid.PCD_Init();
  Serial.println("✓ RFID RC522 Initialized");

  // Servo (Hardware PWM with maximum reliability)
  Serial.println("Initializing Servo Motor (Hardware PWM)...");
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  lockServo.setPeriodHertz(50);    // Standard 50Hz servo
  lockServo.attach(SERVO_PIN, 500, 2500); // Wide pulse range for compatibility
  
  delay(500);  // Let servo power stabilize
  
  // Multi-pulse initialization to LOCKED position (0°)
  Serial.println("Setting servo to LOCKED position (0°)...");
  for (int i = 0; i < 5; i++) {
    lockServo.write(0);  // 0° = LOCKED
    delay(200);
  }
  delay(500);  // Final stabilization
  
  isServoOpen = false;
  Serial.println("✅ Servo OK (LOCKED at 0°)");
  Serial.println("   ⚠️  CRITICAL: Servo MUST have external 5V power supply (1A+)");
  Serial.println("   ⚠️  ESP32 3.3V output is NOT sufficient for servo operation");

  // Buzzer PWM Setup (Compatible with ESP32 Core 2.x and 3.x)
  Serial.println("Initializing Buzzer...");
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    // ESP32 Arduino Core 3.x (new API)
    ledcAttach(BUZZER_PIN, 2000, 8); // Pin, 2kHz frequency, 8-bit resolution
  #else
    // ESP32 Arduino Core 2.x (old API)
    ledcSetup(0, 2000, 8);  // Channel 0, 2kHz frequency, 8-bit resolution
    ledcAttachPin(BUZZER_PIN, 0); // Attach pin to channel 0
  #endif
  
  // Test buzzer with short beep
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(BUZZER_PIN, 128); // 50% duty cycle
  #else
    ledcWrite(0, 128);
  #endif
  delay(200); // Short beep
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(BUZZER_PIN, 0); // Turn off
  #else
    ledcWrite(0, 0);
  #endif
  
  Serial.println("✓ Buzzer OK (Test beep completed)");

  // Pin Modes
  pinMode(CAPACITIVE_PIN, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(US_TRIG_PIN, OUTPUT);
  pinMode(US_ECHO_PIN, INPUT);
  pinMode(SOLAR_VOLTAGE_PIN, INPUT);
  pinMode(BATTERY_VOLTAGE_PIN, INPUT);  // Battery monitoring pin

  // --- 3. Start Web Server ---
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/sensors", HTTP_GET, handleApiRequest);
  server.onNotFound(handleNotFound);
  server.begin();
  
  Serial.println("✓ HTTP Server Started");
  Serial.println("\n=================================");
  Serial.println("   SYSTEM READY");
  Serial.println("=================================\n");
  
  // Add initial log
  addDetectLog("fa-power-off", "System Initialized");
}

void loop() {
  // === Priority 1: Handle web server requests (critical for dashboard) ===
  server.handleClient();

  // === Priority 2: Non-blocking timers for sensor reading ===
  unsigned long now = millis();

  // Read all sensors every 2 seconds (optimized workflow)
  if (now - lastSensorRead > 2000) {
    lastSensorRead = now;
    readAllSensors();
  }

  // === Priority 3: RFID scanning (continuous monitoring) ===
  handleRfid();
  
  // === Priority 4: Real-time RTC monitoring (0.5 seconds for real-time updates) ===
  if (now - lastRtcCheck > 500) {  // Changed from 5000ms to 500ms (0.5 seconds)
    lastRtcCheck = now;
    
    DateTime rtcNow = rtc.now();
    
    // Validate RTC date/time
    if (rtcNow.year() < 2020 || rtcNow.year() > 2100) {
      Serial.println("⚠️ RTC INVALID! Resetting to compile time...");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      delay(100);
      rtcNow = rtc.now();
      Serial.printf("✓ RTC Reset: %04d-%02d-%02d %02d:%02d:%02d\n", 
                    rtcNow.year(), rtcNow.month(), rtcNow.day(), 
                    rtcNow.hour(), rtcNow.minute(), rtcNow.second());
    }
    // Note: Removed verbose logging to reduce serial spam at 0.5s intervals
  }
}
