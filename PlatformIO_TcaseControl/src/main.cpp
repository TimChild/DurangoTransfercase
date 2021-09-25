#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "specifications.h"
#include "motor.h"
#include "switch.h"
#include "output.h"

#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

// const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; // For LCD
// const byte switchModePin = A0;
// const byte motorPWMpin = 9;
// const byte motorDirPin = 8;
// const byte brakeReleasePin = 7;
// const byte motorModePin = A1;

const byte TFT_CS = 10, TFT_DC = 9, TFT_RST = 8; // For TFT


const uint8_t switchModePin = A0;
const byte motorPWMpin = 3;
const byte motorDirPin = 2;
const byte brakeReleasePin = 4;
const uint8_t motorModePin = A1;

// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

OtherOutputs output = OtherOutputs(&tft);
// OtherOutputs output = OtherOutputs();
SelectorSwitch selector = SelectorSwitch(switchModePin, &output);
Motor motor = Motor(motorPWMpin, motorDirPin, brakeReleasePin, motorModePin, &output);
int currentPosition = -1;  // Current position of Motor
byte desiredPosition = 1;
  

void bootTest() {
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
  SPI.begin();
  bootTest();
  // output.setTFT(&tft);
  output.begin();
  #ifdef DEBUG
    Serial.begin(115200); // DEBUGGING
  #endif

  DEBUG_PRINTLN(F("Main: Booting"));

  // output.setLcd(lcd); // Just makes a copy of LCD, not actually using the same object, but that's fine.
  output.setMainMessage(F("Booting"));

  selector.setup();
  // selector.setOutput(&output);  // Pass the address of output so that it is the same object everywhere
  // motor.setOutput(&output);
  output.setMainMessage("");
}


/**
 * Runs repeatedly after Arduino setup()
 */
void loop() {
  // bootTest();
  DEBUG_PRINTLN(F("Main: Starting Loop"));  // Debugging

  selector.checkState(); // This should run frequently to check current switch position
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

  // delay(5000); // DEBUGGING

  // motor.testMotorBackward(1000);
  // motor.testBrake(1000);
  // delay(1000);
  // motor.testMotorForward(1000);
  // delay(1000);
  // motor.testMotorBackward(1000);
  // delay(1000);
}




