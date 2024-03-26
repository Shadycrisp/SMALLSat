#include <Wire.h>
#include <Adafruit_Sensor.h>
//#include <Adafruit_HMC5883_U.h>
//#include <TinyGPSPlus.h>
#include <Servo.h>
//Look at TinyGPS++ courseTo() and distance between, maybe we avoidthe other sensor

//Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
//TinyGPSPlus gps;
Servo servo1;
Servo servo2;
float targetHeading = 0;
float alt = 1000;
float period = 5000;
float currentMillis;
float startMillis;
float change = 5;
float heading = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  servo1.attach(7);
  servo2.attach(8);
  
/*
  if (!mag.begin())
  {
    Serial.println("oopsies");
    while(1);
  }
  */
 
  
  startMillis = millis();
}

void loop() {
 


  currentMillis = millis();
  if (alt <= 50)
  {
    period = 500;
  } else if (alt < 200)
  {
    period = 1200;
  } else if (alt < 500)
  {
    period = 3500;
  }



  if (currentMillis - startMillis >= period)
  {
    servo1.write(0);
    servo2.write(0);
    
    //We NEED to find a way to get the target heading(Probably vector subtraction)
    targetHeading = 85;

    if (heading > targetHeading)
    {
      heading = -(360-heading);
    }

    if (targetHeading < heading + 180)
    {
      Left(abs(targetHeading - heading));
    }  else if (targetHeading > heading + 180)
    {
      Right(abs(targetHeading - heading));
    }
    startMillis = currentMillis;
  }
  


}






float GetHeading()
{
  /*
  sensors_event_t event;
  mag.getEvent(&event);

  float heading = atan2(event.magnetic.y, event.magnetic.x);

  float declination = 0.92;
  heading+= declination;

  //Correct for reversed signals
  if (heading < 0)
  {
    heading += 2*PI;
  }

  //Check for angle over 360
  if (heading > 2*PI)
  {
    heading -= 2*PI;
  }
  float headingDeg = heading * 180/M_PI;
  */
  float headingDeg = heading +change;
  return headingDeg;

}

void Left(float _degrees)
{
  //Reset Positions
  servo1.write(0);
  servo2.write(0);
  //Left Steering
  //This code assumes that servo one is on the left, and servo 2
  //on the right
  servo1.write(90);
  /*
  while (abs(targetHeading - GetHeading()) > 0 )
  {
    Serial.println("Steering Left...");
    Serial.println("Heading:" );
    Serial.println(heading);
    delay(500);
  }
  */
  delay(1000);
  Serial.println("Left");
  heading += 20;
  Serial.println(heading);
  //Reset Positions
  servo1.write(0);
  servo2.write(0);

}

void Right(float _degrees)
{
  //Reset Positions
  servo1.write(0);
  servo2.write(0);
  //Left Steering
  //This code assumes that servo one is on the left, and servo 2
  //on the right
  servo2.write(90);
  /*
  while (abs(targetHeading - GetHeading()) > 0)
  {
    Serial.println("Steering Right...");
    Serial.println("Heading:");
    Serial.println(heading);
    delay(500);
  }
  */
  Serial.println("Right");
  delay(1000);
  heading += 20;
  Serial.println(heading);
  //Reset Positions
  servo1.write(0);
  servo2.write(0);

}

