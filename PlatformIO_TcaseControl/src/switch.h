#pragma once
#include <Arduino.h>
#include "output.h"
#include "specifications.h"

#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

char sw_buf[100];  // DEBUGGING: to use for Serial prints to avoid using String 

class SelectorSwitch {
    private: 
        byte modeSelectPin;
        int lastValidState;
        byte inNeutral = 0;
        int currentState;
        unsigned long timeEnteredState;
        unsigned long timeLastChecked;
        OtherOutputs* output;  // Pointer so that it points to the same object everywhere
        const int FIXED_RESISTOR;

        /**
         * Read position of selector switch
         */ 
        int readSwitchPositionOhms() {
            // Returns resistance of switch
            float Vin = 5.0;
            float Vout = 5.0/1024*analogRead(modeSelectPin);
            int resistance = FIXED_RESISTOR * (Vin - Vout) / Vout;
            output->setSwitchResistance(resistance); 
            return resistance;
        }

        int getSwitchPosition() {
            // Returns position as value from 0 -> 3 or -1 if invalid (or -2 if invalid and out of range)
            int ohms = readSwitchPositionOhms();
            int position;
            if (ohms < SW_SHORTED_HIGH || ohms > SW_OPEN_LOW) {
                position = -2;  // Bad position and out of range
            } else if (ohms > SW_LOCK_LOW && ohms < SW_LOCK_HIGH) {
                position = FOURHI;
            } else if (ohms > SW_AWD_LOW && ohms < SW_AWD_HIGH) {
                position = AWD;
            } else if (ohms > min(SW_N_AWD_LOW, min(SW_N_LOCK_LOW, SW_N_LO_LOW)) && ohms < max(SW_N_AWD_HIGH, max(SW_N_LOCK_HIGH, SW_N_LO_HIGH))) {
                position = NEUTRAL;
            } else if (ohms > SW_LO_LOW && ohms < SW_LO_HIGH) {
                position = FOURLO;
            } else {
                position = -1; // Bad position but in range
            }
            snprintf(sw_buf, sizeof(sw_buf), "Switch>getSwitchPosition: position = %i", position); DEBUG_PRINTLN(sw_buf);  // DEBUGGING

            return position;
        }

        void neutralPressed() {
            output->setMainMessage(F("Neutral Pressed"));
            while (millis() - timeEnteredState < SW_N_PRESS_TIME_S*1000 && getSwitchPosition() == NEUTRAL) {
                delay(10);
                currentState = getSwitchPosition();
            }
            if (millis() - timeEnteredState > SW_N_PRESS_TIME_S*1000) {
                toggleNeutral();
                output->setMainMessage(F("Neutral Toggled"));
                while (getSwitchPosition() == 2) {
                    delay(10);
                    currentState = getSwitchPosition();
                }
                output->setSwitchPos(currentState);
            } else {
                output->showCat();
                delay(2000);
            }
            output->setMainMessage(F(" "));

        }

        void toggleNeutral() {
            inNeutral = !inNeutral;
            if (inNeutral) {
                output->setSwitchPos(NEUTRAL);
            }
            output->setMainMessage(F("Neutral Toggled"));  // TODO: Replace with something that flashes a big N or something like that
            delay(100);
        }

    public:
        SelectorSwitch(int analogInput, OtherOutputs* out, int FIXED_RESISTOR) 
            : modeSelectPin(analogInput)
            , lastValidState(AWD)
            , currentState(-1)
            , output(out)
            , FIXED_RESISTOR(FIXED_RESISTOR)
            {
        }

        void begin(byte NeutralState) {
            if (NeutralState) {
                inNeutral = 1;
            }
            pinMode(modeSelectPin, INPUT);
            checkState();
            delay(SW_DEBOUNCE_S*1000 + 50);
            checkState();
        }

        int getSelection() {
            // Return current selection (last validState after calling check)
            if (timeLastChecked > SW_DEBOUNCE_S)  // Do a full check of what state the switch is in if it's been a while since it was checked 
            {
                unsigned long now = millis();
                while (millis() - now > SW_DEBOUNCE_S*1000+10) {
                    checkState();
                }
            }
            checkState(); 
            output->setSwitchPos(lastValidState);
            return lastValidState;
        }


        void checkState() {
            // Check the switch position. If new update current State and set timeEnteredState
            // If timeEnteredState > X set lastValidState
            int newState = getSwitchPosition();
            if (newState != currentState) {  // If switch has changed state
                currentState = newState;
                timeEnteredState = millis();
            }

            if (currentState == NEUTRAL) { // If Neutral currently pressed, keep checking until either a few S or button released
                neutralPressed();
            } else { // Not a Neutral press, carry on like normal
                if (currentState >= 0 && currentState <= 3 && millis() - timeEnteredState > SW_DEBOUNCE_S*1000) {
                    lastValidState = currentState;
                }
            }

            if (inNeutral) {
                lastValidState = 2;
            }
            output->setSwitchPos(lastValidState);  
            timeLastChecked = millis();
        }
};

