#pragma once
#include <Arduino.h>
#include "output.h"
#include "specifications.h"

// #define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

char sw_buf[100];  // DEBUGGING: to use for Serial prints to avoid using String 

const int messageBufferLength = maxChars*4;
char messageBuffer[messageBufferLength+1];


class SelectorSwitch {
    private: 
        uint8_t modeSelectPin;
        int lastValidState = AWD;  // Defaults to this in case switch isn't connected
        bool inNeutral = false;
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
            analogRead(modeSelectPin); // Throw away reading to make sure the next ones aren't affected by previous readings
            ///// Basic Averaging /////////////////
            float Vout = 0.0;
            for (int i=0; i<10; i++) {
                Vout += Vin*analogRead(modeSelectPin)/1023;  //~100us per read
            } 
            Vout = Vout/10.0;  // Because of averaging
            ///////////////////////////////////////
            ////// Exponential Averaging /////////// (less prone to spikes)
            // int V = analogRead(modeSelectPin);
            // const float alpha = 0.1;  // Lower more stable but slower to vary
            // for (int i=0; i<10; i++) {
            //     V = (alpha*analogRead(modeSelectPin)+(1-alpha)*V);
            // }
            // float Vout = V/1023.0;
            /////////////////////////////////////////
            int resistance = round(FIXED_RESISTOR * (Vin - Vout) / Vout); 
            output->setSwitchResistance(resistance); 
            return resistance;
        }

        void neutralPressed() {
            int currentState;
            output->getMainMessage(messageBuffer, messageBufferLength);
            output->setMainMessage(F("Neutral Pressed"));
            DEBUG_PRINTLN(F("N Pressed"));
            while (millis() - timeEnteredState < SW_N_PRESS_TIME_S*1000 && getSwitchPosition() == NEUTRAL) {
                delay(10);
            }
            if (millis() - timeEnteredState > SW_N_PRESS_TIME_S*1000) {
                DEBUG_PRINTLN(F("N Pressed longer than 0.25s"));
                toggleNeutral();
                while (getSwitchPosition() == NEUTRAL) {
                    delay(10);
                    currentState = getSwitchPosition();
                }
                if (inNeutral) {
                    lastValidState = NEUTRAL;
                } else {
                    if (isValid(currentState)) {
                        lastValidState = currentState;
                    } else {
                        lastValidState = AWD;
                    }
                }
                DEBUG_PRINTLN(F("Setting output lastValidState:"));
                DEBUG_PRINTLN(lastValidState);
                output->setSwitchPos(lastValidState);
            } else {  // Neutral only pressed for short time, show easter egg but do nothing to switch position or lastValidState
                output->showCat(2000);
            }
            DEBUG_PRINTLN(F("setting back to previous message"));
            output->setMainMessage(messageBuffer);
        }

        void toggleNeutral() {
            inNeutral = !inNeutral;
            if (inNeutral) {
                DEBUG_PRINTLN(F("Set output to N"));
                output->setSwitchPos(NEUTRAL);
            } 
            DEBUG_PRINTLN(F("setting message neutral toggled"));
            output->setMainMessage(F("Neutral Toggled"));  // TODO: Replace with something that flashes a big N or something like that
            delay(100);
        }

    public:
        SelectorSwitch(int analogInput, OtherOutputs* out, int FIXED_RESISTOR) 
            : modeSelectPin(analogInput)
            , lastValidState(AWD)
            , output(out)
            , FIXED_RESISTOR(FIXED_RESISTOR)
            {
        }

        void begin(byte NeutralState) {
            if (NeutralState) {
                inNeutral = 1;
                lastValidState = NEUTRAL;
            }
            pinMode(modeSelectPin, INPUT);
            getSelection();
            output->setSwitchPos(lastValidState);
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
            // } else if (ohms > min(SW_N_AWD_LOW, min(SW_N_LOCK_LOW, SW_N_LO_LOW))*0.9 && ohms < max(SW_N_AWD_HIGH, max(SW_N_LOCK_HIGH, SW_N_LO_HIGH))*1.1) {  // 4LO_N reads 228ohm (instead of 226 which should be the max...)
            } else if (ohms > SW_N_LOW && ohms < SW_N_HIGH) {  // 4LO_N reads 228ohm (instead of 226 which should be the max...)
                position = NEUTRAL;
            } else if (ohms > SW_LO_LOW && ohms < SW_LO_HIGH) {
                position = FOURLO;
            } else {
                position = -1; // Bad position but in range
            }
            return position;
        }

        void setLastValidState(byte state) {
            lastValidState = state;
        }

        void readOnly(){
            readSwitchPositionOhms();
        }

        int getSelection() {
            // Return current selection (last validState after calling check)
            checkState();
            return lastValidState;
        }

        void checkState() {
            // Check the switch position. If new wait until we know switch isn't mid change
            int newState = getSwitchPosition();
            timeEnteredState = millis();
            if (newState == NEUTRAL) {
                // if current selection is Neutral check whether entering or leaving and handle appropriately
                neutralPressed();  
            } else if (!inNeutral) {
                if (newState != lastValidState && isValid(newState)) {
                    while (getSwitchPosition() == newState && millis() - timeEnteredState < SW_DEBOUNCE_S*1000) {
                        delay(10);
                    }
                    if (millis() - timeEnteredState > SW_DEBOUNCE_S*1000) { 
                        lastValidState = newState;
                        DEBUG_PRINTLN(F("Not in Neutral, setting LastValidState"));
                        output->setSwitchPos(lastValidState);
                    } else {
                        // Switch is still changing, so don't update lastValidState, wait until this is called again
                    }
                }
            }
            timeLastChecked = millis();
        }
};
