
//Libraries
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <Adafruit_BMP280.h>
#include <SD.h>
#include <LoRa.h>
#include <Servo.h> // This doesnt work with teensy 4.1
#include <Wire.h>

//MAG
const uint8_t RM3100_Address = 0x20; //either 20 or 21 or 22

//GPS
const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
#define TARGET_LAT 4
#define TARGET_LNG 1
float ang; //Heading (Angle)
float tAng; //Target (Taget Angle)

//LoRa
const int frequency = 433E6;
const uint32_t cs = 10;
const uint32_t rst = 9;
const uint32_t dio0 = 13;
int packetNum = 1;

//BMP280 Object
Adafruit_BMP280 bmp;
float alt;
#define GND_P 1013.25

//Servos
Servo servoLeft;
Servo servoRight;
#define LEFT_PIN 7
#define RIGHT_PIN 8

//SD 
File file;
const uint32_t buzzerPin = 2;

//Buzzer
float buzzerTimer = 200;
float buzz = false;
#define BUZZ_PIN 4

float startTime = 0;
float interval = 500; //Frequency of transmittion. Currently 2Hz
float CoursePeriod = 500; //Frequency of course correction. Currently 2Hz





String bmpData;
String GPSData;
String MagData;
String FlightTime;

bool lowPower = false;


void setup()
{
  Serial.begin(115200);
  Serial1.begin(GPSBaud);
  
  if (!bmp.begin(0x77)) Serial.println("No BMP280"); //BMP initialization Undecided fail condition

  servoLeft.attach(LEFT_PIN); //Attaching Left Servo to correct pin
  servoRight.attach(RIGHT_PIN); //Attaching Left Servo to correct pin

  if (!SD.begin(BUILTIN_SDCARD)) Serial.println("No SD"); //SD Initialization Undecided Fail Condition
  file = SD.open("data.txt", FILE_WRITE);
  
  if (!file) Serial.println("File Opening Failed(setup)"); //Checks for successful opening of SD file
  file.println("Log Begin:"); 
  file.close();

  LoRa.setPins(cs, rst, dio0); //Set LoRa pins to correct connections
  if (!LoRa.begin(frequency)) Serial.println("LoRa failed"); //LoRa Initialization Undecided Fail Condition
  
  Wire.begin();
  if (initRM3100()) {
    Serial.println("Mag works");
  } else {
    Serial.println("Magnetometer initialization failed.");
    while(1);
  }

  startTime = millis();
}

void loop()
{
   
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial1.available() > 0){
    if (gps.encode(Serial1.read())) { //Difference between if and else is the ParseGPS() method(Attempting to parse only when there is valid data coming in)
      ParseGPS(true);
    } else {
      ParseGPS(false);
    }
   
    ParseBMP(); //Parses BMP Data
    ParseMAG(); //Parses Magnetometer data
    displayInfo(); //Displays all info to serial/saves data to SD
    if (!lowPower) //If the cansat is not in low power mode, course correcting adjustments can still be made, otherwise the servos are detached to save power.
    {
      if (millis() - startTime > CoursePeriod) { //Course corrects at the set frequency.
        CourseCorrect(ang, tAng);
        startTime = millis();
      } 
    } else {
      servoLeft.detach();
      servoRight.detach();
    }
    
    lowPower = LowPower(); //Checks if the cansat is stationary.

  } 
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("GPS is offline :/");
    ParseBMP();
    ParseMAG();
    displayInfo();
    Serial.println("Course Correct Failure(Unable to get location data)");
    servoLeft.detach();
    servoRight.detach();
  }
}


void displayInfo()
{
  //float flat, flon, deg;
  file = SD.open("data.txt", FILE_WRITE);
  if (!file) Serial.println("File Opening Failed(loop)"); //Open file for writing

  //Save and print GPS Data to Serial and SD card
  Serial.println(GPSData);
  file.println(GPSData);
  //Save and print BMP280 Data to Serial and SD card
  Serial.println(bmpData);
  file.println(bmpData);
  //Save and print Magnetometer Data to Serial and SD card
  Serial.println(MagData);
  file.println(MagData);
  //Save and print flight time Data to Serial and SD card
  Serial.println(FlightTime);
  file.println(FlightTime);
  
  file.println();
  file.close(); //Sclose SD file
  Serial.println();
  /*
  if (lowPower && millis() - startTime > buzzerTimer)
  {
    if (!buzz)
    {
      tone(BUZZ_PIN,1000);
      buzz = true;
    } else
    {
      noTone(BUZZ_PIN);
      buzz = false;
    }
  }
  */




  if (millis() - startTime > interval){ //Checks if the interval has passed for data transmission
    startTime = millis();
    TransmitData(); //Transmits data through LoRa
    tone(BUZZ_PIN, 1000, 200);
  }


}
 //tone(buzzerPin, 1000, 200);
void TransmitData()
{
  LoRa.beginPacket();
  LoRa.print("P#");
  LoRa.println(packetNum);
  LoRa.println(GPSData);
  LoRa.println(bmpData);
  LoRa.println(MagData);
  LoRa.print(FlightTime);
  LoRa.println();
  LoRa.endPacket();
  packetNum++;
}

void CourseCorrect(int heading, int targethdg)
{
  int courseChange = (int)(360 + targethdg - heading) % 360; //Adds 360 to the target  heading then subtracts the heading(To make sure the value is positive). The % 360 handles rotations above 360 degrees
  //Finds least positive coterminal angle 

  if (courseChange >= 345 || courseChange < 15) //If the correction needed is less than 15 degrees, the Cansat goes straight
  {
    servoLeft.write(0);
    servoRight.write(0);
  }
   else if (courseChange < 345 && courseChange > 180) // If the correction needed is between 180 and 345, turning right would be faster(Turns right)
  {
    //Turn Right
    Serial.println("Turning Right");
    servoRight.write(-20);
  }
  else if (courseChange > 15 && courseChange <= 180) //If the correction needed is between 15 and 180, the shortest turn is left(Turns Left)
  {
    Serial.println("Turning Left");
    servoLeft.write(-20);
  }
  
  CourseCorrectTime(alt);
}
//Might be unessasary
void CourseCorrectTime(float _alt)
{
  if (_alt < 350) //increases the Frequency of course correct under a certain altitude(Currently 350m) 
  {
    CoursePeriod = 200; //Increases frequency to 5Hz
  }
}


void ParseBMP()
{
  alt = bmp.readAltitude(GND_P);
  bmpData = ("BMP: " + String(bmp.readTemperature()) + ", " + String(bmp.readPressure()/100.00F) + ", " + String(alt));
}

void ParseGPS(bool fix)
{
  float lat = gps.location.lat();
  float lng = gps.location.lng();
  ang = gps.course.deg(); //Heading
  tAng = gps.courseTo(lat, lng, TARGET_LAT, TARGET_LNG); //Target
  
  if (fix){
    GPSData = ("GPS: " + String(lat) + ", " + String(lng) + ", " + String(ang) + ", " + String(tAng));
  } else{
    GPSData = "GPS: INV, INV, INV, INV";
  }
  
}

void ParseFlightTime(){

  int hour = gps.time.hour();
  int minute = gps.time.minute();
  int second = gps.time.second();
  FlightTime = ("TIME: " + String(hour) + ":" + String(minute) + ":" + String(second));

}

void ParseMAG()
{
  float x, y, z;
  // Assuming data is ready, read from the data registers (24h to 2Ch for X, Y, Z axes)
  Wire.beginTransmission((uint8_t)RM3100_Address);
  Wire.write(0xA4); // Starting address for X-axis data
  Wire.endTransmission();
  Serial.println(Wire.requestFrom(RM3100_Address,(uint8_t) 9)); // Request 9 bytes for X, Y, Z, 
  
  if(Wire.available() == (uint8_t)9) {
    // Convert readings from 2's complement to integer, then to nT
    float sensitivity = 13; // Sensitivity in nT, calculated from cycle count
    
    x = convert(Wire.read(), Wire.read(), Wire.read()) * sensitivity; //3 bytes
    y = convert(Wire.read(), Wire.read(), Wire.read()) * sensitivity; //3 bytes
    z = convert(Wire.read(), Wire.read(), Wire.read()) * sensitivity; //3 bytes
    MagData = ("MAG:" + String(x) + "," + String(y) + "," + String(z));
  }else{
    MagData = "MAG:0,0,0";
    Serial.println("mag data not 9bytes");
  }
}

bool LowPower()
{
  if (gps.speed.value() < 2) //Check if the speed of the cansat is lower than 2m/s
  {
    return true;
  } else
  {
    return false;
  }
}

 //Initializing MAG:

bool initRM3100() {
 
  // Initiate and verify Continuous Measurement Mode

  uint8_t regAddress = 0x00;
  uint8_t valueToSet = 0x79;
  uint8_t isCMMRegister = 0x01;

  Wire.beginTransmission((uint8_t)RM3100_Address);
  Wire.write(regAddress);
  Wire.write(valueToSet);
  if (Wire.endTransmission() == 0) return false; // Check if transmission was successful

  // For CMM register, we skip read-back verification as its value might change due to its operation
  if (isCMMRegister) return true;

  // Read back and verify the value for non-CMM registers
  Wire.beginTransmission(RM3100_Address);
  Wire.write(regAddress);
  if (Wire.endTransmission() == 0) return false; // Restart condition for repeated start
  
  Wire.requestFrom(RM3100_Address,(uint8_t) 1);
  if (Wire.available()) {
    uint8_t readValue = Wire.read();
    if (readValue != valueToSet) return false;
  } else {
    return false; // No data received
  }
  return true; // If all verifications passed
}

int32_t convert(uint8_t msb, uint8_t mid, uint8_t lsb) {
  // Convert 3-byte 2's complement to signed 32-bit integer
  int32_t value = ((int32_t)msb << 16) | ((int32_t)mid << 8) | lsb;
  if (value & 0x00800000) { // Check if the value is negative
    value |= 0xFF000000; // Extend the sign bit
  }
  return value;
}
