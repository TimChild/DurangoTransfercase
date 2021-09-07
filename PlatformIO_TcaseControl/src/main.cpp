#include <Arduino.h>
#include "specifications.h"
#include "motor.h"
#include "switch.h"
#include "output.h"

char buffer[100];  // DEBUGGING: String buffer for serial prints to avoid using String

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; //TODO: sort out these pins
const int switchModePin = A0;
const int motorPWMpin = 9;
const int motorDirPin = 8;
const int brakeReleasePin = 7;
const int motorModePin = A1;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
OtherOutputs output = OtherOutputs();
SelectorSwitch selector = SelectorSwitch(switchModePin, &output);
Motor motor = Motor(motorPWMpin, motorDirPin, brakeReleasePin, motorModePin, &output);
int currentPosition = -1;  // Current position of Motor
int desiredPosition = 1;
  

/**
 * Runs once at Arduino Startup
*/
void setup() {
  Serial.begin(115200); // DEBUGGING

  snprintf(buffer, sizeof(buffer), "Main: Booting"); Serial.println(buffer);  // DEBUGGING

  output.setLcd(lcd); // Just makes a copy of LCD, not actually using the same object, but that's fine.
  output.setMainMessage("Booting");

  selector.setup();

  // selector.setOutput(&output);  // Pass the address of output so that it is the same object everywhere
  // motor.setOutput(&output);
  output.setMainMessage("");
}


/**
 * Runs repeatedly after Arduino setup()
 */
void loop() {
  // motor.testMotorBackward(1000);
  snprintf(buffer, sizeof(buffer), "Main: Starting Loop"); Serial.println(buffer);  // DEBUGGING

  selector.checkState(); // This should run frequently to check current switch position

  desiredPosition = selector.getSelection();
  currentPosition = motor.getPosition();
  snprintf(buffer, sizeof(buffer), "Main: desiredPosition = %i, currentPosition = %i", desiredPosition, currentPosition); Serial.println(buffer);  // DEBUGGING
  if (currentPosition != desiredPosition) {
    // motor.attemptShift(desiredPosition, MAX_SINGLE_SHIFT_ATTEMPTS);
    motor.attemptShift(desiredPosition, 1);
    if (motor.getPosition() != desiredPosition) {
      // TODO: Think about what to do here if either shift failed, or potentially in bad state?
      Serial.println(F("Main: Failed to reach position"));
    }
  }
  // selector.checkState();

  // delay(5000); // DEBUGGING

  // motor.testBrake(1000);
  // delay(1000);
  // motor.testMotorForward(1000);
  // delay(1000);
  // motor.testMotorBackward(1000);
  // delay(1000);
}




