#include <Arduino.h>
#include "specifications.h"
#include "motor.h"
#include "switch.h"
#include "output.h"


/**
 * Runs once at Arduino Startup
*/
void setup() {
  OtherOutputs output;
  
  pinMode(LED_BUILTIN, OUTPUT);
  output.setMainMessage("Running Setup");
  // TODO: What is going to happen to the output message after setup() ends? Does it matter?
}


/**
 * Runs repeatedly after Arduino setup()
 */
void loop() {
  int currentPosition;  // Current position of Motor
  int desiredPosition;

  SelectorSwitch selector;
  Motor motor;
  OtherOutputs output;

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(SW_DEBOUNCE*1000);                       // wait for a second

  desiredPosition = selector.getSelection();
  currentPosition = motor.getPosition();
  if (currentPosition != desiredPosition) {
    motor.attemptShift(desiredPosition);
  }
  selector.checkState();

}




