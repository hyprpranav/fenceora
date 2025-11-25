/*
 * FENCEORA - Simplified Smart Fence System
 * ESP32 Dev Board - Web Server Version
 * Version 1.0 - November 2025
 * 
 * FEATURES:
 * - WiFi Web Server (sends sensor data to frontend via API)
 * - RFID + Servo Access Control
 * - Ultrasonic Proximity Detection
 * - IR Sensor Detection
 * - Current Sensor (ACS712) Detection
 * - Capacitive Sensor Detection
 * - MPU6050 Tamper/Gyroscope Detection
 * - Voltage Monitoring
 * - LED Status Indicator
 * - Buzzer Alarms
 * 
 * NOTIFICATION LOGIC:
 * - ESP32 only sends sensor values to frontend
 * - Frontend monitors value changes and sends notifications
 * - Keeps ESP32 code simple and focused on sensor reading
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
const char* password = "00000000";        // Your WiFi password

// ========================================
// PIN DEFINITIONS - ESP32 Dev Board
// ========================================
#define SS_PIN 5              // RFID SDA
#define RST_PIN 27            // RFID RST
#define SERVO_PIN 15          // Servo Motor
#define TRIG_PIN 33           // Ultrasonic Trigger
#define ECHO_PIN 32           // Ultrasonic Echo
#define BUZZER_PIN 13         // Buzzer/Speaker
#define LED_PIN 2             // Built-in LED (or use GPIO 2)
#define VOLTAGE_PIN 34        // Voltage Sensor (ADC1_CH6)
#define IR_PIN 26             // IR Sensor
#define CURRENT_PIN 35        // ACS712 Current Sensor (ADC1_CH7)
#define CAPACITIVE_PIN 25     // Capacitive Sensor (NEW)

// ========================================
// CONFIGURATION
// ========================================
const int CRITICAL_DISTANCE_CM = 30; 
const int WARNING_DISTANCE_CM = 50;  
const long CRITICAL_ALARM_DURATION_MS = 3000; 
const float TAMPER_THRESHOLD = 5.0;  
byte authorizedUID[4] = {0xD3, 0x2E, 0xE4, 0x2C}; 

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
const int OPEN_ANGLE = 90;
const int CLOSED_ANGLE = 0;
bool isServoOpen = false; 
float baseAx, baseAy, baseAz; 
unsigned long alarmStartTime = 0;
bool isCriticalAlarmActive = false; 
unsigned long lastVoltageCheckTime = 0;
const long VOLTAGE_CHECK_INTERVAL = 30000; // 30 seconds

// Sensor Values (to send to frontend)
struct SensorData {
  float voltage = 0.0;
  int ultrasonic = 0;
  bool irDetected = false;
  bool currentDetected = false;
  bool capacitiveDetected = false;
  bool tamperDetected = false;
  float accelX = 0.0;
  float accelY = 0.0;
  float accelZ = 0.0;
  float gyroX = 0.0;
  float gyroY = 0.0;
  float gyroZ = 0.0;
  bool servoOpen = false;
  String lastRFID = "None";
} sensorData;

// Capacitive Sensor Debouncing
int capacitiveState = LOW;
int lastCapReading = LOW;
unsigned long lastCapDebounceTime = 0;
const unsigned long CAP_DEBOUNCE_DELAY = 50;

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
void checkIRSensor();
void checkCurrentSensor();
void checkCapacitiveSensor();
void handleRFID();
void updateCriticalAlarmState();
void triggerBuzzer(int durationMs);
void triggerDoubleBeep();
void readInputVoltage();
void blinkLED(int times);

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
  pinMode(IR_PIN, INPUT); 
  pinMode(CURRENT_PIN, INPUT);
  pinMode(CAPACITIVE_PIN, INPUT);
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
  servo.write(CLOSED_ANGLE); 
  
  // Init I2C & MPU6050
  Wire.begin(21, 22); 
  if (!mpu.begin(0x69)) { // Using address 0x69 (AD0=HIGH)
    Serial.println("⚠️ Failed to find MPU6050 at 0x69");
  } else {
    Serial.println("✅ MPU6050 Found at 0x69");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    calibrateMPU();
  }
  
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
  
  // Read all sensors
  checkTamper();      
  checkProximity(); 
  checkIRSensor(); 
  checkCurrentSensor();
  checkCapacitiveSensor(); // NEW
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
  doc["irDetected"] = sensorData.irDetected;
  doc["currentDetected"] = sensorData.currentDetected;
  doc["capacitiveDetected"] = sensorData.capacitiveDetected;
  doc["tamperDetected"] = sensorData.tamperDetected;
  doc["servoOpen"] = sensorData.servoOpen;
  doc["lastRFID"] = sensorData.lastRFID;
  
  // MPU6050 values
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
// CAPACITIVE SENSOR (NEW)
// ========================================
void checkCapacitiveSensor() {
  if (isCriticalAlarmActive) return;
  
  int currentReading = digitalRead(CAPACITIVE_PIN);
  
  if (currentReading != lastCapReading) {
    lastCapDebounceTime = millis();
  }
  
  if ((millis() - lastCapDebounceTime) > CAP_DEBOUNCE_DELAY) {
    if (currentReading != capacitiveState) {
      capacitiveState = currentReading;
      
      if (capacitiveState == HIGH) {
        Serial.println("⚡ CAPACITIVE SENSOR DETECTED!");
        sensorData.capacitiveDetected = true;
        
        // Trigger alarm
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
        alarmStartTime = millis();
        isCriticalAlarmActive = true;
      } else {
        sensorData.capacitiveDetected = false;
      }
    }
  }
  
  lastCapReading = currentReading;
}

// ========================================
// CURRENT SENSOR
// ========================================
void calibrateCurrentSensor() {
  Serial.print("Calibrating Current Sensor... ");
  long total = 0;
  for(int i=0; i<100; i++) {
    total += analogRead(CURRENT_PIN);
    delay(5);
  }
  currentZeroPoint = total / 100;
  Serial.print("Zero Point: ");
  Serial.println(currentZeroPoint);
}

void checkCurrentSensor() {
  if (isCriticalAlarmActive) return;
  
  long reading = 0;
  for(int i=0; i<10; i++) {
    reading += analogRead(CURRENT_PIN);
    delay(1);
  }
  int avgReading = reading / 10;
  int difference = abs(avgReading - currentZeroPoint);
  
  if (difference > CURRENT_NOISE_THRESHOLD) {
    Serial.print("⚡ HIGH CURRENT! Deviation: ");
    Serial.println(difference);
    
    sensorData.currentDetected = true;
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    alarmStartTime = millis();
    isCriticalAlarmActive = true;
  } else {
    sensorData.currentDetected = false;
  }
}

// ========================================
// IR SENSOR
// ========================================
void checkIRSensor() {
  if (isCriticalAlarmActive) return;
  
  int irValue = digitalRead(IR_PIN);
  
  if (irValue == LOW) {
    Serial.println("🚨 IR SENSOR TRIGGERED!");
    
    sensorData.irDetected = true;
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    alarmStartTime = millis();
    isCriticalAlarmActive = true;
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
  if (isCriticalAlarmActive) return; 
  
  digitalWrite(TRIG_PIN, LOW); 
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); 
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  int distance = duration * 0.034 / 2;
  
  sensorData.ultrasonic = distance;
  
  if (distance > 0 && distance < CRITICAL_DISTANCE_CM) {
    Serial.print("🚨 CRITICAL PROXIMITY! ");
    Serial.print(distance);
    Serial.println(" cm");
    
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    alarmStartTime = millis();
    isCriticalAlarmActive = true;
  } 
  else if (distance >= CRITICAL_DISTANCE_CM && distance < WARNING_DISTANCE_CM) {
    if (digitalRead(BUZZER_PIN) == LOW) {
      triggerDoubleBeep();
    }
  }
}

// ========================================
// MPU6050 TAMPER DETECTION
// ========================================
void checkTamper() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Update sensor data
  sensorData.accelX = a.acceleration.x;
  sensorData.accelY = a.acceleration.y;
  sensorData.accelZ = a.acceleration.z;
  sensorData.gyroX = g.gyro.x;
  sensorData.gyroY = g.gyro.y;
  sensorData.gyroZ = g.gyro.z;
  
  float delta = abs(a.acceleration.x - baseAx) + 
                abs(a.acceleration.y - baseAy) + 
                abs(a.acceleration.z - baseAz);
  
  if (delta > TAMPER_THRESHOLD) {
    Serial.println("❌ TAMPER ALERT!");
    sensorData.tamperDetected = true;
    triggerBuzzer(150); 
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  } else {
    sensorData.tamperDetected = false;
  }
}

// ========================================
// RFID & SERVO
// ========================================
void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  
  // Read UID
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString += String(rfid.uid.uidByte[i], HEX);
  }
  sensorData.lastRFID = uidString;
  
  if (memcmp(rfid.uid.uidByte, authorizedUID, 4) == 0) {
    isServoOpen = !isServoOpen;
    sensorData.servoOpen = isServoOpen;
    
    if (isServoOpen) {
      servo.write(OPEN_ANGLE);
      Serial.println("✅ Access Granted -> Servo OPEN");
      blinkLED(2);
    } else {
      servo.write(CLOSED_ANGLE);
      Serial.println("✅ Access Granted -> Servo CLOSED");
      blinkLED(1);
    }
    delay(1000); 
  } else {
    Serial.println("❌ Unauthorized Tag: " + uidString);
    triggerBuzzer(100);
    delay(100);
    triggerBuzzer(100);
  }
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ========================================
// HELPER FUNCTIONS
// ========================================
void updateCriticalAlarmState() {
  if (isCriticalAlarmActive && (millis() - alarmStartTime >= CRITICAL_ALARM_DURATION_MS)) {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    isCriticalAlarmActive = false;
    Serial.println("Alarm OFF");
  }
}

void triggerDoubleBeep() {
  if (isCriticalAlarmActive) return; 
  
  digitalWrite(BUZZER_PIN, HIGH); 
  digitalWrite(LED_PIN, HIGH);
  delay(150); 
  digitalWrite(BUZZER_PIN, LOW); 
  digitalWrite(LED_PIN, LOW);
  delay(100); 
  digitalWrite(BUZZER_PIN, HIGH); 
  digitalWrite(LED_PIN, HIGH);
  delay(150);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
}

void triggerBuzzer(int durationMs) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(durationMs);
  digitalWrite(BUZZER_PIN, LOW);
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
  Serial.println("Done");
}
