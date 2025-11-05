# 🔧 FENCEORA Sensor Setup & Calibration Guide

## 📋 Table of Contents
1. [RFID Card Setup](#rfid-card-setup)
2. [Capacitive Sensor Adjustment](#capacitive-sensor-adjustment)
3. [Temperature Sensor Wiring](#temperature-sensor-wiring)
4. [All Sensor Wiring](#complete-wiring-guide)
5. [Testing Each Sensor](#testing-guide)

---

## 🎴 RFID Card Setup

### Problem: "UNKNOWN Card Scanned! Access Denied"

### Solution: Get Your Card's UID

#### Step 1: Upload UID Reader Sketch
1. Open Arduino IDE
2. Open `RFID_UID_Reader.ino`
3. Upload to ESP32
4. Open Serial Monitor (115200 baud)

#### Step 2: Scan Your Card
1. Place your RFID card/tag near the RC522 reader
2. Serial Monitor will show:
   ```
   ================================
   ✓ CARD DETECTED!
   ================================
   UID Size: 4
   UID (HEX): A1 B2 C3 D4
   UID (DEC): 161 178 195 212
   
   >>> COPY THIS LINE TO YOUR CODE <<<
   byte droneCardUID[4] = {0xA1, 0xB2, 0xC3, 0xD4};
   ================================
   ```

#### Step 3: Update Your Code
1. Open `Fenceora_V3_FIXED.ino`
2. Find this line (around line 90):
   ```cpp
   byte droneCardUID[4] = {0xDE, 0xAD, 0xBE, 0xEF};
   ```
3. Replace with YOUR card's UID:
   ```cpp
   byte droneCardUID[4] = {0xA1, 0xB2, 0xC3, 0xD4}; // YOUR UID HERE!
   ```
4. Upload the code
5. Test again - should now say "✓ Drone ID Verified. Unlocking..."

### Multiple Cards:
If you want to authorize multiple cards, modify the code:
```cpp
byte droneCardUID1[4] = {0xA1, 0xB2, 0xC3, 0xD4}; // Card 1
byte droneCardUID2[4] = {0x11, 0x22, 0x33, 0x44}; // Card 2

// In handleRfid() function:
bool droneVerified = false;

// Check card 1
if (memcmp(rfid.uid.uidByte, droneCardUID1, 4) == 0) {
  droneVerified = true;
}
// Check card 2
if (memcmp(rfid.uid.uidByte, droneCardUID2, 4) == 0) {
  droneVerified = true;
}
```

---

## ⚡ Capacitive Sensor Adjustment

### Problem: Sensor triggers when hand is near, not touching

### Cause: Sensitivity too high

### Solution: Adjust the Potentiometer (Knob)

#### Understanding the Sensor:
- **LED on sensor**: Lights up when triggered
- **Potentiometer (small screw/knob)**: Adjusts sensitivity
- **Output**: HIGH when triggered, LOW when not

#### Calibration Steps:

**Step 1: Find the Potentiometer**
- Look for small screw or knob on the capacitive sensor module
- Usually blue or white color
- May require small screwdriver

**Step 2: Adjust Sensitivity**
1. **Too Sensitive** (triggers without touch):
   - Turn potentiometer **CLOCKWISE** (right/decrease)
   - Test: Wave hand near sensor
   - Repeat until LED only lights when touching

2. **Not Sensitive** (doesn't trigger when touching):
   - Turn potentiometer **COUNTER-CLOCKWISE** (left/increase)
   - Test: Touch the orange detection tip
   - Repeat until LED lights reliably on touch

**Step 3: Fine-Tune**
- **Goal**: LED lights ONLY when wire/object makes contact
- Test multiple times
- Should NOT trigger from air movement or nearby hand
- SHOULD trigger when touching the detection surface

**Step 4: Verify in Serial Monitor**
```
Upload code → Open Serial Monitor → Touch sensor
Should see: "Fence Current DETECTED!"
Remove hand
Should see: "Fence Current CLEAR"
```

#### Wiring Check:
```
Capacitive Sensor → ESP32
VCC  → 3.3V (or 5V if sensor supports)
GND  → GND
OUT  → GPIO 25
```

#### If Still Not Working:
- Try different GPIO pin
- Check if sensor needs 5V (some capacitive sensors work better with 5V)
- Use voltage divider if sensor outputs 5V: 
  ```
  Sensor OUT → [10kΩ resistor] → GPIO 25 → [10kΩ resistor] → GND
  ```

---

## 🌡️ Temperature Sensor Wiring

### Sensor: AHT21 (Temperature & Humidity)

### Important: AHT21 uses I2C (No separate temperature pin needed!)

#### Wiring:
```
AHT21 Sensor → ESP32
-----------------------
VIN (or VCC) → 3.3V
GND          → GND
SDA          → GPIO 21 (I2C_SDA)
SCL          → GPIO 22 (I2C_SCL)
```

#### Notes:
1. **No separate pin for temperature!** 
   - Temperature sensor shares I2C bus with RTC and MPU6050
   - GPIO 21 and GPIO 22 are used for ALL I2C sensors

2. **I2C Address**: 
   - AHT21 address: 0x38
   - No jumpers needed

3. **Connection Tips**:
   - Use short wires (< 20cm recommended)
   - Connect GND first, then power
   - SDA and SCL can use 4.7kΩ pull-up resistors (usually not needed)

4. **Verification**:
   - Upload code
   - Check Serial Monitor
   - Should see: `✓ AHT21 OK`
   - Dashboard should show temperature and humidity

---

## 🔌 Complete Wiring Guide

### I2C Devices (All share GPIO 21 & 22):

```
Device          I2C Address    VCC    GND    SDA     SCL
-------------------------------------------------------------
RTC (DS3231)    0x68          3.3V   GND    GPIO21  GPIO22
MPU6050         0x69          3.3V   GND    GPIO21  GPIO22
AHT21           0x38          3.3V   GND    GPIO21  GPIO22
```

**Wiring Diagram:**
```
        ┌──────────────┐
        │    ESP32     │
        │              │
3.3V ───┤ 3.3V         │
        │              │
GND  ───┤ GND          │
        │              │
        │ GPIO21 (SDA) ├─────┬─────┬─────┐
        │              │     │     │     │
        │ GPIO22 (SCL) ├───┐ │   │ │   │ │
        └──────────────┘   │ │   │ │   │ │
                           │ │   │ │   │ │
        ┌──────────┐       │ │   │ │   │ │
        │ RTC      │       │ │   │ │   │ │
        │ DS3231   │       │ │   │ │   │ │
        │ SDA ─────┼───────┘ │   │ │   │ │
        │ SCL ─────┼─────────┘   │ │   │ │
        └──────────┘             │ │   │ │
                                 │ │   │ │
        ┌──────────┐             │ │   │ │
        │ MPU6050  │             │ │   │ │
        │ SDA ─────┼─────────────┘ │   │ │
        │ SCL ─────┼───────────────┘   │ │
        └──────────┘                   │ │
                                       │ │
        ┌──────────┐                   │ │
        │ AHT21    │                   │ │
        │ SDA ─────┼───────────────────┘ │
        │ SCL ─────┼─────────────────────┘
        └──────────┘
```

---

### SPI Device (RFID RC522):

```
RFID RC522 → ESP32
--------------------
SDA   → GPIO 4
SCK   → GPIO 18 (default SPI)
MOSI  → GPIO 23 (default SPI)
MISO  → GPIO 19 (default SPI)
IRQ   → Not connected
GND   → GND
RST   → GPIO 5
3.3V  → 3.3V
```

---

### Digital Sensors:

```
Sensor              ESP32 Pin    Notes
-------------------------------------------------
Capacitive Sensor   GPIO 25      Adjust knob for sensitivity
IR Sensor           GPIO 26      Active LOW (triggers when LOW)
Ultrasonic TRIG     GPIO 33      Trigger signal
Ultrasonic ECHO     GPIO 32      Echo receive
```

---

### Analog Sensors:

```
Solar Voltage       GPIO 36      Use voltage divider (12V → 3.3V)
```

**Voltage Divider for Solar:**
```
Solar Panel (+) → [10kΩ] → GPIO 36 → [3.3kΩ] → GND
                            ↑
                         Measure here
```

This gives: 3.3V / 13.3V = 0.248 ratio
So: 12V solar → ~3V at GPIO 36 (safe for ESP32)

---

### Actuators:

```
Servo Motor         GPIO 13      Control signal
Buzzer              GPIO 12      PWM control
```

---

## 🧪 Testing Guide

### Test 1: I2C Sensors
```cpp
Upload code → Open Serial Monitor
Should see:
✓ RTC OK
✓ MPU6050 OK
✓ AHT21 OK
```

If any fail:
- Check wiring
- Check I2C address
- Use I2C scanner sketch to find devices

---

### Test 2: RFID
```
Upload RFID_UID_Reader.ino
Place card near reader
Should see card UID
```

---

### Test 3: Capacitive Sensor
```
Touch sensor → LED on sensor lights up
Serial Monitor: "Fence Current DETECTED!"
Remove hand → LED turns off
Serial Monitor: "Fence Current CLEAR"
```

If triggers without touch → Adjust potentiometer CLOCKWISE

---

### Test 4: IR Sensor
```
Place hand in front of IR sensor
Serial Monitor: "⚠️ ALERT: Someone tried to hold/tamper with product!"
```

---

### Test 5: Ultrasonic
```
Place object in front of sensor (within 2 meters)
Serial Monitor: "Animal detected at XXcm distance!"
Dashboard: Should show distance
```

---

### Test 6: Temperature
```
Dashboard should show:
- Temperature: XX.X°C
- Humidity: XX.X%
```

Breathe on sensor → Temperature and humidity should increase

---

## 🔍 Troubleshooting

### "✗ AHT21 Failed!"
**Solutions:**
1. Check wiring (SDA=21, SCL=22)
2. Check 3.3V power
3. Try different AHT21 module
4. Check I2C address with scanner

### "✗ MPU6050 Failed!"
**Solutions:**
1. Check AD0 pin is HIGH (for address 0x69)
2. Or change code to 0x68 if AD0 is LOW
3. Check wiring

### "✗ RTC Failed!"
**Solutions:**
1. Check battery on RTC module
2. Check I2C wiring
3. Address is usually 0x68

### Capacitive sensor always triggered
**Solution:**
1. Turn potentiometer CLOCKWISE
2. Keep turning until LED only lights on touch
3. May need 10-20 full turns

### RFID not reading
**Solutions:**
1. Check SPI wiring
2. Card too far (bring within 1-2cm)
3. Check 3.3V power
4. Upload UID reader to test

### Ultrasonic shows wrong distance
**Solutions:**
1. Check TRIG and ECHO wiring
2. Test with known distance object
3. Sensor range: 2cm - 400cm
4. Needs clear line of sight

---

## 📊 Expected Serial Monitor Output

```
=================================
   FENCEORA V3.1 - STARTING
=================================

Connecting to WiFi: Oppo A77s
..........
✓ WiFi Connected!
✓ IP Address: 192.168.43.100

Initializing Sensors...
✓ RTC OK
✓ MPU6050 OK
✓ AHT21 OK
✓ RFID OK
✓ Servo OK (Locked)
✓ Buzzer OK
✓ HTTP Server Started

=================================
   SYSTEM READY
=================================

[DETECT LOG] System Initialized

=== API REQUEST RECEIVED ===
JSON Response Size: 523 bytes
=== API RESPONSE SENT ===

[When you touch capacitive sensor]
[DETECT LOG] Fence Current DETECTED!

[When you scan RFID card]
Card UID: 0xA1, 0xB2, 0xC3, 0xD4
[RFID LOG] ✓ Drone ID Verified. Unlocking...

[When you place hand near IR]
[DETECT LOG] ⚠️ ALERT: Someone tried to hold/tamper with product!

[When object near ultrasonic]
[DETECT LOG] Animal detected at 45cm distance!
```

---

## ✅ Final Checklist

### Hardware:
- [ ] All I2C devices connected to GPIO 21 & 22
- [ ] AHT21 temperature sensor wired correctly
- [ ] RFID wired to correct SPI pins
- [ ] Capacitive sensor adjusted (LED only on touch)
- [ ] IR sensor wired to GPIO 26
- [ ] Ultrasonic TRIG→33, ECHO→32
- [ ] Solar voltage divider connected to GPIO 36
- [ ] Servo on GPIO 13
- [ ] Buzzer on GPIO 12

### Software:
- [ ] Updated RFID UID with your card's UID
- [ ] Uploaded latest code
- [ ] All sensors show ✓ in Serial Monitor
- [ ] Dashboard connected and updating

### Testing:
- [ ] Touch capacitive → logs "Fence Current DETECTED!"
- [ ] Scan RFID → logs "✓ Drone ID Verified"
- [ ] Hand near IR → logs "⚠️ ALERT"
- [ ] Object near ultrasonic → logs "Animal detected"
- [ ] Dashboard shows temperature
- [ ] Dashboard shows humidity
- [ ] All values update every 2-5 seconds

---

**Keep this guide for reference during setup!**
