#include <Arduino.h>
#include <L298NX2.h>
#include <Ultrasonic.h>

#define motorA1 2
#define motorA2 3
#define motorB1 4
#define motorB2 5
#define motorA_EN 6
#define motorB_EN 7

#define MAKERLINE_AN A0
#define MAX_SPEED 255

// PD variables
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

// Set robot features
bool follow_line = 1;
bool avoid_obstacles = 1;
bool comms = 0;

bool leftTrack = 0;

L298NX2 robot(motorA_EN, motorA1, motorA2, motorB_EN, motorB1, motorB2);
Ultrasonic sonarL(33, 32);
Ultrasonic sonarR(35, 34);

void checkObstacle() {
    if (avoid_obstacles) {
        short distL = sonarL.read();
        short distR = sonarR.read();

        if (distL < 20 || distR < 20) {
            leftTrack = 1;
            robot.setSpeed(255);
            if (distL < distR) {
                while (distR < 35) {
                    robot.forwardA();
                    robot.backwardB();
                }
            }
            else if (distL > distR) {
                while (distL < 35) {
                    robot.forwardB();
                    robot.backwardA();
                }
            }
            else {
                robot.backwardFor(400);
                robot.reset();
            }
        }
    }
}

void returnToLine() {
    if (leftTrack) {
        // Check if still on line
        while (analogRead(MAKERLINE_AN) < 51) {
            //Search for line
            robot.setSpeedA(255);
            robot.setSpeedB(160);
            robot.forwardFor(40);
            robot.reset();
        }
        leftTrack = 0;
    }
}

void followLine() {
    currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;

        adcMakerLine = analogRead(MAKERLINE_AN);

        if (adcMakerLine < 51) {
            // Outside line
            robot.setSpeed(0);
        }
        else if (adcMakerLine > 972) { 
            // Detects cross line
            robot.setSpeedA(MAX_SPEED - 25);
            robot.setSpeedB(MAX_SPEED - 25);
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
            robot.forward();
        }
    }
}

void setup() {
    // Place robot at the center of line
    delay(2000);
    adcSetPoint = analogRead(MAKERLINE_AN);
}

void loop() {
    checkObstacle();
    returnToLine();
    followLine();
}
