#include <L298NX2.h>

#define motorA1 2
#define motorA2 3
#define motorB1 4
#define motorB2 5
#define motorA_EN 6
#define motorB_EN 7

#define rxPin 8

#define MAKERLINE_AN  A0
#define MAX_SPEED 255

// PD control variables
int adcMakerLine = 0;
int adcSetPoint = 0;
int proportional = 0;
int lastProportional = 0;
int derivative = 0;
int powerDifference = 0;
int motorLeft = 0;
int motorRight = 0;
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
const int interval = 10;


L298NX2 robot(motorA_EN, motorA1, motorA2, motorB_EN, motorB1, motorB2); //initialize motor driver

void setup () {
   delay(2000);
   // Place robot at the center of line
  adcSetPoint = analogRead(MAKERLINE_AN);
  pinMode(rxPin, INPUT_PULLUP);
}

void loop()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    
    adcMakerLine = analogRead(MAKERLINE_AN);
    
    if (adcMakerLine < 51) { // Out of line
      robot.setSpeed(0);
    }
    else if (adcMakerLine > 972) { // Detects cross line
      robot.setSpeedA(MAX_SPEED-25);
      robot.setSpeedB(MAX_SPEED-25);
    }
    else {
      proportional = adcMakerLine - adcSetPoint;
      derivative = proportional - lastProportional;
      lastProportional = proportional;
  
      powerDifference = (proportional * 1.5) + (derivative * 5);
  
      if (powerDifference > MAX_SPEED) {
        powerDifference = MAX_SPEED;
      }
      if (powerDifference < -MAX_SPEED) {
        powerDifference = -MAX_SPEED;
      }
  
      if (powerDifference < 0) {
        motorLeft = MAX_SPEED + powerDifference;
        motorRight = MAX_SPEED;
      }
      else {
        motorLeft = MAX_SPEED;
        motorRight = MAX_SPEED - powerDifference;
      }

      robot.setSpeedA(motorLeft);
      robot.setSpeedB(motorRight);

      bool halt = digitalRead(rxPin);
      if (halt) {
        robot.stop();
      }
      else {
        robot.forward(); 
      }
    }
  }
}