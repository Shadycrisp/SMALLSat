#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <Adafruit_BMP280.h>
#include <SD.h>
#include <LoRa.h>

//GPS
static const int RXPin = 0, TXPin = 1;
static const uint32_t GPSBaud = 9600;
//LoRa
const long frequency = 433E6;
static const uint32_t csPin = 10;
static const uint32_t rstPin = 9;
// The TinyGPSPlus object
TinyGPSPlus gps;
//BMP280 Object
Adafruit_BMP280 bmp;

//SD File
File file;
static const uint32_t buzzerPin = 2;

float startTime = 0;
float interval = 5000;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(GPSBaud);
  //BMP initialization
 if (!bmp.begin())
 {
  Serial.println("Could not find valid BMP280");
  while (1);
 }

  //SD Initialization
  if (!SD.begin(BUILTIN_SDCARD))
  {
    Serial.println("SD Card Mount Failed");
    while (1);
  }

  file = SD.open("data.txt", FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
  }
  file.println("Log Begin:");
  file.close();

  //LoRa initialization
  

  LoRa.setPins(csPin, rstPin, 13);
  if (!LoRa.begin(frequency))
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  startTime = millis();
}

void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}

void displayInfo()
{
  //float flat, flon, deg;
  file = SD.open("data.txt", FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for logging");
    

  }
  Serial.print(F("Location: "));
  file.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(", "));
    Serial.println(gps.location.lng(), 6);
    //SD
    file.print(gps.location.lat(), 6);
    file.print(F(", "));
    file.println(gps.location.lat(), 6);
  }
  else
  {
    Serial.println(F("INVALID"));
    file.println(F("INVALID"));
  }
  //BMP280
  //Temp
  Serial.print("Temp:");
  Serial.print(bmp.readTemperature());
  Serial.println(" C");
  //Pressure
  Serial.print("Pressure:");
  Serial.print(bmp.readPressure()/100.0F);
  Serial.println(" hPa");
  //Altitude
  Serial.print("Altitude:");
  Serial.print(bmp.readAltitude(1013.25)); //Standard Pressure at Sea level(hPa)
  Serial.println("m");

  //Write to file
  file.print("Temp:");
  file.print(bmp.readTemperature());
  file.println(" C");
  //Pressure
  file.print("Pressure:");
  file.print(bmp.readPressure()/100.0F);
  file.println(" hPa");
  //Altitude
  file.print("Altitude:");
  file.print(bmp.readAltitude(1013.25)); //Standard Pressure at Sea level(hPa)
  file.println("m");

  Serial.print(F("  Date/Time: "));
  file.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
    //SD
    file.print(gps.date.month());
    file.print(F("/"));
    file.print(gps.date.day());
    file.print(F("/"));
    file.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
    file.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10){ Serial.print(F("0")); file.print(F("0"));}
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    //SD
    file.print(gps.time.hour());
    file.print(F(":"));


    if (gps.time.minute() < 10){ Serial.print(F("0")); file.print(F("0"));}
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    //SD
    file.print(gps.time.minute());
    file.print(F(":"));

    if (gps.time.second() < 10){ Serial.print(F("0")); file.print(F("0"));}
    Serial.print(gps.time.second());
    Serial.println(F("."));
    //SD
    file.print(gps.time.second());
    file.println(F("."));

  }
  else
  {
    Serial.println(F("INVALID"));
    file.println(F("Invalid"));
  }
  file.println();
  file.close();
  Serial.println();

  if (millis() - startTime > interval)
  {
    startTime = millis();
    TransmitData();
    //LoRa
    tone(buzzerPin, 1000, 200);
  }

}

void TransmitData()
{
  LoRa.beginPacket();
  LoRa.print("Location:");
  if (gps.location.isValid())
  {
    LoRa.print(gps.location.lat(), 6);
    LoRa.print(F(", "));
    LoRa.println(gps.location.lng(), 6);
  }
  else
  {
    LoRa.println(F("INVALID"));
  }
  LoRa.print("Temp:");
  LoRa.print(bmp.readTemperature());
  LoRa.println(" C");
  //Pressure
  LoRa.print("Pressure:");
  LoRa.print(bmp.readPressure()/100.0F);
  LoRa.println(" hPa");
  //Altitude
  LoRa.print("Altitude:");
  LoRa.print(bmp.readAltitude(1013.25)); //Standard Pressure at Sea level(hPa)
  LoRa.println("m");

  LoRa.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    LoRa.print(gps.date.month());
    LoRa.print(F("/"));
    LoRa.print(gps.date.day());
    LoRa.print(F("/"));
    LoRa.print(gps.date.year());
  }
  else
  {
    LoRa.print(F("INVALID"));
  }
  LoRa.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10){ LoRa.print(F("0"));}
    LoRa.print(gps.time.hour());
    LoRa.print(F(":"));
    if (gps.time.minute() < 10){ LoRa.print(F("0")); }
    LoRa.print(gps.time.minute());
    LoRa.print(F(":"));
    if (gps.time.second() < 10){ LoRa.print(F("0"));}
    LoRa.print(gps.time.second());
    LoRa.println(F("."));
  }
  else
  {
    LoRa.println(F("INVALID"));
  }
  LoRa.println();

  LoRa.endPacket();

}




