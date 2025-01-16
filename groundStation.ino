#include <SPI.h>
#include <LoRa.h>
#include <SD.h>

//LoRa
#define LORA_CS 0 //NEED TO SET
#define LORA_RST 3 //NEED TO SET
#define LORA_DIO0 5 //NEED TO SET

//SD
#define SD_CS 4 //NEED TO SET
File dataFile;
bool packetAv = false;

float packetCount = 0;
String packet;
void setup() {

Serial.begin(115200);


Serial.println("--------------------------------------------------------------------");
Serial.println("SMALL Sat Ground Station(Mission Control)");
Serial.println("--------------------------------------------------------------------");
Serial.println("");
delay(500);

digitalWrite(SD_CS, HIGH); //Disconnect
digitalWrite(LORA_CS, LOW); 
LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
LoRa.setSPIFrequency(5); //NEED TO RESEARCH

if (!LoRa.begin(433E6)) Serial.println("Starting LoRa failed!"); //initialize LoRa
if (!SD.begin(SD_CS)) Serial.println("SD failed to initialize!"); //INitialize SD

}


void loop() {
  FindData(); //Looks for data/parses data
  if (packetAv) SaveData(); //Stores recieived packekts to SD(if available).


  
  
}

void FindData(){
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    packetCount++;
    Serial.print("RP#");
    Serial.print(packetCount);
    Serial.print(":");
    while(LoRa.available())
    {
      Serial.print((char)LoRa.read());
      packet += (char)LoRa.read();
    } 
    packetAv = true;
  }
}
void SaveData(){
  digitalWrite(SD_CS, LOW);//Activate SD
  digitalWrite(LORA_CS, HIGH);
  dataFile = SD.open("groundStation.txt", FILE_WRITE);
  if(dataFile){
    Serial.println("Writing to SD...");
    dataFile.println(packet);
    dataFile.close();
    Serial.println("Saved.");
  }
  else {
    Serial.println("Failed to open groundStation.txt");
  }
  packetAv = false;
  digitalWrite(SD_CS, HIGH);//disable SD
  ditialWrite(LORA_CS, LOW);
}



