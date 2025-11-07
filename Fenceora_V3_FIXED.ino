/*
 * FENCEORA - Smart Electric Fence Node System
 * ESP32 Main Controller Code (Version 3.1 - FIXED FOR DASHBOARD)
 * Project by H^4 HYPERS
 *
 * --- VERSION 3.1 FIXES ---
 * 1. Fixed JSON structure to match dashboard expectations
 * 2. Fixed temperature object structure (added .value property)
 * 3. Fixed CORS headers on all responses
 * 4. Fixed uptime calculation timing issue
 * 5. Added proper error handling and console logging
 * 6. Fixed null log array handling
 */

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
const unsigned long EMAIL_COOLDOWN = 30000; // 30 seconds between emails (prevent spam)

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
#define SOLAR_VOLTAGE_PIN 34 // GPIO 34 (ADC1_CH6) - Voltage Sensor S pin
// WIRING for Voltage Sensor (3 pins: VCC, GND, S):
// Voltage Sensor VCC → ESP32 5V (or 3.3V depending on module)
// Voltage Sensor GND → ESP32 GND
// Voltage Sensor S   → ESP32 GPIO 34 (Analog Input - ADC1 Channel 6)

// Actuators
#define SERVO_PIN 15  // Changed to GPIO 15 (standard servo pin)
#define BUZZER_PIN 12

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
float batteryLevel = 85.0;
String rtcTime = "2025-11-04T07:00:00";
unsigned long startTimeSeconds = 0;

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
  battery["level"] = 0;  // Set to 0 until battery monitoring module is added
  
  JsonObject solar = doc.createNestedObject("solar");
  solar["voltage"] = solarVoltage;  // This shows in "Active Supply" on dashboard
  solar["current"] = solarCurrent;
  solar["power"] = solarVoltage * solarCurrent;

  // --- 5. Other Sensor Data ---
  JsonObject tamper = doc.createNestedObject("tamper");
  tamper["motion"] = tamperMotion;
  tamper["ir"] = tamperIR;
  doc["animal_dist"] = animalDistance;
  
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
// ===           SENSOR READING FUNCTIONS                      ===
// =================================================================

void readAllSensors() {
  // --- 1. Capacitive Sensor (Fence Electric Field Detection) with DEBOUNCING ---
  // IMPORTANT: This sensor detects ELECTRIC FIELD (Voltage), NOT current!
  // Adjust the potentiometer screw on the sensor (turn COUNTER-CLOCKWISE to decrease sensitivity)
  // Goal: Sensor should NOT detect your hand, but SHOULD detect the live fence wire
  
  int currentReading = digitalRead(CAPACITIVE_PIN);
  
  // Check if reading changed (start of flicker)
  if (currentReading != lastCapacitiveReading) {
    lastCapacitiveDebounceTime = millis();
  }
  
  // Wait for debounce delay to pass (50ms)
  if ((millis() - lastCapacitiveDebounceTime) > capacitiveDebounceDelay) {
    // Check if this is a STABLE change
    if (currentReading != capacitiveSensorState) {
      capacitiveSensorState = currentReading;
      
      // Now check OTHER sensors to determine what triggered it
      bool handNearby = tamperIR;  // IR sensor detects hand
      bool deviceMoving = tamperMotion;  // MPU detects movement
      
      if (capacitiveSensorState == HIGH) {
        // Something triggered the capacitive sensor
        if (deviceMoving) {
          addDetectLog("fa-exclamation-triangle", "⚠️ TAMPER - DEVICE IS BEING MOVED!");
        } else if (handNearby) {
          addDetectLog("fa-hand-paper", "⚠️ TAMPER - HAND DETECTED NEAR DEVICE!");
        } else {
          // No hand, no movement = must be fence
          fenceActive = true;
          addDetectLog("fa-bolt", "⚡ FENCE ELECTRIC FIELD DETECTED!");
          
          // Send email notification for fence current
          char voltageStr[20];
          sprintf(voltageStr, "%.2fV", solarVoltage);
          sendEmailNotification(
            "Fence Current Detected",
            "Electric field detected on perimeter fence!",
            String("Voltage: ") + voltageStr + " | Sensor: Capacitive"
          );
        }
      } else {
        // Sensor went LOW - field cleared
        fenceActive = false;
        addDetectLog("fa-bolt", "Fence Electric Field CLEAR");
      }
    }
  }
  
  lastCapacitiveReading = currentReading;

  // --- 2. IR Sensor (Tamper Detection) ---
  bool currentIRState = digitalRead(IR_SENSOR_PIN) == LOW;
  if (currentIRState != tamperIR) {
    tamperIR = currentIRState;
    if (tamperIR) {
      addDetectLog("fa-hand-paper", "⚠️ ALERT: Someone tried to hold/tamper with product!");
      
      // Send email notification for tampering
      sendEmailNotification(
        "Tampering Detected",
        "Hand or object detected near device!",
        "Sensor: Infrared (IR) | Someone is trying to tamper with the fence system"
      );
    }
  }

  // --- 3. DHT11/DHT22 (Temperature & Humidity) ---
  float newTemp = dht.readTemperature();    // Read temperature in Celsius
  float newHumid = dht.readHumidity();      // Read humidity percentage
  
  if (!isnan(newTemp) && !isnan(newHumid)) {
    temperature = newTemp;
    humidity = newHumid;
    // Optional: Serial debugging
    // Serial.printf("DHT: Temp=%.1f°C, Humidity=%.1f%%\n", temperature, humidity);
  } else {
    Serial.println("⚠️ Failed to read from DHT sensor!");
    // Keep previous values instead of crashing
  }

  // --- 4. MPU-6050 (Tamper) ---
  sensors_event_t a, g, mpu_temp;
  if (mpu.getEvent(&a, &g, &mpu_temp)) {
    // Simple motion check: if acceleration > 15 m/s²
    if (abs(a.acceleration.x) > 15 || abs(a.acceleration.y) > 15 || abs(a.acceleration.z) > 15) {
      if (!tamperMotion) {
        addDetectLog("fa-exclamation-triangle", "MOTION TAMPER DETECTED!");
        
        // Send email notification for device movement
        char accelStr[50];
        sprintf(accelStr, "X:%.1f Y:%.1f Z:%.1f m/s²", 
                a.acceleration.x, a.acceleration.y, a.acceleration.z);
        sendEmailNotification(
          "Device Movement Detected",
          "Someone is trying to move the fence system device!",
          String("Sensor: MPU6050 Accelerometer | ") + accelStr
        );
      }
      tamperMotion = true;
    } else {
      tamperMotion = false;
    }
  }

  // --- 5. Ultrasonic (Animal Detection) ---
  digitalWrite(US_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG_PIN, LOW);
  long duration = pulseIn(US_ECHO_PIN, HIGH, 50000);
  float newDistance = duration * 0.034 / 2;
  
  // Update distance and trigger buzzer if animal detected
  if (newDistance > 1 && newDistance < 400) { // Valid range: 1cm to 400cm
    animalDistance = newDistance;
    
    // If animal is within 200cm, log it and activate buzzer
    if (animalDistance < 200) {
      static unsigned long lastAnimalLog = 0;
      // Only log every 5 seconds to avoid spam
      if (millis() - lastAnimalLog > 5000) {
        addDetectLog("fa-paw", "🐾 Animal detected at " + String((int)animalDistance) + "cm distance!");
        lastAnimalLog = millis();
      }
      
      // Activate buzzer to scare animal
      ledcWrite(BUZZER_PIN, 128); // 50% duty cycle (0-255)
    } else {
      // Animal far away, turn off buzzer
      ledcWrite(BUZZER_PIN, 0);
    }
  } else {
    // No valid reading
    animalDistance = 999.0;
    ledcWrite(BUZZER_PIN, 0); // Turn off buzzer
  }
  
  // --- 6. Solar Voltage Sensor (3-pin: VCC, GND, S) ---
  int adcVal = analogRead(SOLAR_VOLTAGE_PIN);
  
  // Convert ADC reading to actual voltage
  // ESP32 ADC: 0-4095 = 0-3.3V
  float sensorVoltage = (adcVal / 4095.0) * 3.3;
  
  // If your voltage sensor has a voltage divider (e.g., 25V module with 5:1 ratio)
  // multiply by the divider ratio. Common voltage sensors:
  // - 0-25V module: multiply by ~7.5
  // - 0-15V module: multiply by ~5
  // - Direct connection: multiply by 1
  solarVoltage = sensorVoltage * 7.5; // Adjust this multiplier for your specific sensor
  
  // Debug output (optional - uncomment to see values)
  Serial.printf("ADC: %d, Sensor V: %.2fV, Solar Panel V: %.2fV\n", adcVal, sensorVoltage, solarVoltage);
  
  // Estimate solar current based on voltage (replace with actual current sensor if available)
  solarCurrent = solarVoltage > 5.0 ? (solarVoltage / 12.0) * 0.5 : 0.05;
  
  // Battery level set to 0% (no battery monitoring module yet)
  // When you add a battery monitoring module (like INA219 or voltage divider),
  // you can read the battery voltage separately and calculate percentage here
  batteryLevel = 0;
}

// =================================================================
// ===           RFID & SERVO LOGIC                          ===
// =================================================================

void handleRfid() {
  // Look for new RFID card
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }
  
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Print card UID for debugging
  Serial.print("Card Scanned - UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Compare scanned UID with authorized UID using memcmp
  if (memcmp(rfid.uid.uidByte, authorizedUID, 4) == 0) {
    // Authorized card detected - Toggle servo
    if (!isServoOpen) {
      // Open servo
      Serial.println("✓ Authorized Card - Opening Servo to 90°");
      addRfidLog("fa-unlock", "✓ Authorized Card - UNLOCKING...");
      for (int pos = 0; pos <= 90; pos += 5) {
        lockServo.write(pos);
        delay(15); // Smooth movement
      }
      isServoOpen = true;
      Serial.println("Servo is now OPEN (90°)");
    } else {
      // Close servo
      Serial.println("✓ Authorized Card - Closing Servo to 0°");
      addRfidLog("fa-lock", "✓ Authorized Card - LOCKING...");
      for (int pos = 90; pos >= 0; pos -= 5) {
        lockServo.write(pos);
        delay(15); // Smooth movement
      }
      isServoOpen = false;
      Serial.println("Servo is now CLOSED (0°)");
    }
  } else {
    // Unauthorized card
    Serial.print("✗ Unauthorized Card with UID:");
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();
    Serial.println("Update authorizedUID[] in code with correct UID!");
    addRfidLog("fa-times-circle", "✗ UNAUTHORIZED Card! Access DENIED.");
    
    // Send email notification for unauthorized access
    char uidStr[20];
    sprintf(uidStr, "%02X %02X %02X %02X", 
            rfid.uid.uidByte[0], rfid.uid.uidByte[1], 
            rfid.uid.uidByte[2], rfid.uid.uidByte[3]);
    sendEmailNotification(
      "Unauthorized RFID Access Attempt",
      "Unknown RFID card was scanned at the fence lock!",
      String("Card UID: ") + uidStr + " | Access DENIED"
    );
  }

  // Halt PICC and stop encryption
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// =================================================================
// ===           EMAIL NOTIFICATION SYSTEM                     ===
// =================================================================

/**
 * Sends email notification to all configured receivers
 * @param alertTitle - Main alert title (e.g., "Fence Current Detected")
 * @param alertMessage - Alert message body
 * @param alertDetails - Additional details (voltage, sensor, etc.)
 */
void sendEmailNotification(String alertTitle, String alertMessage, String alertDetails) {
  // Check cooldown period to prevent email spam
  unsigned long currentTime = millis();
  if (currentTime - lastEmailTime < EMAIL_COOLDOWN) {
    Serial.println("⏳ Email cooldown active. Skipping email.");
    return;
  }
  
  // Check if password is configured
  if (String(SENDER_PASSWORD) == "YOUR_GMAIL_APP_PASSWORD") {
    Serial.println("⚠️ Email notifications not configured!");
    Serial.println("📧 Please set SENDER_PASSWORD in code (see instructions in comments)");
    return;
  }
  
  Serial.println("\n📧 Sending email notification...");
  
  // Configure SMTP session
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = SENDER_EMAIL;
  session.login.password = SENDER_PASSWORD;
  session.login.user_domain = "";
  
  // Create email message
  SMTP_Message message;
  message.sender.name = SENDER_NAME;
  message.sender.email = SENDER_EMAIL;
  message.subject = "🚨 FENCEORA ALERT: " + alertTitle;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
  
  // Add all receivers
  for (int i = 0; i < numReceivers; i++) {
    message.addRecipient("Receiver", receiverEmails[i]);
    Serial.printf("   → Adding receiver: %s\n", receiverEmails[i]);
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
  htmlMsg += "<h1 style='margin: 0; font-size: 36px;'>⚠️ CRITICAL ALERT</h1>";
  htmlMsg += "</div>";
  htmlMsg += "<div style='padding: 20px; background: #f5f5f5;'>";
  htmlMsg += "<h2 style='color: #ff4757;'>" + alertTitle + "</h2>";
  htmlMsg += "<p style='font-size: 18px; color: #333;'><strong>Message:</strong> " + alertMessage + "</p>";
  htmlMsg += "<p style='font-size: 16px; color: #666;'><strong>Details:</strong> " + alertDetails + "</p>";
  htmlMsg += "<p style='font-size: 14px; color: #999;'><strong>Timestamp:</strong> " + String(timestamp) + "</p>";
  htmlMsg += "<hr style='border: 1px solid #ddd; margin: 20px 0;'>";
  htmlMsg += "<p style='font-size: 14px; color: #666;'>Please check your Fenceora dashboard immediately.</p>";
  htmlMsg += "<p style='font-size: 12px; color: #999;'>This is an automated alert from your Fenceora Smart Fence System.</p>";
  htmlMsg += "</div></body></html>";
  
  message.html.content = htmlMsg.c_str();
  message.html.charSet = "utf-8";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  // Connect to SMTP server and send
  if (!smtp.connect(&session)) {
    Serial.println("❌ SMTP connection failed!");
    Serial.println("Error: " + smtp.errorReason());
    return;
  }
  
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("❌ Email send failed!");
    Serial.println("Error: " + smtp.errorReason());
  } else {
    Serial.println("✅ Email sent successfully to all receivers!");
    lastEmailTime = currentTime;
  }
  
  // Close connection
  smtp.closeSession();
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
  
  // RTC
  if (!rtc.begin()) {
    Serial.println("✗ RTC Failed!");
    addDetectLog("fa-exclamation-triangle", "RTC FAILED TO INIT");
  } else {
    Serial.println("✓ RTC OK");
    // Uncomment to set time: rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    DateTime now = rtc.now();
    startTimeSeconds = now.unixtime(); // Store start time AFTER RTC init
  }
  
  // MPU-6050
  if (!mpu.begin(0x69)) {
    Serial.println("✗ MPU6050 Failed! (Check AD0 pin = HIGH for 0x69)");
    addDetectLog("fa-exclamation-triangle", "MPU6050 FAILED");
  } else {
    Serial.println("✓ MPU6050 OK");
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

  // Servo (Allow allocation of all timers for ESP32)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  lockServo.setPeriodHertz(50);    // Standard 50 Hz servo
  lockServo.attach(SERVO_PIN, 500, 2400); // Pin, min pulse width, max pulse width (microseconds)
  lockServo.write(0); // Start in closed position
  isServoOpen = false;
  Serial.println("✓ Servo OK (Initialized at 0° - Locked)");

  // Buzzer (ESP32 v3.x PWM - new API)
  ledcAttach(BUZZER_PIN, 5000, 8); // Pin, 5kHz frequency, 8-bit resolution
  Serial.println("✓ Buzzer OK");

  // Pin Modes
  pinMode(CAPACITIVE_PIN, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(US_TRIG_PIN, OUTPUT);
  pinMode(US_ECHO_PIN, INPUT);
  pinMode(SOLAR_VOLTAGE_PIN, INPUT);

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
  // --- 1. Handle web server ---
  server.handleClient();

  // --- 2. Non-blocking timers ---
  unsigned long now = millis();

  // Task 1: Read sensors every 2 seconds
  if (now - lastSensorRead > 2000) {
    lastSensorRead = now;
    readAllSensors();
  }

  // Task 2: Check RFID continuously (no delay needed)
  handleRfid();
}
