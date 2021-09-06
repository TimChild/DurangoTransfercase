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
    private:
        int lastValidPos = readEEPROMposition();
        int currentPos = 0;  // TODO: Can I use the function to intialize this here? Or can I do that in defined init? 
        int desiredPos = 1;
        int brakeState = 1; // By default the brake is ON and must be disabled by setting brakePin HIGH
        float motorSpeed = 0;
        int motorDirection = 0;
        int singleShiftAttempts = 0;
        unsigned long shiftStart;
        unsigned long lastMotorSetTime = millis();  // Last time motor speed was updated
        unsigned long shiftTimes;  // TODO: Change this to some sort of list
        int dirPin;
        int pwmPin;
        int brakeReleasePin;
        int modePin;
        OtherOutputs *output;

        void initializeShift() {
            singleShiftAttempts = 0;
            // output->setMotorMessage("Beginning shift attempt");
            output->setMainMessage("Beginning shift attempt");  // DEBUGGING
            setBrake(0); 
            delay(BRAKE_RELEASE_TIME_S);  // TODO might want to change these delays to check other things in the meantime
            lastMotorSetTime = millis();  // Reset the time so that first set doesn't think it was ages ago.
            shiftStart = millis();
        }

        int endShift(){
            stopMotor();
            delay(BRAKE_RELEASE_TIME_S);
            setBrake(1);

            if (getPosition() == desiredPos) {
                setLastValidPos(desiredPos);
                output->setMotorMessage("Shift completed successfully");
                delay(1000);
                output->setMotorMessage("");
                return 1;
            }
            return -1;
        }

        int checkShiftWorking(int maxAttempts) {
            if (millis() - shiftStart > MAX_SHIFT_TIME_S*1000) { // If current shift attempt fails
                stopMotor();
                if (singleShiftAttempts > maxAttempts) {  // If failed to shift to new position
                    if (lastValidPos >= 0) {
                        output->setMotorMessage("WARNING: Failed to shift to new position, returning to previous position");
                        int returnPosition = lastValidPos;
                        setLastValidPos(-1);  // Record that the current state is invalid
                        delay(1000);  // Ensure message appears for at least 1s
                        attemptShift(returnPosition, 15);  // Try harder (15x) to return to a valid state
                    } else {  // Already previously failed to get to new position
                        output->setMotorMessage("ERROR: Failed to return to previous position, not currently in valid state!");
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
            float volts = analogRead(modePin)*5.0/1024.0;
            Serial.println((String)"Motor>readPositionVolts: Reading = "+volts);
            return volts;
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
            // 0 for brake OFF (Which is actually brakePin high to release brake)
            // 1 for brake ON (Which is actually brakePin low to leave brake on)
            if (brake == 0 || brake == 1) {
                brakeState = brake;
                Serial.println((String)"Motor>setBrake: Setting brake pin to "+(1-brakeState) + " to achieve brake state " + brakeState);
                digitalWrite(brakeReleasePin, 1-brakeState);  // (1-X) because the brake is ON by default and HIGH turns it OFF. 
            }
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
                int realDir, realPwm;
                realDir = (motorDirection > 0) ? 1 : 0;
                realPwm = max(PWM_MAX_POWER*motorSpeed, PWM_MIN_POWER);
                Serial.println((String)"Motor>setMotor: Dir = "+realDir+", Speed: "+realPwm);
                digitalWrite(dirPin, realDir);
                analogWrite(pwmPin, realPwm);
            } else { // Stop motor
                Serial.println((String)"Motor>setMotor: Dir = 0, Speed: 0");
                digitalWrite(dirPin, 0);
                digitalWrite(pwmPin, 0);
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
                    output->setMotorMessage("Shift not ready and needs to abort");
                    delay(1000); 
                    return -1;  
                }
            }
            return 1;
        }

    public:
        Motor(int pwmPin, int dirPin, int brakeReleasePin, int modePin, OtherOutputs* out)
            : dirPin(dirPin)
            , pwmPin(pwmPin)
            , brakeReleasePin(brakeReleasePin)
            , modePin(modePin)
            , output(out)
        {
            pinMode(this->dirPin, OUTPUT);
            pinMode(this->pwmPin, OUTPUT);
            pinMode(this->brakeReleasePin, OUTPUT);
            pinMode(this->modePin, INPUT);
            // TODO: Possibly set up some stuff about PWM frequency? But probably doesn't matter
        }

        int getPosition() {
            // Check position from motor, if valid update last valid and return, 
            // otherwise return last valid
            float currentPosVolts = readPositionVolts();

            Serial.print("Motor-getPosition: currentPosVolts = " + String(currentPosVolts) + "   ");
            int position;
            if (currentPosVolts < LOW_LIMIT || currentPosVolts > HIGH_LIMIT) {
                position = -2;  // Bad position and out of range
            } else if (currentPosVolts > LOCK_LOW && currentPosVolts < LOCK_HIGH) {
                position = 0;
            } else if (currentPosVolts > AWD_LOW && currentPosVolts < AWD_HIGH) {
                position = 1;
            } else if (currentPosVolts > N_LOW && currentPosVolts < N_HIGH) {
                position = 2;
            } else if (currentPosVolts > LO_LOW && currentPosVolts < LO_HIGH) {
                position = 3;
            } else {
                position = -1; // Bad position but in range
            }
            
            // TODO: Might want this in final version? 
            // if (position >= 0 && position <= 3) {
            //     output->setMotorPos(position);
            // }
            output->setMotorPos(position);
            return position;
        }

        int attemptShift(int desiredPos, int maxAttempts) {
            if (waitForShiftReady() < 0) {
                return -1;  // Shift not ready and needs to be aborted
            }

            initializeShift();
            while (desiredPositionDistance() > 0.05) {
                Serial.println((String)"Motor>attemptShift: desiredPositionDistance = " + desiredPositionDistance());
                addShiftAttempt();
                if (checkShiftWorking(maxAttempts) > 0) {
                    stepShiftSpeed(desiredPositionDirection());
                } else {
                    break;
                }
                output->setMotorVolts(readPositionVolts());
            }
            return endShift();
        }

        void testBrake(int ms) {
            setBrake(0);
            delay(ms);
            setBrake(1);
        }

        void testMotorForward(int ms) {
            output->setMainMessage("Testing Forward");
            motorDirection = 1;
            motorSpeed = 0.1;
            setBrake(0);
            delay(500);
            setMotor();
            delay(ms);
            stopMotor();
            delay(500);
            setBrake(1);
            output->setMainMessage("");
        }

        void testMotorBackward(int ms) {
            output->setMainMessage("Testing Backward");
            motorDirection = -1;
            motorSpeed = 0.1;
            setBrake(0);
            delay(500);
            setMotor();
            delay(ms);
            stopMotor();
            delay(500);
            setBrake(1);
            output->setMainMessage("");
        }


};


