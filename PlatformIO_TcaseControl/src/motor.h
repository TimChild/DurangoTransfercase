#pragma once
#include <Arduino.h>
#include "output.h"

/**
 * Read position of mode sensor in Volts
 */
float readPositionVolts() {
  return 4.5;
}

class Motor {
    private:
        int lastValid;
        int currentPos;
        int brakeState;
        int motorSpeed;

        unsigned long shiftTimes;  // TODO: Change this to some sort of list
        OtherOutputs output;

        int shiftReady() {
            // Return 1 if valid time to shift, 0 if not, -1 if need to abort shift
            return 1;  // TODO: 
        }

        void shift(int direction) {
            // Update motor power based on current state.
            // increase power if not at max and not close to desired pos
            // decrease power if close to desired pos
        }

        float desiredPositionDistance(int desired) {
            // Return distance to desired position
            float currentPosVolts = readPositionVolts();
            currentPosVolts+=1;  // Just clearing warnings
            return 1.0;  // TODO
        }
        
    public:
        int getPosition() {
            // Check position from motor, if valid update last valid and return, 
            // otherwise return last valid
            return 1;
        }
            

        void attemptShift(int desiredPos) {
            // 
            // wait until shift ready or abort
            // repeat calling shift forward or back until in position
            // If not reach position turn off motor
        }
};


