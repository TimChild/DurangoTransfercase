#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "specifications.h"
#include "motor.h"
#include "switch.h"
#include "output.h"

// #define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

////// FOR LCD 
// const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; 
// const byte switchModePin = A0;
// const byte motorPWMpin = 9;
// const byte motorDirPin = 8;
// const byte brakeReleasePin = 7;
// const byte motorModePin = A1;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/////// FOR TFT
const byte TFT_CS = 10, TFT_DC = 9, TFT_RST = 8; 
const uint8_t switchModePin = A0;
const byte motorPWMpin = 3;
const byte motorDirPin = 2;
const byte brakeReleasePin = 4;
const uint8_t motorModePin = A1;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

int SWITCH_FIXED_RESISTOR = 4555;  // Resistance of fixed resistor for detecing mode select resistance in ohms

OtherOutputs output = OtherOutputs(&tft);
SelectorSwitch selector = SelectorSwitch(switchModePin, &output, SWITCH_FIXED_RESISTOR);
Motor motor = Motor(motorPWMpin, motorDirPin, brakeReleasePin, motorModePin, &output);
int currentPosition = -1;  // Current position of Motor
byte desiredPosition = 1;
  
void bootTest() {
  // TODO: Remove this fn, this is just for making sure scripts start running etc
  pinMode(LED_BUILTIN, OUTPUT);
  for (byte i = 0; i < 15; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }
}


/**
 * Runs once at Arduino Startup
*/
void setup() {
  //////////////////////// DEBUG
  Serial.begin(115200); 
  DEBUG_PRINTLN(F("Main: Booting"));
  //////////////////////// End of DEBUG

  output.begin();
  delay(2000);
  motor.begin();
  if (motor.getPosition() == 2)
  {
    selector.begin(1);
  } else {
    selector.begin(0);
  }
}



void testSwitch() {
  desiredPosition = selector.getSelection();
  delay(1000); 
}


void normal() {
  DEBUG_PRINTLN(F("Main: Starting Loop"));  // Debugging

  selector.checkState(); // This should run frequently to check current switch position (i.e. for debouce purposes)
  desiredPosition = selector.getSelection();
  currentPosition = motor.getPosition();

  DEBUG_PRINTLN(desiredPosition);
  DEBUG_PRINTLN(currentPosition);

  if (currentPosition != desiredPosition) {
    // motor.attemptShift(desiredPosition, MAX_SINGLE_SHIFT_ATTEMPTS);
    motor.attemptShift(desiredPosition, 1);
    if (motor.getPosition() != desiredPosition) {
      // TODO: Think about what to do here if either shift failed, or potentially in bad state?

      DEBUG_PRINTLN(F("Main: Failed to reach position"));
    }
  }
  selector.checkState();

}


/**
 * Runs repeatedly after Arduino setup()
 */
void loop() {
  testSwitch();
}




