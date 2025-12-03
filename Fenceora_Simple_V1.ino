/*
 * FENCEORA - Simplified Smart Fence System
 * ESP32 Dev Board - Web Server Version
 * Version 2.0 - November 2025
 * 
 * FEATURES:
 * - WiFi Web Server (sends sensor data to frontend via API)
 * - RFID + Servo Access Control (NO NOTIFICATIONS FOR SERVO)
 * - Ultrasonic Proximity Detection
 * - IR Sensor Detection
 * - Current Sensor (ACS712) Detection
 * - MPU6050 Gyroscope + Accelerometer + Tamper Detection
 * - Voltage Monitoring
 * - LED Status Indicator
 * - Buzzer Alarms
 * 
 * NOTIFICATION LOGIC:
 * - ESP32 sends all sensor values to frontend via API
 * - Frontend monitors value changes and sends notifications
 * - RFID/Servo movements are tracked but NO notifications sent
 * - Only security sensors (IR, Current, Tamper, Proximity) trigger notifications
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h> 
#include <Wire.h>               
#include <Adafruit_MPU6050.h> 
#include <Adafruit_Sensor.h>

// ========================================
// WIFI CONFIGURATION
// ========================================
const char* ssid = "Oppo A77s";           // Your WiFi name
const char* password = "9080061674";      // Your WiFi password

// ========================================
// PIN DEFINITIONS - ESP32 Dev Board (UPDATED)
// ========================================
// RFID PINS (SPI uses default pins: MISO=19, MOSI=23, SCK=18)
#define RST_PIN 22            // RFID Reset Pin
#define SS_PIN 21             // RFID Slave Select (SDA)

// SERVO & BUZZER
#define SERVO_PIN 13          // Servo Motor Signal
#define BUZZER_PIN 27         // Buzzer

// ULTRASONIC
#define TRIG_PIN 12           // Ultrasonic Trigger
#define ECHO_PIN 14           // Ultrasonic Echo

// OTHER SENSORS
#define CAPACITIVE_PIN 15     // Capacitive Touch Sensor (KEY INPUT)
#define LED_PIN 2             // Built-in LED
#define VOLTAGE_PIN 34        // Voltage Sensor (ADC1_CH6)
#define IR_PIN 4              // IR Sensor
#define CURRENT_PIN 35        // ACS712 Current Sensor (ADC1_CH7)

// MPU6050 I2C PINS (Changed from 21/22 to avoid RFID conflict)
#define MPU_SDA 26            // I2C SDA for MPU6050
#define MPU_SCL 25            // I2C SCL for MPU6050

// ========================================
// CONFIGURATION
// ========================================
const int CRITICAL_DISTANCE_CM = 30; 
const long CRITICAL_ALARM_DURATION_MS = 3000; 
const float TAMPER_THRESHOLD = 5.0;  

// RFID & SERVO CONFIG
const int LOCKED_POS = 0;
const int UNLOCKED_POS = 90;
const byte AUTHORIZED_UID[4] = {0xD3, 0x2E, 0xE4, 0x2C}; 

// Voltage & Current Calibration
const float VOLTAGE_CALIBRATION = 0.0041; 
int currentZeroPoint = 0;
const int CURRENT_NOISE_THRESHOLD = 100;

// ========================================
// OBJECTS
// ========================================
WebServer server(80);
MFRC522 rfid(SS_PIN, RST_PIN);
Servo servo;
Adafruit_MPU6050 mpu; 

// ========================================
// STATE VARIABLES
// ========================================
bool isLocked = true;         // True if servo is at LOCKED_POS
float baseAx, baseAy, baseAz; 
unsigned long alarmStartTime = 0;
bool isCriticalAlarmActive = false; 
unsigned long lastVoltageCheckTime = 0;
const long VOLTAGE_CHECK_INTERVAL = 30000; // 30 seconds

// Sensor Values (to send to frontend)
struct SensorData {
  float voltage = 0.0;
  int ultrasonic = 0;
  bool capacitiveDetected = false;
  bool irDetected = false;
  bool currentDetected = false;
  bool tamperDetected = false;
  float accelX = 0.0;
  float accelY = 0.0;
  float accelZ = 0.0;
  float gyroX = 0.0;
  float gyroY = 0.0;
  float gyroZ = 0.0;
  bool servoLocked = true;    // Servo position (for tracking only, no notifications)
  String lastRFID = "None";
} sensorData;

// ========================================
// FUNCTION PROTOTYPES
// ========================================
void setupWiFi();
void setupServer();
void handleRoot();
void handleSensorData();
void handleCORS();
void calibrateMPU();
void calibrateCurrentSensor();
void checkTamper();
void checkProximity();
void checkCapacitiveSensor();
void checkIRSensor();
void checkCurrentSensor();
void handleRFID();
void updateCriticalAlarmState();
void triggerCriticalAlarm(const char* message);
void readInputVoltage();
void blinkLED(int times);
void dump_byte_array(byte *buffer, byte bufferSize);
bool compareUID(byte *uid1, const byte *uid2, byte size);

// ========================================
// SETUP
// ========================================
void setup() {
  Serial.begin(115200); 
  Serial.println("\n\n=== FENCEORA Simple System Starting ===");
  
  // Init Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CAPACITIVE_PIN, INPUT);
  pinMode(IR_PIN, INPUT); 
  pinMode(CURRENT_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  // Init ADC
  analogReadResolution(12); 
  
  // Init SPI & RFID
  SPI.begin(18, 19, 23); 
  rfid.PCD_Init();   
  
  // Init Servo
  ESP32PWM::allocateTimer(0);
  servo.attach(SERVO_PIN);
  servo.write(LOCKED_POS); 
  
  // Init I2C & MPU6050 (Using new pins to avoid RFID conflict)
  Wire.begin(MPU_SDA, MPU_SCL); 
  if (!mpu.begin()) {
    Serial.println("⚠️ Failed to find MPU6050");
  } else {
    Serial.println("✅ MPU6050 Found!");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    calibrateMPU();
  }
  
  // Init RFID (with antenna boost)
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  Serial.print("RFID Firmware Version: ");
  rfid.PCD_DumpVersionToSerial();
  
  // Calibrate Current Sensor
  calibrateCurrentSensor();
  
  // Connect WiFi
  setupWiFi();
  
  // Setup Web Server
  setupServer();
  
  lastVoltageCheckTime = millis();
  blinkLED(3); // Success indicator
  
  Serial.println("=== System Ready ===");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ========================================
// MAIN LOOP
// ========================================
void loop() {
  server.handleClient(); // Handle web requests
  
  updateCriticalAlarmState(); 
  
  // Check voltage every 30 seconds
  if (millis() - lastVoltageCheckTime >= VOLTAGE_CHECK_INTERVAL) {
    readInputVoltage(); 
    lastVoltageCheckTime = millis(); 
  }
  
  // Read security sensors (only when alarm is not active)
  if (!isCriticalAlarmActive) {
    checkTamper();      
    checkProximity();
    checkCapacitiveSensor();
    checkIRSensor(); 
    checkCurrentSensor();
  }
  
  // RFID access control (always active)
  handleRFID();       
  
  delay(50);
}

// ========================================
// WIFI SETUP
// ========================================
void setupWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi Connection Failed!");
  }
}

// ========================================
// WEB SERVER SETUP
// ========================================
void setupServer() {
  // CORS preflight
  server.on("/api/sensors", HTTP_OPTIONS, handleCORS);
  
  // API endpoint for sensor data
  server.on("/api/sensors", HTTP_GET, handleSensorData);
  
  // Root endpoint
  server.on("/", HTTP_GET, handleRoot);
  
  server.begin();
  Serial.println("✅ Web Server Started");
}

void handleCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200, "text/plain", "");
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>FENCEORA System</h1>";
  html += "<p>Status: Online</p>";
  html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
  html += "<p>API Endpoint: <a href='/api/sensors'>/api/sensors</a></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleSensorData() {
  // Create JSON document
  StaticJsonDocument<1024> doc;
  
  // Add sensor values
  doc["voltage"] = sensorData.voltage;
  doc["ultrasonic"] = sensorData.ultrasonic;
  doc["capacitiveDetected"] = sensorData.capacitiveDetected;
  doc["irDetected"] = sensorData.irDetected;
  doc["currentDetected"] = sensorData.currentDetected;
  doc["tamperDetected"] = sensorData.tamperDetected;
  doc["servoLocked"] = sensorData.servoLocked;  // Track servo position (no notifications)
  doc["lastRFID"] = sensorData.lastRFID;
  
  // MPU6050 values (GYROSCOPE + ACCELEROMETER)
  JsonObject mpu = doc.createNestedObject("mpu");
  mpu["accelX"] = sensorData.accelX;
  mpu["accelY"] = sensorData.accelY;
  mpu["accelZ"] = sensorData.accelZ;
  mpu["gyroX"] = sensorData.gyroX;
  mpu["gyroY"] = sensorData.gyroY;
  mpu["gyroZ"] = sensorData.gyroZ;
  
  // Status
  doc["status"] = "online";
  doc["timestamp"] = millis();
  
  // Send response
  String response;
  serializeJson(doc, response);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

// ========================================
// ALARM TRIGGER FUNCTION
// ========================================
void triggerCriticalAlarm(const char* message) {
  if (isCriticalAlarmActive) return; // Don't re-trigger
  
  Serial.print("🚨 CRITICAL ALERT: ");
  Serial.println(message);
  
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  alarmStartTime = millis();
  isCriticalAlarmActive = true;
}

// ========================================
// CURRENT SENSOR
// ========================================
void calibrateCurrentSensor() {
  Serial.print("Calibrating Current Sensor (Ensure no load)... ");
  long total = 0;
  for(int i=0; i<100; i++) {
    total += analogRead(CURRENT_PIN);
    delay(5);
  }
  currentZeroPoint = total / 100;
  Serial.print("Zero Point Set at: ");
  Serial.println(currentZeroPoint);
}

void checkCurrentSensor() {
  long reading = 0;
  for(int i=0; i<10; i++) {
    reading += analogRead(CURRENT_PIN);
    delay(1);
  }
  int avgReading = reading / 10;
  int difference = abs(avgReading - currentZeroPoint);
  
  if (difference > CURRENT_NOISE_THRESHOLD) {
    sensorData.currentDetected = true;
    triggerCriticalAlarm("High Current Detected!");
  } else {
    sensorData.currentDetected = false;
  }
}

// ========================================
// CAPACITIVE TOUCH SENSOR
// ========================================
void checkCapacitiveSensor() {
  int capacitiveValue = digitalRead(CAPACITIVE_PIN);
  
  if (capacitiveValue == HIGH) {
    sensorData.capacitiveDetected = true;
    triggerCriticalAlarm("Capacitive Sensor Triggered!");
  } else {
    sensorData.capacitiveDetected = false;
  }
}

// ========================================
// IR SENSOR
// ========================================
void checkIRSensor() {
  int irValue = digitalRead(IR_PIN);
  
  if (irValue == LOW) {
    sensorData.irDetected = true;
    triggerCriticalAlarm("IR Sensor Object Detected!");
  } else {
    sensorData.irDetected = false;
  }
}

// ========================================
// VOLTAGE SENSOR
// ========================================
void readInputVoltage() {
  int rawValue = analogRead(VOLTAGE_PIN);
  sensorData.voltage = (float)rawValue * VOLTAGE_CALIBRATION;
  
  Serial.print("Input Voltage: "); 
  Serial.print(sensorData.voltage, 2); 
  Serial.println(" V");
}

// ========================================
// ULTRASONIC SENSOR
// ========================================
void checkProximity() {
  digitalWrite(TRIG_PIN, LOW); 
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); 
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;
  
  sensorData.ultrasonic = distance;
  
  if (distance > 0 && distance < CRITICAL_DISTANCE_CM) {
    char msg[32];
    snprintf(msg, sizeof(msg), "Proximity at %d cm!", distance);
    triggerCriticalAlarm(msg);
  }
}

// ========================================
// MPU6050 TAMPER DETECTION (GYROSCOPE + ACCELEROMETER)
// ========================================
void checkTamper() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Update sensor data (GYROSCOPE + ACCELEROMETER)
  sensorData.accelX = a.acceleration.x;
  sensorData.accelY = a.acceleration.y;
  sensorData.accelZ = a.acceleration.z;
  sensorData.gyroX = g.gyro.x;
  sensorData.gyroY = g.gyro.y;
  sensorData.gyroZ = g.gyro.z;
  
  // Check for significant movement (tamper detection)
  float delta = abs(a.acceleration.x - baseAx) + 
                abs(a.acceleration.y - baseAy) + 
                abs(a.acceleration.z - baseAz);
  
  if (delta > TAMPER_THRESHOLD) {
    sensorData.tamperDetected = true;
    triggerCriticalAlarm("Tamper Detected! Device is moving!");
  } else {
    sensorData.tamperDetected = false;
  }
}

// ========================================
// RFID & SERVO (NO NOTIFICATIONS, ONLY TRACKING)
// ========================================
void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  // Read the current tag's UID
  byte currentUID[4];
  for (byte i = 0; i < 4; i++) {
    currentUID[i] = rfid.uid.uidByte[i];
  }
  
  // Convert UID to string for tracking
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString += String(rfid.uid.uidByte[i], HEX);
  }
  sensorData.lastRFID = uidString;
  
  Serial.print("Scanned UID: ");
  dump_byte_array(currentUID, 4);

  // Check for Authorization
  if (compareUID(currentUID, AUTHORIZED_UID, 4)) {
    Serial.println("✅ Access Granted: Authorized Tag!");
    
    // Toggle the lock state
    if (isLocked) {
      Serial.println("   --> UNLOCKING servo...");
      servo.write(UNLOCKED_POS); 
      isLocked = false;
      sensorData.servoLocked = false;
      blinkLED(2);
    } else {
      Serial.println("   --> LOCKING servo...");
      servo.write(LOCKED_POS); 
      isLocked = true;
      sensorData.servoLocked = true;
      blinkLED(1);
    }
  } else {
    Serial.println("❌ Access Denied: Unauthorized Tag!");
    // Quick beep for denied access
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Cleanup
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(500); 
}

// Helper function to print a byte array (UID)
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  Serial.println();
}

// Helper function to compare two byte arrays (UIDs)
bool compareUID(byte *uid1, const byte *uid2, byte size) {
  for (byte i = 0; i < size; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

// ========================================
// HELPER FUNCTIONS
// ========================================
void updateCriticalAlarmState() {
  // Turns off the buzzer after the duration if it was active
  if (isCriticalAlarmActive && (millis() - alarmStartTime >= CRITICAL_ALARM_DURATION_MS)) {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    isCriticalAlarmActive = false;
    Serial.println("Alarm OFF: Cycle finished.");
  }
}

void blinkLED(int times) {
  for(int i=0; i<times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

void calibrateMPU() {
  Serial.print("Calibrating MPU... ");
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  baseAx = a.acceleration.x;
  baseAy = a.acceleration.y;
  baseAz = a.acceleration.z;
  Serial.println(" Done.");
}
