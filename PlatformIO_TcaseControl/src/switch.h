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
        int currentState;
        unsigned long timeEnteredState;
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

        void begin() {
            pinMode(modeSelectPin, INPUT);
            checkState();
            delay(SW_DEBOUNCE_S*1000 + 50);
            checkState();
        }

        int getSelection() {
            // Return current selection (last validState after calling check)
            checkState(); 
            return lastValidState;
        }
        
        void checkState() {
            // Check the switch position. If new update current State and set timeEnteredState
            // If timeEnteredState > X set lastValidState
            int newState = getSwitchPosition();
            if (newState != currentState) {
                currentState = newState;
                timeEnteredState = millis();
            }
            if (currentState >= 0 && currentState <= 3 && millis() - timeEnteredState > SW_DEBOUNCE_S*1000) {
                lastValidState = currentState;
            }
            output->setSwitchPos(lastValidState);  // TODO: Re enable
        }
};

