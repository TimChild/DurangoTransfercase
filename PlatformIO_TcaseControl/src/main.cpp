#include <Arduino.h>
#include "specifications.h"
#include "motor.h"
#include "switch.h"
#include "output.h"

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; //TODO: sort out these pins
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
OtherOutputs output = OtherOutputs(lcd);
SelectorSwitch selector = SelectorSwitch(output);
Motor motor = Motor(output);
int currentPosition;  // Current position of Motor
int desiredPosition;
  
/**
 * Runs once at Arduino Startup
*/
void setup() {
  output.setMainMessage("Running Setup");
  delay(500);
  output.setMainMessage("Done");
}


/**
 * Runs repeatedly after Arduino setup()
 */
void loop() {
  desiredPosition = selector.getSelection();
  currentPosition = motor.getPosition();
  if (currentPosition != desiredPosition) {
    motor.attemptShift(desiredPosition, MAX_SINGLE_SHIFT_ATTEMPTS);
    if (motor.getPosition() != desiredPosition) {
      // TODO: Think about what to do here if either shift failed, or potentially in bad state?
    }
  }
  selector.checkState();
}




