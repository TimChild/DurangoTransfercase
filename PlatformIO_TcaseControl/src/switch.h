#pragma once
#include <Arduino.h>
#include "output.h"
#include "specifications.h"

#ifndef DEBUG_PRINTLN
  #ifdef DEBUG
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINT(x) Serial.print(x)
  #else
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINT(x)
  #endif
#endif

char sw_buf[100];  // DEBUGGING: to use for Serial prints to avoid using String 

class SelectorSwitch {
    private: 
        byte modeSelectPin;
        byte lastValidState;
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
                position = 0;
            } else if (ohms > SW_AWD_LOW && ohms < SW_AWD_HIGH) {
                position = 1;
            } else if (ohms > min(SW_N_AWD_LOW, min(SW_N_LOCK_LOW, SW_N_LO_LOW)) && ohms < max(SW_N_AWD_HIGH, max(SW_N_LOCK_HIGH, SW_N_LO_HIGH))) {
                position = 2;
            } else if (ohms > SW_LO_LOW && ohms < SW_LO_HIGH) {
                position = 3;
            } else {
                position = -1; // Bad position but in range
            }
            snprintf(sw_buf, sizeof(sw_buf), "Switch>getSwitchPosition: position = %i", position); DEBUG_PRINTLN(sw_buf);  // DEBUGGING

            return position;
        }

    public:
        SelectorSwitch(int analogInput, OtherOutputs* out, int FIXED_RESISTOR) 
            : modeSelectPin(analogInput)
            , lastValidState(1)
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
            return lastValidState;
        }

        void toggleNeutral() {
            inNeutral = !inNeutral;
            output->setMainMessage(F("Neutral Toggled"));
            delay(100);
        }
        
        void checkState() {
            // Check the switch position. If new update current State and set timeEnteredState
            // If timeEnteredState > X set lastValidState
            int newState = getSwitchPosition();
            if (newState != currentState) {  // If switch has changed state
                currentState = newState;
                timeEnteredState = millis();
            }

            if (currentState == 2) { // If Neutral currently pressed, keep checking until either a few S or button released
                output->setMainMessage(F("Waiting for N Press duration"));
                while (millis() - timeEnteredState < SW_N_PRESS_TIME_S*1000 && getSwitchPosition() == 2) {
                    delay(10);
                    currentState = getSwitchPosition();
                }
                if (millis() - timeEnteredState > SW_N_PRESS_TIME_S*1000) {
                    toggleNeutral();
                    output->setSwitchPos(currentState);  
                    output->setMainMessage(F("Neutral Toggle Registerd"));
                    while (getSwitchPosition() == 2) {
                        delay(10);
                        currentState = getSwitchPosition();
                        output->setSwitchPos(currentState);  
                    }
                } else {
                    output->showCat();
                    delay(2000);
                }
                output->setMainMessage(F(" "));
            }
            else { // It not a Neutral press, carry on like normal
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

