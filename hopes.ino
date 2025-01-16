//Libraries
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <Adafruit_BMP280.h>
#include <SD.h>
#include <LoRa.h>
#include <Servo.h>
#include <SPI.h>

//GPS
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);

// The TinyGPSPlus object
TinyGPSPlus gps;

//BMP280 Object
Adafruit_BMP280 bmp;
#define GND_P 1013.25

//SD
File file;

String bmpData;
String GPSData;
String MagData;

//LoRa

const long frequency = 433E6;
#define LoRaCS 10
#define RST 4
#define DIO0 7

int packetNum = 1;


float tStart = 0;
float tPeriod = 500;

//Buzzer
//#define BUZZ_PIN 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ss.begin(GPSBaud);

  if (!bmp.begin(0x77)) Serial.println("No BMP280"); //BMP initialization Undecided fail condition
  Serial.println("Huh");
  if (!SD.begin(9)) Serial.println("No SD"); //SD Initialization Undecided Fail Condition
  file = SD.open("data.txt", FILE_WRITE);
  
  if (!file) Serial.println("File Opening Failed(setup)"); //Checks for successful opening of SD file
  file.println("Log Begin:"); 
  file.close();

  LoRa.setPins(LoRaCS, RST, DIO0); //Set LoRa pins to correct connections
  if (!LoRa.begin(frequency)) Serial.println("LoRa failed"); //LoRa Initialization Undecided Fail Condition
   
//  initRM3100(); //Initialize Magnetometer Sensor

  tStart = millis();
}

void loop() {
  Serial.println("Started Loop");
  while(ss.available()>0){
    if (gps.encode(ss.read()))
    {
      ParseGPS(true);
    } else
    {
      ParseGPS(false);
    }
    ParseBMP();
    displayData();
    if (millis() - tStart > tPeriod)
    {
      TransmitData();
      tStart = millis();
    }

  }


}

void ParseBMP()
{
  bmpData = ("BMP: " + String(bmp.readTemperature()) + ", " + String(bmp.readPressure()/100.00F) + ", " + String(bmp.readAltitude(GND_P)));
}

void ParseGPS(bool fix)
{

    
  GPSData = ("GPS: " + String(gps.location.lat()) + ", " + String(gps.location.lng()));
    

}

void TransmitData(){
  digitalWrite(LoRaCS, LOW);
  LoRa.beginPacket();
  LoRa.print("Packet #");
  LoRa.println(packetNum);
  LoRa.print(bmpData);
  LoRa.print(GPSData);
  //LoRa.print(MagData);
  LoRa.endPacket();
  digitalWrite(LoRaCS, HIGH);
 
  packetNum++;
}

void displayData(){
  

  digitalWrite(9, LOW);
  file = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (file) {
 
  file.println(bmpData);
  file.println(GPSData);
  //file.print(MagData); 
  file.close();
  // print to the serial port too:

  Serial.println(bmpData);
  Serial.println(GPSData);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  digitalWrite(9, HIGH);
}


/*
void ParseMAG()
{
  if (digitalRead(drdyPin)) {  // Check if data is ready
    digitalWrite(csPin, LOW);

    SPI1.transfer(0xA4); // Address of the X-axis MSB to initiate reading
    long x = SPI1.transfer(0x00);  //first byte
    x = (x << 8) | SPI1.transfer(0x00);  //second byte
    x = (x << 8) | SPI1.transfer(0x00);  //third byte

    long y = SPI1.transfer(0x00);  // Y
    y = (y << 8) | SPI1.transfer(0x00);
    y = (y << 8) | SPI1.transfer(0x00);

    long z = SPI1.transfer(0x00);  // Z
    z = (z << 8) | SPI1.transfer(0x00);
    z = (z << 8) | SPI1.transfer(0x00);

    MagData = "MAG:" + String(x) + "," + String(y) + "," + String(z);

    digitalWrite(csPin, HIGH);

    Serial.println(MagData);
  }
}

void initRM3100() {
  pinMode(csPin, OUTPUT);
  pinMode(drdyPin, INPUT); 

  SPI1.begin();

  // Configure the RM3100
  digitalWrite(csPin, HIGH);
  delay(10);
  digitalWrite(csPin, LOW);
  SPI1.transfer(0x01); // CMM register address
  SPI1.transfer(0x79); //continuous measurement on all axes, no alarm
  digitalWrite(csPin, HIGH);
}
*/
