#pragma once
#include <Arduino.h>
#include "output.h"
#include "specifications.h"
#include <EEPROM.h>

int readEEPROMposition() {
    int pos = EEPROM.read(EEPROM_POSITION_ADDRESS);
    if (pos >= 0 && pos <= 3) {
        return pos;
    } else {
        switch (pos)
        {
        case 4:
            return -1;
            break;
        case 5:
            return -2;
            break;
        default: // Probably hasn't actually been set yet
            return 1; // Assume AWD
            break;
        }
    }
    // return -3;  // Shoud not reach this
}

void setEEPROMposition(int pos) {
    if (pos < 0) {  // (needs to be positive to save to EEPROM)
        if (pos == -1) {
            pos = 4;  // Invalid but in range 
        } else {
            pos = 5;  // Invalid and out of range
        }

    }
    EEPROM.update(EEPROM_POSITION_ADDRESS, pos);  // Save in EEPROM for next time vehicle turns on
}

class Motor {
    public:
        // Initialization
        Motor(OtherOutputs out) {
            lastValidPos = readEEPROMposition();
            output = out;
            lastMotorSetTime = millis(); 
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
            if (waitForShiftReady() < 0) {
                return;  // Shift not ready and needs to be aborted
            }

            initializeShift();
            while (desiredPositionDistance() > 0.05) {
                addShiftAttempt();
                if (checkShiftWorking(maxAttempts) > 0) {
                    stepShiftSpeed(desiredPositionDirection());
                } else {
                    break;
                }
                output.setMotorVolts(readPositionVolts());
            }
            endShift();
        }

    private:
        int lastValidPos;
        int currentPos;
        int desiredPos;
        int brakeState;
        int motorSpeed;
        int motorDirection;
        int singleShiftAttempts;
        unsigned long shiftStart;
        unsigned long lastMotorSetTime;  // Last time motor speed was updated
        unsigned long shiftTimes;  // TODO: Change this to some sort of list
        OtherOutputs output;

        void initializeShift() {
            singleShiftAttempts = 0;
            output.setMotorMessage("Beginning shift attempt");
            setBrake(0); 
            delay(BRAKE_RELEASE_TIME_S);  // TODO might want to change these delays to check other things in the meantime
            lastMotorSetTime = millis();  // Reset the time so that first set doesn't think it was ages ago.
            shiftStart = millis();
        }

        void endShift(){
            stopMotor();
            delay(BRAKE_RELEASE_TIME_S);
            setBrake(1);

            if (getPosition() == desiredPos) {
                setLastValidPos(desiredPos);
                output.setMotorMessage("Shift completed successfully");
                delay(1000);
                output.setMotorMessage("");
            }
        }

        int checkShiftWorking(int maxAttempts) {
            if (millis() - shiftStart > MAX_SHIFT_TIME_S*1000) { // If current shift attempt fails
                stopMotor();
                if (singleShiftAttempts > maxAttempts) {  // If failed to shift to new position
                    if (lastValidPos >= 0) {
                        output.setMotorMessage("WARNING: Failed to shift to new position, returning to previous position");
                        int returnPosition = lastValidPos;
                        setLastValidPos(-1);  // Record that the current state is invalid
                        delay(1000);  // Ensure message appears for at least 1s
                        attemptShift(returnPosition, 15);  // Try harder (15x) to return to a valid state
                    } else {  // Already previously failed to get to new position
                        output.setMotorMessage("ERROR: Failed to return to previous position, not currently in valid state!");
                        // TODO: Need to think about how to prevent the shift starting again
                        // IMPORTANT TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        delay(1000);  // Just so that the message at least shows up for 1 second
                    }
                }
                return -1; // Current shift failed
            } else {
                return 1; // Shift OK
            }
        }

        void setLastValidPos(int pos) {
            // sets the variable but also writes to EEPROM so that it can be loaded on next bootup
            lastValidPos = pos;
            setEEPROMposition(pos);
        }

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
            // TODO: Set brake pin 
            brakeState = brake;
        }

        void stepShiftSpeed(int direction) {
            // Update motor power based on current state.
            // increase power if not at max and not close to desired pos
            // decrease power if close to desired pos

            // If it has been longer than 1 PWM cycle since last update
            int timeSinceLastSet = max(millis() - lastMotorSetTime, 50); 
            float pwmsSinceLastStep = timeSinceLastSet * PWM_FREQUENCY;
            if (pwmsSinceLastStep > 1.0) {
                if (direction != motorDirection) {  // Change of direction!! 
                    motorSpeed = 0;
                    stopMotor();
                    delay(100);  // Enforce some delay
                }
                motorDirection = direction;

                if (desiredPositionDistance() >= 0.2) {
                    if (abs(motorSpeed) < PWM_MAX_POWER) {
                        changeSpeedValue(1, pwmsSinceLastStep);
                    }
                } else if (motorSpeed > PWM_MIN_POWER) { // If greater than 10% power decellerate
                    changeSpeedValue(-1, pwmsSinceLastStep); 
                }
                setMotor();
            }
        }

        void changeSpeedValue(int increase, float numPWMs) {  
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
            if (brakeState == 0 && motorSpeed > 0 && (motorDirection == 1 || motorDirection == -1)) {
                // TODO: Send motor command with motorSpeed and motorDirection
            } else {
                // TODO: Stop motor (i.e. set pin outputs)
            }
            lastMotorSetTime = millis();
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

        int waitForShiftReady() {
            unsigned long waitStart = millis();
            while (shiftReady() != 1)
            {
                delay(10);  // TODO: Change this to do other things while waiting? I.e. check switchPosition or update screen?
                if (shiftReady() == -1 || millis() - waitStart > 30*1000){
                    output.setMotorMessage("Shift not ready and needs to abort");
                    delay(1000); 
                    return -1;  
                }
            }
            return 1;
        }
};


