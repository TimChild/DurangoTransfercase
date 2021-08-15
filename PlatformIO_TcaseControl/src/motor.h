#pragma once
#include <Arduino.h>
#include "output.h"
#include "specifications.h"


class Motor {
    private:
        int lastValidPos;
        int currentPos;
        int desiredPos;
        int brakeState;
        int motorSpeed;
        int motorDirection;
        unsigned long lastMotorSetTime;  // Last time motor speed was updated
        unsigned long shiftTimes;  // TODO: Change this to some sort of list
        OtherOutputs output;


        /**
         * Read position of mode sensor in Volts
         */
        float readPositionVolts() {
        return 4.5;  // TODO: Read from some pin
        }

        int getPositionLowVolts(int position) {
            // Return low voltage of position
            switch (position){
                case 0: return LOCK_LOW;
                case 1: return AWD_LOW;
                case 2: return N_LOW;
                case 3: return LO_LOW;
                default: return LOW_LIMIT; // TODO: Is this the right thing to return in this case?
            }

        }

        int getPositionHighVolts(int position) {
            // Return high voltage of position
            switch (position){
                case 0: return LOCK_HIGH;
                case 1: return AWD_HIGH;
                case 2: return N_HIGH;
                case 3: return LO_HIGH;
                default: return HIGH_LIMIT; // TODO: Is this the right thing to return in this case?
            }
        }

        int shiftReady() {
            // Return 1 if valid time to shift, 0 if not, -1 if need to abort shift
            return 1;  // TODO: 
        }

        void stepShiftSpeed(int direction) {
            // Update motor power based on current state.
            // increase power if not at max and not close to desired pos
            // decrease power if close to desired pos

            // If it has been longer than 1 PWM cycle since last update
            if (millis() - lastMotorSetTime > 1.0/PWM_FREQUENCY) {
                // if (direction == motorDirection || motorDirection == 0) {
                //     motorDirection = direction;
                // }
                motorDirection = direction;

                if (desiredPositionDistance() > 0.1) {

                }

                if (abs(motorSpeed) < 255) {
                    int newSpeed = motorSpeed + PWM_ACCELERATION * 255 * motorDirection;
                    if (abs(newSpeed) > 255) {
                        if (newSpeed > 0) {
                            newSpeed = abs(newSpeed);
                        } else {
                            newSpeed = -abs(newSpeed);
                        }
                    }
                    motorSpeed = newSpeed;
                }
            }

        }

        float desiredPositionDistance() {
            // Return distance to desired position
            float currentPosVolts = readPositionVolts();
            float desiredLow = getPositionLowVolts(desiredPos);
            float desiredHigh = getPositionHighVolts(desiredPos);

            currentPosVolts+=1;  // Just clearing warnings
            if (currentPosVolts <= desiredHigh && currentPosVolts >= desiredLow) {
                return map(currentPosVolts, desiredLow, desiredHigh, 0.0, 1.0);
            } else {
                return 0.0;  // Return zero distance if out of range to stop motor
            }
        }
        
    public:
        // Initialization
        Motor(OtherOutputs out) {
            output = out;
            lastMotorSetTime = millis();
            lastValidPos = 1;  // Assume AWD as last valid state
        }
        
        int getPosition() {
            // Check position from motor, if valid update last valid and return, 
            // otherwise return last valid
            float currentPosVolts = readPositionVolts();
            if (currentPosVolts < LOW_LIMIT || currentPosVolts > HIGH_LIMIT) {
                return -2;  // Bad position and out of range
            } else if (currentPosVolts > LOCK_LOW && currentPosVolts < LOCK_HIGH) {
                return 0;
            } else if (currentPosVolts > AWD_LOW && currentPosVolts < AWD_HIGH) {
                return 1;
            } else if (currentPosVolts > N_LOW && currentPosVolts < N_HIGH) {
                return 2;
            } else if (currentPosVolts > LO_LOW && currentPosVolts < LO_HIGH) {
                return 3;
            } else {
                return -1; // Bad position but in range
            }
        }
            
        void attemptShift(int desiredPos) {
            // 
            // wait until shift ready or abort
            // repeat calling shift forward or back until in position
            // If not reach position turn off motor
            while (shiftReady() != 1)
            {
                delay(10);  // TODO: Change this to do other things while waiting? I.e. check switchPosition or update screen?
                if (shiftReady() == -1){
                    break;
                }
            }
            if (shiftReady() == -1){
                output.setMotorMessage("Shift not ready and aborted");
                output.writeOutputs();
                return;  
            }
            
        }
};


