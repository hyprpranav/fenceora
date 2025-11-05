/*
 * RFID UID Reader - Get Your Card's UID
 * Upload this sketch to ESP32 to find your card's UID
 * 
 * WIRING:
 * RC522 → ESP32
 * SDA  → GPIO 4
 * SCK  → GPIO 18
 * MOSI → GPIO 23
 * MISO → GPIO 19
 * IRQ  → Not connected
 * GND  → GND
 * RST  → GPIO 5
 * 3.3V → 3.3V
 */

#include <SPI.h>
#include <MFRC522.h>

#define RFID_SDA 4
#define RFID_RST 5

MFRC522 rfid(RFID_SDA, RFID_RST);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n================================");
  Serial.println("   RFID UID READER");
  Serial.println("================================\n");
  
  SPI.begin();
  rfid.PCD_Init();
  
  Serial.println("✓ RFID Reader Initialized");
  Serial.println("\n>>> Place your RFID card near the reader <<<\n");
}

void loop() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Read card serial
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Card detected! Print UID
  Serial.println("================================");
  Serial.println("✓ CARD DETECTED!");
  Serial.println("================================");
  
  Serial.print("UID Size: ");
  Serial.println(rfid.uid.size);
  
  Serial.print("UID (HEX): ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  Serial.print("UID (DEC): ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i]);
    Serial.print(" ");
  }
  Serial.println();
  
  Serial.println("\n>>> COPY THIS LINE TO YOUR CODE <<<");
  Serial.print("byte droneCardUID[");
  Serial.print(rfid.uid.size);
  Serial.print("] = {");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print("0x");
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) Serial.print(", ");
  }
  Serial.println("};");
  Serial.println("================================\n");
  
  Serial.println(">>> Place another card or same card again <<<\n");
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  delay(1000); // Wait 1 second before reading again
}
