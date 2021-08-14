#pragma once
#include <Arduino.h>


class OtherOutputs {
    public:
        String text; // Full text to display
        int brightness; // Might not have this
        String mainMessage;  // Main message text
        int switchPos;
        int motorPos;
        String motorMessage; // Message from Motor

        void writeOutputs() {
            // Output signals to trick car into thinking it's in correct state
            // And display screen
        }

        void writeDisplay() {
            // Combine the different info into a message to display on screen
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
        }
        
};