#pragma once
#include <Arduino.h>


class OtherOutputs {
    private: 
        String text; // Full text to display
        int brightness; // Might not have this
        String mainMessage;  // Main message text
        int switchPos;
        int motorPos;
        float motorVolts;
        String motorMessage; // Message from Motor

        void writeDisplay() {
            // Combine the different info into a message to display on screen
            // TODO: set output to display
        }        

        void writeFakePinOuts() {
            // Set pin outs to trick the Car into thinking it's in a certain state
            // TODO: Set pin outs (probably using pwm analog out with a lowpass filter?)
        }

    public:
        void writeOutputs() {
            // Output signals to trick car into thinking it's in correct state
            // And display screen
            writeFakePinOuts();
            writeDisplay();
        }

        void setBrightness () {
            // Set brightness of screen (might not be supported)
        }

        void setMainMessage (String message) {
            // 
            mainMessage = message;
            writeOutputs();
        }

        void setSwitchPos(int pos) {
            if (0 <= pos && pos <= 3) {
                switchPos = pos;
            }
            writeOutputs();
        }

        void setMotorPos(int pos) {
            if (0 <= pos && pos <= 3) {
                motorPos = pos;
            }
            writeOutputs();
        }

        void setMotorMessage(String message) {
            motorMessage = message;
            writeOutputs();
        }

        void setMotorVolts(float volts) {
            motorVolts = volts;
            writeOutputs();
        }
        
};