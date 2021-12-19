#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
// #include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
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
const uint8_t TFT_CS = 10, TFT_DC = 9, TFT_RST = 8; 
const uint8_t switchModePin = A0;
const uint8_t motorPWMpin = 3;
const uint8_t motorDirPin = 2;
const uint8_t brakeReleasePin = 4;
const uint8_t fakeSwitchPin = 5;
const uint8_t fakeMotorPin = 6;
const uint8_t motorModePin = A1;
const uint8_t backLightPin = 7;
// Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


// int SWITCH_FIXED_RESISTOR = 4555;  // Resistance of fixed resistor for detecing mode select resistance in ohms
int SWITCH_FIXED_RESISTOR = 4620;  // Resistance of fixed resistor for detecing mode select resistance in ohms


OtherOutputs output = OtherOutputs(&tft, fakeSwitchPin, fakeMotorPin);  // TODO: Add backLightPin and some backlight control
SelectorSwitch selector = SelectorSwitch(switchModePin, &output, SWITCH_FIXED_RESISTOR);
// Motor motor = Motor(motorPWMpin, motorDirPin, brakeReleasePin, motorModePin, vOutRead, &output);
Motor motor = Motor(motorPWMpin, motorDirPin, brakeReleasePin, motorModePin, &output);
int currentPosition = -1;  // Current position of Motor
byte desiredPosition = 1;
  
void blink() {
  pinMode(LED_BUILTIN, OUTPUT);
  for (byte i = 0; i < 15; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }
}

void waitUntilLongNpress() {
  output.setMainMessage(F("WARNING: Motor position is not valid. Hold N for 5s to reset"));
  selector.waitForLongNpress(5.0);
  output.setMainMessage(F(""));
}

void waitUntilReset() {
  output.setMainMessage(F("State requires reset: Put switch in motor position"));
  while (selector.getSelection() != motor.getValidPosition()) {
    delay(10);
  }
  if (!isValid(motor.getPosition())) { 
    waitUntilLongNpress();
  }
  output.setMainMessage(F("Reset successful"));
  delay(1000);
  output.setMainMessage(F(""));
}

int analogDisconected(const uint8_t pin) {
  int disconnected = 0;
  pinMode(pin, INPUT_PULLUP);
  delay(5);
  analogRead(pin);  // Apparently first few reads after switching mode can be bad
  analogRead(pin);
  analogRead(pin);
  if (analogRead(pin) > 1000) {
    disconnected = 1;
  }
  pinMode(pin, INPUT);
  delay(5);
  analogRead(pin);  // Apparently first few reads after switching mode can be bad
  analogRead(pin);
  analogRead(pin);
  return disconnected;
}


void normal_setup() {
  #ifdef DEBUG
    Serial.begin(115200); 
    DEBUG_PRINTLN(F("Main: Booting"));
  #endif
  randomSeed(analogRead(A5));  // Makes random() change between boots
  output.begin();
  delay(1000); // Some time for output bootup display to show
  motor.begin();

  if (analogDisconected(motorModePin)) {
    output.setMainMessage(F("Motor Disconnected"));
    delay(2000);
    while (analogDisconected(motorModePin)) {
      selector.checkState();
      motor.getPosition();
    }
    output.setMainMessage(F("Motor Reconnected: Continuing in 60s"));
    delay(60000);
  }

  // int startPos = motor.getPosition();
  int startPos = FOURLO;
  if (isValid(startPos)) {
    if (startPos == NEUTRAL) {
      selector.begin(1);
    } else {
      selector.begin(0);
    }
  } else {
    if (motor.getValidPosition() == NEUTRAL) {
      selector.begin(1); 
    } else {
      selector.begin(0);
    }
    waitUntilReset(); // Motor not in a good state already, wait for input before starting main loop
  }

  if (selector.getSelection() != motor.getPosition()) {
    waitUntilReset(); // Prevent a shift occuring immediately after startup without input
  }
}

void readOnly_setup() {
  randomSeed(analogRead(A5));  // Makes random() change between boots
  output.begin();
  motor.begin();
  selector.begin(0);
}

/**
 * Runs once at Arduino Startup
*/
void setup() {
  normal_setup();
  // readOnly_setup();
}


void testSwitch() {
  desiredPosition = selector.getSelection();
  delay(1000); 
}


void normal() {
  DEBUG_PRINTLN(F("Main: Starting Loop"));  // Debugging
  bool success = false;

  desiredPosition = selector.getSelection();
  if (motor.getPosition() != desiredPosition) {
    success = motor.attemptShift(desiredPosition, MAX_SINGLE_SHIFT_ATTEMPTS);
    if (success) {
      output.setMainMessage(F("Shift completed successfully"));
      delay(1000);
      output.setMainMessage(F(""));
    } else {
      DEBUG_PRINTLN(F("Main: Failed to reach position"));
      waitUntilReset();
    }
  }
}


void readOnly() {
    output.setMainMessage(F("Read Only Mode"));
    selector.getSelection();
    motor.getPosition();
    delay(500);
}

/**
 * Runs repeatedly after Arduino setup()
 */
void loop() {
  // testSwitch();
  normal();
  // readOnly();
}




