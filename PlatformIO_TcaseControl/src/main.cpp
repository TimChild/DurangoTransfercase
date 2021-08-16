#include <Arduino.h>
#include "specifications.h"
#include "motor.h"
#include "switch.h"
#include "output.h"


OtherOutputs output = OtherOutputs();
SelectorSwitch selector = SelectorSwitch(output);
Motor motor = Motor(output);
int currentPosition;  // Current position of Motor
int desiredPosition;
  
/**
 * Runs once at Arduino Startup
*/
void setup() {
  output.setMainMessage("Running Setup");
  pinMode(LED_BUILTIN, OUTPUT);
  
  delay(500);
  output.setMainMessage("Done");
}


/**
 * Runs repeatedly after Arduino setup()
 */
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW

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




