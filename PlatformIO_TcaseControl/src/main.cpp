#include <Arduino.h>
#include "specifications.h"

/**
 * Runs once at Arduino Startup
*/
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

/**
 * Read position of mode sensor
 */
int ReadPosition() {
  return 1;
}

/**
 * Read position of selector switch
 */ 
int ReadSwitchPosition() {
  //   return 1;
}

/**
 * For correcting drift of shift motor
 */
void ReturnToPosition(int position) {
}

/**
 * Try shift to position
 */
void ShiftToPosition(int position) {
}

/**
 * Either return new valid position or previous position passed in
 */
int UpdateValidPosition(int pastValidPosition) {
  int currentPosition;
  currentPosition = ReadPosition();
  if (0 <= currentPosition <= 3) {
    return currentPosition;
  } else {
    return pastValidPosition;
  }  
}

/**
 * Runs repeatedly after Arduino setup()
 */
void loop() {
  int lastValidPosition; // Last valid measured position of Motor
  int currentPosition;  // Current position of Motor
  int desiredPosition;

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(SW_DEBOUNCE*1000);                       // wait for a second

  currentPosition = ReadPosition();
  desiredPosition = ReadSwitchPosition();

  if (currentPosition != lastValidPosition) {
     ReturnToPosition(lastValidPosition);
 }

  if (currentPosition != desiredPosition) {
     ShiftToPosition(desiredPosition);
 }
}


