#pragma once
#include <Arduino.h>
#include "output.h"
#include "specifications.h"
#include <EEPROM.h>

char m_buf[100];  // DEBUGGING: string buffer to avoid use of String

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
        byte lastValidPos = readEEPROMposition();
        int currentPos = 0;  // TODO: Can I use the function to intialize this here? Or can I do that in defined init? 
        byte brakeState = 1; // By default the brake is ON and must be disabled by setting brakePin HIGH
        float motorSpeed = 0.0; // 0.0 - 1.0
        int motorDirection = 0;
        int singleShiftAttempts = 0;
        unsigned long shiftStart;
        unsigned long lastMotorSetTime = millis();  // Last time motor speed was updated
        unsigned long shiftTimes;  // TODO: Change this to some sort of list
        byte dirPin;
        byte pwmPin;
        byte brakeReleasePin;
        byte modePin;
        OtherOutputs *output;

        void initializeShift() {
            // output->setMotorMessage("Beginning shift attempt");
            output->setMainMessage(F("Initializing Shift"));  // DEBUGGING
            // Serial.println("Motor>initializeShift: Initializing Shift");  // DEBUGGING

            singleShiftAttempts = 0;
            setBrake(0); 
            delay(BRAKE_RELEASE_TIME_S);  // TODO might want to change these delays to check other things in the meantime
            lastMotorSetTime = millis();  // Reset the time so that first set doesn't think it was ages ago.
            shiftStart = millis();
            output->setMainMessage("");  // DEBUGGING
        }

        int endShift(int desiredPos){
            Serial.println(F("Motor>endShift: Shift ending"));

            stopMotor();
            delay(BRAKE_RELEASE_TIME_S);
            setBrake(1);
            if (getPosition() == desiredPos) {
                setLastValidPos(desiredPos);
                // output->setMotorMessage("Shift completed successfully");
                output->setMainMessage(F("Shift completed successfully"));
                delay(1000);
                // output->setMotorMessage("");
                output->setMainMessage("");
                return 1;
            }
            return -1;
        }

        int checkShiftWorking(int maxAttempts) {
            if (millis() - shiftStart > MAX_SHIFT_TIME_S*1000) { // If current shift attempt fails
                Serial.println(F("Motor>checkShiftWorking: Max time exceeded, stopping"));  // DEBUGGING
                stopMotor();
                if (singleShiftAttempts > maxAttempts) {  // If failed to shift to new position
                    if (lastValidPos >= 0) {
                        output->setMotorMessage(F("WARNING: Failed to shift to new position, returning to previous position"));
                        Serial.println(F("Motor>checkShiftWorking: Failed to shift to new position, returning to previous"));  // DEBUGGING
                        int returnPosition = lastValidPos;
                        setLastValidPos(-1);  // Record that the current state is invalid
                        delay(1000);  // Ensure message appears for at least 1s

                        // attemptShift(returnPosition, 15);  // Try harder (15x) to return to a valid state
                        attemptShift(returnPosition, 1);  //  DEBUGGING: Reduced attemts to return to previous position
                    } else {  // Already previously failed to get to new position
                        output->setMotorMessage("ERROR: Failed to return to previous position, not currently in valid state!");
                        Serial.println(F("Motor>checkShiftWorking: Failed to return to previous position, not currently in valid state!"));  // DEBUGGING
                        // TODO: Need to think about how to prevent the shift starting again
                        // IMPORTANT TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        delay(1000);  // Just so that the message at least shows up for 1 second
                    }
                }
                return -1; // Current shift failed
            } else {
                Serial.println(F("Motor>checkShiftWorking: Shift OK to continue"));  // DEBUGGING
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
            Serial.print(F("Motor>readPositionVolts: Reading = ")); Serial.println(volts);
            return volts;
        }

        float getPositionLowVolts(int position) {
            // Return low voltage of position
            switch (position){
                case 0: return LOCK_LOW;
                case 1: return AWD_LOW;
                case 2: return N_LOW;
                case 3: return LO_LOW;
                default: return LOW_LIMIT; // TODO: Is this the right thing to return in this case?
            }

        }

        float getPositionHighVolts(int position) {
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
                Serial.print(F("Motor>setBrake: Setting brake pin to ")); Serial.print((1-brakeState)); Serial.print(F(" to achieve brake state " )); Serial.println(brakeState); 
                digitalWrite(brakeReleasePin, 1-brakeState);  // (1-X) because the brake is ON by default and HIGH turns it OFF. 
            }
        }

        void stepShiftSpeed(int direction, int desiredPos) {
            // Update motor power based on current state.
            // increase power if not at max and not close to desired pos
            // decrease power if close to desired pos

            // If it has been longer than 1 PWM cycle since last update
            float timeSinceLastSet = min(millis() - lastMotorSetTime, (unsigned long)50)/1000.0; 

            Serial.print(F("Motor>stepShiftSpeed: T=")); Serial.print(timeSinceLastSet); Serial.print(F(", speed=")); Serial.print(motorSpeed); Serial.print(F(", direction=")); Serial.println(direction); 
            if (timeSinceLastSet*PWM_FREQUENCY > 1.0) {  // More than 1 full duty cycle
                if (motorDirection != 0 && direction != motorDirection) {  // Change of direction!! 
                    motorSpeed = 0.0;
                    stopMotor();
                    delay(100);  // Enforce some delay
                    timeSinceLastSet = 0.05;  // Don't want to step really fast because it's been a long time since last motor set time  TODO: Think about if this is necessary
                }
                motorDirection = direction;

                updateMotorSpeed(desiredPos, timeSinceLastSet);

                // if (desiredPositionDistance(desiredPos) >= 0.5 || motorSpeed < 0.01) {
                //     if (motorSpeed < 1.0) {
                //         Serial.println(F("Motor>stepShiftSpeed: Increasing motor speed"));
                //         changeSpeedValue(1, timeSinceLastSet);
                //         setMotor();
                //     }
                // } else if (motorSpeed > 0.01) { // otherwise decellerate
                //     Serial.println(F("Motor>stepShiftSpeed: Decreasing motor speed"));
                //     changeSpeedValue(-1, timeSinceLastSet); 
                //     setMotor();
                // } 
            }
        }

        void updateMotorSpeed(int desiredPos, float timeElapsed) {
            float posDist = desiredPositionDistance(desiredPos);
            float maxAllowedSpeed;
            if (posDist > 0.5) {
                maxAllowedSpeed = 1.0;
            } else if (posDist > 0.4) {
                maxAllowedSpeed = 0.5;
            } else if (posDist > 0.3) {
                maxAllowedSpeed = 0.3;
            } else if (posDist > 0.1) {
                maxAllowedSpeed = 0.1;
            } else {
                maxAllowedSpeed = 0.01;  // I.e. min speed
            }

            float newSpeed = min(1.0, motorSpeed + min(1.0, PWM_ACCELERATION * timeElapsed));
            newSpeed = min(maxAllowedSpeed, newSpeed);
            if (abs(newSpeed - motorSpeed) > 0.0001) {
                motorSpeed = newSpeed;
                Serial.print(F("Motor>updateMotorSpeed: newSpeed = ")); Serial.println(newSpeed);
                setMotor();
            }
        }

        // void changeSpeedValue(int increase, float timeElapsed) {  
        //     // Change power output (always positive value)
        //     float newSpeed = 0.0;
        //     if (increase > 0) {
        //         newSpeed = min(1.0, motorSpeed + min(1.0, PWM_ACCELERATION * timeElapsed));
        //     } 
        //     if (increase < 0) { // i.e. decrese power
        //         newSpeed = max(0.01, motorSpeed - min(1.0, PWM_ACCELERATION*3 * timeElapsed));  // Never sets exactly ZERO speed
        //     }
        //     motorSpeed = newSpeed;
        //     Serial.print(F("Motor>changeSpeedValue: newSpeed = ")); Serial.println(newSpeed);
        // }

        void stopMotor() {
            motorSpeed = 0.0;
            motorDirection = 0;
            setMotor();
        }

        void setMotor() {
            // TODO: call the motor controller with motorDirection and motorSpeed
            if (brakeState == 0 && motorSpeed > 0.0 && (motorDirection == 1 || motorDirection == -1)) {
                int realDir, realPwm;
                realDir = (motorDirection > 0) ? 1 : 0;
                realPwm = max(PWM_MAX_POWER*motorSpeed, PWM_MIN_POWER);
                snprintf(m_buf, sizeof(m_buf), "Motor>setMotor: Dir = %i, Speed: %i", realDir, realPwm); Serial.println(m_buf);  // DEBUGGING
                digitalWrite(dirPin, realDir);
                analogWrite(pwmPin, realPwm);
            } else { // Stop motor
                Serial.println(F( "Motor>setMotor: Dir = 0, Speed: 0")); // DEBUGGING
                digitalWrite(dirPin, 0);
                digitalWrite(pwmPin, 0);
            }
            lastMotorSetTime = millis();
        }

        float desiredPositionDistance(int desiredPos) {
            // Return distance to desired position
            float currentPosVolts = readPositionVolts();
            float desiredPosVolts = (getPositionLowVolts(desiredPos) + getPositionHighVolts(desiredPos))/2.0; // Aim for middle
            
            // TODO: Think about how to put this in again? Maybe not here?
            // if (currentPosVolts > HIGH_LIMIT || currentPosVolts < LOW_LIMIT) {
            //     return 0.0;  // Return zero distance if reading is out of range (to prevent trying to shift somewhere with a bad reading)
            // }

            // Serial.print(F("Motor>desiredPositionDistance: "));Serial.println(abs(currentPosVolts-desiredPosVolts));  // DEBUGGING
            return min(1.0, abs(currentPosVolts - desiredPosVolts));
        }

        int desiredPositionDirection(int desiredPos) {
            // Return direction of desired position from current position
            float currentPosVolts = readPositionVolts();
            float desiredPosVolts = (getPositionLowVolts(desiredPos) + getPositionHighVolts(desiredPos))/2.0; // Aim for middle

            // TODO: Put this or something similar back in.. probably not in this function
            // if (currentPosVolts > HIGH_LIMIT || currentPosVolts < LOW_LIMIT) {
            //     return 0;  // Return no direction if reading is out of range (to prevent trying to shift somewhere with bad reading)
            // }

            if (currentPosVolts <= desiredPosVolts) {
                Serial.println(F("Motor>desiredPositionDirection: direction = -1"));
                return -1;  
            } else {
                Serial.println(F("Motor>desiredPositionDirection: direction = 1"));
                return 1;
            }
        }

        int waitForShiftReady() {
            unsigned long waitStart = millis();
            while (shiftReady() != 1)
            {
                delay(10);  // TODO: Change this to do other things while waiting? I.e. check switchPosition or update screen?
                if (shiftReady() == -1 || millis() - waitStart > 30*1000){
                    Serial.println(F("Motor>waitForShiftReady: Shift not ready and needs to abort"));
                    output->setMotorMessage(F("Shift not ready and needs to abort"));
                    delay(1000); 
                    return -1;  
                }
            }
            Serial.println(F("Motor>waitForShiftReady: Shift ready"));
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
            Serial.print(F("Motor>getPosition: position = ")); Serial.println(position);
            output->setMotorPos(position);
            return position;
        }

        int attemptShift(int desiredPos, int maxAttempts) {
            if (waitForShiftReady() < 0) {
                return -1;  // Shift not ready and needs to be aborted
            }

            initializeShift();
            Serial.print(F("Motor>attemptShift: desiredPositionDistance() = ")); Serial.println(desiredPositionDistance(desiredPos));
            while (desiredPositionDistance(desiredPos) > 0.01) {
                Serial.print(F("Motor>attemptShift: desiredPositionDistance = "));Serial.println((double)desiredPositionDistance(desiredPos));
                addShiftAttempt();
                if (checkShiftWorking(maxAttempts) > 0) {
                    stepShiftSpeed(desiredPositionDirection(desiredPos), desiredPos);
                } else {
                    break;
                }
                output->setMotorVolts(readPositionVolts());
                Serial.println("");
            }
            return endShift(desiredPos);
        }

        void testBrake(int ms) {
            setBrake(0);
            delay(ms);
            setBrake(1);
        }

        void testMotorForward(int ms) {
            output->setMainMessage(F("Testing Forward"));
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
            output->setMainMessage(F("Testing Backward"));
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


