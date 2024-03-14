#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <LoRa.h>
#include <SPI.h>
#include <TinyGPSPlus.h>
#include <SD.h>
#include <SoftwareSerial.h>


#define gpsSerial Serial1

// Define the LoRa frequency
const long frequency = 433E6; // Adjust according to your region's regulations


// Initialize the BMP280 sensor
Adafruit_BMP280 bmp;


// Initialize GPS
TinyGPSPlus gps;
static const int RX = 0, TX = 1;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RX, TX);

// Setup LoRa with the specific CS and reset pins if needed
const int csPin = 10; // Change as per your LoRa module if not using the built-in SPI
const int resetPin = 9; // Set to your LoRa module's reset pin if any, or -1 if unused


const int buzzerPin = 2; // Buzzer connected to pin 2


File file; // Declare a File object for SD card operations


void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);
  



  // BMP280 initialization
  if (!bmp.begin()) {
  Serial.println("Could not find a valid BMP280 sensor!");
  while (1);
  }


  // SD initialization using the built-in SD card slot
  if (!SD.begin(BUILTIN_SDCARD)) {
  Serial.println("SD Card Mount Failed");
  return;
  }


  // Create or open the file for logging
  file = SD.open("data.txt", FILE_WRITE);
  if (!file) {
  Serial.println("Failed to open file for writing");
  return;
  }
  file.println("Log Begin:");
  file.close();


  // LoRa initialization
  SPI.begin();
  LoRa.setPins(csPin, resetPin, 13); // Set LoRa module pins
  if (!LoRa.begin(frequency)) {
  Serial.println("Starting LoRa failed!");
  while (1);
  }


  pinMode(buzzerPin, OUTPUT); // Initialize the buzzer pin as an output
}


void loop() {
  
  logData();
  Serial.print("Chars:");
  Serial.println(gps.charsProcessed(), 6);
  Serial.print("Sent:");
  Serial.println(gps.sentencesWithFix(), 10);
  Serial.print("Err:");
  Serial.println(gps.failedChecksum(), 9);

  Serial.print("Temperature: ");
  Serial.print(bmp.readTemperature());
  Serial.println(" C");
  Serial.print("Pressure: ");
  Serial.print(bmp.readPressure() / 100.0F);
  Serial.println(" hPa");
  Serial.print("Approx. Altitude: ");
  Serial.print(bmp.readAltitude(1013.25)); // Standard sea-level pressure in hPa
  Serial.println(" m");
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);
  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);
  Serial.print(", Degrees: ");
  Serial.println(gps.course.deg(), 2);
     
  delay(1000); // Adjust delay as needed
  }






void logData() {
  // Open the file for appending
  file = SD.open("data.txt", FILE_WRITE);
  if (!file) {
  Serial.println("Failed to open file for appending");
  return;
  }
  // Log temperature, pressure, and altitude from BMP280
  file.print("Temperature: ");
  file.print(bmp.readTemperature());
  file.println(" C");


  file.print("Pressure: ");
  file.print(bmp.readPressure() / 100.0F);
  file.println(" hPa");


  file.print("Approx. Altitude: ");
  file.print(bmp.readAltitude(1013.25)); // Standard sea-level pressure in hPa
  file.println(" m");


  // Log GPS data
  file.print("Latitude: ");
  file.println(gps.location.lat(), 6);
  file.print("Longitude: ");
  file.println(gps.location.lng(), 6);
  file.print(", Degrees: ");
  file.println(gps.course.deg(), 2);
  file.close();


  // Send data over LoRa
  LoRa.beginPacket();
  LoRa.print("Temperature: ");
  LoRa.print(bmp.readTemperature());
  LoRa.print(", Pressure: ");
  LoRa.print(bmp.readPressure() / 100.0F);
  LoRa.print(", Altitude: ");
  LoRa.print(bmp.readAltitude(1013.25));
  LoRa.print(", Latitude: ");
  LoRa.print(gps.location.lat(), 6);
  LoRa.print(", Longitude: ");
  LoRa.print(gps.location.lng(), 6);
  LoRa.print(", Degrees: ");
  LoRa.print(gps.course.deg(), 2);
  LoRa.endPacket();


  //Debugging
  


  // Close the file


  // Beep for 200 milliseconds at 1000 Hz
  tone(buzzerPin, 1000, 200);
}





