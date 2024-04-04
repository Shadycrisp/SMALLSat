#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <Adafruit_BMP280.h>
#include <SD.h>
#include <LoRa.h>
#include <Servo.h>

//GPS
const int SL = 7, SR = 8;
const uint32_t GPSBaud = 9600;
//LoRa
const int frequency = 433E6;
const uint32_t csPin = 10;
const uint32_t rstPin = 9;
// The TinyGPSPlus object
TinyGPSPlus gps;
//BMP280 Object
Adafruit_BMP280 bmp;

Servo servoLeft;
Servo servoRight;

//SD File
File file;
const uint32_t buzzerPin = 2;

float startTime = 0;
float interval = 500;
float CoursePeriod = 1500;

float preAltitude;
bool launch = false;

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
 preAltitude = bmp.readAltitude(1013.25);
  servoLeft.attach(SL);
  servoLeft.attach(SR);


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
  if (bmp.readAltitude(1013.25) > 650)
  {
    CoursePeriod = 1500;
  } else if (bmp.readAltitude(1013.25) > 400)
  {
    CoursePeriod = 1000;
  } else
  {
    CoursePeriod = 500;
  }





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
    
  }
  if (millis() - startTime > CoursePeriod && !LowPower())
  {
    Serial.println("Heading:");
    Serial.println(gps.course.deg());
    Serial.println("Target:");
    Serial.println(gps.course.deg());
    CourseCorrect(gps.course.deg());
  }
  if (LowPower())
  {
    if (launch)
    {
      tone(buzzerPin, 1000, 200);
    }
    servoLeft.detach();
    servoRight.detach();
  } else
  {
    if (!servoLeft.attached() && !servoRight.attached())
    {
      servoLeft.attach(SL);
      servoRight.attach(SR);
    }
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

void CourseCorrect(float heading)
{
  double courseToDest = gps.courseTo(gps.location.lat(), gps.location.lng(), 48.8584, 2.2945);
  int courseChange = (int)(360 + courseToDest - heading) % 360;

  if (courseChange >= 345 || courseChange < 15)
  {
    servoLeft.write(0);
    servoRight.write(0);
  }
   else if (courseChange < 345 && courseChange > 180)
  {
    //Turn Right
    Serial.println("Turning Right");
    servoRight.write(-20);
  }
  else if (courseChange > 15 && courseChange <= 180)
  {
    Serial.println("Turning Left");
    servoLeft.write(-20);
  }
  

}

bool LowPower()
{
  if (bmp.readAltitude(1013.25) > preAltitude + 5 || bmp.readAltitude(1013.25) < preAltitude - 5)
  {
    launch = true;
    preAltitude = bmp.readAltitude(1013.25);
    return false;
  }
  preAltitude = bmp.readAltitude(1013.25);

  return true;
}
