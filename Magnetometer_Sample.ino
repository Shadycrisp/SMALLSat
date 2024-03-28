#include <Adafruit_LIS3MDL.h>
#include <Wire.h>

Adafruit_LIS3MDL lis3mdl = Adafruit_LIS3MDL();

void setup() {
  Serial.begin(9600);
  while (!Serial) { delay(10); } // Wait for serial monitor
  
  Serial.println("LIS3MDL test");

  if (!lis3mdl.begin_I2C()) { // or use begin_SPI(cs_pin);
    Serial.println("Could not find a valid LIS3MDL sensor, check wiring!");
    while (1);
  }
  // Set up the sensor (optional)
  lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);  
}

void loop() {
  sensors_event_t event; 
  lis3mdl.getEvent(&event);

  Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print(" ");
  Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print(" ");
  Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.println(" uT");
  
  delay(500);
}
