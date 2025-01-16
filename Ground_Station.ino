#include <SPI.h>
#include <SD.h>
#include <LoRa.h>

// Define the LoRa and SD card CS pins
#define LORA_CS 10
#define SD_CS 4

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize SD card
  SD.begin(SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("SD card initialized.");

  // Initialize LoRa module
  LoRa.setPins(LORA_CS, -1, -1); // set CS, reset, IRQ pin
  if (!LoRa.begin(433E6)) { // initialize ratio at 433 MHz
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa initialized at 433 MHz");
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    
    // read packet
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    Serial.print(incoming);
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    // Open the file. Note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    // If the file is available, write to it:
    if (dataFile) {
      dataFile.println(incoming);
      dataFile.close();
      Serial.println("Data written to SD.");
    } else {
      // if the file isn't open, pop up an error:
      Serial.println("Error opening datalog.txt");
    }
  }
}
