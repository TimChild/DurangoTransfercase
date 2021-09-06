#pragma once
#include <Arduino.h>
#include "output.h"
#include "specifications.h"

int FIXED_RESISTOR = 4555;  // Resistance of fixed resistor for detecing mode select resistance in ohms

class SelectorSwitch {
    private: 
        int modeSelectPin;
        int lastValidState;
        int currentState;
        float timeEnteredState;
        OtherOutputs* output;  // Pointer so that it points to the same object everywhere

        /**
         * Read position of selector switch
         */ 
        int readSwitchPositionOhms() {
            // Returns resistance of switch
            float Vin = 5.0;
            float Vout = 5.0/1024*analogRead(modeSelectPin);
            int resistance = FIXED_RESISTOR * (Vin - Vout) / Vout;
            // Serial.print(String(Vout) + ";" + String(resistance) + "\n\n");
            
            output->setMainMessage("Ohms: " + String(resistance)); // DEBUGGING

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
            Serial.print(String(position) + "\n");

            return position;
        }

    public:
        // Initialization
        SelectorSwitch(int analogInput, OtherOutputs* out) 
            : modeSelectPin(analogInput)
            , lastValidState(1)
            , currentState(-1)
            , output(out)
            {
                pinMode(analogInput, INPUT);
            //     if (lastValidState < 0 || lastValidState > 3) {
            //         lastValidState = 1;  // Default to AWD
            // }
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

