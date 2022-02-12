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

// PINS
const uint8_t TFT_CS = 10, TFT_DC = 9, TFT_RST = 8; 
const uint8_t switchModePin = A0;
const uint8_t motorPWMpin = 6;
const uint8_t motorDirPin = 7;
const uint8_t brakeReleasePin = 4;
const uint8_t manualDirectionPin = 2;
const uint8_t manualDrivePin = 3;

// const uint8_t fakeSwitchPin = 5;  // Not used yet
// const uint8_t fakeMotorPin = 6;   // Not used yet
const uint8_t motorModePin = A1;
// const uint8_t backLightPin = 7;
// Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


// int SWITCH_FIXED_RESISTOR = 4555;  // Resistance of fixed resistor for detecing mode select resistance in ohms
int SWITCH_FIXED_RESISTOR = 4675;  // Resistance of fixed resistor for detecting mode select resistance in ohms

bool manualMode = false;


// OtherOutputs output = OtherOutputs(&tft, fakeSwitchPin, fakeMotorPin);  // TODO: Add backLightPin and some backlight control
OtherOutputs output = OtherOutputs(&tft);  // TODO: Add backLightPin and some backlight control
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
  unsigned long time;
  const char msg[] PROGMEM = "WARNING: Motor position is not valid. Hold N for 5s to reset";
  output.setMainMessage(msg);
  while (1) {  // Keep looping through this until N is pressed for duration_s
      while (selector.getSwitchPosition() != NEUTRAL) {
          motor.getPosition();
          delay(10);
      }
      time = millis();
      output.setMainMessage(F("N pressed"));
      while (millis() - time < 5*1000 && selector.getSwitchPosition() == NEUTRAL) {
          motor.getPosition();
          delay(10);
      }
      if (millis() - time > 5*1000) {
          break;
      } else {
          output.setMainMessage(F("N released early"));
          delay(500);
          output.setMainMessage(msg);
      }
  }
  output.setMainMessage(F("Reset Successful. Release N"));
  while (selector.getSwitchPosition() == NEUTRAL) {
    motor.getPosition();
    delay(10);
  }
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
  delay(50);
  analogRead(pin);  // Apparently first few reads after switching mode can be bad
  analogRead(pin);
  analogRead(pin);

  if (analogRead(pin) > 990) {  // ~990 -> 4.85V (4.93+ is detected when actually disconnected)
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
  delay(3000); // Some time for output bootup display to show
  motor.begin();

  if (analogDisconected(motorModePin)) {
    output.setMainMessage(F("Motor Disconnected: Waiting for reconnect"));
    selector.begin(0);
    delay(2000);
    while (analogDisconected(motorModePin)) {
      selector.readOnly();
      delay(10);
      motor.getPosition();
      delay(10);
    }
    output.setMainMessage(F("Motor Reconnected: Continuing in 60s"));
    delay(60000);
    output.setMainMessage(F(""));
  }

  int startPos = motor.getPosition();
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

void manualControlSetup() {
  manualMode = true;
  pinMode(manualDirectionPin, INPUT_PULLUP);

  randomSeed(analogRead(A5));  // Makes random() change between boots
  output.begin();
  delay(300); // Some time for output bootup display to show
  motor.begin();

  output.setMainMessage(F("Manual Mode Enabled"));
  selector.begin(0);
  delay(1000);
  while (digitalRead(manualDrivePin) == LOW) {
    output.setMainMessage(F("Release Drive Button"));
  }
}


void testSwitch() {
  desiredPosition = selector.getSelection();
  delay(1000); 
}

void normal() {
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

void manualControl() {
  int dirRead = digitalRead(manualDirectionPin);
  int dir = 0;
  char msg[30];
  motor.getPosition();
  selector.readOnly();
  if (dirRead == HIGH) { // Towards 4HI
    sprintf(msg, "Toward 4HI");
    // output.setMainMessage(F("Toward 4HI"));
    dir = TOWARD_4HI;
  } else if (dir == LOW) { // Towards 4LO
    sprintf(msg, "Toward 4LO");
    // output.setMainMessage(F("Toward 4LO"));
    dir = TOWARD_4LO;
  } else {  // Invalid read
    sprintf(msg, "ERROR");
  }
  output.setMainMessage(msg);

  if (digitalRead(manualDrivePin) == LOW) { // Starting Drive
    strcat(msg, ":     Driving");
    output.setMainMessage(msg);
    unsigned long startTime = millis();
    while (digitalRead(manualDrivePin) == LOW && millis() - startTime < 5000) { // Button pressed
      motor.manualDrive(dir);
    } 
    motor.manualStop();

    if (digitalRead(manualDrivePin) == LOW) { // Button still pressed (Prevent next loop until released)
      output.setMainMessage(F("Release Manual Drive"));
      while (digitalRead(manualDrivePin) == LOW) {
        delay(10);
      }
    }
  }
}

/**
 * Runs once at Arduino Startup
*/
void setup() {
  // normal_setup();
  // readOnly_setup();
  pinMode(manualDrivePin, INPUT_PULLUP);
  delay(1);
  if (digitalRead(manualDrivePin) == LOW) { // Then booting with manual override
    manualControlSetup();
  } else {  // Normal startup procedure
    normal_setup();
  }
}

/**
 * Runs repeatedly after Arduino setup()
 */
void loop() {
  // readOnly();
  // testSwitch();
  if (manualMode == true) {
    manualControl();
  } else {
    normal();
  }
}




