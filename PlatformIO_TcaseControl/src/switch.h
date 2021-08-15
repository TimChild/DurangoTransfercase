#pragma once
#include <Arduino.h>
#include "output.h"
#include "specifications.h"


class SelectorSwitch {
    private: 
        int lastValidState;
        int currentState;
        float timeEnteredState;
        OtherOutputs output;

        /**
         * Read position of selector switch
         */ 
        int readSwitchPositionOhms() {
            // Returns resistance of switch
            return 2000; // TODO
        }

        int getSwitchPosition() {
            // Returns position as value from 0 -> 3 or -1 if invalid (or -2 if invalid and out of range)
            int ohms = readSwitchPositionOhms();
            if (ohms < SW_SHORTED_HIGH || ohms > SW_OPEN_LOW) {
                return -2;  // Bad position and out of range
            } else if (ohms > SW_LOCK_LOW && ohms < SW_LOCK_HIGH) {
                return 0;
            } else if (ohms > SW_AWD_LOW && ohms < SW_AWD_HIGH) {
                return 1;
            } else if (ohms > min(SW_N_AWD_LOW, min(SW_N_LOCK_LOW, SW_N_LO_LOW)) && ohms < max(SW_N_AWD_HIGH, max(SW_N_LOCK_HIGH, SW_N_LO_HIGH))) {
                return 2;
            } else if (ohms > SW_LO_LOW && ohms < SW_LO_HIGH) {
                return 3;
            } else {
                return -1; // Bad position but in range
            }
        }

    public:
        // Initialization
        SelectorSwitch(OtherOutputs out) {
            output = out;
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
            if (newState >= 0 && newState <= 3 && millis() - timeEnteredState > SW_DEBOUNCE_S*1000) {
                lastValidState = currentState;
            }
            output.setSwitchPos(lastValidState);
            output.writeOutputs();
        }
};

