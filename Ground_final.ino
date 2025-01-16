#include <SPI.h>
#include <SD.h>
#include <LoRa.h>

#define csPin 10 // Chip select for SD
#define ssPin 9  // Chip select for LoRa
#define resetPin 8
#define dio0 2

void setup() {
  Serial.begin(115200);

  Serial.print("Initializing SD card...");
  if (!SD.begin(csPin)) {
    Serial.println("failed");
    while (1);
  }
  Serial.println("Good");

  Serial.print("Initializing LoRa...");
  LoRa.setPins(ssPin, resetPin, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("failed");
    while (1);
  }
  Serial.println("Good");
}


void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet:");

    // Read packet
    while (LoRa.available()) {
      String data = LoRa.readString();
      Serial.print(data);
      Serial.println(" ");

      File dataFile = SD.open("datalog.txt", FILE_WRITE);  // open datalog.txt
      if (dataFile) {
        dataFile.println(data); // write data to SD
      }
      dataFile.close(); //close file
    }
  }
}
