#pragma once
#include <Arduino.h>
#include <Wire.h>  // For LiquidCrystal
#include <LiquidCrystal.h>


class OtherOutputs {
    private: 
        String text; // Full text to display
        int brightness; // Might not have this
        String mainMessage;  // Main message text
        int switchPos;
        int motorPos;
        float motorVolts;
        String motorMessage; // Message from Motor
        LiquidCrystal screen;

        void writeDisplay() {
            // Combine the different info into a message to display on screen
            String fullText;
            fullText = mainMessage + "";
            screen.autoscroll();  // Something like this might be useful
            screen.noAutoscroll(); // And this... might need some delays while writing etc
            screen.clear();
            screen.setCursor(0,0);
            screen.print(mainMessage);
            screen.setCursor(0,1);
            screen.print(switchPos);
            screen.setCursor(4,1);
            screen.print(motorMessage);
            screen.print(motorPos);  // TODO: Fix all of this
        }        

        void writeFakePinOuts() {
            // Set pin outs to trick the Car into thinking it's in a certain state
            // TODO: Set pin outs (probably using pwm analog out with a lowpass filter?)
        }

    public:
        OtherOutputs() : screen(0,0,0,0,0,0) {
        } // TODO: Almost definitely wrong and needs fixing

        OtherOutputs(LiquidCrystal lcd) : screen(lcd) {
            screen.begin(16, 2);
        }

        void writeOutputs() {
            // Output signals to trick car into thinking it's in correct state
            // And display screen
            writeFakePinOuts();
            writeDisplay();
        }

        void setBrightness () {
            // Set brightness of screen (might not be supported)
            // TODO: Set brightness if possible
        }

        void setMainMessage (String message) {
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