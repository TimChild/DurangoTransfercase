#pragma once
#include <Arduino.h>
#include "output.h"
#include "specifications.h"
#include <EEPROM.h>


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
            return 1;  // TODO: Need to figure out how to make an array and stuff. Or maybe just do simple option of only allow shift every X seconds
        }

        void addShiftAttempt() {
            // Add a shift attempt at current time, and clear out any shifts that are too old
            // TODO
        }

        void setBrake(int brake) {
            // TODO
            brakeState = brake;
        }

        void stepShiftSpeed(int direction) {
            // Update motor power based on current state.
            // increase power if not at max and not close to desired pos
            // decrease power if close to desired pos

            // If it has been longer than 1 PWM cycle since last update
            float pwmsSinceLastStep = (millis() - lastMotorSetTime) * PWM_FREQUENCY;
            if (pwmsSinceLastStep > 1.0) {
                if (direction != motorDirection) {  // Change of direction!! 
                    motorSpeed = 0;
                    stopMotor();
                    delay(100);  // Enforce some delay
                }
                motorDirection = direction;

                if (desiredPositionDistance() >= 0.2) {
                    if (abs(motorSpeed) < PWM_MAX_POWER) {
                        changeSpeed(1, pwmsSinceLastStep);
                    }
                } else if (motorSpeed > PWM_MIN_POWER) { // If greater than 10% power decellerate
                    changeSpeed(-1, pwmsSinceLastStep); 
                }
                setMotor();
            }
        }

        void changeSpeed(int increase, float numPWMs) {  
            // Change power output (always positive value)
            int newSpeed;
            if (increase > 0) {
                newSpeed = max(PWM_MAX_POWER, motorSpeed + max(1, PWM_ACCELERATION * numPWMs * PWM_MAX_POWER));
            } 
            if (increase < 0) { // i.e. decrese power
                newSpeed = min(PWM_MIN_POWER, motorSpeed - max(1, PWM_ACCELERATION * numPWMs * PWM_MAX_POWER));
            }
            motorSpeed = newSpeed;
        }

        void stopMotor() {
            motorSpeed = 0;
            motorDirection = 0;
            setMotor();
        }

        void setMotor() {
            // TODO: call the motor controller with motorDirection and motorSpeed
            if (brakeState == 0) {
                // TODO: Send motor command with motorSpeed and motorDirection
            } else {
                // TODO: Stop motor
            }
        }

        float desiredPositionDistance() {
            // Return distance to desired position
            float currentPosVolts = readPositionVolts();
            float desiredPosVolts = (getPositionLowVolts(desiredPos), getPositionHighVolts(desiredPos))/2.0; // Aim for middle
            if (currentPosVolts > HIGH_LIMIT || currentPosVolts < LOW_LIMIT) {
                return 0.0;  // Return zero distance if reading is out of range (to prevent trying to shift somewhere with a bad reading)
            }
            return max(1.0, abs(currentPosVolts - desiredPosVolts));
        }

        int desiredPositionDirection() {
            // Return direction of desired position from current position
            float currentPosVolts = readPositionVolts();
            float desiredPosVolts = (getPositionLowVolts(desiredPos), getPositionHighVolts(desiredPos))/2.0; // Aim for middle
            if (currentPosVolts > HIGH_LIMIT || currentPosVolts < LOW_LIMIT) {
                return 0;  // Return no direction if reading is out of range (to prevent trying to shift somewhere with bad reading)
            }
            if (currentPosVolts <= desiredPosVolts) {
                return 1;  
            } else {
                return -1;
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
            
        void attemptShift(int desiredPos, int maxAttempts) {
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
                delay(1000); // TODO: Do I need to prevent immediately diving back into a shift attempt? Maybe OK actually
                return;  
            }

            unsigned long shiftStart = millis();
            int shiftAttempts = 0;
            output.setMotorMessage("Beginning shift attempt");
            setBrake(0);  // TODO: Need to think about how to prevent the shift starting again
            delay(BRAKE_RELEASE_TIME_S);  // TODO might want to change these delays to check other things in the meantime
            while (desiredPositionDistance() > 0.05) {
                addShiftAttempt();
                if (millis() - shiftStart > MAX_SHIFT_TIME_S*1000) { // If current shift attempt fails
                    stopMotor();
                    shiftAttempts += 1;
                    if (shiftAttempts > maxAttempts) {  // If failed to shift to new position
                        if (lastValidPos < 0) {  // If already previously failed to get to new position
                            output.setMotorMessage("ERROR: Failed to return to previous position, not currently in valid state!");
                            // TODO: Need to think about how to prevent the shift starting again
                            delay(1000);  // Just so that the message at least shows up for 1 second
                            break;
                        }
                        output.setMotorMessage("WARNING: Failed to shift to new position, returning to previous position");
                        int returnPosition = lastValidPos;
                        lastValidPos = -1;  // Record that the current state is invalid
                        delay(RETRY_TIME_S*1000);  // Some delay before attempting to shift again
                        attemptShift(returnPosition, 15);  // Try harder (15x) to return to a valid state
                        break;
                    }
                }
                stepShiftSpeed(desiredPositionDirection());
                output.setMotorVolts(readPositionVolts());
            }
            stopMotor();
            delay(BRAKE_RELEASE_TIME_S);
            setBrake(1);

            if (getPosition() == desiredPos) {
                lastValidPos = desiredPos;
                output.setMotorMessage("Shift completed successfully");
                delay(1000);
            }
        }
};


