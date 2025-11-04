# FENCEORA - ESP32 Setup Guide

## 📋 Table of Contents
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Arduino IDE Setup](#arduino-ide-setup)
- [Library Installation](#library-installation)
- [ESP32 Code Upload](#esp32-code-upload)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)

---

## 🔌 Hardware Requirements

### Main Components
- **ESP32 Development Board** (38-pin version recommended)
- **DS3231 RTC Module** (I2C)
- **MPU-6050 Accelerometer/Gyroscope** (I2C, with AD0 pin connected to VCC for address 0x69)
- **AHT21 Temperature & Humidity Sensor** (I2C)
- **MFRC522 RFID Reader** (SPI)
- **HC-SR04 Ultrasonic Sensor**
- **IR Sensor Module**
- **Capacitive Proximity Sensor** (PNP type with voltage divider)
- **SG90 Servo Motor**
- **Passive Buzzer**
- **Solar Panel** (with voltage divider for monitoring)
- **Battery** (LiPo or equivalent)

### Additional Components
- Resistors for voltage dividers (100kΩ + 10kΩ for solar voltage)
- Jumper wires
- Breadboard or PCB
- Power supply (5V for ESP32, appropriate voltage for other components)

---

## 🔧 Wiring Diagram

### I2C Bus (Shared)
| Component | SDA | SCL | Address |
|-----------|-----|-----|---------|
| DS3231 RTC | GPIO 21 | GPIO 22 | 0x68 |
| MPU-6050 | GPIO 21 | GPIO 22 | 0x69 (AD0=HIGH) |
| AHT21 | GPIO 21 | GPIO 22 | 0x38 |

**Important:** Connect MPU-6050's AD0 pin to VCC (3.3V) to set address to 0x69.

### SPI Bus (RFID)
| RFID Pin | ESP32 Pin |
|----------|-----------|
| SDA/SS | GPIO 4 |
| SCK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| RST | GPIO 5 |
| 3.3V | 3.3V |
| GND | GND |

### Digital Sensors
| Sensor | ESP32 Pin | Notes |
|--------|-----------|-------|
| Capacitive Sensor | GPIO 25 | Use voltage divider if output > 3.3V |
| IR Sensor | GPIO 26 | With internal pullup |
| Ultrasonic TRIG | GPIO 33 | |
| Ultrasonic ECHO | GPIO 32 | |

### Analog Sensors
| Sensor | ESP32 Pin | Notes |
|--------|-----------|-------|
| Solar Voltage | GPIO 36 (VP) | **Must use voltage divider!** Max 3.3V input |

**Solar Voltage Divider Example:**
```
Solar Panel (+) ----[100kΩ]---- GPIO 36 ----[10kΩ]---- GND
```
This gives you a 11:1 ratio (max ~36V measurable)

### Actuators
| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| Servo Motor | GPIO 13 | Connect to 5V power supply |
| Buzzer | GPIO 12 | Passive buzzer, uses PWM |

---

## 💻 Arduino IDE Setup

### 1. Install Arduino IDE
Download from: https://www.arduino.cc/en/software

### 2. Add ESP32 Board Support
1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "ESP32" and install "**esp32 by Espressif Systems**"

### 3. Select Board
- **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
- **Tools → Upload Speed → 115200**
- **Tools → Port → (Select your COM port)**

---

## 📚 Library Installation

### Required Libraries

Install these via **Sketch → Include Library → Manage Libraries**:

1. **AsyncTCP** by me-no-dev
2. **ESPAsyncWebServer** by me-no-dev
3. **ArduinoJson** by Benoit Blanchon (version 6.x)
4. **RTClib** by Adafruit
5. **Adafruit MPU6050** by Adafruit
6. **Adafruit Unified Sensor** by Adafruit
7. **Adafruit AHTX0** by Adafruit (for AHT21)
8. **MFRC522** by GithubCommunity
9. **ESP32Servo** by Kevin Harrington

### Manual Installation (if needed)

If AsyncTCP or ESPAsyncWebServer are not found:
1. Download from:
   - AsyncTCP: https://github.com/me-no-dev/AsyncTCP
   - ESPAsyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer
2. **Sketch → Include Library → Add .ZIP Library**

---

## 📤 ESP32 Code Upload

### 1. Get Your RFID Card UID

Before uploading the main code, run this sketch to find your card's UID:

```cpp
#include <SPI.h>
#include <MFRC522.h>

#define RFID_SDA 4
#define RFID_RST 5

MFRC522 rfid(RFID_SDA, RFID_RST);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Scan your RFID card...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }
  
  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print("0x");
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) Serial.print(", ");
  }
  Serial.println();
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(2000);
}
```

**Note the UID values** (e.g., `0xDE, 0xAD, 0xBE, 0xEF`)

### 2. Main ESP32 Code

Create a new sketch and paste this code:

```cpp
/*
 * FENCEORA - Smart Electric Fence Node System
 * ESP32 Main Controller Code (Version 2)
 * Project by H^4 HYPERS
 *
 * --- VERSION 2 UPDATES ---
 * 1. Added dedicated AHT21 sensor for accurate Temp/Humidity.
 * 2. Fixed CRITICAL blocking delay(3000) in RFID logic.
 * 3. Improved Buzzer logic to use non-blocking ledc PWM.
 * 4. Increased JSON buffer to 2048 for stability.
 */

// --- 1. WiFi & Web Server ---
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// --- 2. I2C Sensors ---
#include <Wire.h>
#include <RTClib.h>              // For RTC (DS3231)
#include <Adafruit_MPU6050.h>    // For MPU-6050
#include <Adafruit_Sensor.h>
#include <Adafruit_AHTX0.h>      // ** NEW: For Temp/Humidity (AHT21) **

// --- 3. SPI Sensors ---
#include <SPI.h>
#include <MFRC522.h>             // For RFID (RC522)

// --- 4. Other Actuators/Sensors ---
#include <ESP32Servo.h>          // For Servo

// === WiFi Credentials ===
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// === Pin Definitions ===
// I2C
#define I2C_SDA 21
#define I2C_SCL 22

// SPI (for RFID)
#define RFID_SDA 4  // SS/SDA pin
#define RFID_RST 5  // Reset pin
// MOSI = 23, MISO = 19, SCK = 18 (Default SPI)

// Digital Sensors
#define CAPACITIVE_PIN 25 // PNP Sensor (via voltage divider)
#define IR_SENSOR_PIN  26 // IR Sensor
#define US_TRIG_PIN    33 // Ultrasonic Trig
#define US_ECHO_PIN    32 // Ultrasonic Echo

// Analog Sensors
#define SOLAR_VOLTAGE_PIN 36 // GPIO 36 (VP)

// Actuators
#define SERVO_PIN 13
#define BUZZER_PIN 12

// === Global Objects ===
AsyncWebServer server(80);
RTC_DS3231 rtc;
Adafruit_MPU6050 mpu;
Adafruit_AHTX0 aht; // ** NEW: AHT21 Object **
MFRC522 rfid(RFID_SDA, RFID_RST);
Servo lockServo;

// === Global State Variables ===
// --- These hold the latest sensor readings ---
bool fenceActive = false;
bool tamperMotion = false;
bool tamperIR = false;
float temperature = 25.0;
float humidity = 60.0; // Will be updated by AHT21
float animalDistance = 999.0;
float solarVoltage = 0.0;
float solarCurrent = 0.0; // Placeholder (needs INA219 or similar)
float batteryLevel = 85.0; // Placeholder
String rtcTime = "2025-11-04T07:00:00"; // ISO 8601 Format

// --- Log Buffers (for the dashboard) ---
#define MAX_LOGS 5
String detectLogs[MAX_LOGS];
String rfidLogs[MAX_LOGS];
int detectLogIndex = 0;
int rfidLogIndex = 0;

// === Timers (to avoid using delay()) ===
unsigned long lastSensorRead = 0;
unsigned long lastRfidScan = 0;
unsigned long servoUnlockTime = 0;
bool servoUnlocked = false;
unsigned long startTime = 0;

// === Project Logic ===
// This is the UID of the "master" drone card
byte droneCardUID[4] = {0xDE, 0xAD, 0xBE, 0xEF}; // !! REPLACE WITH YOUR CARD'S UID !!
#define BUZZER_PWM_CHANNEL 0 // ** NEW: PWM Channel for buzzer **

// =================================================================
// ===           LOGGING FUNCTIONS                               ===
// =================================================================

// Helper to get formatted time string from RTC
String getLogTime() {
  DateTime now = rtc.now();
  char buf[9];
  sprintf(buf, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  return String(buf);
}

// Add a new entry to the detection log
void addDetectLog(String icon, String msg) {
  detectLogs[detectLogIndex] = "{\"icon\":\"" + icon + "\",\"time\":\"" + getLogTime() + "\",\"msg\":\"" + msg + "\"}";
  detectLogIndex = (detectLogIndex + 1) % MAX_LOGS;
}

// Add a new entry to the RFID/Servo log
void addRfidLog(String icon, String msg) {
  rfidLogs[rfidLogIndex] = "{\"icon\":\"" + icon + "\",\"time\":\"" + getLogTime() + "\",\"msg\":\"" + msg + "\"}";
  rfidLogIndex = (rfidLogIndex + 1) % MAX_LOGS;
}

// =================================================================
// ===           API REQUEST HANDLER                           ===
// =================================================================

void handleApiRequest(AsyncWebServerRequest *request) {
  // Create a JSON document (increased to 2048 bytes)
  StaticJsonDocument<2048> doc;

  // --- 1. Fence Status ---
  JsonObject fence = doc.createNestedObject("fence");
  fence["status"] = fenceActive ? "ACTIVE" : "CLEAR";

  // --- 2. Temperature & Humidity (from AHT21) ---
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;

  // --- 3. Battery & Solar ---
  JsonObject battery = doc.createNestedObject("battery");
  battery["level"] = batteryLevel; // Placeholder
  JsonObject solar = doc.createNestedObject("solar");
  solar["voltage"] = solarVoltage;
  solar["current"] = solarCurrent; // Placeholder
  solar["power"] = solarVoltage * solarCurrent; // Placeholder

  // --- 4. Other Sensor Data ---
  JsonObject tamper = doc.createNestedObject("tamper");
  tamper["motion"] = tamperMotion;
  tamper["ir"] = tamperIR;
  doc["animal_dist"] = animalDistance;
  doc["time"] = rtc.now().timestamp(DateTime::TIMESTAMP_FULL);
  
  // --- 5. Uptime & Sync ---
  unsigned long uptimeSeconds = (millis() / 1000);
  unsigned long days = uptimeSeconds / 86400;
  unsigned long hours = (uptimeSeconds % 86400) / 3600;
  unsigned long minutes = (uptimeSeconds % 3600) / 60;
  String uptimeStr = String(days) + "d " + String(hours) + "h " + String(minutes) + "m";
  doc["uptime"] = uptimeStr;
  doc["lastSync"] = getLogTime();

  // --- 6. Logs ---
  JsonObject logs = doc.createNestedObject("logs");
  JsonArray detect = logs.createNestedArray("detect");
  JsonArray rfid = logs.createNestedArray("rfid");

  for (int i = 0; i < MAX_LOGS; i++) {
    if (detectLogs[i] != "") {
      detect.add(serialized(detectLogs[i]));
    }
    if (rfidLogs[i] != "") {
      rfid.add(serialized(rfidLogs[i]));
    }
  }

  // --- 7. Send the response ---
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->addHeader("Access-Control-Allow-Origin", "*"); // IMPORTANT for web
  serializeJson(doc, *response);
  request->send(response);
}


// =================================================================
// ===           SENSOR READING FUNCTIONS                      ===
// =================================================================

void readAllSensors() {
  // --- 1. Capacitive Sensor (Fence) ---
  bool currentFenceState = digitalRead(CAPACITIVE_PIN) == HIGH;
  if (currentFenceState != fenceActive) {
    fenceActive = currentFenceState;
    if (fenceActive) {
      addDetectLog("fa-bolt", "Fence Current DETECTED");
    } else {
      addDetectLog("fa-bolt", "Fence Current CLEAR");
    }
  }

  // --- 2. IR Sensor (Tamper) ---
  bool currentIRState = digitalRead(IR_SENSOR_PIN) == LOW; // (Most IRs are LOW when triggered)
  if (currentIRState != tamperIR) {
    tamperIR = currentIRState;
    if (tamperIR) {
      addDetectLog("fa-hand-paper", "IR Tamper: Hand Detected!");
    }
  }

  // --- 3. AHT21 (Temp/Humidity) ** NEW ** ---
  sensors_event_t humidity_event, temp_event;
  aht.getEvent(&humidity_event, &temp_event);
  temperature = temp_event.temperature;
  humidity = humidity_event.relative_humidity;

  // --- 4. MPU-6050 (Tamper) ---
  sensors_event_t a, g, mpu_temp; // MPU temp is not used now
  mpu.getEvent(&a, &g, &mpu_temp);
  
  // Simple motion check: if acceleration > 15 m/s^2
  if (abs(a.acceleration.x) > 15 || abs(a.acceleration.y) > 15 || abs(a.acceleration.z) > 15) {
    if (!tamperMotion) { // Only log once per event
      addDetectLog("fa-exclamation-triangle", "MOTION TAMPER DETECTED!");
    }
    tamperMotion = true;
  } else {
    tamperMotion = false;
  }

  // --- 5. Ultrasonic (Animal) ---
  digitalWrite(US_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG_PIN, LOW);
  long duration = pulseIn(US_ECHO_PIN, HIGH, 50000); // 50ms timeout
  animalDistance = duration * 0.034 / 2; // Speed of sound wave divided by 2
  
  // --- 6. Solar Voltage ---
  int adcVal = analogRead(SOLAR_VOLTAGE_PIN);
  // !! You MUST calibrate this divider!!
  // Assuming a 11:1 voltage divider (100k & 10k)
  // Vout = adcVal / 4095 * 3.3
  // Vin = Vout * 11
  solarVoltage = (adcVal / 4095.0 * 3.3) * 11.0; // CALIBRATE THIS
  
  // --- 7. Animal Buzzer Logic (Upgrade) ** IMPROVED ** ---
  if (animalDistance < 200 && animalDistance > 1) { // If animal is closer than 2 meters
    if (!tamperMotion) { // Don't buzz if it's just being moved
      addDetectLog("fa-paw", "Animal detected at " + String(animalDistance, 1) + "cm");
      ledcWrite(BUZZER_PWM_CHANNEL, 128); // 50% duty cycle
    }
  } else {
    ledcWrite(BUZZER_PWM_CHANNEL, 0); // Buzzer Off
  }
}

// =================================================================
// ===           RFID & SERVO LOGIC                          ===
// =================================================================

void handleRfid() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return; // No card, exit
  }

  // A card is present. Compare its UID
  bool droneVerified = true;
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] != droneCardUID[i]) {
      droneVerified = false;
    }
  }

  if (droneVerified) {
    addRfidLog("fa-id-card", "Drone ID Verified. Unlocking.");
    lockServo.write(90); // Unlock
    // ** BUG FIX: Set non-blocking timer instead of delay() **
    servoUnlocked = true;
    servoUnlockTime = millis();
  } else {
    addRfidLog("fa-id-card", "UNKNOWN Card Scanned! Access Denied.");
  }

  rfid.PICC_HaltA(); // Stop reading
  rfid.PCD_StopCrypto1();
}

// =================================================================
// ===           SETUP & LOOP                                  ===
// =================================================================

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  // --- 1. Connect to WiFi ---
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // --- 2. Initialize Sensors ---
  // RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    addDetectLog("fa-exclamation-triangle", "RTC FAILED TO INIT");
  } else {
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Uncomment to set time
    Serial.println("RTC OK");
    startTime = rtc.now().unixtime();
  }
  
  // MPU-6050
  if (!mpu.begin(0x69)) { // Remember, address 0x69!
    Serial.println("MPU6050 not found! Check AD0 pin.");
    addDetectLog("fa-exclamation-triangle", "MPU6050 FAILED");
  } else {
    Serial.println("MPU6050 OK");
  }

  // ** NEW: Initialize AHT21 **
  if (!aht.begin()) {
    Serial.println("AHT21 not found! Check wiring.");
    addDetectLog("fa-exclamation-triangle", "AHT21 FAILED");
  } else {
    Serial.println("AHT21 OK");
  }

  // RFID
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID OK");

  // Servo
  lockServo.attach(SERVO_PIN);
  lockServo.write(0); // Start in locked position
  Serial.println("Servo OK");

  // ** NEW: Setup Buzzer PWM **
  ledcSetup(BUZZER_PWM_CHANNEL, 5000, 8); // Channel 0, 5kHz freq, 8-bit res
  ledcAttachPin(BUZZER_PIN, BUZZER_PWM_CHANNEL);

  // PinModes
  pinMode(CAPACITIVE_PIN, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT_PULLUP); // Use pullup for stability
  pinMode(US_TRIG_PIN, OUTPUT);
  pinMode(US_ECHO_PIN, INPUT);
  pinMode(SOLAR_VOLTAGE_PIN, INPUT);
  // BUZZER_PIN is set by ledcAttachPin

  // --- 3. Start Web Server & API ---
  server.on("/api/sensors", HTTP_GET, handleApiRequest);
  
  // Default handler for 404
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println("HTTP Server Started");
}

void loop() {
  // Non-blocking timers
  unsigned long now = millis();

  // Task 1: Read all sensors every 2 seconds
  if (now - lastSensorRead > 2000) {
    lastSensorRead = now;
    readAllSensors();
  }

  // Task 2: Check for RFID card every 500ms
  if (now - lastRfidScan > 500) {
    lastRfidScan = now;
    handleRfid();
  }
  
  // ** NEW: Non-blocking servo re-lock **
  if (servoUnlocked && (now - servoUnlockTime > 3000)) {
    lockServo.write(0); // Re-lock
    servoUnlocked = false;
    addRfidLog("fa-lock", "Servo Locked.");
  }
}
```

### 3. Configuration Steps

1. **Update WiFi Credentials** (Lines 38-39):
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";        // Replace with your WiFi name
   const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your WiFi password
   ```

2. **Update RFID Card UID** (Line 81):
   ```cpp
   byte droneCardUID[4] = {0xDE, 0xAD, 0xBE, 0xEF}; // Use values from step 1
   ```

3. **Calibrate Solar Voltage Divider** (Line 308):
   - Measure your actual solar panel voltage with a multimeter
   - Adjust the multiplier (currently 11.0) to match real readings
   ```cpp
   solarVoltage = (adcVal / 4095.0 * 3.3) * YOUR_CALIBRATION_VALUE;
   ```

4. **Set RTC Time** (Line 381, uncomment if needed):
   ```cpp
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Sets time from compile time
   ```

### 4. Upload to ESP32

1. Connect ESP32 to computer via USB
2. Select correct **Port** in Tools menu
3. Click **Upload** button (→)
4. Wait for "Done uploading" message
5. Open **Serial Monitor** (115200 baud)
6. Note the IP address displayed (e.g., `192.168.1.100`)

---

## ⚙️ Configuration

### Dashboard Setup

1. Open `index.html` in your web browser
2. Click the **Edit** button next to "ESP32 IP Address"
3. Enter the IP address shown in Serial Monitor
4. Click **Save**
5. Dashboard should now show "Device is Connected"

### API Endpoint

The ESP32 serves data at:
```
http://<ESP32_IP>/api/sensors
```

Example response:
```json
{
  "fence": {"status": "CLEAR"},
  "temperature": 26.5,
  "humidity": 65.3,
  "battery": {"level": 85},
  "solar": {"voltage": 12.4, "current": 0.5, "power": 6.2},
  "tamper": {"motion": false, "ir": false},
  "animal_dist": 150.5,
  "time": "2025-11-04T14:23:45",
  "uptime": "0d 2h 15m",
  "lastSync": "14:23:45",
  "logs": {
    "detect": [...],
    "rfid": [...]
  }
}
```

---

## 🐛 Troubleshooting

### ESP32 Won't Connect to WiFi
- Double-check SSID and password
- Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Check signal strength
- Try a WiFi hotspot for testing

### Sensors Not Working

#### RTC (DS3231)
- Check I2C connections (SDA=21, SCL=22)
- Verify 3.3V power
- Run I2C scanner sketch to confirm address 0x68

#### MPU-6050
- **CRITICAL:** AD0 pin MUST be connected to VCC for address 0x69
- If address is wrong, change `mpu.begin(0x69)` to `mpu.begin(0x68)`
- Check I2C connections

#### AHT21
- Verify I2C connections
- Some modules need 5V power (check datasheet)
- Default address is 0x38

#### RFID (MFRC522)
- Check all SPI connections
- Verify 3.3V power (NOT 5V!)
- Use RFID scanner sketch to test

### Dashboard Shows "Connection Lost"
- Verify ESP32 IP address is correct
- Check if ESP32 is powered on
- Try accessing `http://<ESP32_IP>/api/sensors` directly in browser
- Check browser console for CORS errors
- Ensure ESP32 and computer are on same network

### Servo Not Moving
- Check power supply (servo needs 5V, >500mA)
- Verify GPIO 13 connection
- Test with simple servo sweep sketch

### Buzzer Not Working
- Verify GPIO 12 connection
- Check if buzzer is passive type (not active)
- Test frequency change in `ledcSetup()`

### Solar Voltage Reading Wrong
- **CRITICAL:** Never connect solar panel directly to GPIO!
- Verify voltage divider resistors
- Calibrate multiplier in code
- Use multimeter to compare readings

### High Memory Usage
- JSON buffer is 2048 bytes
- If ESP32 crashes, reduce MAX_LOGS or simplify JSON

---

## 📊 Testing Checklist

- [ ] ESP32 connects to WiFi and shows IP
- [ ] Serial Monitor shows "All sensors OK"
- [ ] Dashboard connects and shows "Device is Connected"
- [ ] Fence status changes when capacitive sensor triggered
- [ ] Temperature and humidity update correctly
- [ ] RFID card unlocks servo
- [ ] Ultrasonic detects animals and buzzes
- [ ] MPU-6050 detects motion tamper
- [ ] IR sensor detects hand proximity
- [ ] Solar voltage reads correctly (with multimeter verification)
- [ ] Logs appear in dashboard
- [ ] Chart updates in real-time

---

## 🚀 Next Steps

1. **Test each sensor individually** before full integration
2. **Calibrate solar voltage divider** with multimeter
3. **Set RTC time** to correct date/time
4. **Add your RFID card UID** to authorized list
5. **Mount in weatherproof enclosure** for outdoor use
6. **Add battery monitoring** circuit (INA219 recommended)
7. **Implement power-saving modes** for battery operation

---

## 📝 Notes

- **Power Management:** ESP32 draws ~160mA when WiFi is active. Consider deep sleep modes for battery operation.
- **Safety:** Ensure proper voltage regulation for all sensors. Use optocouplers for high-voltage fence connections.
- **Expandability:** You can add more sensors by using free GPIO pins (15, 16, 17, etc.)
- **Security:** Consider adding password authentication to the web API for production use.

---

## 📞 Support

For issues or questions:
- Check Serial Monitor output for error messages
- Verify all pin connections match this guide
- Test sensors individually with example sketches
- Review Arduino library documentation

---

**PROJECT DONE BY H^4 HYPERS**

© 2025 FENCEORA | Smart Electric Fence Node System
