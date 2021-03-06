#pragma once
#include <Arduino.h>
#include "output.h"
#include "specifications.h"
#include <EEPROM.h>

#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

#define OFF 0
#define ON 1

#define LOCK_POS 0
#define AWD_POS 1
#define N_POS 2
#define LO_POS 3
#define MANUAL_POS 10

#define TOWARD_4HI -1
#define TOWARD_4LO 1

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
        int lastValidPos = 5; // Properly set in .begin()
        int currentPos = 5;  // Properly set in .begin() 
        byte brakeState = ON; // By default the brake is ON and must be disabled by setting brakePin HIGH
        float motorSpeed = 0.0; // 0.0 - 1.0
        int motorDirection = 0;  // -1, 0, 1 (0 for not moving)
        int singleShiftAttempts = 0;  
        unsigned long shiftStart;
        unsigned long lastMotorSetTime = millis();  // Last time motor speed was updated
        uint8_t dirPin;
        uint8_t pwmPin;
        uint8_t brakeReleasePin;
        uint8_t modePin;
        // uint8_t vOutPin;
        OtherOutputs *output;

        void initializeShift() {
            output->setMainMessage(F("Initializing Shift"));  // DEBUGGING
            DEBUG_PRINTLN("Motor>initializeShift: Initializing Shift");  // DEBUGGING

            singleShiftAttempts = 0;
            setBrake(OFF); 
            delay(BRAKE_RELEASE_TIME_S*1000);  // TODO might want to change these delays to check other things in the meantime
            lastMotorSetTime = millis();  // Reset the time so that first set doesn't think it was ages ago.
            shiftStart = millis();
            output->setMainMessage("");  
        }

        bool endShift(int desiredPos){
            // Returns whether shift ended successfully (i.e. reached desired or not)
            DEBUG_PRINTLN(F("Motor>endShift: Shift ending"));
            stopMotor();
            delay(BRAKE_RELEASE_TIME_S*1000);
            setBrake(ON);
            if (getPosition() == desiredPos) {
                return true;
            }
            return false;
        }

        int checkShiftTimeout() {
            // Returns 1 if shift is still OK, otherwise returns < 0 
            if (millis() - shiftStart > MAX_SHIFT_TIME_S*1000) { // If current shift attempt fails by timing out
                DEBUG_PRINTLN(F("Motor>checkShiftWorking: Max time exceeded"));  // DEBUGGING
                return -1;
            } else {
                DEBUG_PRINTLN(F("Motor>checkShiftWorking: Shift OK to continue"));  // DEBUGGING
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
            analogRead(modePin); // Throw away reading to make sure the next ones aren't affected by previous readings
            float Vin = 5.0;
            float volts = 0.0;
            // Basic Average ////////
            for (int i=0; i<10; i++) {
                volts += Vin*analogRead(modePin)/1023;  // ~100us per read
            } 
            volts = volts/10.0;  // Because of averaging
            ///////////////////////
            // Exponetial Average ///////////// (less prone to spikes)
            // volts = analogRead(modePin);
            // const float alpha = 0.1;  // Lower more stable but slower to vary
            // for (int i=0; i<10; i++) {
            //     volts = (alpha*analogRead(modePin)+(1-alpha)*volts);
            // }
            // volts = Vin*volts/1023;
            //////////////////////////////////
            output->setMotorVolts(volts);
            DEBUG_PRINT(F("Motor>readPositionVolts: Reading = ")); DEBUG_PRINTLN(volts);
            return volts;
        }

        float getPositionVolts(int position) {
            switch (position){
                case LOCK_POS: return LOCK_V;
                case AWD_POS: return AWD_V;
                case N_POS: return N_V;
                case LO_POS: return LO_V;
                case MANUAL_POS: return 10.0;  // For manual override use (ensures distance to "desired" position stays large)
                default: return AWD_V; // Safest to assume AWD if bad position passed
            }
        }

        int shiftReady() {
            // Return 1 if valid time to shift, 0 if not, -1 if need to abort shift
            return 1;  // TODO: Only allow a shift every X seconds? 
        }

        void addShiftAttempt() {
            singleShiftAttempts += 1;
        }

        void setBrake(int brake) {
            // 0 for brake OFF (Which is actually brakePin high to release brake)
            // 1 for brake ON (Which is actually brakePin low to leave brake on)
            if (brake == OFF || brake == ON) {
                brakeState = brake;
                DEBUG_PRINT(F("Motor>setBrake: Setting brake pin to ")); DEBUG_PRINT((1-brakeState)); DEBUG_PRINT(F(" to achieve brake state " )); DEBUG_PRINTLN(brakeState); 
                digitalWrite(brakeReleasePin, 1-brakeState);  // (1-X) because the brake is ON by default and HIGH turns it OFF. 
            }
        }

        void stepShiftSpeed(int direction, int desiredPos) {
            // Update motor power based on current state.
            // increase power if not at max and not close to desired pos
            // decrease power if close to desired pos
            float timeSinceLastSet = (float)min(millis() - lastMotorSetTime, (unsigned long)50)/1000.0;  // Smaller of true time elapsed or 50ms

            DEBUG_PRINT(F("Motor>stepShiftSpeed: T=")); DEBUG_PRINT(timeSinceLastSet); DEBUG_PRINT(F(", speed=")); DEBUG_PRINT(motorSpeed); DEBUG_PRINT(F(", direction=")); DEBUG_PRINTLN(direction); 
            if (timeSinceLastSet*PWM_FREQUENCY > 5.0) {  // More than 5 full duty cycles
                if (motorDirection != 0 && direction != motorDirection) {  // Change of direction!! 
                    motorSpeed = 0.0;
                    stopMotor();
                    delay(100);  // Enforce some delay
                    timeSinceLastSet = 0.05;  // Don't want to step really fast because it's been a long time since last motor set time
                }
                motorDirection = direction;
                updateMotorSpeed(desiredPos, timeSinceLastSet);
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

            float newSpeed = motorSpeed + PWM_ACCELERATION * timeElapsed;  // New porportional speed
            newSpeed = min(maxAllowedSpeed, newSpeed);
            if (abs(newSpeed - motorSpeed) > 0.0001) {  // If speed change required
                motorSpeed = newSpeed;
                DEBUG_PRINT(F("Motor>updateMotorSpeed: newSpeed = ")); DEBUG_PRINTLN(newSpeed);
                setMotor();
            }
        }

        void stopMotor() {
            motorSpeed = 0.0;
            motorDirection = 0;
            setMotor();
        }

        void setMotor() {
            if (brakeState == OFF && motorSpeed > 0.0 && (motorDirection == TOWARD_4LO || motorDirection == TOWARD_4HI)) {
                int realDir, realPwm;
                realDir = (motorDirection > 0) ? 1 : 0;
                realPwm = max(PWM_MAX_POWER*motorSpeed, PWM_MIN_POWER);
                // snprintf(m_buf, sizeof(m_buf), "Motor>setMotor: Dir = %i, Speed: %i", realDir, realPwm); DEBUG_PRINTLN(m_buf);  // DEBUGGING
                digitalWrite(dirPin, realDir);
                analogWrite(pwmPin, realPwm);
            } else { // Stop motor
                DEBUG_PRINTLN(F( "Motor>setMotor: Dir = 0, Speed: 0")); // DEBUGGING
                digitalWrite(dirPin, 0);
                digitalWrite(pwmPin, 0);
            }
            lastMotorSetTime = millis();
        }

        float desiredPositionDistance(int desiredPos) {
            // Return distance to desired position
            float currentPosVolts = readPositionVolts();
            float desiredPosVolts = getPositionVolts(desiredPos);
            return min(1.0, abs(currentPosVolts - desiredPosVolts));
        }

        int desiredPositionDirection(int desiredPos) {
            // Return direction of desired position from current position
            float currentPosVolts = readPositionVolts();
            float desiredPosVolts = getPositionVolts(desiredPos);

            if (currentPosVolts <= desiredPosVolts) { // Direction toward 4HI
                DEBUG_PRINTLN(F("Motor>desiredPositionDirection: direction = -1"));
                return TOWARD_4HI;  
            } else {  // Direction toward 4LO
                DEBUG_PRINTLN(F("Motor>desiredPositionDirection: direction = 1"));
                return TOWARD_4LO;
            }
        }

        int waitForShiftReady() {
            unsigned long waitStart = millis();
            while (shiftReady() != 1)
            {
                delay(10);  
                if (shiftReady() == -1 || millis() - waitStart > 10*1000){
                    DEBUG_PRINTLN(F("Motor>waitForShiftReady: Shift not ready and needs to abort"));
                    output->setMainMessage(F("Shift not ready and needs to abort"));
                    delay(1000); 
                    return -1;  
                }
            }
            DEBUG_PRINTLN(F("Motor>waitForShiftReady: Shift ready"));
            return 1;
        }

        void tryRecoverBadShift(int previousDesiredPos) {
            if (previousDesiredPos != lastValidPos && isValid(lastValidPos)) {  // If not already trying to return to a previous valid state, do that now
                output->setMainMessage(F("Shift failed: Attempting to return to last valid state"));
                delay(2000);
                attemptShift(lastValidPos, MAX_RETURN_SHIFT_ATTEMPTS);
                if (getPosition() == lastValidPos) {
                    output->setMainMessage(F("Successfully returned to last valid state"));
                    delay(1000);
                } else {
                    output->setMainMessage(F("WARNING: Failed to get back to a valid state!"));
                    delay(5000);
                }
            }
        }

    public:
        // Motor(uint8_t pwmPin, uint8_t dirPin, uint8_t brakeReleasePin, uint8_t modePin, uint8_t vOutPin,OtherOutputs* out)
        Motor(uint8_t pwmPin, uint8_t dirPin, uint8_t brakeReleasePin, uint8_t modePin, OtherOutputs* out)
            : dirPin(dirPin)
            , pwmPin(pwmPin)
            , brakeReleasePin(brakeReleasePin)
            , modePin(modePin)
            , output(out)
        {
        }

        void begin() {
            pinMode(dirPin, OUTPUT);
            pinMode(pwmPin, OUTPUT);
            pinMode(brakeReleasePin, OUTPUT);
            pinMode(modePin, INPUT);

            lastValidPos = readEEPROMposition();
            currentPos = getPosition();
        }

        int getPosition() {
            // Check current position, returns -1 or -2 for bad positions
            float currentPosVolts = readPositionVolts();
            int position;
            if (currentPosVolts < LOW_LIMIT || currentPosVolts > HIGH_LIMIT) {
                position = -2;  // Bad position and out of range
            } else if (currentPosVolts > LOCK_V - MOTOR_DRIFT_TOLERANCE_V && currentPosVolts < LOCK_V + MOTOR_DRIFT_TOLERANCE_V) {
                position = FOURHI;
            } else if (currentPosVolts > AWD_V - MOTOR_DRIFT_TOLERANCE_V && currentPosVolts < AWD_V + MOTOR_DRIFT_TOLERANCE_V) {
                position = AWD;
            } else if (currentPosVolts > N_V - MOTOR_DRIFT_TOLERANCE_V && currentPosVolts < N_V + MOTOR_DRIFT_TOLERANCE_V) {
                position = NEUTRAL;
            } else if (currentPosVolts > LO_V - MOTOR_DRIFT_TOLERANCE_V && currentPosVolts < LO_V + MOTOR_DRIFT_TOLERANCE_V) {
                position = FOURLO;
            } else {
                position = -1; // Bad position but in range
            }

            if (isValid(position)) {
                setLastValidPos(position);
            }

            DEBUG_PRINT(F("Motor>getPosition: position = ")); DEBUG_PRINTLN(position);
            output->setMotorPos(position, lastValidPos);
            return position;
        }

        int getValidPosition() {
            // Same as getPosition, but will return lastValidPosition if not currently valid
            int position = getPosition();
            if (!isValid(position)) {
                position = lastValidPos;
            }
            return position;
        }

        bool attemptShift(int desiredPos, int maxAttempts) {
            if (waitForShiftReady() < 0) {
                return -1;  // Shift not ready and needs to be aborted
            }

            initializeShift();
            DEBUG_PRINT(F("Motor>attemptShift: desiredPositionDistance() = ")); DEBUG_PRINTLN(desiredPositionDistance(desiredPos));
            while (desiredPositionDistance(desiredPos) > POSITION_TOLERANCE) {
                DEBUG_PRINT(F("Motor>attemptShift: desiredPositionDistance = "));DEBUG_PRINTLN((double)desiredPositionDistance(desiredPos));
                if (checkShiftTimeout() > 0) { 
                    stepShiftSpeed(desiredPositionDirection(desiredPos), desiredPos);
                } else {  // Failed to shift by timeout
                    stopMotor();
                    if (getPosition() == desiredPos) {
                        output->setMainMessage(F("Didn't reach target V, but in desired Position"));
                        delay(2000);
                        break;
                    }  
                    else if (singleShiftAttempts < MAX_SINGLE_SHIFT_ATTEMPTS-1) {
                        output->setMainMessage(F("Shift attempt failed. Will retry"));
                        addShiftAttempt();
                        delay(RETRY_TIME_S*1000);
                        output->setMainMessage(F("Retrying"));
                        shiftStart = millis();
                        continue;
                    } else {
                        tryRecoverBadShift(desiredPos);
                        break;
                    }
                }
            }
            return endShift(desiredPos);
        }

        void manualDrive(int direction) {
            if (brakeState == ON) {
                setBrake(OFF);
            }
            stepShiftSpeed(direction, MANUAL_POS);
        }

        void manualStop() {
            stopMotor();
            setBrake(ON);
        }

        void testBrake(int ms) {
            setBrake(OFF);
            delay(ms);
            setBrake(ON);
        }

        void testMotorForward(int ms) {
            output->setMainMessage(F("Testing toward 4LO"));
            motorDirection = TOWARD_4LO;
            motorSpeed = 0.1;
            setBrake(OFF);
            delay(500);
            setMotor();
            delay(ms);
            stopMotor();
            delay(500);
            setBrake(ON);
            output->setMainMessage("");
        }

        void testMotorBackward(int ms) {
            output->setMainMessage(F("Testing toward 4HI"));
            motorDirection = TOWARD_4HI;
            motorSpeed = 0.1;
            setBrake(OFF);
            delay(500);
            setMotor();
            delay(ms);
            stopMotor();
            delay(500);
            setBrake(ON);
            output->setMainMessage("");
        }


};


