# FENCEORA ESP32 Pin Diagram
**Version 2.0 - Complete Pin Configuration**

---

## 📌 **ESP32 Dev Board Pin Assignments**

### **1. RFID Module (RC522) - SPI Interface**
| RFID Pin | ESP32 Pin | Pin Number | Description |
|----------|-----------|------------|-------------|
| **SDA**  | GPIO 21   | D21        | Slave Select (SS) |
| **SCK**  | GPIO 18   | D18        | SPI Clock (Default) |
| **MOSI** | GPIO 23   | D23        | Master Out Slave In (Default) |
| **MISO** | GPIO 19   | D19        | Master In Slave Out (Default) |
| **RST**  | GPIO 22   | D22        | Reset Pin |
| **GND**  | GND       | GND        | Ground |
| **3.3V** | 3.3V      | 3.3V       | Power Supply |

⚠️ **IMPORTANT:** RFID pins moved to GPIO 21/22 to avoid I2C conflict with MPU6050

---

### **2. Servo Motor**
| Servo Wire | ESP32 Pin | Pin Number | Description |
|------------|-----------|------------|-------------|
| **Signal** | GPIO 13   | D13        | PWM Control Signal |
| **VCC**    | 5V        | 5V/VIN     | Power Supply (External 5V recommended) |
| **GND**    | GND       | GND        | Ground |

⚠️ **IMPORTANT:** Servo requires external 5V power supply (1A minimum). ESP32 cannot provide enough current.

---

### **3. MPU6050 (Gyroscope + Accelerometer) - I2C Interface**
| MPU6050 Pin | ESP32 Pin | Pin Number | Description |
|-------------|-----------|------------|-------------|
| **SDA**     | GPIO 26   | D26        | I2C Data |
| **SCL**     | GPIO 25   | D25        | I2C Clock |
| **VCC**     | 3.3V      | 3.3V       | Power Supply |
| **GND**     | GND       | GND        | Ground |
| **AD0**     | GND       | GND        | Address Select (0x68) |

⚠️ **IMPORTANT:** MPU6050 I2C pins moved to GPIO 26/25 to avoid conflict with RFID SS/RST pins

---

### **4. Ultrasonic Sensor (HC-SR04)**
| Ultrasonic Pin | ESP32 Pin | Pin Number | Description |
|----------------|-----------|------------|-------------|
| **TRIG**       | GPIO 12   | D12        | Trigger Output |
| **ECHO**       | GPIO 14   | D14        | Echo Input |
| **VCC**        | 5V        | 5V         | Power Supply |
| **GND**        | GND       | GND        | Ground |

---

### **5. IR Sensor (Infrared Obstacle Detection)**
| IR Sensor Pin | ESP32 Pin | Pin Number | Description |
|---------------|-----------|------------|-------------|
| **OUT**       | GPIO 4    | D4         | Digital Output |
| **VCC**       | 3.3V/5V   | 3.3V/5V    | Power Supply |
| **GND**       | GND       | GND        | Ground |

---

### **6. Current Sensor (ACS712)**
| ACS712 Pin | ESP32 Pin | Pin Number | Description |
|------------|-----------|------------|-------------|
| **OUT**    | GPIO 35   | A7/D35     | Analog Output (ADC1_CH7) |
| **VCC**    | 5V        | 5V         | Power Supply |
| **GND**    | GND       | GND        | Ground |

⚠️ **NOTE:** GPIO 35 is input-only (no pull-up/pull-down resistors)

---

### **7. Voltage Sensor (Voltage Divider)**
| Voltage Sensor | ESP32 Pin | Pin Number | Description |
|----------------|-----------|------------|-------------|
| **OUT**        | GPIO 34   | A6/D34     | Analog Input (ADC1_CH6) |
| **VCC**        | Battery+  | -          | Voltage Source |
| **GND**        | GND       | GND        | Ground |

⚠️ **NOTE:** GPIO 34 is input-only (no pull-up/pull-down resistors)

---

### **8. Buzzer/Speaker**
| Buzzer Pin | ESP32 Pin | Pin Number | Description |
|------------|-----------|------------|-------------|
| **Signal** | GPIO 16   | D16        | Digital Output |
| **GND**    | GND       | GND        | Ground |

⚠️ **IMPORTANT:** Buzzer moved from GPIO 13 to GPIO 16 (servo now uses GPIO 13)

---

### **9. LED Indicator (Built-in)**
| LED | ESP32 Pin | Pin Number | Description |
|-----|-----------|------------|-------------|
| **Built-in LED** | GPIO 2 | D2 | Status Indicator |

---

## 🔌 **Power Supply Requirements**

| Component | Voltage | Current | Notes |
|-----------|---------|---------|-------|
| ESP32 Dev Board | 3.3V / 5V (USB) | ~500mA | Via USB or VIN pin |
| RFID RC522 | 3.3V | ~50mA | From ESP32 3.3V pin |
| MPU6050 | 3.3V | ~4mA | From ESP32 3.3V pin |
| Servo Motor | **5V** | **1-2A** | ⚠️ **EXTERNAL SUPPLY REQUIRED** |
| Ultrasonic | 5V | ~15mA | From ESP32 5V pin |
| ACS712 | 5V | ~10mA | From ESP32 5V pin |
| IR Sensor | 3.3V-5V | ~20mA | From ESP32 pin |
| Buzzer | 3.3V | ~30mA | From ESP32 GPIO |

⚠️ **CRITICAL:** Servo motor MUST have external 5V power supply (1A minimum). ESP32 cannot provide enough current!

---

## 🔧 **Pin Change Summary (from previous versions)**

| Component | OLD Pins | NEW Pins | Reason |
|-----------|----------|----------|---------|
| **RFID SS** | GPIO 5 | GPIO 21 | Avoid I2C conflict |
| **RFID RST** | GPIO 27 | GPIO 22 | Avoid I2C conflict |
| **MPU6050 SDA** | GPIO 21 | GPIO 26 | Avoid RFID conflict |
| **MPU6050 SCL** | GPIO 22 | GPIO 25 | Avoid RFID conflict |
| **Servo** | GPIO 15 | GPIO 13 | Better PWM support |
| **Buzzer** | GPIO 13 | GPIO 16 | Servo moved to 13 |
| **Ultrasonic TRIG** | GPIO 33 | GPIO 12 | Optimization |
| **Ultrasonic ECHO** | GPIO 32 | GPIO 14 | Optimization |
| **IR Sensor** | GPIO 26 | GPIO 4 | MPU6050 now uses 26 |

---

## 📊 **Visual Pin Layout**

```
ESP32 Dev Board (30-pin version)
================================

LEFT SIDE:                    RIGHT SIDE:
3.3V  ────────────────────    GND
EN    ────────────────────    GPIO 23 (MOSI - RFID)
GPIO 36                       GPIO 22 (RST - RFID)
GPIO 39                       TXD0
GPIO 34 (Voltage Sensor)      RXD0
GPIO 35 (Current Sensor)      GPIO 21 (SS - RFID)
GPIO 32                       GPIO 19 (MISO - RFID)
GPIO 33                       GPIO 18 (SCK - RFID)
GPIO 25 (SCL - MPU6050)       GPIO 5
GPIO 26 (SDA - MPU6050)       GPIO 17
GPIO 27                       GPIO 16 (Buzzer)
GPIO 14 (ECHO - Ultrasonic)   GPIO 4 (IR Sensor)
GPIO 12 (TRIG - Ultrasonic)   GPIO 0
GND   ────────────────────    GPIO 2 (LED)
GPIO 13 (Servo)               GPIO 15
5V    ────────────────────    GND
```

---

## 🧪 **Testing Each Sensor**

### **Test RFID:**
1. Upload code to ESP32
2. Open Serial Monitor (115200 baud)
3. Scan RFID card near reader
4. Should see: "Scanned UID: D3 2E E4 2C"

### **Test Servo:**
1. Connect external 5V power supply to servo
2. Scan authorized RFID card
3. Servo should rotate 0° → 90° (or vice versa)

### **Test MPU6050:**
1. Check Serial Monitor for "MPU6050 Found!"
2. Move/shake the device
3. Should see "TAMPER ALERT!" in serial

### **Test Ultrasonic:**
1. Place hand in front of sensor
2. At distance < 30cm, buzzer should sound
3. Serial shows "CRITICAL PROXIMITY!"

### **Test IR Sensor:**
1. Place hand near IR sensor
2. Buzzer should sound
3. Serial shows "IR SENSOR TRIGGERED!"

### **Test Current Sensor:**
1. Connect load to ACS712
2. Turn on load
3. If current detected, buzzer sounds

---

## 🌐 **WiFi API Endpoint**

After ESP32 connects to WiFi, access sensor data at:

```
http://[ESP32_IP_ADDRESS]/api/sensors
```

**Example Response:**
```json
{
  "voltage": 12.5,
  "ultrasonic": 45,
  "irDetected": false,
  "currentDetected": false,
  "tamperDetected": false,
  "servoLocked": true,
  "lastRFID": "d32ee42c",
  "mpu": {
    "accelX": 0.5,
    "accelY": -0.2,
    "accelZ": 9.8,
    "gyroX": 0.1,
    "gyroY": 0.0,
    "gyroZ": -0.1
  },
  "status": "online",
  "timestamp": 123456
}
```

---

## ⚠️ **Common Issues & Solutions**

### **RFID not detecting cards:**
- Check SPI wiring (SS=21, RST=22, SCK=18, MOSI=23, MISO=19)
- Ensure 3.3V power supply is stable
- Card must be within 5cm of reader

### **Servo not moving:**
- ⚡ **MUST use external 5V power supply (1A minimum)**
- ESP32 cannot provide enough current
- Check signal wire connected to GPIO 13

### **MPU6050 not found:**
- Check I2C wiring (SDA=26, SCL=25)
- Ensure AD0 pin connected to GND (address 0x68)
- Check 3.3V power supply

### **WiFi not connecting:**
- Verify SSID and password in code
- Check Serial Monitor for connection status
- ESP32 must be within WiFi range

---

**Last Updated:** November 25, 2025
**Code Version:** Fenceora_Simple_V1.ino (v2.0)
