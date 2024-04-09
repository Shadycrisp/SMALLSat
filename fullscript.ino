
//Libraries
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <Adafruit_BMP280.h>
#include <SD.h>
#include <LoRa.h>
#include <Servo.h>

//GPS
const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
#define TARGET_LAT 4
#define TARGET_LNG 1
float ang;
float tAng;

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
  
  if (!bmp.begin()) Serial.println("No BMP280"); //BMP initialization Undecided fail condition

  servoLeft.attach(LEFT_PIN); //Attaching Left Servo to correct pin
  servoRight.attach(RIGHT_PIN); //Attaching Left Servo to correct pin

  if (!SD.begin(BUILTIN_SDCARD)) Serial.println("No SD"); //SD Initialization Undecided Fail Condition
  file = SD.open("data.txt", FILE_WRITE);
  
  if (!file) Serial.println("File Opening Failed(setup)"); //Checks for successful opening of SD file
  file.println("Log Begin:"); 
  file.close();

  LoRa.setPins(cs, rst, dio0); //Set LoRa pins to correct connections
  if (!LoRa.begin(frequency)) Serial.println("LoRa failed"); //LoRa Initialization Undecided Fail Condition
  

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
  while (Serial1.available() > 0){
    if (gps.encode(Serial1.read())) { //Difference between if and else is the ParseGPS() method(Attempting to parse only when there is valid data coming in)
      ParseGPS(true);
    } else {
      ParseGPS(false);
    }
   
    ParseBMP(); //Parses BMP Data
    //ParseMAG(); //Parses Magnetometer data
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
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
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
  //Serial.println(MagData);
  //file.println(MagData);
  //Save and print flight time Data to Serial and SD card
  Serial.println(FlightTime);
  file.println(FlightTime);
  
  file.println();
  file.close(); //Sclose SD file
  Serial.println();

  if (millis() - startTime > interval){ //Checks if the interval has passed for data transmission
    startTime = millis();
    TransmitData(); //Transmits data through LoRa
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
  //LoRa.println(MagData);
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
  ang = gps.course.deg();
  tAng = gps.courseTo(lat, lng, TARGET_LAT, TARGET_LNG);
  
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
  //Parse magnetometer info...
}
bool LowPower()
{
  if (gps.speed < 2) //Check if the speed of the cansat is lower than 2m/s
  {
    return true;
  } else
  {
    return false;
  }
}
